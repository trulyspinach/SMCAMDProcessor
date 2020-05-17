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
    
    @IBInspectable var gridWidth: CGFloat = 1;
    @IBInspectable var gridColor: NSColor = NSColor.highlightColor
    
    @IBInspectable var lineColor1: NSColor = NSColor.highlightColor
    @IBInspectable var lineColor2: NSColor = NSColor.highlightColor
    @IBInspectable var lineColorX: NSColor = NSColor.highlightColor
    @IBInspectable var lineWidth: CGFloat = 1;
    @IBInspectable var lineCurviness: CGFloat = 0.1
    
    @IBInspectable var dotRadius: CGFloat = 1;
    @IBInspectable var dotStrokeColor: NSColor = NSColor.highlightColor
    @IBInspectable var dotFillColor: NSColor = NSColor.highlightColor
    
    @IBInspectable var viewTopPercentage: CGFloat = 1;
    @IBInspectable var viewBottomPercentage: CGFloat = 0;
    
    let colorSpace = CGColorSpaceCreateDeviceRGB()
    
    var dataPoints : [[Double]] = []
    var viewTop : CGFloat = 100;
    var viewBottom : CGFloat = 0;
    var viewHeight : CGFloat = 100;
    
    let gridDivLines: [Double] = [0, 0.18, 0.38, 0.68, 1]
    let maxDataPoints = 30
    
    let dummyData: [Double] = [1,3,2]
//    let dummyData: [Double] = [1,1,1,3,2 ,1]
    
    
    var dataMax : Double = 0
    var dataMin : Double = 0
    var dataDiff : Double = 0
   
  
    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)
        
        guard let context = NSGraphicsContext.current?.cgContext else {
            return
        }
        
        NSColor.white.setFill()
        dirtyRect.fill()
        
        viewTop = dirtyRect.height * viewTopPercentage
        viewBottom = dirtyRect.height * viewBottomPercentage
        viewHeight = viewTop - viewBottom;
        
//        fillWithDummyData()

        drawBackground(in: dirtyRect, context: context)

        
        if dataPoints[0].count > 2 {
            drawGrid(in: dirtyRect, context: context)
            for (index, points) in dataPoints.enumerated() {
                let foregroundColors = getForegroundColors(index: index)
                let lineColor = getLineColors(index: index)
                drawLine(in: dirtyRect, context: context, points: points, foregroundColors: foregroundColors, lineColor: lineColor)
                drawDataPoint(in: dirtyRect, context: context, points: points, pointColor: lineColor)
            }
        }
        
        self.layer = layer
        wantsLayer = true
        layer?.cornerRadius = 20
        layer?.masksToBounds = true
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
        for d in dummyData {
            addData(values: [d])
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
    
    func addData(values: [Double]){
        if dataPoints.isEmpty {
            for _ in 0...values.count {
                dataPoints.append([])
            }
        }
        
        for (index, value) in values.enumerated() {
            dataPoints[index].append(value)
            if dataPoints[index].count > maxDataPoints {
                dataPoints[index].remove(at: 0)
            }
        }
                
        let valueMin = values.min()!
        if (valueMin < dataMin || dataMin == 0) {
            dataMin = valueMin.rounded(FloatingPointRoundingRule.down)
        }
        let valueMax = values.max()!
        if (valueMax > dataMax) {
            dataMax = valueMax.rounded(FloatingPointRoundingRule.up)
        }
        
        //thanks to yurkins for the fix
        dataDiff = max(dataMax - dataMin, 1)

        setNeedsDisplay(bounds)
    }
    
    open override func prepareForInterfaceBuilder() {
        super.prepareForInterfaceBuilder()
        fillWithDummyData()
    }
    
    private func drawGrid(in rect: CGRect, context: CGContext){
                
        let attributes = [
            NSAttributedString.Key.font: NSFont.systemFont(ofSize: 12.0),
            NSAttributedString.Key.foregroundColor: gridColor,
        ]
        
        var lastHeight : CGFloat = -1000;
        let minSpacing : CGFloat = 20;
        for v in gridDivLines{
            let valueOfV = dataMin + dataDiff * v
            let lineHeight = viewHeight * CGFloat((valueOfV - dataMin) / (dataDiff)) + viewBottom
            
            if abs(lineHeight - lastHeight)  < minSpacing {
                continue
            }
            
            let path = CGMutablePath()
            path.move(to: CGPoint(x: 0, y: lineHeight))
            path.addLine(to: CGPoint(x: rect.width, y: lineHeight))
            
            context.addPath(path)
            context.setStrokeColor(gridColor.cgColor)
            context.setLineWidth(gridWidth)
            context.strokePath()
            
            let attributedString = NSAttributedString(string: "\(Int(valueOfV))", attributes: attributes)
            attributedString.draw(at: NSPoint(x: 10, y: lineHeight + 2))
            
            lastHeight = lineHeight
        }

    }
    
    private func drawLine(in rect: CGRect, context: CGContext, points: [Double], foregroundColors: [CGColor], lineColor: CGColor) {
        guard let grad = CGGradient.init(colorsSpace: colorSpace,
                                         colors: foregroundColors as CFArray,
                                         locations: [0, 1] as [CGFloat]) else {return}
        
        context.saveGState()
        defer { context.restoreGState() }
        
        let path = CGMutablePath()
        path.move(to: CGPoint(x: -1, y: -1), transform: .identity)
        
        
        let xStep : Double = Double(rect.size.width) / Double(points.count - 1)
        var lastPoint = CGPoint(x:-dotRadius * 2,y:-1);
        
        for (i, v) in points.enumerated(){
            let newPoint = CGPoint(x: CGFloat(Double(i) * xStep),
                                   y: viewBottom + (CGFloat((v - dataMin) / (dataDiff)) * viewHeight))
            
            let difference = newPoint.x - lastPoint.x
            
            var x = lastPoint.x + (difference * lineCurviness)
            var y = lastPoint.y
            let controlPointOne = CGPoint(x: x, y: y)
            
            x = newPoint.x - (difference * lineCurviness)
            y = newPoint.y
            let controlPointTwo = CGPoint(x: x, y: y)
            
            path.addCurve(to: newPoint, control1: controlPointOne, control2: controlPointTwo)
            
            lastPoint = newPoint;
        }
        
        path.addLine(to: CGPoint(x: rect.size.width + dotRadius*2, y: lastPoint.y))
        path.addLine(to: CGPoint(x: rect.size.width + dotRadius*2, y: -1))
        
        context.addPath(path)
        context.setStrokeColor(lineColor)
        context.setLineWidth(lineWidth)
        context.strokePath()
        
        context.addPath(path)
        context.clip()
        
        context.drawLinearGradient(grad, start: CGPoint(x: 0, y: rect.height),
                                   end: CGPoint(x: 0, y: 0), options: [])
    }
    
    private func drawDataPoint(in rect: CGRect, context: CGContext, points: [Double], pointColor: CGColor){
        
        let xStep : Double = Double(rect.size.width) / Double(points.count - 1)
        
        for (i, v) in points.enumerated(){
            let newPoint = CGPoint(x: CGFloat(Double(i) * xStep),
                                   y: viewBottom + (CGFloat((v - dataMin) / (dataDiff)) * viewHeight))
            
            let path = CGMutablePath()
            path.addArc(center: newPoint, radius: dotRadius, startAngle: 0, endAngle: CGFloat(2.0 * Double.pi), clockwise: true)
            path.closeSubpath();
            
            context.addPath(path);
            context.setFillColor(dotFillColor.cgColor)
            context.fillPath();
            
            context.addPath(path);
            context.setStrokeColor(pointColor)
            context.setLineWidth(lineWidth)
            context.strokePath()
        }
    }
}
