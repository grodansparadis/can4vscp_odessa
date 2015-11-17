/* ******************************************************************************
 * 	VSCP (Very Simple Control Protocol) 
 * 	http://www.vscp.org
 *
 *  Paris Smart Relay Module
 * 	akhe@grodansparadis.com
 *
 *  Copyright (C) 1995-2015 Ake Hedman, Grodans Paradis AB
 *                          <akhe@grodansparadis.com>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 
 *	This file is part of VSCP - Very Simple Control Protocol 	
 *	http://www.vscp.org
 *
 * ******************************************************************************
 */

#include "vscp_compiler.h"
#include "vscp_projdefs.h"

#include <p18cxxx.h>
#include <timers.h>
#include <delays.h>
#include <inttypes.h>
#include <ecan.h>
#include <vscp_firmware.h>
#include <vscp_class.h>
#include <vscp_type.h>
#include "odessa.h"
#include "version.h"


// http://gputils.sourceforge.net/html-help/PIC18F26K80-conf.html

// CONFIG1L
#pragma config SOSCSEL = DIG    // RC0/RC is I/O
#pragma config RETEN = OFF      // Ultra low-power regulator is Disabled (Controlled by REGSLP bit).
#pragma config INTOSCSEL = HIGH // LF-INTOSC in High-power mode during Sleep.
#pragma config XINST = OFF      // No extended instruction set

// CONFIG1H
#pragma config FOSC = HS2       // Crystal 10 MHz
#pragma config PLLCFG = ON      // 4 x PLL

// CONFIG2H
#pragma config WDTPS = 1048576  // Watchdog prescaler
#pragma config BOREN = SBORDIS  // Brown out enabled
#pragma config BORV  = 1        // 2.7V

// CONFIG3H
#pragma config CANMX = PORTB    // ECAN TX and RX pins are located on RB2 and RB3, respectively.
#pragma config MSSPMSK = MSK7   // 7 Bit address masking mode.
#pragma config MCLRE = ON       // MCLR Enabled, RE3 Disabled.

// CONFIG4L
#pragma config STVREN = ON      // Stack Overflow Reset enabled
#pragma config BBSIZ = BB2K     // Boot block size 2K

#ifdef DEBUG
#pragma config WDTEN = OFF      // WDT disabled in hardware; SWDTEN bit disabled.
#else
#pragma config WDTEN = ON       // WDT enabled in hardware; 
#endif


// Prototypes
void actionSet( uint8_t dmflags, uint8_t param );
void actionClr( uint8_t dmflags, uint8_t param );
void actionSetAll( uint8_t dmflags, uint8_t param );
void actionClrAll( uint8_t dmflags, uint8_t param );
uint8_t writeControlReg( uint8_t ctrlreg, uint8_t val );
uint8_t readControlReg( uint8_t ctrlreg );

// Calculate and st required filter and mask
// for the current decision matrix
void calculateSetFilterMask( void );

// The device URL (max 32 characters including null termination)
const uint8_t vscp_deviceURL[] = "www.eurosource.se/odessa001.xml";

volatile unsigned long measurement_clock; // Clock for measurments

uint8_t sendTimer;  // Timer for CAN send
uint8_t seconds;    // counter for seconds
uint8_t minutes;    // counter for minutes
uint8_t hours;      // Counter for hours


///////////////////////////////////////////////////////////////////////////////
// Isr() 	- Interrupt Service Routine
//      	- Services Timer0 Overflow
//      	- Services GP3 Pin Change
//////////////////////////////////////////////////////////////////////////////

void interrupt low_priority  interrupt_at_low_vector( void )
{
    // Clock
    if ( INTCONbits.TMR0IF ) { // If a Timer0 Interrupt, Then...

        // Reload value for 1 ms reolution
        WriteTimer0(TIMER0_RELOAD_VALUE);
        
        vscp_timer++;
        vscp_configtimer++;
        measurement_clock++;
        sendTimer++;

        // Check for init button
        if ( INIT_BUTTON ) {
            vscp_initbtncnt = 0;
        } else {
            // Active
            vscp_initbtncnt++;
        }

        // Status LED
        vscp_statuscnt++;
        if ( ( VSCP_LED_BLINK1 == vscp_initledfunc ) &&
                ( vscp_statuscnt > 100 ) ) {

            if ( STATUS_LED ) {
                STATUS_LED = 0;
            }
            else {
                STATUS_LED = 1;
            }

            vscp_statuscnt = 0;

        }
        else if (VSCP_LED_ON == vscp_initledfunc) {
            STATUS_LED = 1;
            vscp_statuscnt = 0;
        }
        else if (VSCP_LED_OFF == vscp_initledfunc) {
            STATUS_LED = 0;
            vscp_statuscnt = 0;
        }

        INTCONbits.TMR0IF = 0; // Clear Timer0 Interrupt Flag

    }

    return;
}


//***************************************************************************
// Main() - Main Routine
//***************************************************************************

void main()
{
    init(); // Initialize Micro controller

    // Check VSCP persistent storage and
    // restore if needed
    if ( !vscp_check_pstorage() ) {

        // Spoiled or not initialized - reinitialize
        init_app_eeprom();
        init_app_ram();     // Needed because some ram positions
                            // are initialized from EEPROM

    }

    vscp_init();    // Initialize the VSCP functionality

    // Restore outputs
    writeControlReg( CONTROL0, eeprom_read( VSCP_EEPROM_END + REG_CONTROL0 ) );
    writeControlReg( CONTROL1, eeprom_read( VSCP_EEPROM_END + REG_CONTROL1 ) );
    writeControlReg( CONTROL2, eeprom_read( VSCP_EEPROM_END + REG_CONTROL2 ) );
    
    while ( 1 ) {   // Loop Forever

        ClrWdt();   // Feed the dog

        if ( ( vscp_initbtncnt > 250 ) &&
                ( VSCP_STATE_INIT != vscp_node_state ) ) {

            // Init. button pressed
            vscp_nickname = VSCP_ADDRESS_FREE;
            eeprom_write( VSCP_EEPROM_NICKNAME, VSCP_ADDRESS_FREE );
            vscp_init();
            
        }

        // Check for a valid  event
        vscp_imsg.flags = 0;
        vscp_getEvent();

        switch ( vscp_node_state ) {

            case VSCP_STATE_STARTUP: // Cold/warm reset

                // Get nickname from EEPROM
                if (VSCP_ADDRESS_FREE == vscp_nickname) {
                    // new on segment need a nickname
                    vscp_node_state = VSCP_STATE_INIT;
                } else {
                    // been here before - go on
                    vscp_node_state = VSCP_STATE_ACTIVE;
                    vscp_goActiveState();
                }
                break;

            case VSCP_STATE_INIT: // Assigning nickname
                vscp_handleProbeState();
                break;

            case VSCP_STATE_PREACTIVE:  // Waiting for host initialisation
                vscp_goActiveState();
                break;

            case VSCP_STATE_ACTIVE:     // The normal state

                // Check for incoming event?
                if (vscp_imsg.flags & VSCP_VALID_MSG) {

                    if ( VSCP_CLASS1_PROTOCOL == vscp_imsg.vscp_class  ) {

                        // Handle protocol event
                        vscp_handleProtocolEvent();

                    }

                    doDM();
					
                }
                break;

            case VSCP_STATE_ERROR: // Everything is *very* *very* bad.
                vscp_error();
                break;

            default: // Should not be here...
                vscp_node_state = VSCP_STATE_STARTUP;
                break;

        }

        // do a measurement if needed
        if ( measurement_clock > 1000 ) {

            measurement_clock = 0;
 
            // Do VSCP one second jobs
            vscp_doOneSecondWork();

            // Temperature report timers are only updated if in active
            // state GUID_reset
            if ( VSCP_STATE_ACTIVE == vscp_node_state ) {

                // Do VSCP one second jobs
                doApplicationOneSecondWork();

            }

        }

        // Timekeeping
        if ( seconds > 59 ) {

            seconds = 0;
            minutes++;

            if ( minutes > 59 ) {
                minutes = 0;
                hours++;
            }

            if ( hours > 23 ) hours = 0;

        }

        doWork();

    } // while
}

///////////////////////////////////////////////////////////////////////////////
// Init - Initialization Routine
//  

void init()
{
    //uint8_t msgdata[ 8 ];

    // Initialize data
    init_app_ram();

    // Initialize the uP
    
    // All AD channels to I/O
    ANCON0 = 0;
    ANCON1 = 0;

    // PORTA
    // RA0/AN0  - Output
    // RA1/AN1  - Output
    // RA2/AN2  - Output
    // RA3/AN3  - Output
    // RA4      - Unused/VCAP
    // RA5/AN4  - Output
    TRISA = 0x00;
    PORTA = 0x00;

    // PortB

    // RB0/AN10     - Output - INT0
    // RB1/AN8      - Output - INT1
    // RB2 CAN TX   - Must be set to input
    // RB3 CAN RX   - Must be set to input
    // RB4/AN9      - Output I/O      
    // RB5/LVPGM    - Output I/O
    // RB6/PGC      - Output I/O
    // RB7/PGD      - Output I/O
    TRISB = 0b00001100;
    PORTB = 0x00;

    // RC0 - Input  - Init. button
    // RC1 - Output - Status LED - Default off
    // RC2 - Output
    // RC3 - Output - SCK1/SCL
    // RC4 - Output - SDI1/SDA
    // RC5 - Output - SDO1/SDO
    // RC6 - Output - TX1
    // RC7 - Output - RX1
    TRISC = 0b00000001;
    PORTC = 0x00;

/*
    // Sensor 0 timer
    OpenTimer0( TIMER_INT_OFF &
                    T0_16BIT &
                    T0_PS_1_2 &
                    T0_SOURCE_INT);

    // Sensor 1 timer
    OpenTimer1( TIMER_INT_OFF &
                    T1_16BIT_RW &
                    T1_SOURCE_INT &
                    T1_PS_1_1 &
                    T1_OSC1EN_OFF &
                    T1_SYNC_EXT_OFF);

    // Timer 2 is used as a 1 ms clock
    // 156 is loaded eight time to give ~1250 cycles
    PR2 = TIMER2_RELOAD_VALUE; // Set reload value
    OpenTimer2( TIMER_INT_ON &
                    T2_PS_1_4 &
                    T2_POST_1_16 );

 */

    OpenTimer0( TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_8 );
    WriteTimer0( TIMER0_RELOAD_VALUE );

    // Initialize CAN
    ECANInitialize();

    // Must be in Config mode to change many of settings.
    //ECANSetOperationMode(ECAN_OP_MODE_CONFIG);

    // Return to Normal mode to communicate.
    //ECANSetOperationMode(ECAN_OP_MODE_NORMAL);

    /*
        msgdata[ 0 ] = 1;
            msgdata[ 1 ] = 2;
            msgdata[ 2 ] = 3;
            msgdata[ 3 ] = 4;

            if ( vscp18f_sendMsg( 0x123,  msgdata, 4, CAN_TX_PRIORITY_0 & CAN_TX_XTD_FRAME & CAN_TX_NO_RTR_FRAME ) ) {
                    ;
            }

     */

    // Enable peripheral interrupt
    INTCONbits.PEIE = 1;

    // Enable global interrupt
    INTCONbits.GIE = 1;

    return;
}

///////////////////////////////////////////////////////////////////////////////
// init_app_ram
//

void init_app_ram( void )
{
    uint8_t i;

    measurement_clock = 0;      // start a new measurement cycle

    seconds = 0;
    minutes = 0;
    hours = 0;
    
}


///////////////////////////////////////////////////////////////////////////////
// init_app_eeprom
//

void init_app_eeprom(void)
{
    unsigned char i, j;

    eeprom_write( VSCP_EEPROM_END + REG_ZONE, 0 );
    eeprom_write( VSCP_EEPROM_END + REG_SUBZONE, 0 );
    
    for ( i=3; i<21; i++ ) {
        eeprom_write( VSCP_EEPROM_END + REG_PIN3_SUBZONE + (i-3), i );
    }
    
    eeprom_write( VSCP_EEPROM_END + REG_CONTROL0, 0 );
    eeprom_write( VSCP_EEPROM_END + REG_CONTROL1, 0 );
    eeprom_write( VSCP_EEPROM_END + REG_CONTROL2, 0 );
    
    // * * * Decision Matrix * * *
    // All elements disabled.
    for ( i = 0; i < DESCION_MATRIX_ROWS; i++ ) {
        for ( j = 0; j < 8; j++ ) {
            eeprom_write( VSCP_EEPROM_END + REG_FIRST_PAGE_END + REG_DESCION_MATRIX + i * 8 + j, 0 );
        }
    }

}

///////////////////////////////////////////////////////////////////////////////
// doApplicationOneSecondWork
//

void doApplicationOneSecondWork(void)
{
    // Do work that should be done once a second here
}



///////////////////////////////////////////////////////////////////////////////
// Get Major version number for this hardware module
//

unsigned char getMajorVersion()
{
    return FIRMWARE_MAJOR_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// Get Minor version number for this hardware module
//

unsigned char getMinorVersion()
{
    return FIRMWARE_MINOR_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// Get Subminor version number for this hardware module
//

unsigned char getSubMinorVersion()
{
    return FIRMWARE_SUB_MINOR_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// Get GUID from EEPROM
//

#ifdef ENABLE_WRITE_2PROTECTED_LOCATIONS

void vscp_setGUID(uint8_t idx, uint8_t data ) {
    if ( idx>15 ) return;
    eeprom_write(VSCP_EEPROM_REG_GUID + idx, data);
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Get Manufacturer id and subid from EEPROM
//

#ifdef ENABLE_WRITE_2PROTECTED_LOCATIONS

void vscp_setManufacturerId( uint8_t idx, uint8_t data ) {
    if ( idx>7 ) return;
    eeprom_write(VSCP_EEPROM_REG_MANUFACTUR_ID0 + idx, data);
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Get the bootloader algorithm code
//

unsigned char getBootLoaderAlgorithm(void)
{
    return VSCP_BOOTLOADER_PIC1;
}

///////////////////////////////////////////////////////////////////////////////
// Get the buffer size
//

unsigned char getBufferSize(void)
{
    return 8; // Standard CAN frame
}

///////////////////////////////////////////////////////////////////////////////
//  vscp_readNicknamePermanent
//

uint8_t vscp_readNicknamePermanent(void)
{
    return eeprom_read( VSCP_EEPROM_NICKNAME );
}

///////////////////////////////////////////////////////////////////////////////
//  vscp_writeNicknamePermanent
//

void vscp_writeNicknamePermanent(uint8_t nickname)
{
    eeprom_write( VSCP_EEPROM_NICKNAME, nickname );
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getZone
//

uint8_t vscp_getZone(void)
{
    return eeprom_read( VSCP_EEPROM_END + REG_ZONE );
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getSubzone
//

uint8_t vscp_getSubzone(void)
{
    return eeprom_read( VSCP_EEPROM_END + REG_SUBZONE );
}

///////////////////////////////////////////////////////////////////////////////
// doWork
//
// The actual work is done here.
//

void doWork(void)
{
    if ( VSCP_STATE_ACTIVE == vscp_node_state ) {
        // Do work when active here
		;
    }
}

///////////////////////////////////////////////////////////////////////////////
// vscp_readAppReg
//

uint8_t vscp_readAppReg(uint8_t reg)
{    
    uint8_t rv;

    rv = 0x00; // default read

    // * * *  Page = 0
    if ( 0 == vscp_page_select ) {
        // Zone
        if ( reg == 0x00 ) {
            rv = eeprom_read(VSCP_EEPROM_END + REG_ZONE);
        }
        // SubZone
        else if ( reg == 0x01 ) {
            rv = eeprom_read(VSCP_EEPROM_END + REG_SUBZONE);
        }
        // SubZone for pins
        else if ( ( reg >= REG_PIN3_SUBZONE ) && ( reg <= REG_PIN20_SUBZONE ) ) {
            rv = eeprom_read( VSCP_EEPROM_END + REG_PIN3_SUBZONE + 
                                ( reg - REG_PIN3_SUBZONE ) );
        }
        // Control reg 0
        else if ( reg == REG_CONTROL0 ) {
            rv = readControlReg( 0 );
        }
        // Control reg 1
        else if ( reg == REG_CONTROL1 ) {
            rv = readControlReg( 1 );
        }
        // Control reg 2
        else if ( reg == REG_CONTROL2 ) {
            rv = readControlReg( 2 );
            rv &= 0x03; // Take away unused bits
        }
    }
    // * * *  Page = 1
    else if ( 1 == vscp_page_select ) {
        
        // DM REG_FIRST_PAGE_END + REG_DESCION_MATRIX + i * 8 + j
        if ( ( reg >= REG_DESCION_MATRIX ) && ( reg <= ( REG_DESCION_MATRIX + 
                ( 8 * DESCION_MATRIX_ROWS ) ) ) ) {
            rv = eeprom_read(VSCP_EEPROM_END + REG_FIRST_PAGE_END + 
                    ( reg - REG_DESCION_MATRIX ) );
        }
        
    }

    return rv;

}

///////////////////////////////////////////////////////////////////////////////
// vscp_writeAppReg
//

uint8_t vscp_writeAppReg( uint8_t reg, uint8_t val )
{
    uint8_t rv;

    rv = ~val; // error return

    // * * *  Page = 0
    if ( 0 == vscp_page_select ) {
        
        // Zone
        if ( reg == REG_ZONE ) {
            eeprom_write(VSCP_EEPROM_END + REG_ZONE, val);
            rv = eeprom_read(VSCP_EEPROM_END + REG_ZONE);
        }
        else if ( reg == REG_SUBZONE ) {
            // SubZone
            eeprom_write(VSCP_EEPROM_END + REG_SUBZONE, val);
            rv = eeprom_read(VSCP_EEPROM_END + REG_SUBZONE);
        }
        // SubZone for pins
        else if ( ( reg >= REG_PIN3_SUBZONE ) && ( reg <= REG_PIN20_SUBZONE ) ) {
            eeprom_write(VSCP_EEPROM_END + REG_PIN3_SUBZONE + 
                                ( reg - REG_PIN3_SUBZONE ), val);
            rv = eeprom_read( VSCP_EEPROM_END + REG_PIN3_SUBZONE + 
                                ( reg - REG_PIN3_SUBZONE ) );
        }
        // Control reg 0
        else if ( reg == REG_CONTROL0 ) {
            eeprom_write(VSCP_EEPROM_END + REG_CONTROL0, val);
            rv = writeControlReg( CONTROL0, val );
        }
        // Control reg 1
        else if ( reg == REG_CONTROL1 ) {
            eeprom_write(VSCP_EEPROM_END + REG_CONTROL1, val);
            rv = writeControlReg( CONTROL1, val );
        }
        // Control reg 2
        else if ( reg == REG_CONTROL2 ) {
            eeprom_write(VSCP_EEPROM_END + REG_CONTROL2, val);
            rv = writeControlReg( CONTROL2, val );
            rv &= 0x03; // Take away unused bits
        }
    
    }
	// * * *  Page = 1
    else if ( 1 == vscp_page_select ) {
        
        // DM
        if ( ( reg >= REG_DESCION_MATRIX ) && ( reg <= ( REG_DESCION_MATRIX + 
                ( 8 * DESCION_MATRIX_ROWS ) ) ) ) {
            eeprom_write(VSCP_EEPROM_END + REG_FIRST_PAGE_END + 
                        ( reg - REG_DESCION_MATRIX ), val);
            rv = eeprom_read(VSCP_EEPROM_END + REG_FIRST_PAGE_END + 
                        ( reg - REG_DESCION_MATRIX ) );
        }
        
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// writeControlReg
//

uint8_t writeControlReg( uint8_t ctrlreg, uint8_t val )
{
    uint8_t rv = 0;
    
    switch ( ctrlreg ) {
    
        case CONTROL0:
            PORTCbits.RC7 = ( val & 0x01 ) ? 1 : 0;
            PORTCbits.RC6 = ( val & 0x02 ) ? 1 : 0;
            PORTCbits.RC3 = ( val & 0x04 ) ? 1 : 0;
            PORTCbits.RC4 = ( val & 0x08 ) ? 1 : 0;
            PORTCbits.RC5 = ( val & 0x10 ) ? 1 : 0;
            PORTAbits.RA0 = ( val & 0x20 ) ? 1 : 0;
            PORTAbits.RA1 = ( val & 0x40 ) ? 1 : 0;
            PORTAbits.RA2 = ( val & 0x80 ) ? 1 : 0;
            rv = readControlReg( CONTROL0 );
            break;
            
        case CONTROL1:
            PORTAbits.RA3 = ( val & 0x01 ) ? 1 : 0;
            PORTAbits.RA5 = ( val & 0x02 ) ? 1 : 0;
            // Pin 13 no connect
            // Pin 14 RESET
            PORTBbits.RB4 = ( val & 0x10 ) ? 1 : 0;
            PORTCbits.RC2 = ( val & 0x20 ) ? 1 : 0;
            PORTBbits.RB1 = ( val & 0x40 ) ? 1 : 0;
            PORTBbits.RB0 = ( val & 0x80 ) ? 1 : 0;
            rv = readControlReg( CONTROL1 );
            break;
            
        case CONTROL2:
            PORTBbits.RB6 = ( val & 0x01 ) ? 1 : 0;
            PORTBbits.RB5 = ( val & 0x02 ) ? 1 : 0;
            rv = readControlReg( CONTROL2 );
            break;    
    }
  
    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// readControlReg
//

uint8_t readControlReg( uint8_t ctrlreg )
{
    uint8_t rv = 0;
    
    switch ( ctrlreg ) {
    
        case CONTROL0:
            rv = ( PORTCbits.RC7 << 0 ) +
                    ( PORTCbits.RC6 << 1 ) + 
                    ( PORTCbits.RC3 << 2 ) +
                    ( PORTCbits.RC4 << 3 ) +
                    ( PORTCbits.RC5 << 4 ) +
                    ( PORTAbits.RA0 << 5 ) +
                    ( PORTAbits.RA1 << 6 ) +
                    ( PORTAbits.RA2 << 7 );
            break;
            
        case CONTROL1:
            rv = ( PORTAbits.RA3 << 0 ) +
                    ( PORTAbits.RA5  << 1 ) +
                    // Pin 13 no connect
                    // Pin 14 RESET
                    ( PORTBbits.RB4 << 4 ) +
                    ( PORTCbits.RC2 << 5 ) +
                    ( PORTBbits.RB1 << 6 ) +
                    ( PORTBbits.RB0 << 7 );
            break;
            
        case CONTROL2:
            rv = ( PORTBbits.RB6 << 0 ) +
                    ( PORTBbits.RB5 << 1 );
            break;    
    }
    
    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// Send Decision Matrix Information
//

void sendDMatrixInfo(void)
{
    vscp_omsg.priority = VSCP_PRIORITY_MEDIUM;
    vscp_omsg.flags = VSCP_VALID_MSG + 2;
    vscp_omsg.vscp_class = VSCP_CLASS1_PROTOCOL;
    vscp_omsg.vscp_type = VSCP_TYPE_PROTOCOL_GET_MATRIX_INFO_RESPONSE;

    vscp_omsg.data[ 0 ] = DESCION_MATRIX_ROWS;
    vscp_omsg.data[ 1 ] = REG_DESCION_MATRIX;

    vscp_sendEvent(); // Send data
}


///////////////////////////////////////////////////////////////////////////////
// SendInformationEvent
//

void SendInformationEvent( unsigned char idx,
                            unsigned char eventClass,
                            unsigned char eventTypeId )
{
    uint8_t data[3];
    idx -= 3;
    
    data[ 0 ] = idx; // Register
    data[ 1 ] = eeprom_read( VSCP_EEPROM_END + REG_ZONE );
    data[ 2 ] = eeprom_read( VSCP_EEPROM_END + REG_PIN3_SUBZONE + idx );
    sendVSCPFrame( eventClass,
                    eventTypeId,
                    vscp_nickname,
                    VSCP_PRIORITY_MEDIUM,
                    3,
                    data );
}

///////////////////////////////////////////////////////////////////////////////
// Do decision Matrix handling
// 
// The routine expects vscp_imsg to contain a valid incoming event
//

void doDM(void)
{
    unsigned char i;
    unsigned char dmflags;
    unsigned short class_filter;
    unsigned short class_mask;
    unsigned char type_filter;
    unsigned char type_mask;

    // Don't deal with the protocol functionality
    if ( VSCP_CLASS1_PROTOCOL == vscp_imsg.vscp_class ) return;

    for (i = 0; i < DESCION_MATRIX_ROWS; i++) {

        // Get DM flags for this row
        dmflags = eeprom_read( VSCP_EEPROM_END + REG_FIRST_PAGE_END + 
                                    REG_DESCION_MATRIX + 1 + (8 * i) );

        // Is the DM row enabled?
        if ( dmflags & VSCP_DM_FLAG_ENABLED ) {

            // Should the originating id be checked and if so is it the same?
            if ( ( dmflags & VSCP_DM_FLAG_CHECK_OADDR ) &&
                    ( vscp_imsg.oaddr != eeprom_read( VSCP_EEPROM_END + REG_FIRST_PAGE_END + 
                                                        REG_DESCION_MATRIX + (8 * i) ) ) ) {
                continue;
            }

            // Check if zone should match and if so if it match
            if ( dmflags & VSCP_DM_FLAG_CHECK_ZONE ) {
                if ( 255 != vscp_imsg.data[ 1 ] ) {
                    if ( vscp_imsg.data[ 1 ] != eeprom_read( VSCP_EEPROM_END + REG_ZONE ) ) {
                        continue;
                    }
                }
            }

            // Check if sub zone should match and if so if it match
            if ( dmflags & VSCP_DM_FLAG_CHECK_SUBZONE ) {
                if ( 255 != vscp_imsg.data[ 2 ] ) {
                    if ( vscp_imsg.data[ 2 ] != eeprom_read( VSCP_EEPROM_END + REG_ZONE ) ) {
                        continue;
                    }
                }
            }
            
            class_filter = ( dmflags & VSCP_DM_FLAG_CLASS_FILTER)*256 +
                    eeprom_read( VSCP_EEPROM_END + REG_FIRST_PAGE_END + 
                                REG_DESCION_MATRIX +
                                (8 * i) +
                                VSCP_DM_POS_CLASSFILTER);
            class_mask = ( dmflags & VSCP_DM_FLAG_CLASS_MASK)*256 +
                    eeprom_read( VSCP_EEPROM_END + REG_FIRST_PAGE_END + 
                                    REG_DESCION_MATRIX +
                                    (8 * i) +
                                    VSCP_DM_POS_CLASSMASK);
            type_filter = eeprom_read( VSCP_EEPROM_END + REG_FIRST_PAGE_END + 
                                        REG_DESCION_MATRIX + 
                                        (8 * i) +
                                        VSCP_DM_POS_TYPEFILTER);
            type_mask = eeprom_read( VSCP_EEPROM_END + REG_FIRST_PAGE_END + 
                                        REG_DESCION_MATRIX +
                                        (8 * i) +
                                        VSCP_DM_POS_TYPEMASK);

            if ( !( ( class_filter ^ vscp_imsg.vscp_class ) & class_mask ) &&
                    !( ( type_filter ^ vscp_imsg.vscp_type ) & type_mask ) ) {

                // OK Trigger this action
                switch ( eeprom_read( VSCP_EEPROM_END + REG_FIRST_PAGE_END + 
                                        REG_DESCION_MATRIX + (8 * i) + 
                                        VSCP_DM_POS_ACTION ) ) {

                    case ACTION_NOOP: // Do nothing
                        break;
                        
                    case ACTION_SET: // Set pin to active state
                        actionSet( dmflags, 
                                eeprom_read( VSCP_EEPROM_END + REG_FIRST_PAGE_END + 
                                                REG_DESCION_MATRIX + (8 * i) + 
                                                VSCP_DM_POS_ACTIONPARAM ) );
                        break;

                    case ACTION_CLR: // Set pin to inactive state
                        actionClr( dmflags, 
                                eeprom_read( VSCP_EEPROM_END + REG_FIRST_PAGE_END + 
                                                REG_DESCION_MATRIX + (8 * i) + 
                                                VSCP_DM_POS_ACTIONPARAM ) );
                        break;

                    case ACTION_SETALL: // Activate all pins
                        actionSetAll( dmflags, 
                                eeprom_read( VSCP_EEPROM_END + REG_FIRST_PAGE_END + 
                                                REG_DESCION_MATRIX + (8 * i) + 
                                                VSCP_DM_POS_ACTIONPARAM ) );
                        break;

                    case ACTION_CLRALL: // Inactivate all pins
                        actionClrAll( dmflags, 
                                eeprom_read( VSCP_EEPROM_END + REG_FIRST_PAGE_END + 
                                                REG_DESCION_MATRIX + (8 * i) + 
                                                VSCP_DM_POS_ACTIONPARAM ) );
                        break;

                } // case
 
            } // Filter/mask
        } // Row enabled
    } // for each row
}


///////////////////////////////////////////////////////////////////////////////
// actionSet
// 
// Do action SET
//

void actionSet( uint8_t dmflags, uint8_t param )
{    
    // We should check sub zone
    if ( param & 0x80 ) {
        
        param &= 0x7f;
        
        if ( eeprom_read( VSCP_EEPROM_END + REG_PIN3_SUBZONE + (param - 3) ) 
                != vscp_imsg.data[ 2 ] )  {
                return;
        }
    }
    
    if ( param < 3) return;
    if ( param > 20 ) return;
    
    SendInformationEvent( param, 
                            VSCP_CLASS1_INFORMATION, 
                            VSCP_TYPE_INFORMATION_ON );
    
    switch ( param ) {
        
        case 3:
            PORTCbits.RC7 = 1;
            break;
       
        case 4:
            PORTCbits.RC6 = 1;
            break;
            
        case 5:
            PORTCbits.RC3 = 1;
            break;
            
        case 6:
            PORTCbits.RC4 = 1;
            break;
            
        case 7:
            PORTCbits.RC5 = 1;
            break;
            
        case 8:
            PORTAbits.RA0 = 1;
            break;
            
        case 9:
            PORTAbits.RA1 = 1;
            break;
            
        case 10:
            PORTAbits.RA2 = 1;
            break;
            
        case 11:
            PORTAbits.RA3 = 1;
            break;
            
        case 12:
            PORTAbits.RA5 = 1;
            break;
            
        case 13:
            // Pin 13 no connect
            break;
            
        case 14:
            // Pin 14 RESET
            break;
            
        case 15:
            PORTBbits.RB4 = 1;
            break;
            
        case 16:
            PORTCbits.RC2 = 1;
            break;
            
        case 17:
            PORTBbits.RB1 = 1;
            break;
        
        case 18:
            PORTBbits.RB0 = 1;
            break;
            
        case 19:
            PORTBbits.RB6 = 1;
            break;
            
        case 20:
            PORTBbits.RB5 = 1;
            break;            
                        
    }
}

///////////////////////////////////////////////////////////////////////////////
// actionClr
// 
// Do action CLR
//

void actionClr( uint8_t dmflags, uint8_t param )
{
    // We should check sub zone
    if ( param & 0x80 ) {
        
        param &= 0x7f;
        
        if ( eeprom_read( VSCP_EEPROM_END + REG_PIN3_SUBZONE + (param - 3) ) 
                != vscp_imsg.data[ 2 ] )  {
                return;
        }
    }
    
    if ( param < 3) return;
    if ( param > 20 ) return;
    
    SendInformationEvent( param, 
                            VSCP_CLASS1_INFORMATION, 
                            VSCP_TYPE_INFORMATION_OFF );
    
    switch ( param ) {
        
        case 3:
            PORTCbits.RC7 = 0;
            break;
       
        case 4:
            PORTCbits.RC6 = 0;
            break;
            
        case 5:
            PORTCbits.RC3 = 0;
            break;
            
        case 6:
            PORTCbits.RC4 = 0;
            break;
            
        case 7:
            PORTCbits.RC5 = 0;
            break;
            
        case 8:
            PORTAbits.RA0 = 0;
            break;
            
        case 9:
            PORTAbits.RA1 = 0;
            break;
            
        case 10:
            PORTAbits.RA2 = 0;
            break;
            
        case 11:
            PORTAbits.RA3 = 0;
            break;
            
        case 12:
            PORTAbits.RA5 = 0;
            break;
            
        case 13:
            // Pin 13 no connect
            break;
            
        case 14:
            // Pin 14 RESET
            break;
            
        case 15:
            PORTBbits.RB4 = 0;
            break;
            
        case 16:
            PORTCbits.RC2 = 0;
            break;
            
        case 17:
            PORTBbits.RB1 = 0;
            break;
        
        case 18:
            PORTBbits.RB0 = 0;
            break;
            
        case 19:
            PORTBbits.RB6 = 0;
            break;
            
        case 20:
            PORTBbits.RB5 = 0;
            break;            
                        
    }
}


///////////////////////////////////////////////////////////////////////////////
// actionSetAll
// 
// Do action SETALL
//

void actionSetAll( uint8_t dmflags, uint8_t param )
{
    PORTA = 0xff;
    PORTB = 0xff;
    PORTC = 0xff;
    
    for ( int i=3; i<21; i++ ) {   
        SendInformationEvent( i, 
                                VSCP_CLASS1_INFORMATION, 
                                VSCP_TYPE_INFORMATION_ON );
    }
}

///////////////////////////////////////////////////////////////////////////////
// actionClrAll
// 
// Do action CLRALL
//

void actionClrAll( uint8_t dmflags, uint8_t param )
{
    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    
    for ( int i=3; i<21; i++ ) {   
        SendInformationEvent( i, 
                                VSCP_CLASS1_INFORMATION, 
                                VSCP_TYPE_INFORMATION_OFF );
    }
}


///////////////////////////////////////////////////////////////////////////////
//                        VSCP Required Methods
//////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// Get Major version number for this hardware module
//

unsigned char vscp_getMajorVersion()
{
    return FIRMWARE_MAJOR_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// Get Minor version number for this hardware module
//

unsigned char vscp_getMinorVersion()
{
    return FIRMWARE_MINOR_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// Get Subminor version number for this hardware module
//

unsigned char vscp_getSubMinorVersion()
{
    return FIRMWARE_SUB_MINOR_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// getVSCP_GUID
//
// Get GUID from EEPROM
//

uint8_t vscp_getGUID(uint8_t idx)
{
    return eeprom_read( VSCP_EEPROM_REG_GUID + idx );
}


///////////////////////////////////////////////////////////////////////////////
// getDeviceURL
//
// Get device URL from EEPROM
//

uint8_t vscp_getMDF_URL(uint8_t idx)
{
    return vscp_deviceURL[ idx ];
}

///////////////////////////////////////////////////////////////////////////////
// Get Manufacturer id and subid from EEPROM
//

uint8_t vscp_getUserID(uint8_t idx)
{
    return eeprom_read( VSCP_EEPROM_REG_USERID + idx );
}

///////////////////////////////////////////////////////////////////////////////
//  setVSCPUserID
//

void vscp_setUserID(uint8_t idx, uint8_t data)
{
    eeprom_write( idx + VSCP_EEPROM_REG_USERID, data );
}

///////////////////////////////////////////////////////////////////////////////
// getVSCPManufacturerId
// 
// Get Manufacturer id and subid from EEPROM
//

uint8_t vscp_getManufacturerId(uint8_t idx)
{
    return eeprom_read( VSCP_EEPROM_REG_MANUFACTUR_ID0 + idx );
}

///////////////////////////////////////////////////////////////////////////////
// Get the bootloader algorithm code
//

uint8_t vscp_getBootLoaderAlgorithm(void)
{
    return VSCP_BOOTLOADER_PIC1;
}

///////////////////////////////////////////////////////////////////////////////
// Get the buffer size
//

uint8_t vscp_getBufferSize(void)
{
    return 8; // Standard CAN frame
}


///////////////////////////////////////////////////////////////////////////////
//  getNickname
//

uint8_t vscp_getNickname(void)
{
    return eeprom_read(VSCP_EEPROM_NICKNAME);
}

///////////////////////////////////////////////////////////////////////////////
//  setNickname
//

void vscp_setNickname(uint8_t nickname)
{
    eeprom_write(VSCP_EEPROM_NICKNAME, nickname);
}

///////////////////////////////////////////////////////////////////////////////
//  getSegmentCRC
//

uint8_t vscp_getSegmentCRC(void)
{
    return eeprom_read( VSCP_EEPROM_SEGMENT_CRC );
}

///////////////////////////////////////////////////////////////////////////////
//  setSegmentCRC
//

void vscp_setSegmentCRC(uint8_t crc)
{
    eeprom_write( VSCP_EEPROM_SEGMENT_CRC, crc );
}

///////////////////////////////////////////////////////////////////////////////
//  setVSCPControlByte
//

void vscp_setControlByte(uint8_t ctrl)
{
    eeprom_write(VSCP_EEPROM_CONTROL, ctrl);
}


///////////////////////////////////////////////////////////////////////////////
//  getVSCPControlByte
//

uint8_t vscp_getControlByte(void)
{
    return eeprom_read(VSCP_EEPROM_CONTROL);
}

///////////////////////////////////////////////////////////////////////////////
//  vscp_getEmbeddedMdfInfo
//

void vscp_getEmbeddedMdfInfo(void)
{
    // No embedded DM so we respond with info about that

    vscp_omsg.priority = VSCP_PRIORITY_NORMAL;
    vscp_omsg.flags = VSCP_VALID_MSG + 3;
    vscp_omsg.vscp_class = VSCP_CLASS1_PROTOCOL;
    vscp_omsg.vscp_type = VSCP_TYPE_PROTOCOL_RW_RESPONSE;

    vscp_omsg.data[ 0 ] = 0;
    vscp_omsg.data[ 1 ] = 0;
    vscp_omsg.data[ 2 ] = 0;

    // send the event
    vscp_sendEvent();
}


///////////////////////////////////////////////////////////////////////////////
// vscp_goBootloaderMode
//

void vscp_goBootloaderMode( uint8_t algorithm )
{
    if ( VSCP_BOOTLOADER_PIC1 != algorithm  ) return;

    // OK, We should enter boot loader mode
    // 	First, activate bootloader mode
    eeprom_write(VSCP_EEPROM_BOOTLOADER_FLAG, VSCP_BOOT_FLAG);

    // Reset processor
    Reset();
}

///////////////////////////////////////////////////////////////////////////////
//  vscp_getMatrixInfo
//

void vscp_getMatrixInfo(char *pData)
{
    uint8_t i;

    vscp_omsg.data[ 0 ] = DESCION_MATRIX_ROWS;  // Matrix is seven rows
    vscp_omsg.data[ 1 ] = REG_DESCION_MATRIX;   // Matrix start offset
    vscp_omsg.data[ 2 ] = 0;                    // Matrix start page
    vscp_omsg.data[ 3 ] = DESCION_MATRIX_PAGE;
    vscp_omsg.data[ 4 ] = 0;                    // Matrix end page
    vscp_omsg.data[ 5 ] = DESCION_MATRIX_PAGE;
    vscp_omsg.data[ 6 ] = 0;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getFamilyCode
//

uint32_t vscp_getFamilyCode() {
    return 0L;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_getFamilyType
//

uint32_t vscp_getFamilyType() {
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_restoreDefaults
//

void vscp_restoreDefaults() {
    init_app_eeprom();
    init_app_ram();
}


///////////////////////////////////////////////////////////////////////////////
// vscp_getRegisterPagesUsed
//

uint8_t vscp_getRegisterPagesUsed( void )
{
    return 1; // One pae used
}

///////////////////////////////////////////////////////////////////////////////
// sendVSCPFrame
//

int8_t sendVSCPFrame( uint16_t vscpclass,
                        uint8_t vscptype,
                        uint8_t nodeid,
                        uint8_t priority,
                        uint8_t size,
                        uint8_t *pData )
{
    uint32_t id = ( (uint32_t)priority << 26 ) |
                    ( (uint32_t)vscpclass << 16 ) |
                    ( (uint32_t)vscptype << 8 ) |
                    nodeid; // node address (our address)

    if ( !sendCANFrame( id, size, pData ) ) {
        return FALSE;
    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// getVSCPFrame
//

int8_t getVSCPFrame(uint16_t *pvscpclass,
                        uint8_t *pvscptype,
                        uint8_t *pNodeId,
                        uint8_t *pPriority,
                        uint8_t *pSize,
                        uint8_t *pData)
{
    uint32_t id;

    if ( !getCANFrame(&id, pSize, pData) ) {
        return FALSE;
    }

    *pNodeId = id & 0x0ff;
    *pvscptype = (id >> 8) & 0xff;
    *pvscpclass = (id >> 16) & 0x1ff;
    *pPriority = (uint16_t) (0x07 & (id >> 26));

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// sendCANFrame
//

int8_t sendCANFrame(uint32_t id, uint8_t dlc, uint8_t *pdata)
{
    uint8_t rv = FALSE;
    sendTimer = 0;

    while ( sendTimer < 1000 ) {
        if ( ECANSendMessage( id, pdata, dlc, ECAN_TX_XTD_FRAME ) ) {
            rv = TRUE;
            break;
        }
    }

    vscp_omsg.flags = 0;

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// getCANFrame
//

int8_t getCANFrame(uint32_t *pid, uint8_t *pdlc, uint8_t *pdata)
{
    ECAN_RX_MSG_FLAGS flags;

    // Dont read in new event if there already is a event
    // in the input buffer
    if (vscp_imsg.flags & VSCP_VALID_MSG) return FALSE;

    if ( ECANReceiveMessage( pid, pdata, pdlc, &flags) ) {

        // RTR not interesting
        if (flags & ECAN_RX_RTR_FRAME) return FALSE;

        // Must be extended frame
        if (!(flags & ECAN_RX_XTD_FRAME)) return FALSE;

        return TRUE;

    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// calculateSetFilterMask
//
// Calculate and set required filter and mask
// for the current decision matrix
//

void calculateSetFilterMask( void )
{
    uint8_t i,j;
    uint8_t lastOID;
    uint32_t rowmask;
    uint32_t rowfilter;

    // Reset filter masks
    uint32_t mask = 0xffffffff; // Just id 0x00000000 will come true
    uint32_t filter = 0;

    // Go through all DM rows
    for ( i=0; i < DESCION_MATRIX_ROWS; i++ ) {

        // No need to check not active DM rows
        if ( eeprom_read( VSCP_EEPROM_END + 8*i + 1 ) & 0x80 ) {

            // build the mask
            // ==============
            // We receive
            //  - all priorities
            //  - hardcoded and not hardcoded
            //  - from all nodes

            rowmask =
                    // Bit 9 of class mask
                    ( (uint32_t)( eeprom_read( VSCP_EEPROM_END + 8*i + 1 ) & 2 ) << 23 ) |
                    // Rest of class mask
                    ( (uint32_t)eeprom_read( VSCP_EEPROM_END + 8*i + 2 ) << 16 ) |
                    // Type mask
                    ( (uint32_t)eeprom_read( VSCP_EEPROM_END + 8*i + 4 ) << 8 ) |
					// OID  - handle later
					0xff;
                    /*( ( eeprom_read( VSCP_EEPROM_END + 8*i + 1 ) & 0x20 ) << 20 )*/;   // Hardcoded bit

            // build the filter
            // ================

            rowfilter =
                    // Bit 9 of class filter
                    ( (uint32_t)( eeprom_read( VSCP_EEPROM_END + 8*i + 1 ) & 1 ) << 24 ) |
                    // Rest of class filter
                    ( (uint32_t)eeprom_read( VSCP_EEPROM_END + 8*i + 3 ) << 16 ) |
                    // Type filter
                    ( (uint32_t)eeprom_read( VSCP_EEPROM_END + 8*i + 5 ) << 8 ) |
                    // OID Mask cleard if not same OID for all or one or more
                    // rows don't have OID check flag set.
                    eeprom_read( VSCP_EEPROM_END + 8*i );

            if ( 0 == i ) filter = rowfilter;   // Hack for first iteration loop

            // Form the mask - if one mask have a don't care (0)
            // the final mask should also have a don't care on that position
            mask &= rowmask;

            // Check the calculated filter and the current
            // filter to see if the bits are the same. If they
            // are not then clear the mask at that position
            for ( j=0; j<32; j++ ) {
                // If two bits are different we clear the mask bit
                if ( ( ( filter >> j ) & 1 ) != ( ( rowfilter >> j ) & 1 ) ) {
                    mask &= ~(1<<j);
                }
            }

            // Form the filter
            // if two bits are not the same they will be zero
            // All zeros will be zero
            // All ones will be one
            filter &= rowfilter;

            // Not check OID?
            if ( !eeprom_read( VSCP_EEPROM_END + 8*i + 1 ) & 0x40 ) {
                // No should not be checked for this position
                // This mean that we can't filter on a specific OID
                // so mask must be a don't care
                mask &= ~0xff;
            }

            if (i) {
                // If the current OID is different than the previous
                // we accept all
                for (j = 0; j < 8; j++) {
                    if ((lastOID >> i & 1)
                            != (eeprom_read(VSCP_EEPROM_END + 8 * i) >> i & 1)) {
                        mask &= (1 << i);
                    }
                }

                lastOID = eeprom_read(VSCP_EEPROM_END + 8 * i);

            } 
            else {
                // First round we just store the OID
                lastOID = eeprom_read(VSCP_EEPROM_END + 8 * i);
            }

        }
    }
    
    // Must be in Config mode to change settings.
    ECANSetOperationMode( ECAN_OP_MODE_CONFIG );

    //Set mask 1
    ECANSetRXM1Value( mask, ECAN_MSG_XTD );

    // Set filter 1
    ECANSetRXF1Value( filter, ECAN_MSG_XTD );

    // Return to Normal mode to communicate.
    ECANSetOperationMode( ECAN_OP_MODE_NORMAL );

}