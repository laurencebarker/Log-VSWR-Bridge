/////////////////////////////////////////////////////////////////////////
//
// Log VSWR Bridge Display sketch by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2020
//
// this sketch provides a VSWR bridge display
//
// the code is written for an Arduino Nano Every module
//
// analogueio.h
// this file holds the code to read the analogue inputs
/////////////////////////////////////////////////////////////////////////

#ifndef __ANALOGUEIO_H
#define __ANALOGUEIO_H


//
// externally accessible globals:
//
extern int GForwardTenthdBm;
extern int GReverseTenthdBm;
extern unsigned int GFwdLineVoltageTenth;
extern unsigned int GRevLineVoltageTenth;
extern unsigned int GVSWR;                               // 1 decimal place. set to 9999 if impossible




//
// AnalogueIO initialise
//
void AnalogueIOInit(void);



//
// fast tick: read one ADC value
//
void AnalogueIOFastTick();



//
// AnalogueIO tick
// read the ADC values
//
void AnalogueIOTick(void);


//
// find peak power by searching buffer
// returns a power peak value
// 1st parameter true for forward power
// 2nd paramter true for units of tenths of a watt
//
unsigned int FindPeakPower(bool IsFwdPower, bool InTenths);

//
// returns a power value
// 1st parameter true for forward power
// 2nd paramter true for units of tenths of a watt
//
unsigned int GetPowerReading(bool IsFwdPower, bool InTenths);


#endif      // file sentry
