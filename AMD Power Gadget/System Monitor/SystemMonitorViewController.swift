//
//  SystemMonitorViewController.swift
//  AMD Power Gadget
//
//  Created by Qi HaoYan on 5/16/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

import Cocoa

class SystemMonitorViewController: NSViewController, NSTableViewDelegate, NSTableViewDataSource {

    var timer : Timer?
    
    var numFans = 0
    var fanNames : [String] = []
    var driverLoaded = false
    var fanRpms : [UInt64] = []
    var fanThrottles : [UInt8] = []
    var fanOverrided : [Bool] = []
    
    @IBOutlet weak var tableView: NSTableView!
    
    @IBOutlet weak var chipIntelLabel: NSTextField!
    
    static var activeSelf : SystemMonitorViewController?
    static func launch() {
        if let vc = SystemMonitorViewController.activeSelf {
            vc.view.window?.orderFrontRegardless()
        } else {
            let mainStoryboard = NSStoryboard.init(name: NSStoryboard.Name("Main"), bundle: nil)
            let controller = mainStoryboard.instantiateController(withIdentifier: NSStoryboard.SceneIdentifier("SystemMonitor")) as! NSWindowController
            controller.showWindow(self)
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        //init smc
        let initRes = ProcessorModel.shared.kernelGetUInt64(count: 2, selector: 90)
        if initRes[0] != 1{
            
            let alert = NSAlert()
            alert.messageText = "Could not found driver for your SMC."
            alert.informativeText = "Your SMC chip is likely not supported yet."
            alert.alertStyle = .critical
            alert.addButton(withTitle: "Done")
            alert.addButton(withTitle: "Report Issue")
            let res = alert.runModal()
            
            if res == .alertSecondButtonReturn {
                NSWorkspace.shared.open(URL(string: "https://github.com/trulyspinach/SMCAMDProcessor/issues/66")!)
            }
            
        } else {driverLoaded = true}
        
        ProcessorModel.shared.fetchSMCChipSupport(chipIntel: Int(initRes[1]), working: initRes[0] == 1)
        
        chipIntelLabel.stringValue =
            String(format: "Chip ID: %X, Revision: %X, \(driverLoaded ? "Connected" : "Not yet supported")",
                    initRes[1] >> 8, initRes[1] & 0xff)
        
        if driverLoaded {
            numFans = Int(ProcessorModel.shared.kernelGetUInt64(count: 1, selector: 91)[0])
            
            for i in 0...numFans-1 {
                fanNames.append(ProcessorModel.shared.kernelGetString(selector: 92, args: [UInt64(i)]))
            }
            
            fanRpms = [UInt64](repeating: 0, count: numFans)
            fanThrottles = [UInt8](repeating: 0, count: numFans)
            fanOverrided = [Bool](repeating: false, count: numFans)
            
            timer = Timer.scheduledTimer(withTimeInterval: 0.5, repeats: true, block: { (t) in
                self.updateFanRPMs()
            })
        }

        
        tableView.dataSource = self
        tableView.delegate = self
        
        tableView.sizeToFit()
    }
    
    func updateFanRPMs() {
        fanRpms = ProcessorModel.shared.kernelGetUInt64(count: numFans, selector: 93)
        tableView.reloadData()
        
        let ctrls = ProcessorModel.shared.kernelGetUInt64(count: numFans, selector: 94)
        for i in 0...numFans-1 {
            fanThrottles[i] = UInt8(ctrls[i] >> 8)
            fanOverrided[i] = (ctrls[i] & 0xff) == 0
        }
    }
    
    func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
        return 20
    }
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        return numFans
    }
    
    @IBAction func autobutton(_ sender: Any) {
        if ProcessorModel.shared.kernelSetUInt64(selector: 97, args: [0]) {
            updateFanRPMs()
        }
    }
    
    @IBAction func takeOffButton(_ sender: Any) {
        if ProcessorModel.shared.kernelSetUInt64(selector: 97, args: [1]) {
            updateFanRPMs()
        }
    }
    
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        if tableColumn!.identifier.rawValue == "fan" {
            return fanNames[row]
        }
        
        if tableColumn!.identifier.rawValue == "rpm" {
            return fanRpms.count > row ? "\(fanRpms[row])" : "NULL"
        }
        
        if tableColumn!.identifier.rawValue == "override" {
            return fanOverrided[row]
        }
        
        if tableColumn!.identifier.rawValue == "control" {
            return fanThrottles[row]
        }
        
        return 0
    }
    
    
    func tableView(_ tableView: NSTableView, setObjectValue object: Any?, for tableColumn: NSTableColumn?, row: Int) {
        if tableColumn!.identifier.rawValue == "control" {
            let thr = UInt8(round(Double(truncating: object as! NSNumber)))
            if ProcessorModel.shared.kernelSetUInt64(selector: 95, args: [UInt64(row), UInt64(thr)]) {
                updateFanRPMs()
            }
        }
        
        if tableColumn!.identifier.rawValue == "override" {
            let v = object as! Bool
            
            if !v && ProcessorModel.shared.kernelSetUInt64(selector: 96, args: [UInt64(row)]) {
                updateFanRPMs()
            }
        }
    }
}
