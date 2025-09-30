
## Most current information

You can find the most current information about the Odessa module at
<https://github.com/grodansparadis/can4vscp-odessa>. On the site you can
also find links to the latest firmware, drivers and schematics etc for
its use.

## The raw facts

| Parameter | Value |
| ----------- | ----------- |
|  Current hardware reversion    |     A         |
|  Current firmware version      |    1.0.0      |
|  Supply voltage                |    +9-+28 VDC |
|  PCB Size                      |   42mm x 72mm |
|  Power requirements            |  0.1W         |
|  Communication: CAN4VSCP (CAN) |  125 kbps     |

## Schematics

![](images/odessa_sch_rev_a.png)

Schematics: Revision A

## Connectors

![](images/odessa_connectors.png) 

### Zeus expansion interface

| Pin |  Description | Odessa(PIC18F26K80) pin |
|-----|--------------|--------------------------|
| 1   | Ground      | Ground                   |
| 2   | Ground      | Ground                   |
| 3   | TX, Serial output from processor. | RC7/CANRX/RX1/DT1/CCP4 |
| 4   | RX, Serial input to processor. | RC6/CANTX/TX1/CK1/CCP3 |
| 5   | SCK - SPI or I2C clock                | RC3/REFO/SCL/SCK |
| 6   | MISO - SPI data in to processor or I2C Data.     | RC4/SDA/SDI |
| 7   | MOSI - SPI data our from processor.   | RC5/SDO |
| 8   | AN0 - Analog input 0                  | RA0/CVREF/AN0/ULPWU |
| 9   | AN1 - Analog input 1                  | RA1/AN1 |
| 10  | AN2 - Analog input 2                  | RA2/VREF-/AN2 |
| 11  | AN3 - Analog input 3                  | RA3/VREF+/AN3 |
| 12  | AN4 - Analog input 4                  | RA5/AN4/C2INB/HLVDIN/T1CKI/SS/CTMUI |
| 13  | I/O 0 - Input/Output 0                | No connect |
| 14  | RESET                                 | MCLR |
| 15  | I/O 2 - Input/Output 2                | RB4/AN9/C2INA/ECCP1/P1A/CTPLS/KBI0 |
| 16  | I/O 1 - Input/Output 1                | RC2/T1G/CCP2 |
| 17  | INT1 - Interrupt 1 (low flank)        | RB1/AN8/C1INB/P1B/CTDIN/INT1 |
| 18  | INT0 - Interrupt 0 (low flank)        | RB0/AN10/C1INA/FLT0/INT0 |
| 19  | I/O 4 - Input/Output 4                | RB6/PGC/TX2/CK2/KBI2 |
| 20  | I/O 3 - Input/Output 3                | RB5/T0CKI/T3CKI/CCP5/KBI1 |
| 21  | Vin - +9V - +28V                      | Vin |
| 22  | Vin - +9V - +28V                      | Vin |
| 23  | +3.3V                                 | +3.3V (+5V on 5V version) |
| 24  | +3.3V                                 | +3.3V (+5V on 5V version) |

**!** Note that the 5V version is not compatible with the Zeus expansion interface (ZEI) and should not be used with expansion boards built for this interface.

### RJ-XX pin-out

The unit is powered over the CAN4VSCP bus. The CAN4VSCP normally uses CAT5 or better twisted pair cable. You can use other cables if you wish. The important thing is that the CANH and CANL signals uses a twisted cable. For connectors you can use RJ10, RJ11, RJ12 or the most common RJ45 connectors.

Recommended connector is RJ-34/RJ-12 or RJ-11 with pin out as in this table.

  | Pin  |   Use    |     RJ-11   |   RJ-12   |   RJ-45   |   Patch Cable wire color T568B |
  |------|----------|-------------|-----------|-----------|---------------------------------|
  | 1    | +9-28V DC| RJ-11      | RJ-12     | RJ-45     | Orange/White                    |
  | 2    | +9-28V DC| RJ-12      | RJ-45     |           | Orange                           |
  | 3    | +9-28V DC| RJ-11      | RJ-12     | RJ-45     | Green/White                     |
  | 4    | CANH     | RJ-11      | RJ-12     | RJ-45     | Blue                             |
  | 5    | CANL     | RJ-11      | RJ-12     | RJ-45     | Blue/White                       |
  | 6    | GND      | RJ-11      | RJ-12     | RJ-45     | Green                           |
  | 7    | GND      |             | RJ-12     | RJ-45     | Brown/White                     |
  | 8    | GND      |             |           | RJ-45     | Brown                           |

![RJ-11/12/45 pin-out](images/rj45.jpg) 

**RJ-11/12/45 pin-out**

**!** Always use a pair of twisted wires for CANH/CANL for best noise immunity. If the EIA/TIA 56B standard is used this condition will be satisfied. This is good as most Ethernet networks already is wired this way.

### Inter module connector

The inter module connector can be used to connect modules that are physically close to each other together in an easy way. Remember that the minimum length of a connection cable is 30 cm.

| Pin   | Description |
| ----- | ------------------------- |
| 1     | Power from CAN4VSCP bus  |
| 2     | CANH                     |
| 3     | CANL                     |
| 4     | GND                      |



![](images/odessa_inter_module_connector.png)

### PIC programming Connector

| Pin   | Description |
| ----- | ----------- |
| 1     | Reset       |
| 2     | VCC         |
| 3     | GND        |
| 4     | PGD (RX of second serial port is here to) |
| 5     | PGC (TX of second serial port is here to) |
| 6     | LWPGM      |

![](images/odessa_programming_connector.png)

### Functionality of the status LED

The LED is used to indicate the status of the module. It will light steady when the firmware is running and will blink when the module is in the nickname discovery process.

| LED        | Description                                                                                                           |
| ---------- | --------------------------------------------------------------------------------------------------------------------- |
| Steady     | No error. Firmware running.                                                                                           |
| Blinking   | Module is going through the [nickname discovery process](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_level_i_specifics?id=address-or-nickname-assignment-for-level-i-nodes). |

### CAN

CAN4VSCP is a CAN based bus running at 125 kbps with the addition of DC
power. If you are interested in how CAN works you have a pretty good
intro [here](http://www.eeherald.com/section/design-guide/esmod9.html).

CAN is known to be robust and is there for used in vehicles and in the
industry.

[filename](./bottom-copyright.md ':include')


