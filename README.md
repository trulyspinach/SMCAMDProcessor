SMCAMDProcessor
========

AMD Processor power management plugin for [VirtualSMC](https://github.com/acidanthera/VirtualSMC).


Please note that this release is at very initial stage of development, make sure you have a proper backup of your EFI folder and never run on any system that matters. 

### Now introducing "AMD Power Gadget". Just like the Intel one!


<img src="imgs/APG.png" width="40%">

## Features
* Supports for reading of temperature, energy and clock data on AMD 17h Processors.

## Tested Processors
* Ryzen 9 3900X
* Ryzen 7 3700X
* Ryzen 7 2700X
* Ryzen 5 3600
* Threadripper 2990WX

<img src="imgs/iStats.png" width="40%">

## Credits
* [necross2](https://github.com/necross2) for adding support to temperature sensor offset.

## Notes
* I am still fairly new to macOS kernel development, this software project was initally a hobby project to get some reading on my newly built AMD hackintosh computer.

* With that being said, please bear with some of the spaghetti and not-idiomatic codes. Any criticism is much welcomed :)
