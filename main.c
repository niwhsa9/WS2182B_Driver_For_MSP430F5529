#include <msp430.h> 

/* SetVcoreUp taken directly from MSP430x5xx and MSP430x6xx datasheet */
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


int main(void)
{

    volatile int i, n;


    WDTCTL = WDTPW | WDTHOLD;	                                            // Stop watchdog timer

    SetVcoreUp (0x01);
    SetVcoreUp (0x02);
    SetVcoreUp (0x03);

    __bis_SR_register(SCG0);                                                //Disable FLL
    __bis_SR_register(GIE);                                                 //Set special assembly register that enables maskable interrupts

    P5SEL |= (BIT2 + BIT3);                                                 //Set crystal ports
    UCSCTL6 &= ~XT2OFF;                                                     //Enable XT2

    UCSCTL0 = 0x000;                                                        //DCO 0 bit
    UCSCTL1 = DCORSEL_3;                                                    //Select frequency range on http://www.ti.com/lit/ds/symlink/msp430f5529.pdf page 31
    UCSCTL2 = (1);                                                          //Select 0 loop divider/2 multiplier
    UCSCTL3 |= SELREF__XT2CLK;                                              //Set FLL reference to external 2 (4 MHz resonator)
    UCSCTL4 |= (SELS__DCOCLK + SELM__DCOCLK + SELA__REFOCLK);               //Select SMCLK clock source to be DCOCLK

    P1DIR |= (BIT0 + BIT2);                                                 //irrelevant
    P2DIR |= BIT2;                                                          //Set P2.2 as out direction to monitor clock
    P2SEL |= BIT2;                                                          //Select P2.2 as SMCLK output defined in http://www.ti.com/lit/ds/symlink/msp430f5529.pdf pg 84

    __bic_SR_register(SCG0);                                                //Enable FLL
    __delay_cycles(5000000);                                                //delay for OFFIG

    UCB0CTL1 = UCSWRST;                                                     //Put UCSI in reset state for configuration
    UCB0CTL0 |= (UCSYNC + UCMST + UCMSB + UCCKPH);                          //Enable settings: SPI mode (insetad of UART) + Master + Most sig byte + UCCKPH
    P3SEL |= BIT0;                                                          //Port select
    P3DIR |= BIT0;
    UCB0CTL1 |= UCSSEL1;                                                    //Disable UCSI reset state and select SMCLK src
    UCB0CTL1 &= ~UCSWRST;


    const char high = 0xFC;                                                  //11111000
    const char low = 0xE0;                                                   //11100000

    while (1) {
        for(n = 0; n < 12; n++) {
            for(i = 0; i < 8; i++) {
                while(!(UCB0IFG & UCTXIFG));                                 //Delay until UCB0TXBUF is empty
                UCB0TXBUF = low;                                             //transmit

            }
            for(i = 0; i < 8; i++) {
                while(!(UCB0IFG & UCTXIFG));
                UCB0TXBUF = high;

            }

            for(i = 0; i < 8; i++) {
                while(!(UCB0IFG & UCTXIFG));
                UCB0TXBUF = low;

            }
        }

       __delay_cycles(1000);                                               //Latch
    }
    return 0;
}


