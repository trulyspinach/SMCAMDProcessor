//
//  GraphViewLineLayer.swift
//  AMD Power Gadget
//
//  Created by Qi HaoYan on 5/18/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

import Cocoa

class GraphViewLineLayer: CALayer {

    @NSManaged var lineColor : CGColor
    @NSManaged var dotFillColor : CGColor
    @NSManaged var pointColor : CGColor

    @NSManaged var lineWidth : CGFloat
    @NSManaged var dotRadius : CGFloat
    
    @NSManaged var viewBottom : CGFloat
    @NSManaged var viewHeight : CGFloat
    
    
    @NSManaged var dataMin : CGFloat
    @NSManaged var dataDiff : CGFloat
    @NSManaged var dataXScale : CGFloat
    @NSManaged var pointsX : [CGFloat]
    @NSManaged var pointsY : [CGFloat]
    @NSManaged var xOffset : CGFloat
    
    @NSManaged var scrollIndexBar : CGFloat
    
    
    var colorSpace = CGColorSpaceCreateDeviceRGB()
    @NSManaged var foregroundColors : [CGColor]
    
    override class func needsDisplay(forKey key: String) -> Bool {
        if key == #keyPath(lineColor) ||
            key == #keyPath(dotFillColor) ||
            key == #keyPath(pointColor) ||
            key == #keyPath(lineWidth) ||
            key == #keyPath(dotRadius) ||
            key == #keyPath(viewBottom) ||
            key == #keyPath(viewHeight) ||
            key == #keyPath(dataMin) ||
            key == #keyPath(dataDiff) ||
            key == #keyPath(pointsX) ||
            key == #keyPath(pointsY) ||
            key == #keyPath(dataXScale) ||
            key == #keyPath(xOffset){
            return true
        }
        return super.needsDisplay(forKey:key)
    }
    
    override func draw(in ctx: CGContext) {
        if dataDiff == 0{return}
        guard let grad = CGGradient.init(colorsSpace: colorSpace,
                                         colors: foregroundColors as CFArray,
                                         locations: [0, 1] as [CGFloat]) else {return}

        let path = CGMutablePath()
        path.move(to: CGPoint(x: -1, y: -1), transform: .identity)
        
        
        var lastPoint = CGPoint(x:-lineWidth * 2,y:-1);
        for (i, v) in pointsY.enumerated(){
            let newPoint = CGPoint(x: pointsX[i] * dataXScale + xOffset,
                                   y: viewBottom + ((v - dataMin) / (dataDiff)) * viewHeight)
            
            let difference = newPoint.x - lastPoint.x
            
            var x = lastPoint.x + (difference * 0.2)
            var y = lastPoint.y
            let controlPointOne = CGPoint(x: x, y: y)
            
            x = newPoint.x - (difference * 0.2)
            y = newPoint.y
            let controlPointTwo = CGPoint(x: x, y: y)
            
            path.addCurve(to: newPoint, control1: controlPointOne, control2: controlPointTwo)
            
            lastPoint = newPoint;
        }
        
        path.addLine(to: CGPoint(x: frame.size.width + lineWidth*2, y: lastPoint.y))
        path.addLine(to: CGPoint(x: frame.size.width + lineWidth*2, y: -1))
        
        ctx.addPath(path)
        ctx.setStrokeColor(lineColor)
        ctx.setLineWidth(lineWidth)
        ctx.strokePath()
        ctx.addPath(path)
        
        ctx.saveGState()
        ctx.clip()
        
        ctx.drawLinearGradient(grad, start: CGPoint(x: 0, y: frame.height),
                                   end: CGPoint(x: 0, y: 0), options: [])
        ctx.restoreGState()
       
        
        drawDataPoint(ctx: ctx)
    }
    
    private func drawDataPoint(ctx: CGContext){

        for (i, v) in pointsY.enumerated(){
            let newPoint = CGPoint(x: pointsX[i] * dataXScale + xOffset,
                                   y: viewBottom + (CGFloat((v - dataMin) / (dataDiff)) * viewHeight))
            
            let path = CGMutablePath()
            path.addArc(center: newPoint, radius: dotRadius, startAngle: 0, endAngle: CGFloat(2.0 * Double.pi), clockwise: true)
            path.closeSubpath();
            
            ctx.addPath(path);
            ctx.setFillColor(dotFillColor)
            ctx.fillPath();
            
            ctx.addPath(path);
            ctx.setStrokeColor(pointColor)
            ctx.setLineWidth(lineWidth)
            ctx.strokePath()
        }
    }

    
    override func action(forKey key: String) -> CAAction? {
        
        if key == #keyPath(dataMin) ||
            key == #keyPath(dataDiff){
            let ba = CASpringAnimation(keyPath: key)
            ba.damping = 7
            ba.fromValue = self.presentation()?.value(forKey:key)
            ba.duration = 1
            return ba
        }
        
        if key == #keyPath(dataXScale) ||
            key == #keyPath(xOffset){
            let ba = CABasicAnimation(keyPath: key)
            ba.fromValue = self.presentation()?.value(forKey:key)
            ba.duration = 1
            return ba
        }
        return super.action(forKey:key)
    }
    
}
