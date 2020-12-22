/////////////////////////////////////////////////////////////////////////
//
// Log VSWR Bridge Display sketch by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2020
//
// this sketch provides a VSWR bridge display
//
// the code is written for an Arduino Nano Every module
//
// "main" file with setup() and loop()
/////////////////////////////////////////////////////////////////////////


#include "iopins.h"
#include "display.h"
#include "analogueio.h"
#include "configdata.h"


#define VSLOWTICKCOUNT 20                     // 20 1ms fast ticks to get one slow tick.
#define VLEDBLINKHALFPERIOD 25                // slow ticks to update LED. This is half the period in slow ticks


//
// global variables
//
// for heartbeat LED:
bool ledOn = false;
byte GSlowTickCounter = VSLOWTICKCOUNT;
bool GSlowTickTriggered;                  // true if a 20ms "slow" tick has been triggered
byte TickLEDCounter = VLEDBLINKHALFPERIOD;


//
// counter clocked by CK/64 (4us)
//
void SetupTimerForInterrupt(int Milliseconds)
{
  int Count;

  Count = 250* Milliseconds;                // temp value - not sure why not 250!
  TCB0.CTRLB = TCB_CNTMODE_INT_gc; // Use timer compare mode  
  TCB0.CCMP = Count; // Value to compare with. This is 1/5th of the tick rate, so 5 Hz
  TCB0.INTCTRL = TCB_CAPT_bm; // Enable the interrupt
  TCB0.CTRLA = TCB_CLKSEL_CLKTCA_gc | TCB_ENABLE_bm; // Use Timer A as clock, enable timer
}



void setup()
{
  ConfigIOPins();
  Serial.begin(9600);
//
// initialise timer to give 1ms tick interrupt
// will count a slower period for main code
//
  SetupTimerForInterrupt(1);                                      // 1ms tick
//
// check that the flash is programmed, then load to RAM
//  
  LoadSettingsFromEEprom();
  
  DisplayInit();
  AnalogueIOInit();
}


//
// periodic timer tick handler.
// set to 1ms
//
ISR(TCB0_INT_vect)
{
   // Clear interrupt flag
  TCB0.INTFLAGS = TCB_CAPT_bm;

  AnalogueIOFastTick();
  if(--GSlowTickCounter == 0)
  {
    GSlowTickTriggered = true;
    GSlowTickCounter = VSLOWTICKCOUNT;  
  }
}


//
// sits and waits for time tick to have happened
// 20ms "slow" tick
//
void loop() 
{
  while (GSlowTickTriggered)
  {
    GSlowTickTriggered = false;
// heartbeat LED
    if (--TickLEDCounter == 0)
    {
      TickLEDCounter=VLEDBLINKHALFPERIOD;
      ledOn = !ledOn;
      if (ledOn)
        digitalWrite(LED_BUILTIN, HIGH); // Led on, off, on, off...
       else
        digitalWrite(LED_BUILTIN, LOW);
    }

//
// get analogue values
//
    AnalogueIOTick();
//
// display update
//
    DisplayTick();
      
  }
}


//
// set pinmode for all I/O pins used
// and write any output initial states
//
void ConfigIOPins(void)
{
  pinMode(LED_BUILTIN, OUTPUT);                         // LED output
}
