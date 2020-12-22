/////////////////////////////////////////////////////////////////////////
//
// Log VSWR Bridge Display sketch by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2020
//
// this sketch provides a VSWR bridge display
//
// the code is written for an Arduino Nano Every module
//
// display.cpp
// this file holds the code to control a Nextion 2.4" display
// it is 320x240 pixels
/////////////////////////////////////////////////////////////////////////

#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <Arduino.h>


//
// this type enumerates the Nextion display pages:
//
enum EDisplayPage
{
  eSplashPage,                              // startup splash page
  eCrossedNeedlePage,                       // crossed needle VSWR page display
  ePowerBargraphPage,                       // linear watts bargraph page display
  eLogBargraphPage,                         // dBm bargraph page display
  eMeterPage,                               // analogue power meter
  eEngineeringPage                          // engineering page with raw ADC values
};



//
// display initialise
//
void DisplayInit(void);


//
// display tick
//
void DisplayTick(void);




#endif //#ifndef
