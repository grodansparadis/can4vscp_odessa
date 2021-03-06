/* ******************************************************************************
 * 	VSCP (Very Simple Control Protocol) 
 * 	http://www.vscp.org
 *
 *  Odessa expansion Module
 *  ========================
 *
 *  Copyright (C)1995-2020 Ake Hedman, Grodans Paradis AB
 *                          http://www.grodansparadis.com
 *                          <akhe@grodansparadis.com>
 *
 *  This work is licensed under the Creative Common 
 *  Attribution-NonCommercial-ShareAlike 3.0 Unported license. The full
 *  license is available in the top folder of this project (LICENSE) or here
 *  http://creativecommons.org/licenses/by-nc-sa/3.0/legalcode
 *  It is also available in a human readable form here 
 *  http://creativecommons.org/licenses/by-nc-sa/3.0/
 * 
 *	This file is part of VSCP - Very Simple Control Protocol 	
 *	http://www.vscp.org
 *
 * ******************************************************************************
 */

#ifndef ODESSA_H
#define ODESSA_H

//Defines
#define	TRUE			1
#define	FALSE			0


// EEPROM Storage
#define VSCP_EEPROM_BOOTLOADER_FLAG			0x00	// Reserved for bootloader	 

#define VSCP_EEPROM_NICKNAME				0x01	// Persistant nickname id storage
#define VSCP_EEPROM_SEGMENT_CRC				0x02	// Persistant segment crc storage
#define VSCP_EEPROM_CONTROL1                0x03	// Persistant control byte
#define VSCP_EEPROM_CONTROL2                0x04	// Persistant control byte

#define VSCP_EEPROM_REG_USERID				0x05
#define VSCP_EEPROM_REG_USERID1				0x06
#define VSCP_EEPROM_REG_USERID2				0x07
#define VSCP_EEPROM_REG_USERID3				0x08
#define VSCP_EEPROM_REG_USERID4				0x09

#define VSCP_EEPROM_REG_MANUFACTUR_ID0      0x0A
#define VSCP_EEPROM_REG_MANUFACTUR_ID1      0x0B
#define VSCP_EEPROM_REG_MANUFACTUR_ID2      0x0C
#define VSCP_EEPROM_REG_MANUFACTUR_ID3      0x0D

#define VSCP_EEPROM_REG_MANUFACTUR_SUBID0	0x0E	
#define VSCP_EEPROM_REG_MANUFACTUR_SUBID1	0x0F	
#define VSCP_EEPROM_REG_MANUFACTUR_SUBID2	0x10
#define VSCP_EEPROM_REG_MANUFACTUR_SUBID3	0x11

#define VSCP_EEPROM_REG_GUID                0x12    // Start of GUID MSB	
                                                    // 		0x11 - 0x20

#define VSCP_EEPROM_END                     0x22	// marks end of VSCP EEPROM usage
                                                    //   (next free position)

//
// 8 MHz with PLL => 8 MHz
// 1:4 prescaler => 1 MHz (1 uS cycle )
// 1 ms == 1000 uS
// 65535 - 1000 = 64535 = 0xfc17
//
// Timer2 use 250 and prescaler 1:4
//
//#define TIMER0_RELOAD_VALUE		0xfc17

//
// 10 MHz with PLL => 40 MHz
// 1:4 prescaler => 1.25 MHz ( 0.800 uS cycle )
// 1 ms == 1000 uS
// 65535 - 1250 = 64285 = 0xfb1d
//
// Timer2 use 156 and prescaler 1:8
//
#define TIMER0_RELOAD_VALUE         0xfb1d

//
// Timer 2 is used as a 1 ms clock
// 156 is loaded eight time to give ~1250 cycles
// Timer2 use 156 and prescaler 1:4, Postscaler 1:16
// 100 ns * 56 * 4 * 16 ~ 1 ms
//
#define TIMER2_RELOAD_VALUE         156



#define STATUS_LED  PORTCbits.RC1
#define INIT_BUTTON PORTCbits.RC0

// -----------------------------------------------

// * * *  Registers - Page=0 * * *
#define REG_ZONE                    0
#define REG_SUBZONE                 1

#define REG_CONTROL0                2
#define REG_CONTROL1                3
#define REG_CONTROL2                4

#define REG_PIN3_SUBZONE            5
#define REG_PIN4_SUBZONE            6
#define REG_PIN5_SUBZONE            7
#define REG_PIN6_SUBZONE            8
#define REG_PIN7_SUBZONE            9
#define REG_PIN8_SUBZONE            10
#define REG_PIN9_SUBZONE            11
#define REG_PIN10_SUBZONE           12
#define REG_PIN11_SUBZONE           13
#define REG_PIN12_SUBZONE           14
#define REG_PIN13_SUBZONE           15
#define REG_PIN14_SUBZONE           16
#define REG_PIN15_SUBZONE           17
#define REG_PIN16_SUBZONE           18
#define REG_PIN17_SUBZONE           19
#define REG_PIN18_SUBZONE           20
#define REG_PIN19_SUBZONE           21
#define REG_PIN20_SUBZONE           22

#define REG_FIRST_PAGE_END          23
// * * *  Registers - Page=1  * * *

// Decision Matrix
#define REG_DESCION_MATRIX          0   // Start of matrix, page 1
#define DESCION_MATRIX_ROWS         8   // Rows in DM
#define DESCION_MATRIX_PAGE         1

// --------------------------------------------------------------------------------

// * * * Actions * * *
#define ACTION_NOOP                 0
#define ACTION_SET                  1
#define ACTION_CLR                  2
#define ACTION_SETALL               3
#define ACTION_CLRALL               4


// * * * Control registers
#define CONTROL0                    0
#define CONTROL1                    1
#define CONTROL2                    2


// Function Prototypes

// Function Prototypes

void doWork( void );
void init( void );
void init_app_ram( void );
void init_app_eeprom( void ); 
void read_app_register( unsigned char reg );
void write_app_register( unsigned char reg, unsigned char val );
void sendDMatrixInfo( void );
void SendInformationEvent( unsigned char idx, unsigned char eventClass, unsigned char eventTypeId );

void doDM( void );

void doActionOn( unsigned char dmflags, unsigned char arg );
void doActionOff( unsigned char dmflags, unsigned char arg );
void doActionPulse( unsigned char dmflags, unsigned char arg );
void doActionStatus( unsigned char dmflags, unsigned char arg );
void doActionDisable( unsigned char dmflags, unsigned char arg );
void doActionToggle( unsigned char dmflags, unsigned char arg );

void doApplicationOneSecondWork( void );

/*!
	Send Extended ID CAN frame
	@param id CAN extended ID for frame.
	@param size Number of data bytes 0-8
	@param pData Pointer to data bytes of frame.
	@return TRUE (!=0) on success, FALSE (==0) on failure.
*/
int8_t sendCANFrame( uint32_t id, uint8_t size, uint8_t *pData );

/*!
	Get extended ID CAN frame
	@param pid Pointer to CAN extended ID for frame.
	@param psize Pointer to number of data bytes 0-8
	@param pData Pointer to data bytes of frame.
	@return TRUE (!=0) on success, FALSE (==0) on failure.
*/
int8_t getCANFrame( uint32_t *pid, uint8_t *psize, uint8_t *pData );


#endif
