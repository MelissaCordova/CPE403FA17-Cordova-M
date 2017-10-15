#include <stdint.h>         //variable definitions for the C99 standard
#include <stdbool.h>        //boolean definitions for the C99 standard
#include "inc/tm4c123gh6pm.h"   //definitions for the interrupt and register assig
#include "inc/hw_memmap.h"      //macros defining the memory map of the TivaC
#include "inc/hw_types.h"       //defines common types and macros
#include "driverlib/sysctl.h"   //defines and macros for System Control API
#include "driverlib/interrupt.h"    //defines and macros for NVIC Controller
#include "driverlib/gpio.h"     //Defines and macros for GPIO API of DriverLib
#include "driverlib/timer.h"    //defines and macros for Timer API of DriverLib

int main (void)
{

    uint32_t ui32Period;        //variable ui32Period with unsigned 32-bit int

    //system clock to run at 40MHz
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);        //enable GPIO peripheral
    //configure pins
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    //configures Timer 0 as a 32-bit timer in periodic mode
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    //toggle GPIO at 50Hz and at 75% duty cycle
    ui32Period = (SysCtlClockGet()/50) /2;
    //load into Timer's Interval Load register
    TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period-1);

    IntEnable(INT_TIMER0A);  //enable specific vector associated with Timer0A
    //master interrupt enable API for all interrupts
    TimerIntEnable(TIMER0_BASE,TIMER_TIMA_TIMEOUT);
    IntMasterEnable();

    TimerEnable(TIMER0_BASE, TIMER_A);  //enable the timer

    while(1)
    {

    }

}

void Timer0IntHandler(void)
{
    //Clear the timer interrupt
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    //Read the current state of the GPIO pin and
    //write back the opposites state

    if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_2))
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
     }
     else
     {
         GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);
         //75% duty cycle
         SysCtlDelay(2000000);
     }

}
