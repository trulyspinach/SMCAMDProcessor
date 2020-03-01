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
    
    @IBInspectable var foregroundColor1: NSColor = NSColor.highlightColor
    @IBInspectable var foregroundColor2: NSColor = NSColor.highlightColor
    
    @IBInspectable var gridWidth: CGFloat = 1;
    @IBInspectable var gridColor: NSColor = NSColor.highlightColor
    
    @IBInspectable var lineColor: NSColor = NSColor.highlightColor
    @IBInspectable var lineWidth: CGFloat = 1;
    @IBInspectable var lineCurviness: CGFloat = 0.1
    
    @IBInspectable var dotRadius: CGFloat = 1;
    @IBInspectable var dotStrokeColor: NSColor = NSColor.highlightColor
    @IBInspectable var dotFillColor: NSColor = NSColor.highlightColor
    
    @IBInspectable var viewTopPercentage: CGFloat = 1;
    @IBInspectable var viewBottomPercentage: CGFloat = 0;
    
    var dataPoints : [Double] = []
    var sortedDataPoint : [Double] = []
    var viewTop : CGFloat = 100;
    var viewBottom : CGFloat = 0;
    var viewHeight : CGFloat = 100;
    
    let gridDivLines: [Double] = [0, 0.15, 0.25, 0.35, 0.5, 0.6, 0.8, 0.9, 1]
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

        drawBackground(in: dirtyRect, context: context, colorSpace: CGColorSpaceCreateDeviceRGB())

        
        if dataPoints.count > 2 {
            drawGrid(in: dirtyRect, context: context)
            drawLine(in: dirtyRect, context: context, colorSpace: CGColorSpaceCreateDeviceRGB())
            drawDataPoint(in: dirtyRect, context: context)
        }
        
        self.layer = layer
        wantsLayer = true
        layer?.cornerRadius = 20
        layer?.masksToBounds = true
    }
    
    private func fillWithDummyData(){
        for d in dummyData {
            addData(value: d)
        }
    }
    
    private func drawBackground(in rect: CGRect, context: CGContext, colorSpace: CGColorSpace?) {

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
    
    func addData(value: Double){
        dataPoints.append(value);
        sortedDataPoint.append(value.rounded())
        
        if dataPoints.count > maxDataPoints {
            dataPoints.remove(at: 0)
        }
        
        if sortedDataPoint.count > maxDataPoints {
            let countedSet = NSCountedSet(array: sortedDataPoint)
            let mostFrequent = countedSet.max { countedSet.count(for: $0) < countedSet.count(for: $1) }
            
            sortedDataPoint.remove(at: sortedDataPoint.firstIndex(of: mostFrequent as! Double)!)
        }
        
        sortedDataPoint = sortedDataPoint.sorted()
        
        dataMax = sortedDataPoint.max()!
        dataMin = sortedDataPoint.min()!
        
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
            let valueOfV = sortedDataPoint[Int(Double(sortedDataPoint.count-1) * v)]
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
    
    private func drawLine(in rect: CGRect, context: CGContext, colorSpace: CGColorSpace?) {
        
        let rectWidth = rect.size.width
        
        
        guard let grad = CGGradient.init(colorsSpace: colorSpace,
                                         colors: [foregroundColor1.cgColor, foregroundColor2.cgColor] as CFArray,
                                         locations: [0, 1] as [CGFloat]) else {return}
        
        
        
        context.saveGState()
        defer { context.restoreGState() }
        
        let path = CGMutablePath()
        path.move(to: CGPoint(x: 0, y: 0), transform: .identity)
        
        
        let xStep : Double = Double(rect.size.width) / Double(dataPoints.count - 1)
        var lastPoint = CGPoint(x:0,y:0);
        
        for (i, v) in dataPoints.enumerated(){
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
        
        
        
        path.addLine(to: CGPoint(x: rect.size.width, y: 0))
        
        
        context.addPath(path)
        context.setStrokeColor(lineColor.cgColor)
        context.setLineWidth(lineWidth)
        context.strokePath()
        
        context.addPath(path)
        context.clip()
        
        
        context.drawLinearGradient(grad, start: CGPoint(x: 0, y: rect.height),
                                   end: CGPoint(x: 0, y: 0), options: [])
    }
    
    private func drawDataPoint(in rect: CGRect, context: CGContext){
        
        let xStep : Double = Double(rect.size.width) / Double(dataPoints.count - 1)
        
        for (i, v) in dataPoints.enumerated(){
            let newPoint = CGPoint(x: CGFloat(Double(i) * xStep),
                                   y: viewBottom + (CGFloat((v - dataMin) / (dataDiff)) * viewHeight))
            
            let path = CGMutablePath()
            
    
            path.addArc(center: newPoint, radius: dotRadius, startAngle: 0, endAngle: CGFloat(2.0 * Double.pi), clockwise: true)
            path.closeSubpath();
            
            context.addPath(path);
            context.setFillColor(dotFillColor.cgColor)
            context.fillPath();
            
            context.addPath(path);
            context.setStrokeColor(dotStrokeColor.cgColor)
            context.setLineWidth(lineWidth)
            context.strokePath()

            
        }
    }
}
