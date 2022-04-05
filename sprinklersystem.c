#include "fsl_device_registers.h"

unsigned int nr_overflows = 0;
unsigned int nightflag = 0;

unsigned int get_elapsed_ticks()
{
    return (nr_overflows << 16) | FTM3_CNT;
}
void GPIOSetUp(void)
{
    SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK; /* Enable Port C Clock Gate Control*/
    SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK; /* Enable Port A Clock Gate Control*/
    // GPIO Configurations
    PORTA_PCR1 = 0x100;    // Port A Pin 1
    PORTA_PCR1 = 0xA0100;  /*Configure Port A pin 1 for GPIO and interrupt on falling edge*/
    PORTA_ISFR = (1 << 1); /* Clear ISFR for Port A Pin 1*/
    PORTC_PCR0 = 0x100;    // Port C Pin 0
    PORTC_PCR1 = 0x100;    // Port C Pin 1
    PORTC_PCR2 = 0x100;    // Port C Pin 2
    PORTC_PCR3 = 0x100;    // Port C Pin 3
    PORTC_PCR4 = 0x100;    // Port C Pin 4
    // Output Configurations
    GPIOC_PDDR |= (1 << 0);     /*Setting the bit 0 of the port C as Output*/
    GPIOC_PDDR |= (1 << 1);     /*Setting the bit 1 of the port C as Output*/
    GPIOC_PDDR |= (1 << 2);     /*Setting the bit 2 of the port C as Output*/
    GPIOC_PDDR |= (1 << 3);     /*Setting the bit 3 of the port C as Output*/
    GPIOC_PDDR |= (1 << 4);     /*Setting the bit 4 of the port C as Output*/
    NVIC_EnableIRQ(PORTA_IRQn); /* Enable interrupts from Port A*/
}

void FTM3_IRQHandler(void)
{
    nr_overflows++;
    uint32_t SC_VAL = FTM3_SC;
    FTM3_SC &= 0x7F; // clear TOF
}

void ADCThreshold(unsigned int adcval)
{
    if (adcval > 0x00000000)
    {
        nightflag = 0;
    }
    else
    {
        nightflag = 1;
    }
}

unsigned short ADC_read16b(void)
{
    ADC0_SC1A = 0x00; // Write to SC1A to start conversion from ADC_0
    while (ADC0_SC2 & ADC_SC2_ADACT_MASK)
        ; // Conversion in progress
    while (!(ADC0_SC1A & ADC_SC1_COCO_MASK))
        ; // Until conversion complete
    return ADC0_RA;
}

int GetADCValue(void)
{
    unsigned int ADC_Data = 0;
    ADC_Data = ADC_read16b();
    return ADC_Data;
}

void ADCSetUp(void)
{
    SIM_SCGC6 |= SIM_SCGC6_ADC0_MASK; /* Enable ADC0 Clock Gate Control*/
    // insert setup for PORT X, Pin n below
    ADC0_CFG1 = 0x0C; // Configure ADC for 16 bits, and to use bus clock
    ADC0_SC1A = 0x1F; // Disable ADC0 Module, ADCH = 11111
}

void PORTA_IRQHandler(void)
{
    unsigned int ADCval = 0;
    NVIC_ClearPendingIRQ(PORTA_IRQn); // Clear pending interrupts
    ADCval = GetADCValue();
    PORTC_ISFR = 0xFFFFFFFF; // Clear ISFR for Port C
    PORTA_ISFR = (1 << 1);   // Clear ISFR for Port A pin 1
    ADCThreshold(ADCval);
}

void TimerSetUp(void)
{
    SIM_SCGC3 |= SIM_SCGC3_FTM3_MASK; // FTM3 clock gating on
    FTM3_MODE = 0x5;                  // Enable FTM3
    FTM3_MOD = 0xFFFF;                // Top count before interrupt
    FTM3_SC = 0x0F;                   // System clock / 128, biggest we could
    NVIC_EnableIRQ(FTM3_IRQn);        // Enable FTM3 interrupts
    FTM3_SC |= 0x40;                  // Enable TOF
}

void StepperMotorRun(void)
{
    unsigned long i = 0;
    unsigned long direction = 0;
    unsigned long speed = 0;
    unsigned long cnt = 0;
    unsigned int ticks = get_elapsed_ticks() + 9843750;
    while (get_elapsed_ticks() < ticks)
    { // period of one minute for when the sprinkler system is active
        cnt = 0;
        if (direction == 0 && speed == 0)
        {
            while (cnt < 24)
            {
                GPIOC_PDOR = 0x36;
                for (i = 0; i < 140000; i++)
                    ;
                GPIOC_PDOR = 0x35;
                for (i = 0; i < 140000; i++)
                    ;
                GPIOC_PDOR = 0x39;
                for (i = 0; i < 140000; i++)
                    ;
                GPIOC_PDOR = 0x3A;
                for (i = 0; i < 140000; i++)
                    ;
                cnt++;
            }
            cnt = 0;
            direction = 1;
            speed = 1;
        }
        // Clockwise Direction, Speed @ 180 degrees
        else if (direction == 1 && speed == 1)
        {
            while (cnt < 24)
            {
                GPIOC_PDOR = 0x0A;
                for (i = 0; i < 17500; i++)
                    ;
                GPIOC_PDOR = 0x09;
                for (i = 0; i < 17500; i++)
                    ;
                GPIOC_PDOR = 0x05;
                for (i = 0; i < 17500; i++)
                    ;
                GPIOC_PDOR = 0x06;
                for (i = 0; i < 17500; i++)
                    ;
                cnt++;
            }
            cnt = 0;
            direction = 0;
            speed = 0;
        }
    }
    if (direction == 1 && speed == 1)
    {
        while (cnt < 24)
        {
            GPIOC_PDOR = 0x0A;
            for (i = 0; i < 17500; i++)
                ;
            GPIOC_PDOR = 0x09;
            for (i = 0; i < 17500; i++)
                ;
            GPIOC_PDOR = 0x05;
            for (i = 0; i < 17500; i++)
                ;
            GPIOC_PDOR = 0x06;
            for (i = 0; i < 17500; i++)
                ;
            cnt++;
        }
        cnt = 0;
        direction = 0;
        speed = 0;
    }
}
int main(void)
{
    unsigned int mainTicks = 0; // checking day/night status
    GPIOSetUp();
    ADCSetUp();
    TimerSetUp();
    mainTicks = get_elapsed_ticks() + 9843750;
    for (;;)
    {
        if (get_elapsed_ticks() > mainTicks)
        {
            if (nightflag == 1)
            { // if it is night time, run motor
                StepperMotorRun();
                nr_overflows = 0;
                mainTicks = get_elapsed_ticks() + 19687500;
                nightflag = 0; // back to day time after one minute
            }
            else
            { // if day time, wait for one minute again
                mainTicks = get_elapsed_ticks() + 9843750;
            }
        }
    }
    return 0;
}