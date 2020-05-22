//
//  CPUPowerStepView.swift
//  AMD Power Gadget
//
//  Created by Qi HaoYan on 3/3/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

import Cocoa

@IBDesignable
class CPUPowerStepView: NSView {
    
    var viewTop : CGFloat = 100;
    var viewBottom : CGFloat = 0;
    var viewHeight : CGFloat = 100;
    
    @IBInspectable var viewTopPercentage: CGFloat = 1;
    @IBInspectable var viewBottomPercentage: CGFloat = 0;
    
    @IBInspectable var barMarginLeft: CGFloat = 20;
    @IBInspectable var barMarginRight: CGFloat = 20;
    @IBInspectable var barSpacing: CGFloat = 30;
    @IBInspectable var maxBarWidth: CGFloat = 30;
    @IBInspectable var barCorner: CGFloat = 30;
    @IBInspectable var barTopColor: NSColor = NSColor.white
    @IBInspectable var barTopWidth: CGFloat = 1;
    @IBInspectable var lineCurviness: CGFloat = 0.1
    
    @IBInspectable var backgroundColor1: NSColor = NSColor.highlightColor
    @IBInspectable var backgroundColorSide: NSColor = NSColor.highlightColor
    @IBInspectable var textColor2: NSColor = NSColor.highlightColor
    
    @IBInspectable var barColor1: NSColor = NSColor.white
    @IBInspectable var barColor2: NSColor = NSColor.red
    
    @IBInspectable var gridWidth: CGFloat = 1;
    @IBInspectable var gridColor: NSColor = NSColor.highlightColor
    
    @IBInspectable var borderWidth: CGFloat = 1;
    @IBInspectable var borderColor: NSColor = NSColor.highlightColor
    
    var cpuFreqData : [Float] = []
    var cpuPstateData : [Float] = []
    var cpuLoadData : [Float] = []
    var dataMax : Float = 0
    var dataMin : Float = 1800
    var dataDiff : Float = 0
    var dataMaxCur : Float = 0
    var dataMinCur : Float = 0
    var dataDiffCur : Float = 0
    
    let dummyCpuFreq : [Float] = [3750, 3800, 3800]
    let dummyCpuLoad : [Float] = [0.23,0.445,0.345]
    
    var barLayers : [CPUBarLayer] = []
    var verticalLines : [CGFloat] = []
    

    
    override var mouseDownCanMoveWindow: Bool{
        get {return true}
    }
    
    func setup(totalCores : Int) {
        self.layer = layer
        wantsLayer = true


       
        layer?.cornerRadius = 20
        layer?.masksToBounds = true
        // layer?.shadowOpacity = 1
        // layer?.shadowColor = NSColor.white.cgColor
        viewTop = frame.height * viewTopPercentage
        viewBottom = frame.height * viewBottomPercentage
        viewHeight = viewTop - viewBottom
        
        let barViewWidth = frame.width - barMarginRight - barMarginLeft
        var barWidth = (barViewWidth - barSpacing *
            CGFloat(totalCores + 1)) / CGFloat(totalCores)
        var actualBarSpacing = barSpacing
        if barWidth > maxBarWidth {
            barWidth = maxBarWidth
            actualBarSpacing = (barViewWidth - barWidth *
                CGFloat(totalCores)) / CGFloat(totalCores + 1)
        }
    
        
        for i in 0...totalCores-1 {
            let barLayer = CPUBarLayer()
            let s = actualBarSpacing + (CGFloat(i) * (barWidth + actualBarSpacing)) + barMarginLeft
            
            barLayer.frame = NSRect(x: s, y: viewBottom, width: barWidth, height: viewHeight)
            
            barLayer.frequencyColor = barColor1.cgColor
            barLayer.loadColor = barColor2.cgColor
            
            
            
            self.layer!.addSublayer(barLayer)
            barLayer.display()
            
            barLayers.append(barLayer)
            
            verticalLines.append(s + (barWidth + actualBarSpacing)*0.5 - barWidth*0.5-gridWidth*2)
        }
        
    }
    
    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)
        
        guard let context = NSGraphicsContext.current?.cgContext else {
            return
        }

        // backgroundColor1.setFill()
        // dirtyRect.fill()
        //
        // // context.addRect(CGRect(x: 0, y: 0, width: barMarginLeft, height: frame.height))
        // // context.setFillColor(backgroundColorSide.cgColor)
        // // context.fillPath()


        drawGrid(in: dirtyRect, context: context)
        
    }
    var ttt : CGFloat = 0
    func setFreqData(data : [Float], states : [Float], load : [Float]){
        cpuFreqData = data
        cpuPstateData = states
        cpuLoadData = load
        dataMax = max(dataMax, cpuFreqData.max()!, cpuPstateData.max()!)
        dataMin = min(dataMin, cpuFreqData.min()!, cpuPstateData.min()!)
        dataDiff = dataMax - dataMin
        
        
        dataMaxCur = cpuFreqData.max()!
        dataMinCur = cpuFreqData.min()!
        dataDiffCur = dataMaxCur - dataMinCur
        
        
        drawCPUBars()
        
        setNeedsDisplay(bounds)
    }
    
    override var isOpaque: Bool{
        get {return false}
    }
    
    override func prepareForInterfaceBuilder() {
        setup(totalCores: dummyCpuFreq.count)
        setFreqData(data: dummyCpuFreq,
                    states: [3800, 2800, 2200],
                    load : dummyCpuLoad)
        
    }
    
    
    private func drawCPUBars() {
        if barLayers.count <= 0 {
            return
        }
        for (i, v) in cpuFreqData.enumerated() {
            let ratio = CGFloat((v - dataMin) / (dataDiff))

            barLayers[i].frequencyFill = ratio
            barLayers[i].loadFill = CGFloat(cpuLoadData[i])
        }
    }
    
    private func drawGrid(in rect: CGRect, context: CGContext){
        
        let attributes = [
            NSAttributedString.Key.font: NSFont.systemFont(ofSize: 12),
            NSAttributedString.Key.foregroundColor: textColor2,
        ]
        
        let lineEnd = rect.width - barMarginRight
        
        for v in cpuPstateData{
            let valueOfV = v
            let lineHeight = viewHeight * CGFloat((valueOfV - dataMin) / (dataDiff)) + viewBottom
            
            let path = CGMutablePath()
            path.move(to: CGPoint(x: barMarginLeft - 34, y: lineHeight))
            path.addLine(to: CGPoint(x: lineEnd, y: lineHeight))
            
            context.addPath(path)
            context.setStrokeColor(gridColor.cgColor)
            context.setLineWidth(gridWidth)
            context.strokePath()
            
            let attributedString = NSAttributedString(string: "\(Int(valueOfV))", attributes: attributes)
            attributedString.draw(at: NSPoint(x: barMarginLeft - 34, y: lineHeight))
            
        }
        
        for v in verticalLines {
            context.move(to: CGPoint(x: v,y: 0))
            context.addLine(to: CGPoint(x: v, y: frame.height))
            context.setStrokeColor(gridColor.cgColor)
            context.setLineWidth(2)
            context.setLineDash(phase: 0, lengths: [3,0])
            context.strokePath()
        }
    }
}

extension NSColor {
    func interpolateRGBColorTo(_ end: NSColor, fraction: CGFloat) -> NSColor? {
        let f = min(max(0, fraction), 1)
        
        guard let c1 = self.cgColor.components, let c2 = end.cgColor.components else { return nil }
        
        let r: CGFloat = CGFloat(c1[0] + (c2[0] - c1[0]) * f)
        let g: CGFloat = CGFloat(c1[1] + (c2[1] - c1[1]) * f)
        let b: CGFloat = CGFloat(c1[2] + (c2[2] - c1[2]) * f)
        let a: CGFloat = CGFloat(c1[3] + (c2[3] - c1[3]) * f)
        
        return NSColor(red: r, green: g, blue: b, alpha: a)
    }
}
