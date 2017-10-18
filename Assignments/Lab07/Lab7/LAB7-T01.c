#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"

void UARTIntHandler(void)
{
    uint32_t ui32Status; //variable

    //get interrupt status
    ui32Status = UARTIntStatus(UART0_BASE, true);

    //clear the asserted interrupts
    UARTIntClear(UART0_BASE, ui32Status);

    //loop while there are chars
    while(UARTCharsAvail(UART0_BASE))
    {
        //echo character
        UARTCharPutNonBlocking(UART0_BASE, UARTCharGetNonBlocking(UART0_BASE));
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); //blink LED
        SysCtlDelay(SysCtlClockGet() / (1000 * 3)); //delay ~1 msec
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); //turn off LED
    }
}

int main(void)
{
    //set up system clock
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    //enable UART and GPIOA peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    //configure pins for receiver and transmitter using GPIOPinConfigure
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //initialize the GPIO peripheral and pin for LED
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);

    //initialize parameters for UART:115200, 8-1-N
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    //enable processor interrupts
    IntMasterEnable();
    //enable UART interrupt
    IntEnable(INT_UART0);
    //select which individual UART interrupts to enable
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);

    //UARTCharPut calls to create a prompt
    UARTCharPut(UART0_BASE, 'E');
    UARTCharPut(UART0_BASE, 'n');
    UARTCharPut(UART0_BASE, 't');
    UARTCharPut(UART0_BASE, 'e');
    UARTCharPut(UART0_BASE, 'r');
    UARTCharPut(UART0_BASE, ' ');
    UARTCharPut(UART0_BASE, 'T');
    UARTCharPut(UART0_BASE, 'e');
    UARTCharPut(UART0_BASE, 'x');
    UARTCharPut(UART0_BASE, 't');
    UARTCharPut(UART0_BASE, ':');
    UARTCharPut(UART0_BASE, ' ');

    int inChar; //variable
    while (1)
    {
        //if there is a char in receiver, its read, and then written to the transmitter
        if (UARTCharsAvail(UART0_BASE))
        {
            inChar = UARTCharGet(UART0_BASE); //get char
            if(inChar >= 97 && inChar <= 122) //check lower case
            {
                inChar -= 32;
            }
            //if upper case add 32 to make lower case
            else if(inChar >= 65 && inChar <= 90)
            {
                inChar += 32;
            }
            //char on terminal
            UARTCharPut(UART0_BASE, inChar);
        }

    }

}
