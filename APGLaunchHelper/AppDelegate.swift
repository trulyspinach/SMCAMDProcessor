//
//  AppDelegate.swift
//  APGLaunchHelper
//
//  Created by trulyspinach on 7/30/21.
//

import Cocoa

class AppDelegate: NSObject, NSApplicationDelegate {

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        let runningApps = NSWorkspace.shared.runningApplications
        print("hello world")
        let isRunning = runningApps.contains {
            $0.bundleIdentifier == "wtf.spinach.AMD-Power-Gadget"
        }
        
        if !isRunning {
            var path = URL(fileURLWithPath: Bundle.main.bundlePath as String)
        
            for _ in 1...4 {
                path = path.deletingLastPathComponent()
            }
            
            let config = NSWorkspace.OpenConfiguration()
            NSWorkspace.shared.openApplication(at: path, configuration: config) { instance, e in
                exit(0)
            }
        }
        
    }

}

