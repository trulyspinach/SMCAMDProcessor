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

    @IBAction func openPage(_ sender: Any) {
        NSWorkspace.shared.open(URL(string: "https://github.com/trulyspinach/AMDRyzenCPUPowerManagement")!)
    }
    
    @IBAction func gadget(_ sender: Any) {
        ViewController.launch()
        
    }
    
    @IBAction func tool(_ sender: Any) {
        PowerToolViewController.launch()
    }
    
    @IBAction func sysmonitor(_ sender: Any) {
        SystemMonitorViewController.launch()
    }
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }

}

