//
//  GraphViewGridLineLayer.swift
//  AMD Power Gadget
//
//  Created by Qi HaoYan on 5/18/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

import Cocoa

class GraphViewGridLineLayer: CALayer {
    @NSManaged var gridColor : NSColor
    @NSManaged var gridWidth : CGFloat
    
    @NSManaged var viewBottom : CGFloat
    @NSManaged var viewHeight : CGFloat
    
    
    @NSManaged var dataMin : CGFloat
    @NSManaged var dataDiff : CGFloat
    @NSManaged var dataY : CGFloat

    var textLayer : CATextLayer = CATextLayer()
    
    override class func needsDisplay(forKey key: String) -> Bool {
        if key == #keyPath(gridColor) ||
            key == #keyPath(gridWidth) ||
            key == #keyPath(viewBottom) ||
            key == #keyPath(viewHeight) ||
            key == #keyPath(dataMin) ||
            key == #keyPath(dataDiff) ||
            key == #keyPath(dataY){
            return true
        }
        return super.needsDisplay(forKey:key)
    }
    
    func setup(){

    }
    
    override func draw(in ctx: CGContext) {
        if dataDiff == 0 {return}
        let attributes = [
            NSAttributedString.Key.font: NSFont.systemFont(ofSize: 12.0),
            NSAttributedString.Key.foregroundColor: gridColor,
        ]

        let height = viewBottom + (CGFloat((dataY - dataMin) / (dataDiff)) * viewHeight)
        let path = CGMutablePath()
        path.move(to: CGPoint(x: 0, y: height))
        path.addLine(to: CGPoint(x: frame.width, y: height))
            
        ctx.addPath(path)
        ctx.setStrokeColor(gridColor.cgColor)
        ctx.setLineWidth(gridWidth)
        ctx.strokePath()
        
        NSGraphicsContext.saveGraphicsState()
        let nsctx = NSGraphicsContext(cgContext: ctx, flipped: false)
        NSGraphicsContext.current = nsctx
        
        let attributedString = NSAttributedString(string: "\(Int(dataY))", attributes: attributes)
        attributedString.draw(at: NSPoint(x: 10, y: height + 2))
        NSGraphicsContext.restoreGraphicsState()
        
    }
    
    override func action(forKey key: String) -> CAAction? {
        if key == #keyPath(dataY) ||
            key == #keyPath(dataMin) ||
            key == #keyPath(dataDiff){
            let ba = CASpringAnimation(keyPath: key)
            ba.damping = 7
            ba.fromValue = self.presentation()?.value(forKey:key)
            ba.duration = 1
            return ba
        }
        
        return super.action(forKey:key)
    }
}
