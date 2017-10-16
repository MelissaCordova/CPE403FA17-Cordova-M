//
/*
 * Melissa Cordova
 * CPE 403 - LAB 5
 *
 * main.c
 */

#include <stdint.h>     //variable deifinitions for the C99 standard
#include <stdbool.h>    //boolean definitions for the C99 standard
#include "inc/hw_memmap.h"  //macros defining the memory map of the TivaC
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_types.h"   //define common types and macros
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"   //defines and macros for System Control API
#include "driverlib/adc.h"  //definitions for using the ADC driver
#include "driverlib/gpio.h" //defines macros for GPIO
#include "driverlib/interrupt.h"    //defines and macros for NVIC
#include "driverlib/timer.h" //defines and macros for Timer API
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"

#ifdef DEBUG
void__error__(char *pcFilename, uint32_t u132lLine)
{

}
#endif


      uint32_t ui32ADC0Value[4];  //create an array to store data from ADC FIFO
                                  //with depth of 4

      volatile uint32_t ui32TempAvg;  //sorts the average of the temp
      volatile uint32_t ui32TempValueC;   //store the Celsius temp
      volatile uint32_t ui32TempValueF;   //store the Fahrenheit temp


int main(void)
{
    uint32_t ui32Period;


    //system clock to run at 40MHz
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);

    //enable the ADC0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);

     //calculate and set delay
    ui32Period = SysCtlClockGet()/2;
    TimerLoadSet(TIMER1_BASE, TIMER_A, ui32Period-1);

    ADCHardwareOversampleConfigure(ADC0_BASE, 32);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3);

    //ADC sequencer
    ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0);

    //Configuring steps 0-2 on sequencer 2 to sample temp sensor
    ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 2, 2, ADC_CTL_TS);

    //Sample temp sensor and configure interrupt flag to set
    ADCSequenceStepConfigure(ADC0_BASE, 2, 3, ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);

    //enable ADC
    ADCSequenceEnable(ADC0_BASE, 2);


    //enable interrupts
    IntEnable(INT_TIMER1A);
    //set timer 1 to interrupt
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();
    //enable timer
    TimerEnable(TIMER1_BASE, TIMER_A);


    while(1)
    {

    }
}
void IntTimer1Handler(void)
{


       TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
        //ADC conversion
       ADCIntClear(ADC0_BASE, 2);
        //ADC conversion with software
       ADCProcessorTrigger(ADC0_BASE, 2);

        while(!ADCIntStatus(ADC0_BASE, 2, false))
        {

        }

        //ADC value from ADC Sampler Sequence
        ADCSequenceDataGet(ADC0_BASE, 2, ui32ADC0Value);

        //Average of temp sensor data
        ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;

        //calculate celsius value of temp
        ui32TempValueC = (1475- ((2475*ui32TempAvg)) / 4096) /10;

        //calculate Fahrenheit temp
        ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;

        if(ui32TempValueF > 75)  //if temp is > 75 degrees F 
        {
            //Turn on LED at PF1 because i could not get to turn on at PF3
            ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 2);
        }
        else
        {
            //Turn off all LEDs
            ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
        }
        

    }


