//
//  MenubarController.swift
//  AMD Power Gadget
//
//  Created by Qi HaoYan on 7/29/21.
//  Copyright © 2021 trulyspinach. All rights reserved.
//

import Cocoa

fileprivate class StatusbarView: NSView{
    
    var meanFreq: Float = 0
    var maxFreq: Float = 0
    var temp: Float = 0
    var pwr: Float = 0
    
    var normalLabel: [NSAttributedString.Key : NSObject]?
    var compactLabel: [NSAttributedString.Key : NSObject]?
    var normalValue: [NSAttributedString.Key : NSObject]?
    var compactValue: [NSAttributedString.Key : NSObject]?
    
    
    func setup(){
        let compactLH: CGFloat = 6
        
        let p = NSMutableParagraphStyle()
        p.minimumLineHeight = compactLH
        p.maximumLineHeight = compactLH
        compactLabel = [
            
            NSAttributedString.Key.font: NSFont.init(name: "Monaco", size: 7.2)!,
            NSAttributedString.Key.foregroundColor: NSColor.labelColor,
            NSAttributedString.Key.paragraphStyle: p
        ]
        
        normalValue = [
            NSAttributedString.Key.font: NSFont.systemFont(ofSize: 14, weight: NSFont.Weight.regular),
            NSAttributedString.Key.foregroundColor: NSColor.labelColor,
        ]
        
        compactValue = [
            NSAttributedString.Key.font: NSFont.systemFont(ofSize: 9, weight: NSFont.Weight.semibold),
            NSAttributedString.Key.foregroundColor: NSColor.labelColor,
        ]
        
        normalLabel = [
            NSAttributedString.Key.font: NSFont.systemFont(ofSize: 13, weight: NSFont.Weight.regular),
            NSAttributedString.Key.foregroundColor: NSColor.labelColor,
        ]
    }
    
    override func draw(_ dirtyRect: NSRect) {
        guard let context = NSGraphicsContext.current?.cgContext else { return }
        
        let fr = String(format: "%.1f", meanFreq * 0.001)
//        drawCompactSingle(label: "AVG", value: "\(fr) Ghz", x: 0)
        
        let mfr = String(format: "%.1f", maxFreq * 0.001)
        
        
        drawDoubleCompactValue(label: "CPU", up: "\(mfr) Ghz", down: "\(fr) Ghz", x: 0)
        drawCompactSingle(label: "TEM", value: "\(Int(round(temp)))º", x: 80)
        drawCompactSingle(label: "PWR", value: "\(Int(round(pwr)))W", x: 122)
    }
    
    func drawDoubleCompactValue(label: String, up: String, down: String, x: CGFloat) {
        let attributedString = NSAttributedString(string: label, attributes: normalLabel)
        attributedString.draw(at: NSPoint(x: x, y: 2.5))
        
        let up = NSAttributedString(string: up, attributes: compactValue)
        up.draw(at: NSPoint(x: x + 33, y: 10))
        
        let down = NSAttributedString(string: down, attributes: compactValue)
        down.draw(at: NSPoint(x: x + 33, y: 0))
    }
    
    func drawCompactSingle(label: String, value: String, x: CGFloat) {
        let attributedString = NSAttributedString(string: label, attributes: compactLabel)
        attributedString.draw(in: NSRect(x: x, y: -4.5, width: 7, height: frame.height))
        
        let value = NSAttributedString(string: value, attributes: normalValue)
        value.draw(at: NSPoint(x: x + 10, y: 2.5))
    }
}

class StatusbarController {
    
    var statusItem: NSStatusItem!
    fileprivate var view: StatusbarView!
    
    var updateTimer: Timer?
    var menu: NSMenu?
    
    init() {
        statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        statusItem.isVisible = true
        
        view = StatusbarView()
        view.setup()
        statusItem.button?.wantsLayer = true
        statusItem.button?.addSubview(view)
//        statusItem.button?.layer?.backgroundColor = NSColor.cyan.cgColor
        
        statusItem.button?.target = self
        statusItem.button?.action = #selector(itemClicked)
        statusItem.button?.sendAction(on: [.leftMouseUp, .rightMouseUp])
        
        statusItem.length = 170
        view.frame = statusItem.button!.bounds
        
        addMenuItems()
        
        updateTimer = Timer.scheduledTimer(withTimeInterval: 1, repeats: true, block: { _ in
            self.update()
        })
    }
    
    func update() {
        let numberOfCores = ProcessorModel.shared.getNumOfCore()
        let outputStr: [Float] = ProcessorModel.shared.getMetric(forced: false)

        let power = outputStr[0]
        let temperature = outputStr[1]
        var frequencies : [Float] = []
        for i in 0...(numberOfCores-1) {
            frequencies.append(outputStr[Int(i + 3)])
        }
        
        let meanFre = Float(frequencies.reduce(0, +) / Float(frequencies.count))
        let maxFre = Float(frequencies.max()!)
        
        view?.meanFreq = meanFre
        view?.maxFreq = maxFre
        view?.temp = temperature
        view?.pwr = power
        view.setNeedsDisplay(view.frame)
        
    }
    
    @objc func itemClicked(){
        switch NSApp.currentEvent?.type {
        case .leftMouseUp:
            ViewController.launch(forceFocus: true)
            
        case .rightMouseUp:
            guard let m = menu else {return}
            statusItem.popUpMenu(m)
            
        default: break
        }
        
    }
    
    @objc func gadget(){
        ViewController.launch(forceFocus: true)
    }
    
    @objc func tool(){
        PowerToolViewController.launch(forceFocus: true)
    }
    
    @objc func fans(){
        SystemMonitorViewController.launch(forceFocus: true)
    }
    
    @objc func exitApp(){
        exit(0)
    }
    
    private func addMenuItems() {
        menu = NSMenu()
        guard let m = menu else {return}
        var item = NSMenuItem(title: "AMD Power Gadget", action: #selector(gadget), keyEquivalent: ""); item.target = self
        m.addItem(item)
        item = NSMenuItem(title: "AMD Power Tool", action: #selector(tool), keyEquivalent: ""); item.target = self
        m.addItem(item)
        item = NSMenuItem(title: "SMC Fans", action: #selector(fans), keyEquivalent: ""); item.target = self
        m.addItem(item)
        
        
        m.addItem(NSMenuItem.separator())
        
        item = NSMenuItem(title: "Exit", action: #selector(exitApp), keyEquivalent: ""); item.target = self
        m.addItem(item)
        
//        statusItem.menu = menu
    }
}


