//
//  AppDelegate.swift
//  AMD Power Gadget
//
//  Created by Qi HaoYan on 2/22/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {

    var mbController: StatusbarController?
    
    @IBOutlet weak var appearanceToggle: NSMenuItem!
    @IBOutlet weak var statusbarToggle: NSMenuItem!
    
    
    @IBAction func openPage(_ sender: Any) {
        NSWorkspace.shared.open(URL(string: "https://github.com/trulyspinach/SMCAMDProcessor")!)
    }
    
    @IBAction func gadget(_ sender: Any) {
        ViewController.launch()
        
    }
    
    @IBAction func tool(_ sender: Any) {
        PowerToolViewController.launch()
    }
    
    static func launchGadget(){
        ViewController.launch()
    }
    
    static func haveActiveWindows() -> Bool {
        if !UserDefaults.standard.bool(forKey: "statusbarenabled") {return true}
        
        return ViewController.activeSelf != nil
            || PowerToolViewController.activeSelf != nil
            || SystemMonitorViewController.activeSelf != nil
    }
    
    static func updateDockIcon() {
        NSApplication.shared.setActivationPolicy(haveActiveWindows() ? .regular : .accessory)
    }
    
    @IBAction func changeAppearance(_ sender: Any) {
        applyAppearanceSwitch(translucency: appearanceToggle.state == .off)
    }
    
    @IBAction func toggleStatusBar(_ sender: Any) {
        applyStatusBarSwitch(enabled: statusbarToggle.state == .off)
    }
    
    @IBAction func sysmonitor(_ sender: Any) {
        SystemMonitorViewController.launch()
    }
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        UserDefaults.standard.register(defaults: ["usetranslucency" : false,
                                                  "statusbarenabled": true])
        
        let useTran = UserDefaults.standard.bool(forKey: "usetranslucency")
        let sb = UserDefaults.standard.bool(forKey: "statusbarenabled")
        
        applyStatusBarSwitch(enabled: sb)
        applyAppearanceSwitch(translucency: useTran)
        
        if !sb {
            ViewController.launch()
        }
    }
    
    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
        ProcessorModel.shared.closeDriver()
    }
    
    func applyAppearanceSwitch(translucency : Bool) {
        appearanceToggle.state = translucency ? .on : .off
        ViewController.activeSelf?.toggleTranslucency(enabled: translucency)
        PowerToolViewController.activeSelf?.toggleTranslucency(enabled: translucency)
        
        UserDefaults.standard.set(translucency, forKey: "usetranslucency")
    }
    
    func applyStatusBarSwitch(enabled: Bool) {
        statusbarToggle.state = enabled ? .on : .off
        if enabled {
            if mbController == nil {
                mbController = StatusbarController()
                AppDelegate.updateDockIcon()
            }
        } else {
            mbController?.dismiss()
            mbController = nil
        }
        
        UserDefaults.standard.set(enabled, forKey: "statusbarenabled")
    }
}

