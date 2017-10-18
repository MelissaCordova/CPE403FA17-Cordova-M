#include <stdinT.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inC/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"

#define PWM_FREQUENCY 55

int main(void)
{
    //variables to program the PWM
    volatile uint32_t ui32Load;
    volatile uint32_t ui32PWMClock;
    volatile uint8_t ui8Adjust;
    ui8Adjust = 83;

    //run CPU at 40MHz
         ROM_SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

    //enable PWM1 and GPIOD modules
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    //port D pin 0(PD0) must be configured as a PWM
    ROM_GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);
    ROM_GPIOPinConfigure(GPIO_PF1_M1PWM5);

    //unlock the GPIO commit control register
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0X01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
    //configures PF0 & 4 as inputs
    ROM_GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0,GPIO_DIR_MODE_IN);
    //configures the internal pull-up resistors on both pins
    ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    //PWM clock is SYSCLK/64 then divide it by frequency to be loaded
    ui32PWMClock = SysCtlClockGet()/64;
    ui32Load = (ui32PWMClock / PWM_FREQUENCY)-1;
    //Configure module 1 PWM generator 0 as a down-counter and load value
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);

    //enable PWM settings setting the pulse-width
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust * ui32Load/1000);
    //PWM module 1, generator needs to be enabled
    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_2);

    while(1)
    {
        //read the PF4 pin to see if SW1 is pressed
        if(ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4)==0x00)
        {
            //decrement until 1ms limit--- from 10% duty cycle
            ui8Adjust--;
            if(ui8Adjust < 100)
            {
                ui8Adjust = 100;
            }
            ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust * ui32Load/1000);

        }
        //read the PF0 pin to see if SW2 is pressed to increment the pulse width
        if(ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)==0X00)
        {
            //increment until 1.9ms ---to 90% duty cycle
            ui8Adjust++;
           if(ui8Adjust > 900)
           {
               ui8Adjust = 900;
           }
           ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust * ui32Load/1000);
        }
        //determines the speed of the loop
        ROM_SysCtlDelay(100000);

    }

}
