//
//  GraphView.swift
//  AMD Power Gadget
//
//  Created by Qi HaoYan on 2/22/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

import Cocoa
import CoreText

@IBDesignable
class GraphView: NSView {
    
    @IBInspectable var backgroundColor1: NSColor = NSColor.highlightColor
    @IBInspectable var backgroundColor2: NSColor = NSColor.highlightColor
    
    @IBInspectable var foregroundColor1_1: NSColor = NSColor.highlightColor
    @IBInspectable var foregroundColor1_2: NSColor = NSColor.highlightColor
    
    @IBInspectable var foregroundColor2_1: NSColor = NSColor.highlightColor
    @IBInspectable var foregroundColor2_2: NSColor = NSColor.highlightColor
    
    @IBInspectable var foregroundColorX_1: NSColor = NSColor.highlightColor
    @IBInspectable var foregroundColorX_2: NSColor = NSColor.highlightColor
    
    @IBInspectable var gridWidth: CGFloat = 1
    @IBInspectable var gridColor: NSColor = NSColor.highlightColor
    
    @IBInspectable var lineColor1: NSColor = NSColor.highlightColor
    @IBInspectable var lineColor2: NSColor = NSColor.highlightColor
    @IBInspectable var lineColorX: NSColor = NSColor.highlightColor
    @IBInspectable var lineWidth: CGFloat = 1
    @IBInspectable var lineCurviness: CGFloat = 0.1
    
    @IBInspectable var dotRadius: CGFloat = 1
    @IBInspectable var dotStrokeColor: NSColor = NSColor.highlightColor
    @IBInspectable var dotFillColor: NSColor = NSColor.highlightColor
    
    @IBInspectable var viewTopPercentage: CGFloat = 1
    @IBInspectable var viewBottomPercentage: CGFloat = 0
    
    let colorSpace = CGColorSpaceCreateDeviceRGB()
    
    var viewTop : CGFloat = 100
    var viewBottom : CGFloat = 0
    var viewHeight : CGFloat = 100
    
    let gridDivLines: [CGFloat] = [0, 0.18, 0.38, 0.68, 1]
    let maxDataPoints = 30
    
    let dummyData: [Double] = [1,3,2]
//    let dummyData: [Double] = [1,1,1,3,2 ,1]
    
    
    var dataMax : CGFloat = 0
    var dataMin : CGFloat = 0
    var dataDiff : CGFloat = 0
   
    var lastMaxGrid : CGFloat = 0
    var lastMinGrid : CGFloat = 0
    
    var lines : [GraphViewLineLayer] = []
    
    var gridLines : [GraphViewGridLineLayer] = []
  
    func setup() {
        viewTop = frame.height * viewTopPercentage
        viewBottom = frame.height * viewBottomPercentage
        viewHeight = viewTop - viewBottom

        self.layer = layer
        wantsLayer = true
        layer?.cornerRadius = 20
        layer?.masksToBounds = true
    }
    
    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)
        
        guard let context = NSGraphicsContext.current?.cgContext else {
            return
        }
        
        NSColor.white.setFill()
        dirtyRect.fill()
        drawBackground(in: dirtyRect, context: context)
    }
    
    func addLine() -> Int {
        let lineID = lines.count
        let newLayer = GraphViewLineLayer()
        lines.append(newLayer)
        
        newLayer.lineColor = getLineColors(index: lineID)
        newLayer.dotFillColor = dotFillColor.cgColor
        newLayer.pointColor = getLineColors(index: lineID)
        newLayer.lineWidth = lineWidth
        newLayer.viewBottom = viewBottom
        newLayer.viewHeight = viewHeight
        newLayer.dotRadius = dotRadius
        newLayer.foregroundColors = getForegroundColors(index: lineID)
        
        newLayer.frame = CGRect(x: 0, y: 0, width: frame.width, height: frame.height)
        self.layer!.addSublayer(newLayer)
        newLayer.display()
        
        return lineID
    }
    
    func addGridLine(v : Double) {
        let newLayer = GraphViewGridLineLayer()
        gridLines.append(newLayer)
        
        newLayer.viewBottom = viewBottom
        newLayer.viewHeight = viewHeight
        newLayer.gridColor = gridColor
        newLayer.gridWidth = gridWidth
        newLayer.dataMin = dataMin
        newLayer.dataDiff = dataDiff
        newLayer.dataY = CGFloat(v)
        
        gridLines.sort { (a, b) -> Bool in
            return a.dataY > b.dataY
        }
        
        newLayer.frame = CGRect(x: 0, y: 0, width: frame.width, height: frame.height)
        self.layer!.addSublayer(newLayer)
        newLayer.display()
    }
    
    private func getForegroundColors(index: Int) -> [CGColor] {
        if index == 0 { return [foregroundColor1_1.cgColor, foregroundColor1_2.cgColor] }
        else if index == 1 { return [foregroundColor2_1.cgColor, foregroundColor2_2.cgColor] }
        else { return [foregroundColorX_1.cgColor, foregroundColorX_2.cgColor] }
    }
    private func getLineColors(index: Int) -> CGColor {
        if index == 0 { return lineColor1.cgColor}
        else if index == 1 { return lineColor2.cgColor }
        else { return lineColorX.cgColor }
    }
    
    private func fillWithDummyData(){
        let lid = addLine()
        
        for (i, d) in dummyData.enumerated() {
            addData(forline: lid, x: Double(i), y: d)
        }
    }
    
    private func drawBackground(in rect: CGRect, context: CGContext) {
        context.saveGState()
        defer { context.restoreGState() }
        

        let baseColor = backgroundColor1
        let middleStop = backgroundColor2
        
        let gradientColors = [baseColor.cgColor, middleStop.cgColor]
        let locations: [CGFloat] = [0.0, 1]
        
        guard let gradient = CGGradient(
            colorsSpace: colorSpace,
            colors: gradientColors as CFArray,
            locations: locations)
            else {
                return
        }
        
        let startPoint = CGPoint(x: rect.size.height / 2, y: 0)
        let endPoint = CGPoint(x: rect.size.height / 2, y: rect.size.width)
        context.drawLinearGradient(gradient, start: startPoint, end: endPoint, options: [])
    }
    
    func addData(forline: Int, x: Double, y: Double){
        let line = lines[forline]
        let cgvalue = CGFloat(y)
        
        var xs = line.pointsX
        var ys = line.pointsY
        
        xs.append(CGFloat(x))
        ys.append(CGFloat(y))
        
        if xs.count > maxDataPoints {
            xs.remove(at: 0)
            ys.remove(at: 0)
        }

        line.pointsX = xs
        line.pointsY = ys
        
        if cgvalue < dataMin || dataMin == 0 {
            if lastMinGrid == 0 {
                addGridLine(v: y)
                lastMinGrid = CGFloat(y)
            } else {
                assert(lastMinGrid >= cgvalue)
                if lastMinGrid - cgvalue > dataDiff * 0.25{
                    addGridLine(v: y)
                    lastMinGrid = CGFloat(y)
                }
            }

            dataMin = CGFloat(cgvalue)
        }
        
        if cgvalue > dataMax || dataMax == 0 {
            if lastMaxGrid == 0 {
                addGridLine(v: y)
                lastMaxGrid = CGFloat(y)
            } else {
                assert(lastMaxGrid <= cgvalue)
                if cgvalue - lastMaxGrid > dataDiff * 0.25{
                    
                    addGridLine(v: y)
                    lastMaxGrid = CGFloat(y)
                }
            }
            
            dataMax = CGFloat(cgvalue)
        }
        
        //thanks to yurkins for the fix
        dataDiff = max(dataMax - dataMin, 1)
        
        line.dataMin = dataMin
        line.dataDiff = dataDiff
        
        line.scrollIndexBar = -1
        if xs.count > 2 {
            let ds = (line.frame.width) / (xs.last! - xs[1])
            line.dataXScale = ds
            line.xOffset = -xs[1] * ds
        }
        
        updateGridLine()
        setNeedsDisplay(bounds)
    }
    
    func updateGridLine() {
        if gridLines.isEmpty {return}
        gridLines.first!.opacity = 1
        gridLines.first!.dataDiff = dataDiff
        gridLines.first!.dataMin = dataMin
        
        if gridLines.count < 2 {return}
        var lastHeight = viewBottom + (CGFloat((gridLines.first!.dataY - dataMin) / (dataDiff)) * viewHeight)
        for gl in gridLines[1...gridLines.count-1]{
            gl.dataDiff = dataDiff
            gl.dataMin = dataMin
            
            let curHeight = viewBottom + (CGFloat((gl.dataY - dataMin) / (dataDiff)) * viewHeight)
            
            let hide = abs(curHeight - lastHeight) > 20
            gl.opacity = hide ? 1 : 0.2
            
            if hide {lastHeight = curHeight}
        }
    }
    
    open override func prepareForInterfaceBuilder() {
        super.prepareForInterfaceBuilder()
        setup()
        fillWithDummyData()
    }
}
