//
//  CPUBarLayer.swift
//  AMD Power Gadget
//
//  Created by trulyspinach on 3/5/20.
//

import Cocoa

class CPUBarLayer: CALayer {
    
    @NSManaged var frequencyColor : CGColor
    @NSManaged var loadColor : CGColor
    
    @NSManaged var frequencyFill : CGFloat
    @NSManaged var loadFill : CGFloat
    
    override class func needsDisplay(forKey key: String) -> Bool {
        if key == #keyPath(frequencyFill) ||  key == #keyPath(loadFill){
            return true
        }
        return super.needsDisplay(forKey:key)
    }
    
    override func draw(in ctx: CGContext) {
        
        let h = Float(frame.height * frequencyFill)

        
        ctx.move(to: CGPoint(x: frame.width * 0.5, y: 0))
        ctx.addLine(to: CGPoint(x: frame.width * 0.5, y: CGFloat(h)))
        
        ctx.setLineWidth(frame.width)
        ctx.setLineDash(phase: 0, lengths: [2,3])
        ctx.setStrokeColor(frequencyColor)
        ctx.strokePath()
        
        ctx.move(to: CGPoint(x: frame.width * 0.5, y: 0))
        ctx.addLine(to: CGPoint(x: frame.width * 0.5, y: frame.height * loadFill ))
        
        ctx.setLineWidth(frame.width)
        ctx.setLineDash(phase: 0, lengths: [2,3])
        ctx.setStrokeColor(loadColor)
        ctx.strokePath()
        
    }
    

    
    override func action(forKey key: String) -> CAAction? {
        
        if key == #keyPath(frequencyFill) || key == #keyPath(loadFill) {
            let ba = CABasicAnimation(keyPath: key)
            ba.fromValue = self.presentation()?.value(forKey:key)
            ba.duration = 0.5
            return ba
        }
        return super.action(forKey:key)
    }
}
