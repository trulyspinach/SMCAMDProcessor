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
        NSWorkspace.shared.open(URL(string: "https://github.com/trulyspinach/SMCAMDProcessor")!)
    }
    
    @IBAction func gadget(_ sender: Any) {
        let mainStoryboard = NSStoryboard.init(name: NSStoryboard.Name("Main"), bundle: nil)
        let controller = mainStoryboard.instantiateController(withIdentifier: NSStoryboard.SceneIdentifier("AMDPowerGadget")) as! NSWindowController
        controller.showWindow(self)
    }
    
    @IBAction func tool(_ sender: Any) {
        let mainStoryboard = NSStoryboard.init(name: NSStoryboard.Name("Main"), bundle: nil)
        let controller = mainStoryboard.instantiateController(withIdentifier: NSStoryboard.SceneIdentifier("AMDPowerTool")) as! NSWindowController
        controller.showWindow(self)

        controller.window?.isMovableByWindowBackground = true
    }
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }

}

