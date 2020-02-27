//
//  ViewController.swift
//  AMD Power Gadget
//
//  Created by Qi HaoYan on 2/22/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

import Cocoa
import IOKit

class ViewController: NSViewController, NSWindowDelegate {
    
    @IBOutlet weak var scrollView: NSScrollView!
    @IBOutlet weak var contentView: NSView!
    
    @IBOutlet weak var subtitleLabel: NSTextField!
    
    @IBOutlet weak var frequencyGraphView: GraphView!
    @IBOutlet weak var temperatureGraphView: GraphView!
    @IBOutlet weak var powerGraphView: GraphView!
    
    
    @IBOutlet weak var frequencyLabel: NSTextField!
    @IBOutlet weak var temperatureLabel: NSTextField!
    @IBOutlet weak var powerLabel: NSTextField!
    
    
    private var connect: io_connect_t = 0
    
    var timer : Timer?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        view.window?.delegate = self;
        // Do any additional setup after loading the view.
        
        
        timer = Timer.scheduledTimer(withTimeInterval: 1, repeats: true, block: { (t) in
            self.sampleData()
        })
        
        if !initDriver() {
            alertAndQuit()
        }
        
        //        scrollView.documentView?.setFrameSize(NSSize(width: scrollView.frame.size.width - 20, height: 900))
        
        scrollView.scroll(NSPoint(x: 0,y: 0))
        
        subtitleLabel.stringValue = cpuBrandString()
        sampleData()
        sampleData()
        sampleData()
        
        
    }
    
    func alertAndQuit(){
        let alert = NSAlert()
        alert.messageText = "No SMCAMDProcessor Found!"
        alert.informativeText = "Please download SMCAMDProcessor from the release page."
        alert.alertStyle = .warning
        alert.addButton(withTitle: "Quit")
        alert.addButton(withTitle: "Quit and Download")
        let res = alert.runModal()
        
        if res == .alertSecondButtonReturn {
            NSWorkspace.shared.open(URL(string: "https://github.com/trulyspinach/SMCAMDProcessor")!)
        }
        
        NSApplication.shared.terminate(self)
    }
    
    func cpuBrandString() -> String {
        var size = 0
        sysctlbyname("machdep.cpu.brand_string", nil, &size, nil, 0)
        var machine = [CChar](repeating: 0,  count: size)
        sysctlbyname("machdep.cpu.brand_string", &machine, &size, nil, 0)
        return String(cString: machine)
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
    
    override var representedObject: Any? {
        didSet {
            // Update the view, if already loaded.
        }
    }
    
    func sampleData(){
//        print("ss")
        
        var numberOfCores: UInt64 = 0
        var outputCount: UInt32 = 1
        
        let maxStrLength = 66 //MaxCpu + 2
        var outputStr: [Float] = [Float](repeating: 0, count: maxStrLength)
        var outputStrCount: Int = 4 * maxStrLength
        let res = IOConnectCallMethod(connect, 4, nil, 0, nil, 0,
                                      &numberOfCores, &outputCount,
                                      &outputStr, &outputStrCount)
        
        
        
        if res != KERN_SUCCESS {
            print(String(cString: mach_error_string(res)))
        }
        
        let power = outputStr[0]
        let temperature = outputStr[1]
        var frequencies : [Float] = []
        for i in 0...(numberOfCores-1) {
            frequencies.append(outputStr[Int(i + 2)])
        }
        
        powerGraphView.addData(value: Double(power))
        temperatureGraphView.addData(value: Double(temperature))
        
        let meanFre = Double(frequencies.reduce(0, +) / Float(frequencies.count))
        frequencyGraphView.addData(value: meanFre)
        frequencyLabel.stringValue = String(format: "Average of %d Cores: %.2f Ghz, Max: %.2f Ghz", numberOfCores, meanFre * 0.001, frequencies.max()! * 0.001)
        
        temperatureLabel.stringValue = String(format: "%.2f °C", temperature)
        powerLabel.stringValue = String(format: "%.2f Watt", power)
    }
    
    func windowDidResize(_ notification: Notification) {
        print(scrollView.frame.size)
        print(contentView.frame.size)
    }
    
    @IBAction func buttonPressed(_ sender: Any) {
        NSWorkspace.shared.open(URL(string: "https://github.com/trulyspinach/SMCAMDProcessor")!)
    }
    
    
}

