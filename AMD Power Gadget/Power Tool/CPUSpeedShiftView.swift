//
//  CPUSpeedShiftView.swift
//  AMD Power Gadget
//
//  Created by trulyspinach on 3/8/20.
//

import Cocoa

@IBDesignable
class CPUSpeedShiftView: NSControl {
    
    var selectedItem : Int = 0
    var highlightedItem : Int = -1
    
    @IBInspectable var borderWidth: CGFloat = 1
    @IBInspectable var borderColor: NSColor = NSColor.highlightColor
    
    @IBInspectable var backgroundColor: NSColor = NSColor.highlightColor
    @IBInspectable var textColor: NSColor = NSColor.highlightColor
    
    @IBInspectable var stepColor1: NSColor = NSColor.highlightColor
    @IBInspectable var stepColor2: NSColor = NSColor.highlightColor
    @IBInspectable var stepColor3: NSColor = NSColor.highlightColor
    
    var options : [String] = ["3800Mhz", "2034Mhz", "1400Mhz", "1400Mhz", "1400Mhz"]
    

    override func viewWillMove(toWindow newWindow: NSWindow?) {
        let ta = NSTrackingArea(rect: bounds,
                                options: [.activeInActiveApp, .mouseMoved, .mouseEnteredAndExited],
                                owner: self, userInfo: nil)
        addTrackingArea(ta)
    }
    
    override func mouseMoved(with event: NSEvent) {
        let mp = convert(event.locationInWindow, from: nil)
        let nv = options.count - 1 - Int(mp.y / frame.height * CGFloat(options.count))
        if highlightedItem == nv { return }
        highlightedItem = nv
        setNeedsDisplay(bounds)
    }
    
    override func mouseExited(with event: NSEvent) {
        highlightedItem = -1
        setNeedsDisplay(bounds)
    }
    
    override var acceptsFirstResponder: Bool {return true}
    override func becomeFirstResponder() -> Bool {return true}

    override func mouseDown(with event: NSEvent) {
//        window?.makeFirstResponder(self)
        let mp = convert(event.locationInWindow, from: nil)
        let nv = options.count - 1 - Int(mp.y / frame.height * CGFloat(options.count))
        if selectedItem == nv { return }
        
        selectedItem = nv
        if let action = action {
            NSApp.sendAction(action, to: target, from: self)
        }
        
        setNeedsDisplay(bounds)
    }
    
    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)
        
        guard let context = NSGraphicsContext.current?.cgContext else {
            return
        }
        
        self.layer = layer
        wantsLayer = true
        
        layer?.cornerRadius = 20
        layer?.masksToBounds = true
        
        backgroundColor.setFill()
        dirtyRect.fill()
        
        drawGrid(in: dirtyRect, ctx: context)
    }
    
    func setOptions(newOptions : [String]?, selection : Int){
        if newOptions != nil{
            options = newOptions!
        }
        selectedItem = selection
        setNeedsDisplay(bounds)
    }
    
    private func drawGrid(in rect: CGRect, ctx: CGContext){
        
        let paragraphStyle = NSMutableParagraphStyle()
        paragraphStyle.alignment = .center
        let font = NSFont.systemFont(ofSize: 14)
        let attributes = [
            NSAttributedString.Key.font: font,
            NSAttributedString.Key.foregroundColor: textColor,
            NSAttributedString.Key.paragraphStyle: paragraphStyle,
        ]
        
        let h = CGFloat(frame.height / CGFloat(options.count))
        
        for (i, v) in options.reversed().enumerated(){
            let xC = CGFloat(i) * h
            let r = CGRect(x: 0, y: xC, width: frame.width, height: h)
            ctx.addRect(r)
            ctx.setStrokeColor(borderColor.cgColor)
            ctx.setLineWidth(borderWidth)
            ctx.strokePath()
            
            ctx.addRect(r)
            
            let s = Float(i) / Float(options.count-1)
            let sc = s > 0.75 ? stepColor3 : (s > 0.35 ? stepColor2 : stepColor1)
            
            let i = options.count-i-1
            ctx.setFillColor(i == selectedItem ? sc.lighterColor.cgColor : (i == highlightedItem ? sc.slightLighterColor.cgColor : sc.cgColor))
            ctx.fillPath()
            
            let attributedString = NSAttributedString(string: v, attributes: attributes)
            attributedString.draw(with: CGRect(x: 0, y: xC + ((h - font.xHeight) / 2),
                                               width: frame.width, height: h))
        }
    }
    
}

extension NSColor {
    var lighterColor: NSColor {
        return lighterColor(removeSaturation: 0.5, resultAlpha: -1)
    }
    
    var slightLighterColor: NSColor {
        return lighterColor(removeSaturation: 0.15, resultAlpha: -1)
    }

    func lighterColor(removeSaturation val: CGFloat, resultAlpha alpha: CGFloat) -> NSColor {
        var h: CGFloat = 0, s: CGFloat = 0
        var b: CGFloat = 0, a: CGFloat = 0
        getHue(&h, saturation: &s, brightness: &b, alpha: &a)
        return NSColor(hue: h,
                       saturation: s,
                       brightness: min(b + val, 1),
                       alpha: alpha == -1 ? a : alpha)
    }
}
