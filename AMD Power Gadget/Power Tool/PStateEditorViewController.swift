//
//  PStateEditorViewController.swift
//  AMD Power Gadget
//
//  Created by Qi HaoYan on 3/10/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

import Cocoa

class PStateEditorViewController: NSViewController, NSTableViewDelegate, NSTableViewDataSource {

    var data : [[String: UInt32]] = []
    
    var changed = false
    
    @IBOutlet weak var tableView: NSTableView!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do view setup here.
        
        tableView.dataSource = self
        tableView.delegate = self
        
        tableView.sizeToFit()
        
        data = ProcessorModel.shared.getPStateDef().map({value2Dict(v: $0)})
    }
    
    func value2Dict(v : UInt64) -> [String: UInt32]{
        var r = [String: UInt32]()
        
        r["enabled"] = UInt32(v >> 63)
        r["IddDiv"] = UInt32((v >> 30) & 0x3)
        r["IddValue"] = UInt32((v >> 22) & 0xff)
        r["CpuVid"] = UInt32((v >> 14) & 0xff)
        r["CpuDfsId"] = UInt32((v >> 8) & 0x1f)
        r["CpuFid"] = UInt32(v & 0xff)

        
        return r
    }

    func dict2Value(d : [String: UInt32]) -> UInt64 {
        var r : UInt64 = 0
        
        r |= UInt64(d["enabled"]!) << 63
        r |= (UInt64(d["IddDiv"]!) & 0x3) << 30
        r |= (UInt64(d["IddValue"]!) & 0xff) << 22
        r |= (UInt64(d["CpuVid"]!) & 0xff) << 14
        r |= (UInt64(d["CpuDfsId"]!) & 0x1f) << 8
        r |= UInt64(d["CpuFid"]!) & 0xff
        
        return r
    }
    
    func dict2Speed(v : [String: UInt32]) -> Float{
        return Float(v["CpuFid"]!) / Float(v["CpuDfsId"]!) * 200.0
    }
    
    func speed2Dict(v : [String: UInt32], speed : UInt32) -> [String: UInt32] {
        var nd = v
        let targetFid = Float(speed) / 200.0 * Float(v["CpuDfsId"]!)
        nd["CpuFid"] = UInt32(targetFid)
        
        return nd
    }
    
    func scale2Speed(scale : Float) -> UInt32 {
        return UInt32(dict2Speed(v: data[0]) * scale)
    }
    
    func dict2scale(v : [String: UInt32]) -> Float{
        return dict2Speed(v: v) / dict2Speed(v: data[0])
    }
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        return 8
    }
    
    @IBAction func apply(_ sender: Any) {
        let arr = data.map{ dict2Value(d: $0) }
        let err = ProcessorModel.shared.setPState(def: arr)
        if err != 0 {
            alertNoPrivilege()
        } else {
            changed = false
        }
    }
    
    func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
        return 24
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        if tableColumn!.identifier.rawValue == "id"{
            return row
        }
        
        if tableColumn!.identifier.rawValue == "speed"{
            return dict2Speed(v: data[row])
        }
        
        if tableColumn!.identifier.rawValue == "scale"{
            return dict2scale(v: data[row])
        }
        
        return String(format: "%X", data[row][tableColumn!.identifier.rawValue]!)
    }
    
    func tableView(_ tableView: NSTableView, setObjectValue object: Any?, for tableColumn: NSTableColumn?, row: Int) {
        if tableColumn!.identifier.rawValue == "enabled"{
            data[row]["enabled"] = (object as! UInt32)
            changed = true
        } else if tableColumn!.identifier.rawValue == "scale"{
            if row == 0 {
                return
            }
            data[row] = speed2Dict(v: data[row], speed: scale2Speed(scale: Float(truncating: object as! NSNumber)))
            changed = true
            tableView.reloadData(forRowIndexes: IndexSet(arrayLiteral: row), columnIndexes: IndexSet(6...8))
            
        } else {
            if let v = UInt32(object as! String, radix: 16){
                data[row][tableColumn!.identifier.rawValue] = v
                changed = true
            } else {
                alertInvaildInput()
            }
        }
    }
    
    func alertInvaildInput(){
        let alert = NSAlert()
        alert.messageText = "Invaild Input"
        alert.informativeText = "Please type in numbers in hexadecimal."
        alert.alertStyle = .critical
        alert.addButton(withTitle: "Done")
        alert.beginSheetModal(for: view.window!, completionHandler: nil)
    }
    
    func alertNoPrivilege(){
        let alert = NSAlert()
        alert.messageText = "Unable to Set PStateDef"
        alert.informativeText = """
        Action was denied by kernel as current user does not have enough privilege.
        Or was canceled by current user.
        
        
        Run AMD Power Gadget as root user or disable privilege check with boot-arg '-amdpnopchk'
        
        Otherwise, click 'Authorize' to in the warning message to allow this change.
        """
        alert.alertStyle = .critical
        alert.addButton(withTitle: "Done")
        alert.beginSheetModal(for: view.window!, completionHandler: nil)
    }
    
    @IBAction func revert(_ sender: Any) {
        data = ProcessorModel.shared.getPStateDef().map({value2Dict(v: $0)})
        tableView.reloadData()
    }
    
    @IBAction func close(_ sender: Any) {
        if !changed{
            closeActually()
            return
        }
        
        let alert = NSAlert()
        alert.messageText = "Your changes will not be saved"
        alert.informativeText = "Click apply to save changes before closing this windows."
        alert.alertStyle = .warning
        alert.addButton(withTitle: "Cancel")
        alert.addButton(withTitle: "Close without saving")
        alert.beginSheetModal(for: view.window!) { (res) in
            if res == NSApplication.ModalResponse.alertSecondButtonReturn {
                self.closeActually()
            }
        }
    }
    
    func closeActually() {
        if let vc = presentingViewController as? PowerToolViewController{
            vc.updatePStateDef()
        }
        
        presentingViewController?.dismiss(self)
    }
    
    @IBAction func `import`(_ sender: Any) {
        let op = NSOpenPanel()
        op.runModal()
        
        if op.url == nil {
            return;
        }
        
        let arr = NSArray.init(contentsOf: op.url!) as! [UInt64]

        data = arr.map({value2Dict(v: $0)})
        tableView.reloadData()
    }
    
    @IBAction func export(_ sender: Any) {
        let op = NSSavePanel()
        op.isExtensionHidden = false
        op.allowedFileTypes = ["pstate"]
        
        op.runModal()
        
        let arr = data.map{ dict2Value(d: $0) }
        (arr as NSArray).write(to: op.url!, atomically: true)
    }
}
