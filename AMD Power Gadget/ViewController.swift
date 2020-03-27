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

    var timer : Timer?
    
    static var activeSelf : ViewController?
    static func launch() {
        if let vc = ViewController.activeSelf {
            vc.view.window?.orderFrontRegardless()
        } else {
            let mainStoryboard = NSStoryboard.init(name: NSStoryboard.Name("Main"), bundle: nil)
            let controller = mainStoryboard.instantiateController(withIdentifier: NSStoryboard.SceneIdentifier("AMDPowerGadget")) as! NSWindowController
            controller.showWindow(self)
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        view.window?.delegate = self;
        
        timer = Timer.scheduledTimer(withTimeInterval: 1, repeats: true, block: { (t) in
            self.sampleData(forced: true)
        })
        
        scrollView.scroll(NSPoint(x: 0,y: 0))
        
        
        subtitleLabel.stringValue = ProcessorModel.sysctlString(key: "machdep.cpu.brand_string")
        sampleData(forced: true)
        sampleData(forced: true)
        sampleData(forced: true)
        
        ViewController.activeSelf = self
    }
    
    override func viewWillAppear() {
        view.window?.delegate = self
    }
    
    override var representedObject: Any? {
        didSet {
            // Update the view, if already loaded.
        }
    }
    
    func sampleData(forced : Bool){
        let numberOfCores = ProcessorModel.shared.getNumOfCore()
        let outputStr: [Float] = ProcessorModel.shared.getMetric(forced: forced)

        let power = outputStr[0]
        let temperature = outputStr[1]
        var frequencies : [Float] = []
        for i in 0...(numberOfCores-1) {
            frequencies.append(outputStr[Int(i + 3)])
        }

        if power < 1000 {
            powerGraphView.addData(value: Double(power))
        }

        temperatureGraphView.addData(value: Double(temperature))

        let meanFre = Double(frequencies.reduce(0, +) / Float(frequencies.count))
        frequencyGraphView.addData(value: meanFre)
        frequencyLabel.stringValue = String(format: "Average of %d Cores: %.2f Ghz, Max: %.2f Ghz", numberOfCores, meanFre * 0.001, frequencies.max()! * 0.001)


        temperatureLabel.stringValue = String(format: "%.2f °C", temperature)
        powerLabel.stringValue = String(format: "%.2f Watt", power)
    }
    
    @IBAction func launchPowerTool(_ sender: Any) {
        PowerToolViewController.launch()
    }

    func windowWillClose(_ notification: Notification) {
        ViewController.activeSelf = nil
    }
    
    @IBAction func buttonPressed(_ sender: Any) {
        NSWorkspace.shared.open(URL(string: "https://github.com/trulyspinach/AMDRyzenCPUPowerManagement")!)
    }
    
    
}

