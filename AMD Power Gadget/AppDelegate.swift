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

    @IBOutlet weak var appearanceToggle: NSMenuItem!
    
    
    @IBAction func openPage(_ sender: Any) {
        NSWorkspace.shared.open(URL(string: "https://github.com/trulyspinach/SMCAMDProcessor")!)
    }
    
    @IBAction func gadget(_ sender: Any) {
        ViewController.launch()
        
    }
    
    @IBAction func tool(_ sender: Any) {
        PowerToolViewController.launch()
    }
    
    @IBAction func changeAppearance(_ sender: Any) {
        applyAppearanceSwitch(translucency: appearanceToggle.state == .off)
    }
    @IBAction func sysmonitor(_ sender: Any) {
        SystemMonitorViewController.launch()
    }
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        let useTran = UserDefaults.standard.bool(forKey: "usetranslucency")
        applyAppearanceSwitch(translucency: useTran)
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
}

