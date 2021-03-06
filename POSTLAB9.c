/* 
 * File:   POSTLAB9.c
 * Author: Esli Vivar
 *
 * Created on 30 de abril de 2022
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>

/*------------------------------------------------------------------------------
 * CONSTANTES 
 ------------------------------------------------------------------------------*/
#define _XTAL_FREQ 4000000
#define _tmr0_value 250       // tiempo de retardo

/*------------------------------------------------------------------------------
 * VARIABLES 
 ------------------------------------------------------------------------------*/
uint8_t contador;
uint8_t pot3;
/*------------------------------------------------------------------------------
 * PROTOTIPO DE FUNCIONES 
 ------------------------------------------------------------------------------*/
void setup(void);

/*------------------------------------------------------------------------------
 * INTERRUPCIONES 
 ------------------------------------------------------------------------------*/
void __interrupt() isr (void)
{
    if (PIR1bits.ADIF)  // Interrupci?n del ADC
    {
         if (ADCON0bits.CHS == 0){
            CCPR1L = (ADRESH>>1)+30;
            CCP1CONbits.DC1B1 = ADRESH & 0b01;
            CCP1CONbits.DC1B0 = ADRESL >> 7;
         }
           
        else if (ADCON0bits.CHS == 1){
            CCPR2L = (ADRESH>>1)+45;
            CCP2CONbits.DC2B1 = ADRESH & 0b01;
            CCP2CONbits.DC2B0 = ADRESL >> 7;
        }
        else
            pot3 = ADRESH;
        PIR1bits.ADIF = 0;
        
    }

    if(T0IF == 1){             // Fue interrupci?n del TMR0
        INTCONbits.T0IF = 0;
        contador ++;
        TMR0 = _tmr0_value;
        
        if (contador < pot3)  // Si es menor el valor del contador entonces se enciende el led
            PORTDbits.RD0 = 1;
        else
            PORTDbits.RD0 = 0;
          
        INTCONbits.T0IF = 0;
    }    
}
/*------------------------------------------------------------------------------
 * Ciclo principal
------------------------------------------------------------------------------*/
void main(void) {
    setup();
    while(1){
    
    if (ADCON0bits.GO == 0) {
        if (ADCON0bits.CHS == 0)
            ADCON0bits.CHS = 1;
        else if (ADCON0bits.CHS == 1)
            ADCON0bits.CHS = 2;
        else
            ADCON0bits.CHS = 0;
            
        __delay_us(1000);
        ADCON0bits.GO = 1;
        
    } 
        
    }        
}

/*------------------------------------------------------------------------------
 * Configuraci?n
------------------------------------------------------------------------------*/
void setup (void){
    
    // Configuraci?n de los puertos
    ANSEL = 0b00000111;          //AN0  como entradas anal?gicas
    ANSELH = 0;
    
    TRISA = 0b00000111;         // PORTA como salida, RA0 & RA1 como entradas 
    PORTA = 0;                  // Limpiamos PORTA 
    
    TRISD = 0;
    PORTD = 0;
    
    // Configuraci?n del oscilador
    OSCCONbits.IRCF = 0b0110;  // IRCF <2:0> -> 111 4 MHz
    OSCCONbits.SCS = 1;         // Oscilador interno

    // Configuraci?n del TMR0
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.T0SE = 0;
    OPTION_REGbits.PSA = 0;     //Un TMR0 con un Prescaler 1:256
    OPTION_REGbits.PS2 = 0;
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS0 = 0;     
    TMR0 = _tmr0_value;          // Reiniciamos el TMR0 a 217 para tener un retardo de 5ms
        
    INTCONbits.T0IE = 1;        // Habilitamos interrupciones del TMR0
    INTCONbits.T0IF = 0;        // Limpiamos bandera de interrupci?n del TMR0
        
    //COnfiguraci?n del ADC
    ADCON0bits.ADCS = 0b01;     // ADCS <1:0> -> 10 FOSC/32
    ADCON0bits.CHS = 0;    // CHS  <3:0> -> 0000 AN0
    ADCON1bits.ADFM = 0;        // Justificado a la izquierda
    ADCON1bits.VCFG0 = 0;       // Referencia en VDD
    ADCON1bits.VCFG1 = 0;       // Referencia en VSS
    
    
    // Configuraci?n del PWM
    
    TRISCbits.TRISC2 = 1;       // RC2 -> CCP1 como entrada
    TRISCbits.TRISC1 = 1;       // RC1 -> CCP2 como entrada
    PR2 = 155;                  // Periodo del TIMER2
    CCP1CONbits.P1M = 0;        // Salida simple
    CCP1CONbits.CCP1M = 0b1100; // asignaci?n del modo a PWM1
    CCP2CONbits.CCP2M = 0b1100; // asignaci?n del modo a PWM2
            
    CCPR1L = 0x0F;              // Valor inicial del duty cycle
    CCP1CONbits.DC1B = 0;       // CONFIG bits menos significativos
    
    CCPR2L = 0x0F;              // Valor inicial del duty cycle
    CCP2CONbits.DC2B0 = 0;       // CONFIG bits menos significativos
    
    // Configuraci?n del TIMER2
    PIR1bits.TMR2IF = 0;        // Flag del TIMER2 en 0    
    T2CONbits.T2CKPS = 0b11;    // Prescaler 1:16
    T2CONbits.TMR2ON = 1;       // Encendemos TIMER2
    
    while (PIR1bits.TMR2IF == 0); // Esperamos una interrupci?n del TIMER2
    PIR1bits.TMR2IF = 0;
    
    TRISCbits.TRISC2 = 0;       // RC2 -> CCP1 como salida del PWM
    TRISCbits.TRISC1 = 0;       // RC1 -> CCP2 como salida
    
    //Configuraci?n de las interrupciones
    PIE1bits.ADIE = 1;          // Habilitamos interrupciones del ADC  
    PIR1bits.ADIF = 0;          // Flag del ADC en 0
    INTCONbits.PEIE = 1;        // Habilitamos interrupciones de los puertos
    INTCONbits.GIE = 1;         // Habilitamos interrupciones globales
    
    __delay_us(50);
    ADCON0bits.ADON = 1;        // Encender ADC
    
    return;
   
}