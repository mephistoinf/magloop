#ifndef _MAGLOOPTX
#define _MAGLOOPTX

#ifdef _MAGLOOPRX
#error Cannot implement receiver and transmitter on same device! only include magloop_rx.h _or_ magloop_tx.h
#endif

//**************************************************************************************
//  5KHz ASK transmitter for magnetic loop communications
//  Runs on ATMEGA88, ATMEGA168 and ATMEGA328P based boards, possiply others too,
//  Dzl 2013
//**************************************************************************************

#include <avr/signal.h>

#define SET(x,y) (x |=(1<<y))
#define CLR(x,y) (x &= (~(1<<y)))
#define CHK(x,y) (x & (1<<y))
#define TOG(x,y) (x^=(1<<y))

#define N_BIT 120    //-Nbr. of cycles per bit

//---------------------------------
//  Signal generator
//---------------------------------

volatile unsigned char txdone=true;
volatile unsigned char txstate=0;
volatile unsigned char txdata=0;
volatile unsigned char idle=true;

volatile unsigned char ncount=N_BIT-1;

SIGNAL(TIMER1_COMPA_vect)
{
  SET(TIFR1,OCF1A);			//-Retrigger INT
  if(ncount)
    ncount--;
  else
  {
    ncount=N_BIT-1;
    switch(txstate)
    {
    case 0:
      if(idle)
        break;
        TCCR1A=0b00000010;        //-Set up frequency generator
//      CLR(TCCR1A,7);
      txstate=1;
      break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
      if(txdata&0x01)
    TCCR1A=0b10110010;        //-Set up frequency generator
//        SET(TCCR1A,7);
      else
    TCCR1A=0b00000010;        //-Set up frequency generator
//        CLR(TCCR1A,7);
      txdata>>=1;
      txstate++;
      break;
    case 9:
    TCCR1A=0b10110010;        //-Set up frequency generator
//      SET(TCCR1A,7);
      txstate++;
      break;

    case 10:
      txstate=0;
      txdone=true;
      idle=true;
      break;
    }
  }
}

//---------------------------------
//  Bit macros
//---------------------------------

#define SET(x,y) (x |=(1<<y))
#define CLR(x,y) (x &= (~(1<<y)))
#define CHK(x,y) (x & (1<<y))
#define TOG(x,y) (x^=(1<<y))

class TMagLoopTX
{
public:

  void begin()
  {
    TCCR1A=0b10110010;        //-Set up frequency generator
    TCCR1B=0b00011001;        //-+

    TCCR1B&=0xfe;             //-Stop generator
    TCNT1=0;                  //-Clear timer
    ICR1=3200;                // |
    OCR1A=(3200/2);           //-+
    OCR1B=(3200/2);           //-+
    TCCR1B|=0x01;             //-Restart generator
    SET(TIMSK1,OCIE1A);
    SET(DDRB,1);
    SET(DDRB,2);
    SET(TCCR1A,7);

  }
  unsigned char ready()
  {
    if(idle)
      return true;
    return false;
  }

  void write(unsigned char c)
  {
    txdata=c;
    idle=false;
  }

  void mute()
  {
    TCCR1A=0b00000010;        //-Set up frequency generator
    TCCR1B&=0xfe;             //-Stop generator
  }

  void resume()
  {
    TCCR1A=0b10110010;        //-Set up frequency generator
    TCCR1B|=0x01;             //-Restart generator
  }
};

TMagLoopTX magLoopTX;

#endif

