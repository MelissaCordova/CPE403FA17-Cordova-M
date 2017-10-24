//*****************************************************************************
//
// spi_master.c - Example demonstrating how to configure SSI0 in SPI master
// mode.
//
// Copyright (c) 2010-2011 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
// This is part of revision 8034 of the Stellaris Firmware Development Package.
//
//*****************************************************************************


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "inc/hw_types.h"
#include "inc/hw_timer.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"

#include "utils/uartstdio.h"

//*****************************************************************************
//
//! \addtogroup ssi_examples_list
//! <h1>SPI Master (spi_master)</h1>
//!
//! This example shows how to configure the SSI0 as SPI Master.  The code will
//! send three characters on the master Tx then polls the receive FIFO until
//! 3 characters are received on the master Rx.
//!
//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board:
//! - SSI0 peripheral
//! - GPIO Port A peripheral (for SSI0 pins)
//! - SSI0CLK - PA2
//! - SSI0Fss - PA3
//! - SSI0Rx  - PA4
//! - SSI0Tx  - PA5
//!
//! The following UART signals are configured only for displaying console
//! messages for this example.  These are not required for operation of SSI0.
//! - UART0 peripheral
//! - GPIO Port A peripheral (for UART0 pins)
//! - UART0RX - PA0
//! - UART0TX - PA1
//!
//! This example uses the following interrupt handlers.  To use this example
//! in your own application you must add these interrupt handlers to your
//! vector table.
//! - None.
//!
//
//*****************************************************************************

#define WREN    0x06
#define EWSR    0x50
#define WRSR    0x01
#define BYTEPROM    0x02
#define CHIPERASE   0x60
#define READ    0x03
#define HREAD   0x0B
#define RDSR    0x05

#define NUM_SSI_DATA 3

void   InitConsole(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinConfigure(GPIO_PD6_U2RX);
    GPIOPinConfigure(GPIO_PD7_U2TX);
    GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);
    UARTStdioConfig(2, 115200, 16000000);
}

//*****************************************************************************
//
// Configure SSI1 in master Freescale (SPI) mode.
//
//*****************************************************************************
int  main(void)
{
    uint32_t ulDataTx[NUM_SSI_DATA];
    uint32_t ulDataRx[NUM_SSI_DATA];
    uint32_t ulindex;

    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    //SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    InitConsole();
    UARTprintf("SSI ->\n");
    UARTprintf("Mode: SPI\n");
    UARTprintf("Data: 8-bit\n\n");
    ulDataRx[0]=1;
    //
    // The SSI0 peripheral and port D must be enabled for use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    // Configure the pin muxing for SSI1 functions on port D.
    GPIOPinConfigure(GPIO_PD0_SSI1CLK);
    //GPIOPinConfigure(GPIO_PD1_SSI1FSS);   //sherry
    GPIOPinConfigure(GPIO_PD2_SSI1RX);
    GPIOPinConfigure(GPIO_PD3_SSI1TX);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_1);//sherry

    GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3);    //sherry    GPIO_PIN_1 |
    GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,GPIO_PIN_1);    //Ω˚”√π¶ƒ‹£¨Ω˚”√∆¨—°FLASH

    //
    // Configure and enable the SSI port for SPI master mode.  Use SSI1,
    // system clock supply, idle clock level low and active low clock in
    // freescale SPI mode, master mode, 1MHz SSI frequency, and 8-bit data.
    // For SPI mode, you can set the polarity of the SSI clock when the SSI
    // unit is idle.  You can also configure what clock edge you want to
    // capture data on.  Please reference the datasheet for more information on
    // the different SPI modes.
    //
    SSIClockSourceSet(SSI1_BASE,SSI_CLOCK_SYSTEM);
    SSIConfigSetExpClk(SSI1_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 1000000, 8);

    SSIEnable(SSI1_BASE);

    //
    // Read any residual data from the SSI port.  This makes sure the receive
    // FIFOs are empty, so we don't read any unwanted junk.  This is done here
    // because the SPI SSI mode is full-duplex, which allows you to send and
    // receive at the same time.  The SSIDataGetNonBlocking function returns
    // "true" when data was returned, and "false" when no data was returned.
    // The "non-blocking" function checks if there is any data in the receive
    // FIFO and does not "hang" if there isn't.
    //
    while(SSIDataGetNonBlocking(SSI1_BASE, &ulDataRx[0]));
    //
    // Initialize the data to send.
    //
    ulDataTx[0] = 's';
    ulDataTx[1] = 'p';
    ulDataTx[2] = 'i';

    //
    // Display indication that the SSI is transmitting data.
    //
    UARTprintf("Sent:\n  ");
    //
    // Send the data using the "blocking" put function.  This function
    // will wait until there is room in the send FIFO before returning.
    // This allows you to assure that all the data you send makes it into
    // the send FIFO.
    //
//  GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,0);
//  SSIDataPut(SSI1_BASE,WREN); //–¥ πƒ‹
//  while(SSIBusy(SSI1_BASE));
//  GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,GPIO_PIN_1);

    GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,0);
    SSIDataPut(SSI1_BASE,RDSR);
    while(SSIBusy(SSI1_BASE));
    SSIDataGet(SSI1_BASE, &ulDataRx[0]);
    GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,GPIO_PIN_1);
    UARTprintf("'%X' ", ulDataRx[0]);

    GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,0);
    SSIDataPut(SSI1_BASE,EWSR);
    while(SSIBusy(SSI1_BASE));
    GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,GPIO_PIN_1);

        //SysCtlDelay(250 * (SysCtlClockGet()/3000)); // …Ë∂®√ø¥Œ—” ± ±º‰

    GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,0);
    SSIDataPut(SSI1_BASE,WRSR);
    while(SSIBusy(SSI1_BASE));
    SSIDataPut(SSI1_BASE,0x38);
    while(SSIBusy(SSI1_BASE));
    GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,GPIO_PIN_1);

        //SysCtlDelay(250 * (SysCtlClockGet()/3000)); // …Ë∂®√ø¥Œ—” ± ±º‰

    GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,0);
    SSIDataPut(SSI1_BASE,RDSR);
    while(SSIBusy(SSI1_BASE));
    SSIDataGet(SSI1_BASE, &ulDataRx[0]);
    GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,GPIO_PIN_1);
    UARTprintf("'%X' ", ulDataRx[0]);

//  GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,0);
//  SSIDataPut(SSI1_BASE,WREN); //–¥ πƒ‹
//  while(SSIBusy(SSI1_BASE));
//  GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,GPIO_PIN_1);
//
//    for(ulindex = 0; ulindex < NUM_SSI_DATA; ulindex++)           //NUM_SSI_DATA    sherry
//    {
//        //
//        // Display the data that SSI is transferring.
//        //
//        UARTprintf("'%c' ", ulDataTx[ulindex]);
//      GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,0);
//      SSIDataPut(SSI1_BASE,BYTEPROM);
//      SSIDataPut(SSI1_BASE,0x00);
//      SSIDataPut(SSI1_BASE,0x00);
//      SSIDataPut(SSI1_BASE,0x00);
//      SSIDataPut(SSI1_BASE,ulDataTx[ulindex]);
//      while(SSIBusy(SSI1_BASE));
//      GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,GPIO_PIN_1);
//    }

//    //
//    // Wait until SSI1 is done transferring all the data in the transmit FIFO.
//    //
//    while(SSIBusy(SSI1_BASE));
//    //
//    // Display indication that the SSI is receiving data.
//    //
//    UARTprintf("\nReceived:\n  ");
//
//    //
//    // Receive 3 bytes of data.
//    //
//    for(ulindex = 0; ulindex < NUM_SSI_DATA; ulindex++)    //NUM_SSI_DATA    sherry
//    {
//      GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,0);
//      SSIDataPut(SSI1_BASE,READ);
//      SSIDataPut(SSI1_BASE,0x00);
//      SSIDataPut(SSI1_BASE,0x00);
//      SSIDataPut(SSI1_BASE,0x00);
//      SSIDataGet(SSI1_BASE, &ulDataRx[ulindex]);
//      while(SSIBusy(SSI1_BASE));
//      GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,GPIO_PIN_1);
//      //
//        // Receive the data using the "blocking" Get function. This function
//        // will wait until there is data in the receive FIFO before returning.
//        //
//
//        //
//        // Since we are using 8-bit data, mask off the MSB.
//        //
//        ulDataRx[ulindex] &= 0x00FF;
//
//        //
//        // Display the data that SSI0 received.
//        //
//        UARTprintf("'%X' ", ulDataRx[ulindex]);
//    }

    //
    // Return no errors
    //
    while(1);
    return(0);
}
