//==============================================================
// Sets up stuff... This next section only if not using M2
#ifndef J1850_INTERFACE
#define J1850_INTERFACE
#define J1850_PIN  3
#define J1850_INT  1
#define LED_PIN 13
#endif

#ifndef M2
#define M2 0
#endif

#ifndef _VPW_LIBRARY_
#define _VPW_LIBRARY_
// Timing for start of frame
#define SOF_TIME      200
#define SOF_DEV        18

// Timing for end of frame
#define EOF_TIME      200
#define EOF_DEV        18

// Timing for a long bit pulse
#define LONGBIT_TIME   128
#define LONGBIT_DEV     16

// Timing for short bit pulse
#define SHORTBIT_TIME   64
#define SHORTBIT_DEV    15

// timeout after 250 microsec
#define  TMR_PRELOAD (65536 - (EOF_TIME*16))

#define TMROVF_INT_OFF   TIMSK1 &= (~_BV(TOIE1)) 
#define TMROVF_INT_ON    TIMSK1 |= _BV(TOIE1) 
#define TMROVF_INT_CLR   TIFR1 &= _BV(TOV1)

// Storage, max 11 data bytes + CRC
#define BUFSIZE 13
#endif


