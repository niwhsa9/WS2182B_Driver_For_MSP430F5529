#include <msp430.h> 

const int size = 12;
const char high = 0xFC;                                                    //11111000
const char low = 0xE0;                                                     //11100000
volatile int i, n;
//unsigned char g[size];
//unsigned char r[size];
//unsigned char b[size];
const int fsize = size * 3;
unsigned char raw[fsize];



                                                                           // SetVcoreUp taken directly from MSP430x5xx datasheet
void SetVcoreUp (unsigned int level)
{
  PMMCTL0_H = PMMPW_H;
  SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
  SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
  while ((PMMIFG & SVSMLDLYIFG) == 0);
  PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
  PMMCTL0_L = PMMCOREV0 * level;
  if ((PMMIFG & SVMLIFG))
    while ((PMMIFG & SVMLVLRIFG) == 0);
  SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
  PMMCTL0_H = 0x00;
}

void setClock() {
    SetVcoreUp (0x01);
    SetVcoreUp (0x02);
    SetVcoreUp (0x03);
    __bis_SR_register(SCG0);                                                //Disable FLL
    P5SEL |= (BIT2 + BIT3);                                                 //Set crystal ports
    UCSCTL6 &= ~XT2OFF;                                                     //Enable XT2
    UCSCTL0 = 0x000;                                                        //DCO 0 bit
    UCSCTL1 = DCORSEL_3;                                                    //Select frequency range
    UCSCTL2 = (1);                                                          //Select 0 loop divider/2 multiplier
    UCSCTL3 |= SELREF__XT2CLK;                                              //Set FLL reference to external 2 (4 MHz resonator)
    UCSCTL4 |= (SELS__DCOCLK + SELM__DCOCLK + SELA__REFOCLK);               //Select SMCLK clock source to be DCOCLK
    P1DIR |= (BIT0 + BIT2);                                                 //irrelevant
    P2DIR |= BIT2;                                                          //Set P2.2 as out direction to monitor clock
    P2SEL |= BIT2;                                                          //Select P2.2 as SMCLK output
    __bic_SR_register(SCG0);                                                //Enable FLL
}

void configureSPI() {
    __bis_SR_register(GIE);                                                 //Set special assembly register that enables maskable interrupts
    UCB0CTL1 = UCSWRST;                                                     //Put UCSI in reset state for configuration
    UCB0CTL0 |= (UCSYNC + UCMST + UCMSB + UCCKPH);                          //Enable settings: SPI mode (insetad of UART) + Master + Most sig byte + UCCKPH
    P3SEL |= BIT0;                                                          //Port select
    P3DIR |= BIT0;
    UCB0CTL1 |= UCSSEL1;                                                    //Disable UCSI reset state and select SMCLK src
    UCB0CTL1 &= ~UCSWRST;
}

void latchStrip() {
  //  raw[0] = 0; //DEBUG
    unsigned char curBit;
    unsigned char out;
    for(i = 0; i < fsize; i++) {
        curBit = 0x80;
        while(curBit) {
            if(curBit & raw[i]) {
                out = high;
            }
            else out = low;
            while(!(UCB0IFG & UCTXIFG));
            UCB0TXBUF = out;
            curBit = curBit >> 1;
        }
    }
    __delay_cycles(1000);
}

void setColor(unsigned int index, unsigned char red, unsigned char green, unsigned char blu) {
    raw[index * 3] = green;
    raw[index * 3 + 1] = red;
    raw[index * 3 + 2] = blu;
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	                                            // Stop watchdog timer
    setClock();
    configureSPI();
    for(i = 0; i < fsize; i++) raw[i] = 0;
    __delay_cycles(5000000);
    setColor(0, 255, 0, 0);
    setColor(1, 255, 0, 0);
    setColor(2, 0, 0 , 255);
    latchStrip();
    while(1) {
        //latchStrip();
    };
    return 0;
}


