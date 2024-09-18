//
//  PowerToolViewController.swift
//  AMD Power Gadget
//
//  Created by trulyspinach on 3/3/20.
//

import Cocoa

class PowerToolViewController: NSViewController, NSWindowDelegate {

    @IBOutlet weak var cpuFreqGraph: CPUPowerStepView!
    
    var timer : Timer?
    
    var vaildStatesClock : [Float] = []
    
    @IBOutlet weak var basicLabel: NSTextField!
    @IBOutlet weak var overviewSpeedShift: CPUSpeedShiftView!
    
    @IBOutlet weak var configTabView: NSTabView!
    @IBOutlet weak var configControl: NSSegmentedControl!
    @IBAction func onConfigControl(_ sender: Any) {
        configTabView.selectTabViewItem(at: configControl.indexOfSelectedItem)
    }
    
    @IBOutlet weak var pStateControl: NSSegmentedControl!
    @IBAction func onPStateControl(_ sender: Any) {
        
    }
    
    
    @IBOutlet var vsView: NSVisualEffectView!
    @IBOutlet weak var boxView: NSBox!
    func toggleTranslucency(enabled : Bool) {
        vsView.state = enabled ? .active : .inactive
        boxView.isTransparent = enabled
    }
    
    @IBOutlet weak var boardHelpButton: NSButton!
    @IBAction func boardHelp(_ sender: Any) {
        let alert = NSAlert()
        alert.messageText = "To enable motherboard display:"
        alert.informativeText = """
        Open your OpenCore config file,
        
        Go to Misc -> Security -> ExposeSensitiveData,
        
        Set the value to 0x08 to expose board information.
        """
        alert.alertStyle = .informational
        alert.addButton(withTitle: "Done")
        alert.beginSheetModal(for: view.window!, completionHandler: nil)
    }
    @IBOutlet weak var cpbSupportedBox: NSButton!
    @IBOutlet weak var cpbEnabledBox: NSButton!
    @IBAction func cpbEnabledBox(_ sender: Any) {
        ProcessorModel.shared.setCPB(enabled: cpbEnabledBox.state == .on)
        updateCPB()
    }
    
    @IBOutlet weak var asaBox: NSButton!
    @IBAction func onASA(_ sender: Any) {
        ProcessorModel.shared.setPPM(enabled: asaBox.state == .on)

        overviewSpeedShift.setOptions(newOptions: nil, selection: asaBox.state == .on ? -1 : 0)

        updateASA()
    }
    
    @IBOutlet weak var lpmBox: NSButton!
    @IBAction func onLPM(_ sender: Any) {
        ProcessorModel.shared.setLPM(enabled: lpmBox.state == .on)
        updateLPM()
    }
    
    @IBOutlet weak var topLabel1: NSTextField!
    @IBOutlet weak var topLabel2: NSTextField!
    
    var updateTime : Double = 0.25
    var freqMax : Float = 0
    var instDelta : UInt64 = 0
    var sumCount = 0
    
    static var activeSelf : PowerToolViewController?
    static func launch(forceFocus: Bool = false){
        if let vc = PowerToolViewController.activeSelf {
            vc.view.window?.orderFrontRegardless()
        } else {
            let mainStoryboard = NSStoryboard.init(name: NSStoryboard.Name("Main"), bundle: nil)
            let controller = mainStoryboard.instantiateController(withIdentifier: NSStoryboard.SceneIdentifier("AMDPowerTool")) as! NSWindowController
            controller.showWindow(self)

            controller.window?.isMovableByWindowBackground = true
            
            if forceFocus {controller.window?.orderFrontRegardless()}
        }
    }
    
    @IBAction func fanControlButton(_ sender: Any) {
        SystemMonitorViewController.launch()
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        vaildStatesClock = ProcessorModel.shared.getVaildPStateClocks()
        cpuFreqGraph.setup(totalCores: ProcessorModel.shared.getNumOfCore())

        sampleCPUGraph()

        timer = Timer.scheduledTimer(withTimeInterval: updateTime, repeats: true, block: { (_) in
            self.sampleCPUGraph()
        })

        setupOverview()

        updatePStateDef()
        updateCPB()
        updateASA()
        
        topLabel1.font = NSFont(name: "SF Pro Rounded", size: 32)
        topLabel2.font = NSFont(name: "SF Pro Rounded", size: 32)
        
        toggleTranslucency(enabled: UserDefaults.usetranslucency)
        
        PowerToolViewController.activeSelf = self
        AppDelegate.updateDockIcon()
    }
    
    override func viewWillAppear() {
        view.window!.delegate = self
        if timer == nil || !timer!.isValid{
            timer = Timer.scheduledTimer(withTimeInterval: updateTime, repeats: true, block: { (_) in
                self.sampleCPUGraph()
            })
        }
    }
    
    override func viewWillDisappear() {
        timer?.invalidate()
    }
    

    
    func windowWillClose(_ notification: Notification) {
        PowerToolViewController.activeSelf = nil
        AppDelegate.updateDockIcon()
    }
    
    func sampleCPUGraph() {
        let metric = ProcessorModel.shared.getMetric(forced: true)
        let load = ProcessorModel.shared.getLoadIndex()
        let a = ProcessorModel.shared.getInstructionDelta()
        let freqs = Array(metric[3...metric.count-1])
        
        cpuFreqGraph.setFreqData(data: Array(metric[3...metric.count-1]),
                                 states: vaildStatesClock, load: load)

        freqMax = max(freqMax, freqs.max()!)
        instDelta += a.reduce(0, +)

        if sumCount >= Int(1 / updateTime) {
            
            topLabel1.stringValue = String(format: "%.1f Ghz", freqMax * 0.001)
            topLabel2.stringValue = suffixNumber(number: NSNumber(value: instDelta))

            sumCount = 0
            freqMax = 0
            instDelta = 0
        } else {
            sumCount += 1
        }
        updatePStateDef()
    }
    
    @IBAction func onOverviewSpeedStep(_ sender: Any) {
        ProcessorModel.shared.setPPM(enabled: false)
        ProcessorModel.shared.setPState(state: overviewSpeedShift.selectedItem)
        updateASA()
    }
    
    func setupOverview() {
        let id = ProcessorModel.shared.cpuidBasic
        let supported = id[7] == 1 ? "Yes" : "Not yet :)"
        
        let memGB = Int(ProcessorModel.shared.systemConfig["mem"]!)! / 1024
        var storageGB = "?"
        if let rs = ProcessorModel.shared.systemConfig["rs"] {
            storageGB = "\(Int(rs)! / 1024)"
        }
         
        basicLabel.stringValue = """
        \(ProcessorModel.shared.systemConfig["cpu"]!)
        Family: \(String(format:"%02X", id[0]))h, Model: \(String(format:"%02X", id[1]))h
        Physical: \(id[2]), Logical: \(id[3])
        L1(Total): \(id[4] * id[2]) KB, L2(Total): \(id[5] * id[2] / 1024) MB, L3(Shared): \(id[6] / 1024) MB
        
        Motherboard: \(ProcessorModel.shared.boardName)
        \(ProcessorModel.shared.boardVender)
        Graphics: \(ProcessorModel.shared.systemConfig["gpu"]!)
        Memory: \(memGB)GB, Storage: \(storageGB)GB
        
        macOS Version: \(ProcessorModel.shared.systemConfig["os"]!)
        AMDRyzenCPUPowerManagement:
          Version: \(ProcessorModel.shared.AMDRyzenCPUPowerManagementVersion), CPU Supported: \(supported)
        
        """
        if ProcessorModel.shared.boardValid {
            boardHelpButton.removeFromSuperview()
        }
    }
    
    func updatePStateDef() {
        ProcessorModel.shared.refreshPStateDef()
        vaildStatesClock = ProcessorModel.shared.getVaildPStateClocks()
        let pstateCur = ProcessorModel.shared.getPState()
        let s = ProcessorModel.shared.getPPM()
        let pdt = vaildStatesClock.map{ "\(Int($0))Mhz" }
        overviewSpeedShift.setOptions(newOptions: pdt, selection: s ? -1 : pstateCur)
    }
    
    func updateCPB() {
        let s = ProcessorModel.shared.getCPB()
        
        cpbSupportedBox.state = s[0] ? NSControl.StateValue.on : NSControl.StateValue.off
        cpbEnabledBox.state = s[1] ? NSControl.StateValue.on : NSControl.StateValue.off
    }
    
    func updateASA() {
        let s = ProcessorModel.shared.getPPM()
        
        asaBox.state = s ? NSControl.StateValue.on : NSControl.StateValue.off
        updateLPM()
    }
    
    func updateLPM() {
        lpmBox.isEnabled = ProcessorModel.shared.getPPM()
        
        let s = ProcessorModel.shared.getLPM()
        
        lpmBox.state = s ? NSControl.StateValue.on : NSControl.StateValue.off
    }
    
    @IBAction func openGitHub(_ sender: Any) {
        NSWorkspace.shared.open(URL(string: "https://github.com/trulyspinach/SMCAMDProcessor")!)
    }
    
    func suffixNumber(number:NSNumber) -> String {
        var num:Double = number.doubleValue
        let sign = ((num < 0) ? "-" : "" )

        num = fabs(num)
        if (num < 1000.0){
            return "\(sign)\(num)"
        }
        let exp:Int = Int(log10(num) / 3.0 )
        let units:[String] = ["K","M","G","T","P","E"]
        let roundedNum:Double = round(10 * num / pow(1000.0,Double(exp))) / 10

        return "\(sign)\(roundedNum)\(units[exp-1])"
    }
}
