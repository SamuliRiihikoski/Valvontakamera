#include <msp430.h> 

#define RESX 320
#define RESY 320

// 0x00 starts new image. 0x00 -> startX startY endX endY, (0 0 0 0 = default (320 x 320))

void newImage(int resX, int resY);
void sendData(unsigned int count);

int image = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    UCA1CTLW0 |= UCSWRST;
    UCA1CTLW0 |= UCSSEL__SMCLK;
    UCA1BRW |= 8;
    UCA1MCTLW |= 0xD600;

    P4SEL1 &= ~BIT3;
    P4SEL0 |= BIT3;

    P4DIR &= ~BIT1;
    P4REN |= BIT1;
    P4OUT |= BIT1;
    P4IES |= BIT1;

    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    PM5CTL0 &= ~LOCKLPM5;

    UCA1CTLW0 &= ~UCSWRST;

    P4IFG &= ~BIT1;
    P4IE |= BIT1;
    __enable_interrupt();

    int i;
    int count = 1;

    while(1)
    {
        if (image) {
            for (i = 0; i < 1000; i++) {}
            newImage(RESX, RESY);
            for (i = 0; i < 1000; i++) {}
            image = 0;
        }
        sendData(count);
        count++;
        if (count == 245) count = 1;
    }

    return 0;
}

void newImage(int resX, int resY)
{
    unsigned int count = 255;
    int i;
    for (i = 0; i < 30000; i++) {} // without misses first flags in serial

    UCA1TXBUF = 0; // new image flag
    for (i = 0; i < 50; i++) {}

    if (resX == 320 && resY == 320)     // sends => 0 0 0 0 == 320x320
    {
        UCA1TXBUF = 0;                  // StartX ( 0 x 32 = 0)
        for (i = 0; i < 50; i++) {}
        UCA1TXBUF = 3;                  // StartY
        for (i = 0; i < 50; i++) {}
        UCA1TXBUF = 7;                 // EndX ( 10 x 32 = 320)
        for (i = 0; i < 50; i++) {}
        UCA1TXBUF = 5;                 // EndY
        for (i = 0; i < 50; i++) {}
    }


}

void sendData(unsigned int count)
{
    int i;
    UCA1TXBUF = count;
    for (i = 0; i < 500; i++) {}
}

#pragma vector = PORT4_VECTOR
__interrupt void ISR_Port4_S1(void)
{
    //P1OUT ^= BIT0;
    P4IFG &= ~BIT1;
    image = 1;


}

