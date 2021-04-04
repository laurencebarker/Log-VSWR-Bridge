/////////////////////////////////////////////////////////////////////////
//
// Log VSWR Bridge Display sketch by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2020
//
// this sketch provides a VSWR bridge display
//
// the code is written for an Arduino Nano Every module
//
// configdata.h
// this file holds the code to save and load settings to/from EEprom
/////////////////////////////////////////////////////////////////////////

#ifndef __CONFIGDATA_H
#define __CONFIGDATA_H



//
// RAM storage of loaded settings
// these are loaded from EEprom after boot up
//
extern byte GDisplayPageInUse;                              // display page to start at
extern byte GDisplayScaleInUse;                             // display scale 0:2W   1: 20W   2: 200W   3: 2kW
extern bool GPeakDisplayInUse;                              // true if we are displaying peak values

//
// function to copy all config settings to EEprom
//
void CopySettingsToEEprom(void);


//
// function to load config settings from EEprom
//
void LoadSettingsFromEEprom(void);

//
// function to write new display page
//
void EEWritePage(byte Value);

//
// function to write new display scale
//
void EEWriteScale(byte Value);

//
// function to write new display average/peak
//
void EEWritePeak(bool Value);

#endif  //not defined
