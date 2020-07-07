/*File:   TC74_BCD.c
 * Author: Joshua Harrell
 * University of North Florida
 * Created on May 13, 2020, 2:51 AM
 */

// CONFIG1L
#pragma config CPUDIV = NOCLKDIV// CPU System Clock Selection bits (No CPU System Clock divide)
#pragma config USBDIV = OFF     // USB Clock Selection bit (USB clock comes directly from the OSC1/OSC2 oscillator block; no divide)

// CONFIG1H
#pragma config FOSC = IRC       // Oscillator Selection bits (Internal RC oscillator)
#pragma config PLLEN = OFF      // 4 X PLL Enable bit (PLL is under software control)
#pragma config PCLKEN = ON      // Primary Clock Enable bit (Primary clock enabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRTEN = ON      // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 30        // Brown-out Reset Voltage bits (VBOR set to 3.0 V nominal)

// CONFIG2H
#pragma config WDTEN = OFF      // Watchdog Timer Enable bit (WDT is controlled by SWDTEN bit of the WDTCON register)
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config HFOFST = OFF     // HFINTOSC Fast Start-up bit (The system clock is held off until the HFINTOSC is stable.)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RA3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config BBSIZ = ON       // Boot Block Size Select bit (2kW boot block size)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Table Write Protection bit (Block 0 not write-protected)
#pragma config WRT1 = OFF       // Table Write Protection bit (Block 1 not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block not protected from table reads executed in other blocks)

#include <xc.h>
/*Set Fosc to 4Mhz*/
#define _XTAL_FREQ 4000000  // Page 314 of XC8 compiler User Guide

/*Assign Shift Register pins*/
#define shift1_data LATCbits.LC5
#define shift2_data LATCbits.LC4
#define shift1_latch LATCbits.LC3
#define shift2_latch LATCbits.LC6
#define shift1_clock LATCbits.LC0
#define shift2_clock LATCbits.LC1
#define shift_reset LATCbits.LC2

int temp;

/*Device Subroutines*/
void Initialize();
void get_Temp();
unsigned char encode_Temp(int x);
void shift_Temp(unsigned number, int SIPO);

/*I2C Subroutines*/
void start_bit();
void stop_bit();
void write_byte(unsigned x);
void I2C_wait();
int read_byte();

void main(void) {
    __delay_ms(300);
    Initialize();
    while(1){
    get_Temp();
    int temp_f = (temp * 9/5) + 32;
    int tens = temp_f / 10;   // Tens place value determined
    int ones = temp_f % 10;   // Ones place value determined
    unsigned char BCD1 = encode_Temp(tens);
    int SIPO = 1;
    shift_Temp(BCD1, SIPO);
    unsigned char BCD2 = encode_Temp(ones);
    SIPO = 2;
    shift_Temp(BCD2, SIPO);
    __delay_ms(1000);
    }
}

void Initialize(){
    OSCCON = 0x58; //Turn on 4MHz clock
    TRISCbits.RC0 = 0; // Set Port C pin 0 as an output  
    TRISCbits.RC1 = 0; // Set Port C pin 1 as an output
    TRISCbits.RC2 = 0; // Set Port C pin 2 as an output
    TRISCbits.RC3 = 0; // Set Port C pin 3 as an output
    TRISCbits.RC4 = 0; // Set Port C pin 4 as an output
    TRISCbits.RC5 = 0; // Set Port C pin 5 as an output
    TRISCbits.RC6 = 0; // Set Port C pin 6 as an output
    SSPCON1 = 0x28; //Set up I2C bus as master mode and enabling I2C bus
    SSPCON2 = 0x00; // No changes needed
    SSPADD = 0x09; // Set clock rate to 100kHz
    SSPSTAT = 0x00; // No changes needed
    shift_reset = 0;
    __delay_ms(50);
    shift_reset = 1;
}

/*Start condition for I2C*/
void start_bit(){
    I2C_wait();
    SEN = 1;  //set start bit
}

/*Stop condition for I2C*/
void stop_bit() {
    I2C_wait();
    PEN = 1;  //set stop bit
}

/*Writes a byte to TC74*/
void write_byte(unsigned x) {
    I2C_wait();
    SSPBUF = x;
}

/*Reads temperature sent from TC74*/
int read_byte() {
    I2C_wait();
    int x;
    RCEN = 1;
    I2C_wait();
    x = SSPBUF;
    I2C_wait();
    ACKDT = 0;              //Set the Acknowledge data bit as a NACK, per the TC74 datasheet
    ACKEN = 1;              //enable the acknowledge sequence
    return x;
}

/*Hold I2C bus*/
void I2C_wait(){
    while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F)); //wait for I2C bus to be ready
}

void get_Temp(){
    start_bit();
    write_byte(0x90);
    write_byte(0x00);
    start_bit();
    write_byte(0x91);
    temp = read_byte();
    stop_bit();
}

unsigned char encode_Temp(int x) {
  unsigned char sequence;
  switch(x){
    case 1:
      sequence = 0x60;
      break;
    case 2:
      sequence = 0xDA;
      break;
    case 3:
      sequence = 0xF2;
      break;
    case 4:
      sequence = 0x66;
      break;
    case 5:
      sequence = 0xB6;
      break;
    case 6:
      sequence = 0xBE;
      break;
    case 7:
      sequence = 0xE0;
      break;
    case 8:
      sequence = 0xFE;
      break;
    case 9:
      sequence = 0xF6;
    default:
      sequence = 0xFC;
      break;
  }
  return sequence;
}

void shift_Temp(unsigned number, int SIPO) {
  if (SIPO == 1) {
    shift1_latch = 0;
    shift1_clock = 0;
    shift1_data = ((number >> 7) & 0x01);
    shift1_clock = 1;
    shift1_clock = 0;
    shift1_data = ((number >> 6) & 0x01);
    shift1_clock = 1;
    shift1_clock = 0;
    shift1_data = ((number >> 5) & 0x01);
    shift1_clock = 1;
    shift1_clock = 0;
    shift1_data = ((number >> 4) & 0x01);
    shift1_clock = 1;
    shift1_clock = 0;
    shift1_data = ((number >> 3) & 0x01);
    shift1_clock = 1;
    shift1_clock = 0;
    shift1_data = ((number >> 2) & 0x01);
    shift1_clock = 1;
    shift1_clock = 0;
    shift1_data = ((number >> 1) & 0x01);
    shift1_clock = 1;
    shift1_clock = 0;
    shift1_data = (number & 0x01);
    shift1_clock = 1;
    __delay_ms(20);
    shift1_latch = 1;
  }
  if (SIPO == 2) {
    shift2_latch = 0;
    shift2_clock = 0;
    shift2_data = ((number >> 7) & 0x01);
    shift2_clock = 1;
    shift2_clock = 0;
    shift2_data = ((number >> 6) & 0x01);
    shift2_clock = 1;
    shift2_clock = 0;
    shift2_data = ((number >> 5) & 0x01);
    shift2_clock = 1;
    shift2_clock = 0;
    shift2_data = ((number >> 4) & 0x01);
    shift2_clock = 1;
    shift2_clock = 0;
    shift2_data = ((number >> 3) & 0x01);
    shift2_clock = 1;
    shift2_clock = 0;
    shift2_data = ((number >> 2) & 0x01);
    shift2_clock = 1;
    shift2_clock = 0;
    shift2_data = ((number >> 1) & 0x01);
    shift2_clock = 1;
    shift2_clock = 0;
    shift2_data = (number & 0x01);
    shift2_clock = 1;
    __delay_ms(20);
    shift2_latch = 1;
  }
}