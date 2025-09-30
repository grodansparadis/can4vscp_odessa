# Decision Matrix

The full functionality of the decision matrix is explained [in the
specification](http://www.vscp.org/docs/vscpspec/doku.php?id=decision_matrix).

The demo firmware installed on Odessa have a decision matrix consisting
of eight entries. This matrix can be used to control the outputs.
Possible actions are listed in the table below.

  Action       Action code   Parameter      Description
  ------------ ------------- -------------- ----------------------------------------------------------------------------
  **NOOP**     0             Not used       No operation. Will do absolutely nothing.
  **SET**      1             3-20/131-148   Will set on of the pins (valid parameter is 3-20) to it\'s active state.
  **CLR**      2             3-20/131-148   Will set on of the pins (valid parameter is 3-20) to it\'s inactive state.
  **SETALL**   3             Not used       Will set all of the pins to the active state.
  **CLRALL**   4             Not used       Will set all of the pins to the inactive state.

If parameter bit 7 is set the five lowest bits specifies the pin number
and this pin number should be the same as the sub zone set for that pin
to trigger the action.

## Example

You want to activate output on pin 3 when a [CLASS1.CONTROL, TurnOn,
Type=5
event](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.control#type_5_0x05_turnon)
is received.

A matrix row consist of

  Byte   Description
  ------ ------------------
  0      oaddr
  1      flags
  2      class-mask
  3      class-filter
  4      type-mask
  5      type-filter
  6      Action code
  7      Action parameter

So populate the row with the following values

  Byte   Value      Description
  ------ ---------- ------------------
  0      **0x00**   oaddr
  1      **0x80**   flags
  2      **0xff**   class-mask
  3      **0x1e**   class-filter
  4      **0xff**   type-mask
  5      **0x05**   type-filter
  6      **0x01**   Action code
  7      **0x83**   Action parameter

-   **oaddr** set to zero as i is not used.
-    **flags** have one bit set. Enable row. We could have set bit 4
    Match Zone to test zone also trigger DM.
-   **class-mask** is set to 0xff as we have just one event we will
    trigger on.
-   **class-filter** is set to CLASS1.CONTROL.
-   **type-mask** is set to 0xff as we have just one event we will
    trigger on.
-   **type-filter** is set to 5 which is TurnOn.
-   **Action code** i set to the SET action.
-   **Action parameter** have bit 7 set to check corresponding subzone
    and 3 for pin3.

If you double click a decision matrix row a dialog comes up which help
you to set the values.

![](/odessa_dm_row_edit.png){width="400" query="?400"}

Now if [CLASS1.CONTROL, TurnOn, Type=5
event](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.control#type_5_0x05_turnon)
appears on the bus that have data=x,x,3 pin3 will be activated

The row will look like this in the decision matrix

![](/odessa_dm_row.png){width="600" query="?600"}

You can load a register file
[here](http://www.grodansparadis.com/odessa/downloads/odessa_dm_demo1.reg)
that can be loaded in VSCP works with decision matrix entries to turn
on/off pin3 from TurnOn/TurnOff events.

You can also download a transmission set for VSCP Works
[here](http://www.grodansparadis.com/odessa/downloads/odessa_on_off3.txd)
that can be loaded.

This is how it looks in the session window.

![](/odessa_on_off_text.png){width="600" query="?600"}

\
\-\-\-- ![](/grodan_logo.png){.align-center width="100" query="?100"}\
`<center>`{=html} **Paradise of the Frog AB** Brattbergavägen 17 820 50
LOS SWEDEN\
**email:** [info@grodansparadis.com](info@grodansparadis.com) **phone:**
+46 (0)8 40011835\
**web:**<http://www.grodansparadis.com>\
`</center>`{=html}
