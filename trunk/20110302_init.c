/*****************************************************
This program was produced by the
CodeWizardAVR V2.03.9 Standard
Automatic Program Generator
© Copyright 1998-2008 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : Robo Water
Version : 1.0
Date    : 02.03.2011
Author  : Admin
Company : Microsoft
Comments:


Chip type               : ATmega16
Program type            : Application
AVR Core Clock frequency: 8,000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 256
*****************************************************/
#include <mega16.h>

// I2C Bus functions
#asm
   .equ __i2c_port=0x15 ;PORTC
   .equ __sda_bit=1
   .equ __scl_bit=0
#endasm
#include <i2c.h>

// DS1307 Real Time Clock functions
#include <ds1307.h>

// 1 Wire Bus functions
#asm
   .equ __w1_port=0x18 ;PORTB
   .equ __w1_bit=0
#endasm
#include <1wire.h>

// DS1820 Temperature Sensor functions
#include <ds1820.h>
// Standard Input/Output functions
#include <stdlib.h>  // for abs
#include <stdio.h>
#include <delay.h>
#include "robowater.h"

// maximum number of DS1820 devices connected to the 1 Wire bus
#define MAX_DS1820 8
// number of DS1820 devices connected to the 1 Wire bus
unsigned char ds1820_devices;
// DS1820 devices ROM code storage area, 9 bytes are used for each device (see the w1_search function description in the help)
unsigned char ds1820_rom_codes[MAX_DS1820][9];
byte cHH, cMM, cSS, cyy, cmm, cdd;
byte prn;

// External Interrupt 0 service routine
interrupt [EXT_INT0] void ext_int0_isr(void)
{
// Place your code here

}

// External Interrupt 1 service routine
interrupt [EXT_INT1] void ext_int1_isr(void)
{
// Place your code here

}

// External Interrupt 2 service routine
interrupt [EXT_INT2] void ext_int2_isr(void)
{
    rtc_get_time(&cHH,&cMM,&cSS);
    rtc_get_date(&cdd,&cmm,&cyy);
    prn = 1;
}


// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
// Place your code here

}

// Timer 1 overflow interrupt service routine
interrupt [TIM1_OVF] void timer1_ovf_isr(void)
{
// Place your code here

}

// Timer 2 overflow interrupt service routine
interrupt [TIM2_OVF] void timer2_ovf_isr(void)
{
}

#define ADC_VREF_TYPE 0xC0

/* // Read the AD conversion result
unsigned int read_adc(unsigned char adc_input) {
    ADMUX=adc_input | (ADC_VREF_TYPE & 0xff);
    delay_us(10);   // Delay needed for the stabilization of the ADC input voltage
    ADCSRA|=0x40;   // Start the AD conversion
    while ((ADCSRA & 0x10)==0); // Wait for the AD conversion to complete
    ADCSRA|=0x10;
    return ADCW;
} */

void init(void) {
    // Port A initialization
    /* Func7=Out Func6=Out Func5=Out Func4=Out Func3=Out Func2=Out Func1=In Func0=In
    // State7=0 State6=0 State5=0 State4=0 State3=0 State2=0 State1=T State0=T
    */
    PORTA=0x00; DDRA=0xFC;

    // Port B initialization
    /* Func7=Out Func6=Out Func5=Out Func4=Out Func3=Out Func2=In Func1=In Func0=In
    // State7=0 State6=0 State5=0 State4=0 State3=0 State2=P State1=P State0=P
    */
    PORTB=0x07; DDRB=0xF8;

    // Port C initialization
    /* Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
    // State7=P State6=P State5=P State4=P State3=P State2=P State1=P State0=P
    */
    PORTC=0xFF; DDRC=0x00;

    // Port D initialization
    /* Func7=Out Func6=In Func5=Out Func4=Out Func3=In Func2=In Func1=Out Func0=In
    // State7=0 State6=P State5=0 State4=0 State3=P State2=P State1=0 State0=P
    */
    PORTD=0x4D; DDRD=0xB2;

    // Timer/Counter 0 initialization
    /* Clock source: System Clock
    // Clock value: 31,250 kHz
    // Mode: Fast PWM top=FFh
    // OC0 output: Non-Inverted PWM
    */
    TCCR0=0x6C; TCNT0=0x00; OCR0=0x00;

    // Timer/Counter 1 initialization
    /* Clock source: System Clock
    // Clock value: 125,000 kHz
    // Mode: Normal top=FFFFh
    // OC1A output: Discon.
    // OC1B output: Discon.
    // Noise Canceler: Off
    // Input Capture on Falling Edge
    // Timer 1 Overflow Interrupt: On
    // Input Capture Interrupt: Off
    // Compare A Match Interrupt: Off
    // Compare B Match Interrupt: Off
    */
    TCCR1A=0x00; TCCR1B=0x03; TCNT1H=0x00; TCNT1L=0x00; ICR1H=0x00; ICR1L=0x00; OCR1AH=0x00; OCR1AL=0x00; OCR1BH=0x00; OCR1BL=0x00;

    // Timer/Counter 2 initialization
    /* Clock source: System Clock
    // Clock value: 31,250 kHz
    // Mode: Fast PWM top=FFh
    // OC2 output: Inverted PWM
    */
    ASSR=0x00; TCCR2=0x7E; TCNT2=0x00; OCR2=0x00;

    // External Interrupt(s) initialization
    /* INT0: On INT0 Mode: Falling Edge
    // INT1: On INT1 Mode: Falling Edge
    // INT2: On  INT2 Mode: Falling Edge
    */
    GICR|=0xE0; MCUCR=0x0A; MCUCSR=0x00; GIFR=0xE0;
    // Timer(s)/Counter(s) Interrupt(s) initialization
    TIMSK=0x45;

    // USART initialization
    /* Communication Parameters: 8 Data, 1 Stop, No Parity
    // USART Receiver: On
    // USART Transmitter: On
    // USART Mode: Asynchronous
    // USART Baud Rate: 38400
    */
    UCSRA=0x00; UCSRB=0x18; UCSRC=0x86; UBRRH=0x00; UBRRL=0x0C;

    // Analog Comparator initialization
    /* Analog Comparator: Off
    // Analog Comparator Input Capture by Timer/Counter 1: Off
    */
    ACSR=0x80; SFIOR=0x00;

    // ADC initialization
    /* ADC Clock frequency: 125,000 kHz
    // ADC Voltage Reference: Int., cap. on AREF
    // ADC Auto Trigger Source: Free Running
    */
    ADMUX=ADC_VREF_TYPE & 0xff; ADCSRA=0xA6; SFIOR&=0x1F;

    // I2C Bus initialization
    i2c_init();

    // DS1307 Real Time Clock initialization
    /* Square wave output on pin SQW/OUT: On
    // Square wave frequency: 1Hz
    */
    rtc_init(0,1,0);

    // Determine the number of DS1820 devices connected to the 1 Wire bus
    ds1820_devices=w1_search(0xf0,ds1820_rom_codes);

    // Инициализируем часы реального времени датой разработки этой программы
    rtc_set_date(2, 3, 11); 	// 02.03.2011
    rtc_set_time(22, 07, 0); 	// 22:07:00
    delay_ms (500);
    rtc_get_time(&cHH, &cMM, &cSS); rtc_get_date(&cdd, &cmm, &cyy);		// вычитываем время и дату
} // Конец функции init
// Чтение температуры (аргумент - номер термометра начиная с 0)
int read_term(byte num) {
	int lterm;
    lterm = ds1820_temperature_10(&ds1820_rom_codes[num][0]);
	return (lterm);
};
// Печать всех термометров
void printallterms(void) {
	byte i;
    int term;
    if(!ds1820_devices) return;			// если термометры не обнаружены - просто выходим из функции
    printf("\t");						// печатаем знак табуляции в терминале
    for(i=0; i<ds1820_devices; i++) {
    	term = read_term(i);
        printf("t%-u = %-i.%-uC; ", (i+1), term/100, abs(term%100));
    }
	printf("\r\n");
}

void main(void) {
    byte status, inbyte;	// Описание локальных переменных
    init();      			// Инициализация всей периферии
    #asm("sei")         	// Global enable interrupts
    prn = 1;
    printf("Старт программы RoboWater. Версия 1.09\r\n");
    while(1) {
        status = UCSRA;     		// Вычитываем статус порта
        if (status & RX_COMPLETE) { // Пришло ли что-нибудь
            inbyte = UDR;
            switch (inbyte) {
                case 0x7A:				// символ 'z'
                	printf("Время: %02u:%02u:%02u, дата:%02u.%02u.%02u, найдено %u термометров\r\n",
                    		cHH, cMM, cSS, cdd, cmm, cyy, ds1820_devices+1);
                    printallterms();
                    break;
                case 0x71:				// символ 'q'
                	printf("Нажат символ 'q'\r\n");
                    break;
                case 0x61:				// символ 'a'
                	printf("Нажат символ 'a'\r\n");
                    break;
                default:
                	printf("Нажат символ 0x%x\r\n", inbyte);
            };
        }
        // проверяем и присваиваем биту PORTD.4 состояния PIND.6
        if(PIND.6) PORTD &= ~(1<<4); else PORTD |= (1<<4);
    };
}
