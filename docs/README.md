
## Manual for the Odessa module

**Document version:** ${/var/document-version} - ${/var/creation-time}
[HISTORY](./history.md)


![Odessa](/images/odessa17.png)

Odessa is a board that have a [Zeus expansion interface](https://github.com/grodansparadis/vscp/wiki/Zeus-expansion-interface) and normally is the base for other products with an expansion board permanently mounted for some particular use. However. as the module also can be useful as a starting point for custom projects it is also useful on its own. All you need to do is to add your custom expansion card to the module and write the code to interface it or use it as a general I/O module

The module comes with a GUID and programmed with a bootloader and VSCP module functionality that out of the box (as a sample) work as a digital output module but which easily can be extended to set up with your own functionality.

The module have a [[http://www.microchip.com/wwwproducts/Devices.aspx?product=PIC18F26K80|PIC18F26K80]] installed which have 64 Kbytes of Flash, 1Kbytes of EEPROM and 3,648 bytes of ram. 

The module comes with a GUID and is programmed with a bootloader and
VSCP module functionality that works right out of the box.

* [Repository for the module](https://github.com/grodansparadis/can4vscp-odessa)
  * This manual is available [here](https://grodansparadis.github.io/can4vscp-odessa/)
  * Latest schema for the module is available [here](https://github.com/grodansparadis/can4vscp-Odessa/tree/master/eagle)
  * Latest firmware for the module is available [here](https://github.com/grodansparadis/can4vscp-odessa/tree/master/firmware)
  * [MDF for the module](https://github.com/grodansparadis/can4vscp-Odessa/tree/master/mdf)


## VSCP

![VSCP logo](./images/logo_100.png)

VSCP is a free and open automation protocol for IoT and m2m devices. Visit [the VSCP site](https://www.vscp.org) for more information.

**VSCP is free.** Placed in the **public domain**. Free to use. Free to change. Free to do whatever you want to do with it. VSCP is not owned by anyone. VSCP will stay free and gratis forever.

The specification for the VSCP protocol is [here](https://docs.vscp.org) 

VSCP documentation for various parts can be found [here](https://docs.vscp.org/).

If you use VSCP please consider contributing resources or time to the project ([https://github.com/sponsors/grodansparadis](https://github.com/sponsors/grodansparadis)).

## Buy a module

<img src="./images/grodan_logo.png" alt="Grodans PAradis AB" width="200"/>

Ready made modules can be bought from [Grodans Paradis AB](https://www.grodansparadis.com).

## Document license

This document is licensed under [Creative Commons BY 4.0](https://creativecommons.org/licenses/by/4.0/) and can be freely copied, redistributed, remixed, transformed, built upon as long as you give credits to the author.



[filename](./bottom-copyright.md ':include')
