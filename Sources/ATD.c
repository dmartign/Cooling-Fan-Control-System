#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "util.h"
#include "ATD.h"

/*
ATD0CTL4
       SRES8  |  SMP1 |  SMP0   | PRES4 | PRES3 | PRES2 | PRES1 | PRES0
	   //resolution, holds clocks, prescale N
	   
	   
ATD0CTL2
       ADPU |  AFFC | AWAIT | ETRIGLE | ETRIGP | ETRIGE | ASCIE | ASCIF
	   
       //power, fast flag clear, wait, ext trig source, interrupt enable	   
	   
ATD0CTL3
	   0   |  S8C  |  S4C | S2C | S1C | FIFO | FRZ1 | FRZ0
	   
	   //seq conv. len 0000--> 8, 1XXX --> 8, 0001, 0010,...,0111
	   //FIFO/non-FIFO, freeze
    	   
ATD0CTL5
       DJM | DSGN | SCAn | MULT | 0 | CC | CB | CA
	   
	   //select chan, MULT, SCAn, sign, right justification
	   //start converion
	   
ATD0STAT0
       SCF |                              | CC2 | CC1 | CC0
*/
       
void ATD0_init() {
    ATD0CTL2 = 0x80; //power on ATD
    delay_1ms();
    ATD0CTL3 = 1 << 5;  //4 conversions
    ATD0CTL4 = 11;     //10 bits, 2 MHz ATD clock
}

int ATD0_convertFrom(int channel){
    int v;
    int add_one_or_zero;
    channel &= 7;
    ATD0CTL5 = (1<<7) | channel;
    while(! (ATD0STAT0 & (1<<7)));    
    v = ATD0DR0 + ATD0DR1 + ATD0DR2 + ATD0DR3;
    add_one_or_zero = (v & 2) == 2;
    v /= 4;
    v += add_one_or_zero;
    return v;
}



