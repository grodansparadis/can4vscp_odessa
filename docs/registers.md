# Registers

The Odessa module comes with a firmware sample that have 18 output pins that can be controlled with VSCP. The pins can be controlled by writing values to one of three control registers or program the [decision matrix](decisionmatrix) to react on some specified event(s) present in the system.

Note that this is just some demo code for test of the Odessa module.

| Register   | Page   | Description |
| ---------- | ------ | ----------- |
| 0          | 0      | The zone this module belongs to |
| 1          | 0      | The sub zone this module belongs to |
| 2          | 0      | Writing a value to the control register will activate/deactivate the output. <br>**0** - The output is inactivated.<br>**1** - The output is activated.<br>                    Reading a value from the control register is read as a one if the output is activated and as a zero if the output is deactivated.<br>**Bit 0** - Output on pin 3.<br>**Bit 1** - Output on pin 4.<br>**Bit 2** - Output on pin 5.<br>**Bit 3** - Output on pin 6.<br>**Bit 4** - Output on pin 7.<br>**Bit 5** - Output on pin 8.<br>**Bit 6** - Output on pin 9.<br>**Bit 7** - Output on pin 10. |
  | 3     |     0      | Writing a value to the control register will activate/deactivate the output.
                    \
                    **0** - The output is inactivated.\
                    **1** - The output is activated.\
                    \
                    Reading a value from the control register is read as a one if the output is activated and as a zero if the output is deactivated.\
                    \
                    **Bit 0** - Output on pin 11.\
                    **Bit 1** - Output on pin 12.\
                    **Bit 2** - Pin 13 **Unused**\
                    **Bit 3** - Pin 14 **RESET**.\
                    **Bit 4** - Output on pin 15.\
                    **Bit 5** - Output on pin 16.\
                    **Bit 6** - Output on pin 17.\
                    **Bit 7** - Output on pin 18. |
  4          0      Writing a value to the control register will activate/deactivate the output.\
                    \
                    **0** - The output is inactivated.\
                    **1** - The output is activated.\
                    \
                    Reading a value from the control register is read as a one if the output is activated and as a zero if the output is deactivated.\
                    \
                    **Bit 0** - Output on pin 19.\
                    **Bit 1** - Output on pin 20.\
                    **Bit 2** - Reserved.\
                    **Bit 3** - Reserved.\
                    **Bit 4** - Reserved.\
                    **Bit 5** - Reserved.\
                    **Bit 6** - Reserved.\
                    **Bit 7** - Reserved.

  5          0      Sub zone for pin 3.

  6          0      Sub zone for pin 4.

  7          0      Sub zone for pin 5.

  8          0      Sub zone for pin 6.

  9          0      Sub zone for pin 7.

  10         0      Sub zone for pin 8.

  11         0      Sub zone for pin 9.

  12         0      Sub zone for pin 10.

  13         0      Sub zone for pin 11.

  14         0      Sub zone for pin 12.

  15         0      Sub zone for pin 13.

  16         0      Sub zone for pin 14.

  17         0      Sub zone for pin 15.

  18         0      Sub zone for pin 16.

  19         0      Sub zone for pin 17.

  20         0      Sub zone for pin 18.

  21         0      Sub zone for pin 19.

  22         0      Sub zone for pin 20.

  0          1      Decision matrix starts here
  ------------------------------------------------------------------------------------------------------------------------------------------------------

[filename](./bottom-copyright.md ':include')
