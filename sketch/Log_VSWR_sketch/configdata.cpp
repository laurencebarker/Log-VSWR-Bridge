/////////////////////////////////////////////////////////////////////////
//
// Log VSWR Bridge Display sketch by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2020
//
// this sketch provides a VSWR bridge display
//
// the code is written for an Arduino Nano Every module
//
// configdata.cpp
// this file holds the code to save and load settings to/from EEPROM
/////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include "globalinclude.h"

#include <EEPROM.h>

#define VEEINITPATTERN 0x6F                     // addr 0 set to this if configured
#define VPAGEADDR 1                             // address in EEPROM for display page setting
#define VSCALEADDR 2                            // address in EEPROM for display scale setting
#define VPEAKADDR 3                             // address in EEPROM for display scale setting

byte GDisplayPageInUse;                         // display page to start at
byte GDisplayScaleInUse;                        // display scale 0:2W   1: 20W   2: 200W   3: 2kW
bool GPeakDisplayInUse;                         // true if we are displaying peak values



//
// function to copy all config settings to EEprom
// this copies the current RAM vaiables to the persistent storage
// addr 0: defined pattern (to know EEPROM has been initialised)
// addr 1: display page
// addr 2: display scale
//
void CopySettingsToEEprom(void)
{
  int Addr=1;
  byte Setting;
  int Cntr;
  
//
// first set that we have initialised the EEprom
//
  EEPROM.write(0, VEEINITPATTERN);
//
// now copy settings from RAM data
//
  Setting = (byte) GDisplayPageInUse;
  EEPROM.write(Addr++, Setting);
  Setting = (byte) GDisplayScaleInUse;
  EEPROM.write(Addr++, Setting);
  Setting = (byte) GPeakDisplayInUse;
  EEPROM.write(Addr++, Setting);
}



//
// function to copy initial settings to EEprom
// this sets the factory defaults
// the settings here should match the fornt panel legend!
//
void InitialiseEEprom(void)
{
  int Cntr;
  
  GDisplayPageInUse = 1;                        // crossed needles
  GDisplayScaleInUse = 0;                       // 2W
  GPeakDisplayInUse = false;                    // average mode 
// now copy them to FLASH
  CopySettingsToEEprom();
}



//
// function to load config settings from flash
//
void LoadSettingsFromEEprom(void)
{
  int Addr=1;
  byte Setting;
  int Cntr;

//
// first see if we have initialised the EEprom previously
// if not, copy settings to it
//
  Setting = EEPROM.read(0);
  if (Setting != VEEINITPATTERN)
    InitialiseEEprom();
//
// now copy out settings to RAM data
//
  GDisplayPageInUse = (byte)EEPROM.read(Addr++);
  GDisplayScaleInUse = (byte)EEPROM.read(Addr++);
  GPeakDisplayInUse = (bool)EEPROM.read(Addr++);
}



//
// function to write new display page
//
void EEWritePage(byte Value)
{
  GDisplayPageInUse = Value;
  EEPROM.write(VPAGEADDR, Value);
}


//
// function to write new display scale
//
void EEWriteScale(byte Value)
{
  GDisplayScaleInUse = Value;
  EEPROM.write(VSCALEADDR, Value);
}


//
// function to write new display scale
//
void EEWritePeak(bool Value)
{
  GPeakDisplayInUse = Value;
  EEPROM.write(VPEAKADDR, Value);
}
