//
//  ProcessorModel.swift
//  AMD Power Gadget
//
//  Created by Qi HaoYan on 3/3/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

import Cocoa


class ProcessorModel {
    static let shared = ProcessorModel()
    
    private var connect: io_connect_t = 0
    
    private var cachedMetric : [Float] = []
    private var numberOfCores : Int = 0
    private var lastMLoad : Double = 0
    
    private var PStateDef : [UInt64] = []
    private var PStateCur : Int = 0
    private var instructionDelta : [UInt64] = []
    private var loadIndex : [Float] = []
    private var PStateDefClock : [Float] = []
    private var vaildPStateLength : Int = 0
    
    private var cpuListedAsSupported : Bool = false
    
    var SMCAMDProcessorVersion : String = ""
    var cpuidBasic : [UInt64] = []
    
    init() {
        if !initDriver() {
            alertAndQuit()
        }
        
        var scalerOut: UInt64 = 0
        var outputCount: UInt32 = 0

        let maxStrLength = 16
        var outputStr: [CChar] = [CChar](repeating: 0, count: maxStrLength)
        var outputStrCount: Int = maxStrLength
        let _ = IOConnectCallMethod(connect, 8, nil, 0, nil, 0,
                                      &scalerOut, &outputCount,
                                      &outputStr, &outputStrCount)
        SMCAMDProcessorVersion = String(cString: Array(outputStr[0...outputStrCount-1]))
        
        loadCPUID()
        loadMetric()
        loadPStateDef()
        loadPStateDefClock()
        
        if numberOfCores < 1{
            let alert = NSAlert()
            alert.messageText = "Error reading CPU data."
            alert.informativeText = "This application can not be launched due to SMCAMDProcessor is reporting incorrect data."
            alert.alertStyle = .critical
            alert.addButton(withTitle: "Quit")
            alert.runModal()
            NSApplication.shared.terminate(self)
        }
        
        fetchSupportedProcessor()
    }
    
    func initDriver() -> Bool {
        let serviceObject = IOServiceGetMatchingService(kIOMasterPortDefault,
                                                        IOServiceMatching("SMCAMDProcessor"))
        if serviceObject == 0 {
            return false
        }
        
        let status = IOServiceOpen(serviceObject, mach_task_self_, 0, &connect)
        return status == KERN_SUCCESS
    }
    
    func alertAndQuit(){
        let alert = NSAlert()
        alert.messageText = "No SMCAMDProcessor Found!"
        alert.informativeText = "Please download SMCAMDProcessor from the release page."
        alert.alertStyle = .critical
        alert.addButton(withTitle: "Quit")
        alert.addButton(withTitle: "Quit and Download")
        let res = alert.runModal()
        
        if res == .alertSecondButtonReturn {
            NSWorkspace.shared.open(URL(string: "https://github.com/trulyspinach/SMCAMDProcessor")!)
        }
        
        NSApplication.shared.terminate(self)
    }
    
    private func kernelGetFloats(count : Int, selector : UInt32) -> [Float] {
        var scalerOut: UInt64 = 0
        var outputCount: UInt32 = 1

        let maxStrLength = count //MaxCpu + 3
        var outputStr: [Float] = [Float](repeating: 0, count: maxStrLength)
        var outputStrCount: Int = 4/*sizeof(float)*/ * maxStrLength
        let res = IOConnectCallMethod(connect, selector, nil, 0, nil, 0,
                                      &scalerOut, &outputCount,
                                      &outputStr, &outputStrCount)
        
        if res != KERN_SUCCESS {
            print(String(cString: mach_error_string(res)))
            return []
        }
        
        return outputStr
    }
    
    private func kernelGetUInt64(count : Int, selector : UInt32) -> [UInt64] {
        var scalerOut: UInt64 = 0
        var outputCount: UInt32 = 1

        let maxStrLength = count //MaxCpu + 3
        var outputStr: [UInt64] = [UInt64](repeating: 0, count: maxStrLength)
        var outputStrCount: Int = 8/*sizeof(uint64_t)*/ * maxStrLength
        let res = IOConnectCallMethod(connect, selector, nil, 0, nil, 0,
                                      &scalerOut, &outputCount,
                                      &outputStr, &outputStrCount)
        
        if res != KERN_SUCCESS {
            print(String(cString: mach_error_string(res)))
            return []
        }
        
        return outputStr
    }
    
    private func loadMetric(){
        var scalerOut: UInt64 = 0
        var outputCount: UInt32 = 1

        let maxStrLength = 67 //MaxCpu + 3
        var outputStr: [Float] = [Float](repeating: 0, count: maxStrLength)
        var outputStrCount: Int = 4/*sizeof(float)*/ * maxStrLength
        let res = IOConnectCallMethod(connect, 4, nil, 0, nil, 0,
                                      &scalerOut, &outputCount,
                                      &outputStr, &outputStrCount)
        
        if res != KERN_SUCCESS {
            print(String(cString: mach_error_string(res)))
            return
        }
        
        numberOfCores = Int(scalerOut)
        cachedMetric = Array(outputStr[0...numberOfCores + 2])
        PStateCur = Int(outputStr[2])
        
        
        lastMLoad = NSDate().timeIntervalSince1970
    }
    
    private func loadLoadIndex(){
        let o = kernelGetFloats(count: numberOfCores, selector: 6)
        loadIndex = Array(o[0...numberOfCores - 1])
    }
    
    private func loadPStateDef(){

        PStateDef = kernelGetUInt64(count: 8, selector: 0)
        
        var i = 0
        while i < 8 {
            if (PStateDef[i] & 0x8000000000000000) == 0 { //LOL Swift
                break;
            }
            i += 1
        }
        vaildPStateLength = i
        
    }
    
    private func loadCPUID(){
        cpuidBasic = kernelGetUInt64(count: 8, selector: 7)
    }
    
    private func loadPStateDefClock(){
        PStateDefClock = kernelGetFloats(count: 10, selector: 1)
    }
    
    func refreshPStateDef() {
        loadPStateDefClock()
    }
    
    func setPState(state : Int) {
        var input: [UInt64] = [UInt64(state)]
        let res = IOConnectCallMethod(connect, 10, &input, 1, nil, 0,
                                      nil, nil,
                                      nil, nil)
        
        if res != KERN_SUCCESS {
            print(String(cString: mach_error_string(res)))
            return
        }
    }
    
    func getPState() -> Int {
        return PStateCur
    }
    
    func getPStateDef() -> [UInt64]{
        return PStateDef
    }
    
    func getVaildPStateClocks() -> [Float] {
        return Array(PStateDefClock[0...vaildPStateLength-1])
    }
    
    func getMetric(forced : Bool) -> [Float] {
        if forced || (NSDate().timeIntervalSince1970 - lastMLoad >= 1.0) {
            loadMetric()
        }
        return cachedMetric
    }
    
    func getNumOfCore() -> Int {
        return numberOfCores
    }
    
    func getLoadIndex() -> [Float] {
        loadLoadIndex()
        return loadIndex
    }
    
    func getCPB() -> [Bool] {
        let o = kernelGetUInt64(count: 2, selector: 11)
        return o.map{ $0 == 0 ? false : true }
    }
    
    func setCPB(enabled : Bool){
        var input: [UInt64] = [UInt64(enabled ? 1 : 0)]
        let res = IOConnectCallMethod(connect, 12, &input, 1, nil, 0,
                                      nil, nil,
                                      nil, nil)
        
        if res != KERN_SUCCESS {
            print(String(cString: mach_error_string(res)))
            return
        }
    }
    
    func getPPM() -> Bool {
        let o = kernelGetUInt64(count: 2, selector: 13)
        return o[0] == 0 ? false : true
    }
    
    func setPPM(enabled : Bool){
        var input: [UInt64] = [UInt64(enabled ? 1 : 0)]
        let res = IOConnectCallMethod(connect, 14, &input, 1, nil, 0,
                                      nil, nil,
                                      nil, nil)
        
        if res != KERN_SUCCESS {
            print(String(cString: mach_error_string(res)))
            return
        }
    }
    
    func getInstructionDelta() -> [UInt64]{
        let o = kernelGetUInt64(count: numberOfCores, selector: 5)
        return Array(o[0...numberOfCores - 1])
    }
    
    func setPState(def : [UInt64]) -> Int{
        if def.count != 8 {
            return -1
        }
        
        var input: [UInt64] = def
        let res = IOConnectCallMethod(connect, 15, &input, 8, nil, 0,
                                      nil, nil,
                                      nil, nil)
        
        
        if res != KERN_SUCCESS {
            print(String(cString: mach_error_string(res)))
            return Int(res)
        }
        
        loadPStateDef()
        loadPStateDefClock()
        return 0
    }
    
    static func sysctlString(key : String) -> String {
        var size = 0
        sysctlbyname(key, nil, &size, nil, 0)
        var machine = [CChar](repeating: 0,  count: size)
        sysctlbyname(key, &machine, &size, nil, 0)
        return String(cString: machine)
    }
    
    static func sysctlInt64(key : String) -> Int64 {
        var v: Int64 = 0
        var size = MemoryLayout<Int64>.size
        sysctlbyname(key, &v, &size, nil, 0)
        return v
    }
    
    func fetchSupportedProcessor() {
        let url = URL(string: "https://trulyspinach.github.io/SMCAMDProcessor/testeddevices.json")!

        let task = URLSession.shared.dataTask(with: url) {(data, response, error) in
            let s = String(data: data!, encoding: .utf8)!
            let supported = s.components(separatedBy: "\n")
                
            self.cpuListedAsSupported = supported.contains(
                ProcessorModel.sysctlString(key: "machdep.cpu.brand_string"))
//            print(supported)
//            print(self.cpuListedAsSupported)
        }

        task.resume()
        
       
        
    }
}
