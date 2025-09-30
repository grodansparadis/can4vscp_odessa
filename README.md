<h1>Odessa general CAN4VSCP module</h1>

This module is part of the [VSCP project](https://www.vscp.org).  It is free to use, modify and sell. The only thing we kindly ask is that improvements and extensions are contributed back to the project (at your will). This to make the project better for everyone. All design files is licensed under the [MIT license](https://en.wikipedia.org/wiki/MIT_License).

<img src="docs/images/odessa17.png" width="600">

## Abstract

This document describes the Odessa CAN4VSCP module, its functionality, and usage.
The module is a general-purpose module that connects to a CAN4VSCP bus and can read digital inputs, A/D inputs, and control digital outputs and more. Its functionality is exposed over a 24 pin connector that can be used to connect sensors and actuators.

TRhe module is constructed to be used for custom projects where you need a CAN4VSCP interface to read sensors and control actuators. It can be used in home automation, industrial control, remote monitoring, and more. The module is based on a PIC18F26K83 microcontroller with built-in CAN and A/D converters. It has 8 digital inputs, 4 digital outputs, and 4 A/D inputs. The digital inputs can be used to read switches, buttons, or other digital signals. The digital outputs can be used to control relays, LEDs, or other actuators. The A/D inputs can be used to read analog sensors such as temperature sensors, light sensors, or potentiometers.

Firmware is available that give the module a general I/O functionality. This firmware can be used as a starting point for your own projects.

The module can be attached to a standard DIN Rail or be mounted directly on a wall. 
The module fully adopts to the CAN4VSCP specification and can be powered directly over the bus with a 9-28V DC power source. It has a rich register set for configuration and any information events defined. It also have a decision matrix for easy dynamic event handling.

VSCP CAN modules are designed to work on a VSCP4CAN bus which use ordinary RJ-45 connectors and is powered with 9-30V DC over the same cable. This means there is no need for a separate power cable. All that is needed is a CAT5 or better twisted pair cable. Bus length can be a maximum of 500 meters with drops of maximum 24 meters length (up to a total of 120 meters). As for all VSCP4CAN modules the communication speed is fixed at 125 kbps.

All VSCP modules contains information of there own setup, manual, hardware version, manufacturer etc. You just ask the module for the information you need and you will get it. When they are started up they have a default functionality that often is all that is needed to get a working setup. If the module have something to report it will send you an event and if it is setup to react on a certain type of event it will do it's work when you send event(s) to it.

<hr>

---

## Project files

### User manual
  * [User Manual](https://grodansparadis.github.io/can4vscp-odessa/#)

### Schematic, PCB, 3D files etc
  * [Schematics reversion A](https://github.com/grodansparadis/can4vscp-odessa/blob/master/eagle/odessa_sch_rev_a.png)
  * Hardware design files is made in [KiCad](https://kicad.org) and can be found in the `kicad` directory. Valid from reversion A of the hardware.
  * Gerber files for PCB production can be found in the `gerber` directory (in the `kicad` folder).
  * Eagle schema and board files for reversion A and B can be found in the `eagle` directory. They are no longer actively updated.

 ### Firmware

 The firmware is developed in [MPLAB X IDE](https://www.microchip.com/mplab/mplab-x-ide) using the [XC8 compiler](https://www.microchip.com/mplab/compilers).

  * Binary release files is available [here](https://github.com/grodansparadis/can4vscp-odessa/releases)

### MDF - Module Description File(s)
  * [MDF file version: 1 Release date: 2020-05-15](http://www.eurosource.se/odessa001.xml)

### Support
If you need support, please open an issue in the [GitHub repository](https://github.com/grodansparadis/can4vscp-odessa/issues).

### Buy a ready made modules
You can buy a ready made module from [Grodans Paradis](http://www.grodansparadis.com).

### Project related links
  * [VSCP project](https://www.vscp.org)
  * [VSCP Documentation site](https://docs.vscp.org/)
  * [VSCP Wiki](https://github.com/grodansparadis/vscp/wiki)

