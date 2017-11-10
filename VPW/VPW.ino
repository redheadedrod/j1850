//Force loading of M2 stuff - This will be changed
#define _M2_ 
//==============================================================
#ifdef _M2_
#include "M2.h"
#endif
#include "VPW.h"

// forward decl
void j1850_interrupt(void);

volatile boolean idle = true;

volatile uint8_t msgbuf[BUFSIZE];
volatile uint8_t msgLen;


//
// Initialization
//
void setup(void) 
{ 
  if (M2 == 1) {
  pinMode(PS_J1850_9141, OUTPUT);
  pinMode(PWM_nVPW, OUTPUT);
  digitalWrite(PS_J1850_9141,HIGH);
  digitalWrite(PWM_nVPW, LOW);
  }
  pinMode(J1850_PIN, INPUT);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Serial.begin(115200);
  delay(2000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
  Serial.println(F("j1850decoder/v2 including M2 support"));

  TMROVF_INT_OFF;
  TCCR1A = 0;
  TCNT1 = TMR_PRELOAD;  // preload timer 65536-16MHz/256/2Hz
  TCCR1B = _BV(CS10);  // no prescaler, start timer 

  idle = true;
  msgLen = 0;
  attachInterrupt(J1850_INT, j1850_interrupt, CHANGE);
  interrupts();
}


//
// Background loop - print message when available
//
void loop(void)
{
  if (msgLen > 0) {
    Serial.print(F("> "));
    for (int i = 0; i < msgLen; i++) {
          if ((i == 3) || (i == msgLen -1)){
        Serial.print("   ");
      }
      if (msgbuf[i] < 16) Serial.print("0");
      Serial.print(msgbuf[i], HEX);
    }
    Serial.println();
    msgLen = 0;
  }
}

//
// Interrupt routine for changes on j1850 data pin
//

volatile unsigned long lastInt = 0;
volatile uint8_t bitCnt;
volatile long delta;
volatile unsigned long tstamp;
volatile uint8_t aByte;
volatile uint8_t buf[BUFSIZE];
volatile uint8_t bufIdx;


void j1850_interrupt(void)
{
  tstamp = micros();

  uint8_t pin = digitalRead(J1850_PIN);

  // reload the overflow timer with EOF timeout
  TCNT1 = TMR_PRELOAD;           

  delta = tstamp - lastInt;
  long longbit, shortbit;

  if (idle)
  {
    if (pin == 0) 
    {
      longbit = delta - SOF_TIME;
      if (abs(longbit) < SOF_DEV)
      {
        // found SOF, start header/data sampling
        idle = false;
        bitCnt = 0;
        bufIdx = 0;
        aByte = 0;
//        digitalWrite(LED_PIN, LOW);
      }
    }
  } 
  else
  {
    shortbit = delta - SHORTBIT_TIME;
    longbit = delta - LONGBIT_TIME;

    if (abs(shortbit) < SHORTBIT_DEV) {
      // short pulse
      if (pin == 0)
        // short pulse & pulse was high => active "1"
        aByte = (aByte << 1) | 0x01;
      else
        // short pulse & pulse was low => passive "0"
        aByte = (aByte << 1) & 0xFE;
      bitCnt++;

    } 
    else if (abs(longbit) < LONGBIT_DEV) {
      // long pulse
      if (pin == 0)
        // long pulse & pulse was high => active "0"
        aByte = (aByte << 1) & 0xFE;
      else
        // long pulse & pulse was low => passive "1"
        aByte = (aByte << 1) | 0x01;
      bitCnt++;

    } 
    else {
      // unknown bit, reset
      TMROVF_INT_OFF; 
      idle = true;
      lastInt = tstamp;
      return;
    }

    if (bitCnt >= 8) {

      buf[bufIdx++] = aByte;
      bitCnt = 0;
      if (bufIdx >= sizeof(buf)) {
        // too many data bytes, error
        TMROVF_INT_OFF; 
        idle = true;
      } 
      else {
        // if all is ok, start the EOF timeout
        TMROVF_INT_CLR; 
        TMROVF_INT_ON; 
      }
    }
  }
  lastInt = tstamp;
}

// Timer overlflow interrupt
// Occurs when the EOF pulse times out the timer
//
ISR(TIMER1_OVF_vect)  
{
  TCNT1 = TMR_PRELOAD;  
  TMROVF_INT_OFF; 
//  digitalWrite(LED_PIN, HIGH);

  // copy the data so that we can start to fill the buffer again
  // but only if the buffer has been consumed in the background
  if (bufIdx > 0 && msgLen == 0)
  {
    memcpy((void*)msgbuf, (const void*)buf, bufIdx);
    msgLen = bufIdx;
  }
  idle = true;
}
