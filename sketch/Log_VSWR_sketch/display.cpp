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

#include <Arduino.h>
#include "display.h"
#include "stdio.h"
#include "globalinclude.h"
#include "analogueio.h"
#include "configdata.h"
#include "iopins.h"
#include <Nextion.h>                        // uses the Nextion class library


//
// display full scale values for power graphs
// done this way it is easy to add more!
//
#define VMAXSCALESETTING 3
const unsigned int GPowerFullScale[VMAXSCALESETTING + 1]=
{
  2,
  20,
  200, 
  2000
};


//
// picture id values for nextion display bargraphs
// these ID values must match the image ID values in the editor!
//
const unsigned int GPowerForeground[VMAXSCALESETTING + 1] =
{
  8,            // 2W display
  10,           // 20W display
  12,           // 200W display
  14            // 2KW display
};

const unsigned int GPowerBackground[VMAXSCALESETTING + 1] =
{
  7,            // 2W display
  9,            // 20W display
  11,           // 200W display
  13            // 2KW display
};


// picture ID values for the background image for the meter display
const unsigned int GMeterPicture[VMAXSCALESETTING + 1] =
{
  17,           // 2W display
  18,           // 20W display
  19,           // 200W display
  20            // 2KW display
};



// picture ID values for the background image for the crossed needle display
const unsigned int GCrossedNeedlePicture[VMAXSCALESETTING + 1] =
{
  21,           // 2W display
  0,            // 20W display
  1,            // 200W display
  22            // 2KW display
};


//
// define colours for  Nextion display
//
#define NEXBLACK 0L
#define NEXWHITE 65535L
#define NEXRED 63488L
#define NEXGREEN 2016L
#define NEXBLUE 31L

#define VHALFSECOND 25                        // 50 ticks per half second
#define VTENTHSECOND 5                        // 10 ticks per tenth of a second
#define VFIVESECONDS 250                      // 500 ticks for 5 seconds

#define VLOGBARMIN -350.0F                    // units tenths of dB
#define VLOGBARMAX 650.0F
#define VVSWRFULLSCALE 10.0F                  // full scale VSWR indication
#define VOVERSCALEDISPLAYTICKS 5             // duration to display an overscale for (units currently 100ms)

//
// paramters for crossed needle display
//
#define VNEEDLESIZE 234.0
#define VMINXNEEDLEANGLE 13.5F                // angle for 0W
#define VMAXXNEEDLEANGLE 73.0F                // angle for full scale power
#define VXNEEDLEY1 239                        // y start position (px)
#define VXNEEDLEFWDX1 243                     // X needle start position (px)
#define VXNEEDLEREVX1 35                     // X needle start position (px)


EDisplayPage GDisplayPage;                    // global set to current display page number
int GSplashCountdown;                         // counter for splash page
byte GUpdateItem;                             // display item being updated
byte GCrossedNeedleItem;                      // display item in crossed needle page
int GDisplayedForward, GDisplayedReverse;     // displayed meter angle values, to find if needle has moved
int GReqdAngleForward, GReqdAngleReverse;     // required angles for crossed needle
byte GForwardOverscale, GReverseOverscale;    // set non zero if an overscale detected. =no. ticks to display for
bool GInitialisePage;                         // true if page needs to be initialised
bool GCrossedNeedleRedrawing;                 // true if display is being redrawn
unsigned char GUpdateMeterTicks;              // number of ticks since a meter display updated

//
// declare pages:
//
NexPage page0 = NexPage(0, 0, "page0");       // creates touch event for "splash" page
NexPage page1 = NexPage(1, 0, "page1");       // creates touch event for "crossed needle" page
NexPage page2 = NexPage(2, 0, "page2");       // creates touch event for "power bargraph" page
NexPage page3 = NexPage(3, 0, "page3");       // creates touch event for "dBm bargraph" page
NexPage page4 = NexPage(4, 0, "page4");       // creates touch event for "analogue meter" page
NexPage page5 = NexPage(5, 0, "page5");       // creates touch event for "engineering" page

//
// page 0 objects:
// 
NexText p0SWVersion = NexText(0, 5, "p0t4");                   // software version

//
// page 1 objects:
// 
NexButton p1ScaleBtn = NexButton(1, 3, "p1b1");                   // Scale pushbutton
NexPicture p1Axes = NexPicture(1, 1, "p1p0");                     // graph axes
NexButton p1DisplayBtn = NexButton(1, 2, "p1b0");                 // Display pushbutton
NexDSButton p1PeakBtn = NexDSButton(1, 4, "p1bt0");               // normal/peak button

//
// page 2 objects:
// 
NexButton p2DisplayBtn = NexButton(2, 1, "p2b0");                 // Display pushbutton
NexText p2FwdPower = NexText(2,5, "p2t2");                        // Forward power (W)
NexText p2VSWRTxt = NexText(2,11, "p2t3");                        // VSWR
NexProgressBar p2FwdBar = NexProgressBar(2, 3, "p2j0");           // forward power bar
NexProgressBar p2VSWRBar = NexProgressBar(2, 7, "p2j1");          // VSWR bar
NexButton p2ScaleBtn = NexButton(2, 9, "p2b1");                   // power scale pushbutton
NexDSButton p2Overrange = NexDSButton(2, 6, "p2bt0");             // overscale indicator
NexDSButton p2PeakBtn = NexDSButton(2, 10, "p2bt1");              // normal/peak button

//
// page 3 objects:
// 
NexText p3FwddBm = NexText(3,6, "p3t5");                          // forward power (dBm)
NexText p3RevdBm = NexText(3,7, "p3t6");                          // reverse power (dBm)
NexProgressBar p3FwddBmBar = NexProgressBar(3, 8, "p3j0");        // forward power bar
NexProgressBar p3RevdBmBar = NexProgressBar(3, 9, "p3j1");        // reverse power bar
NexButton p3DisplayBtn = NexButton(3, 1, "p3b0");                 // Display pushbutton

//
// page 4 objects:
// 
NexButton p4DisplayBtn = NexButton(4, 3, "p4b0");                 // Display pushbutton
NexButton p4ScaleBtn = NexButton(4, 4, "p4b1");                   // power scale pushbutton
NexDSButton p4PeakBtn = NexDSButton(4, 5, "p4bt1");               // normal/peak button
NexGauge p4Meter = NexGauge(4, 1, "p4z0");                        // power meter
NexProgressBar p4VSWRBar = NexProgressBar(4, 2, "p4j0");          // VSWR bar

//
// page 5 objects:
// 
NexText p5FwdVolts = NexText(5, 8, "p5t6");                     // forward voltage
NexText p5RevVolts = NexText(5, 9, "p5t7");                     // reverse voltage
NexText p5FwddBm = NexText(5, 19, "p5t17");                     // forward dBm
NexText p5RevdBm = NexText(5, 20, "p5t18");                     // reverse dBm
NexText p5FwdPower = NexText(5, 11, "p5t9");                    // forward power
NexText p5RevPower = NexText(5, 12, "p5t10");                   // reverse power
NexText p5FwdPeak = NexText(5, 14, "p5t12");                    // forward peak power
NexText p5RevPeak = NexText(5, 15, "p5t13");                    // reverse peak power
NexText p5VSWR = NexText(5, 17, "p5t15");                       // VSWR
NexButton p5DisplayBtn = NexButton(5, 1, "p5b0");                 // Display pushbutton

#define VMAXENGITEM 9                         // number of text items on engineering display


//
// declare touch event objects to the touch event list
// this tells the code what touch events too look for
//
NexTouch *nex_listen_list[] = 
{
  &p1ScaleBtn,                                // button pressed
  &p2ScaleBtn,                                // button pressed
  &p4ScaleBtn,                                // button pressed
  &p1PeakBtn,                                 // button pressed
  &p2PeakBtn,                                 // button pressed
  &p4PeakBtn,                                 // button pressed
  &p1DisplayBtn,                              // display button pressed
  &p2DisplayBtn,                              // display button pressed
  &p3DisplayBtn,                              // display button pressed
  &p4DisplayBtn,                              // display button pressed
  &p5DisplayBtn,                              // display button pressed
  NULL                                        // terminates the list
};



//
// convert from power value to degree angle for the cross needle meter
// return integer angle from 13.5 to 90
//
int GetCrossedNeedleDegrees(bool IsForward, bool IsPeak)
{
  float FullScale;
  unsigned int Power;
  float Degrees;
  
  FullScale = (float)GPowerFullScale[GDisplayScaleInUse];
  if(!IsForward)
    FullScale *= 0.2;                                     // reverse scale = a fifth of forward

  if(IsPeak)
    Power = FindPeakPower(IsForward, true);               // get power in units of 0.1 watts
  else
    Power = GetPowerReading(IsForward, true);

//
// calculate angle. Not full scale ~73 degrees but we allow up to 90 degrees
// not the power reading is in tenths of watts so scale to watts too.
//
  Degrees = VMINXNEEDLEANGLE + (VMAXXNEEDLEANGLE - VMINXNEEDLEANGLE)* 0.1 * (float)Power / FullScale;
  if (Degrees > 90.0)                                    // now clip, and set overscale if needed
  {
    Degrees = 90.0;
    if(IsForward)
      GForwardOverscale = VOVERSCALEDISPLAYTICKS;
    else
      GReverseOverscale = VOVERSCALEDISPLAYTICKS;
  }
  return (int)Degrees;
}




//
// convert from power value to degree angle for meter
// return 0 to 180
//
int GetPowerMeterDegrees(bool IsForward, bool IsPeak)
{
  int FullScale;
  unsigned int Power;
  float Degrees;
  
  FullScale = GPowerFullScale[GDisplayScaleInUse];
  if(IsPeak)
    Power = FindPeakPower(IsForward, true);            // get power in units of 0.1 watts
  else
    Power = GetPowerReading(IsForward, true);

  Degrees = 18.0 * (float)Power / (float)FullScale;
  if (Degrees > 180.0)                                    // now clip, and set overscale if needed
  {
    Degrees = 180.0;
    if(IsForward)
      GForwardOverscale = VOVERSCALEDISPLAYTICKS;
    else
      GReverseOverscale = VOVERSCALEDISPLAYTICKS;
  }
  return (int)Degrees;
}



//
// convert from power value to % of full scale
// return 0 to 100
//
int GetPowerPercent(bool IsForward, bool IsPeak)
{
  int FullScale;
  unsigned int Power;
  float Percent;
  
  FullScale = GPowerFullScale[GDisplayScaleInUse];
  if(IsPeak)
    Power = FindPeakPower(IsForward, true);            // get power in units of 0.1 watts
  else
    Power = GetPowerReading(IsForward, true);

  Percent = 10.0 * (float)Power / (float)FullScale;
  if (Percent > 100.0)                                    // now clip, and set overscale if needed
  {
    Percent = 100.0;
    if(IsForward)
      GForwardOverscale = VOVERSCALEDISPLAYTICKS;
    else
      GReverseOverscale = VOVERSCALEDISPLAYTICKS;
  }
  return (int)Percent;
}



//
// convert from VSWR value to % of full scale
// begins with a number that has 1 fixed point
// return 0 to 100
//
int GetVSWRPercent(void)
{
  int Power;
  float Percent;
  int Result;

  Percent = (float)GVSWR * 10.0 / VVSWRFULLSCALE;
  if (Percent > 100.0)
    Result = 100;
  else
    Result = (int)Percent;
  return Result;
}



//
// convert from dBm to percent
//
// return 0 to 100
//
int GetLogPowerPercent(bool IsForward)
{
  float PowerValue;
  float Percent;
  int Result;
  
  if(IsForward)
    PowerValue = (float) GForwardTenthdBm;        // tenths of a dBm
  else
    PowerValue = (float) GReverseTenthdBm;

  Percent = (PowerValue - VLOGBARMIN) * 100.0 / (VLOGBARMAX-VLOGBARMIN);
  Result = (int)Percent;
  if(Result < 0)
    Result = 0;
  else if (Result > 100)
    Result = 100;
  return Result;
}


#define VASCII0 0x30                // zero character in ASCII
//
// local version of "sprintf like" function
// Adds a decimal point before last digit if 3rd parameter set
// note integer value is signed and may be negative!
//
unsigned char mysprintf(char *dest, int Value, bool AddDP)
{
  unsigned char Digit;              // calculated digit
  bool HadADigit = false;           // true when found a non zero digit
  unsigned char DigitCount = 0;     // number of returned digits
  unsigned int Divisor = 10000;     // power of 10 being calculated
//
// deal with negative values first
//
  if (Value < 0)
  {
    *dest++ = '-';    // add to output
    DigitCount++;
    Value = -Value;
  }
//
// now convert the (definitely posirive) number
//
  while (Divisor >= 10)
  {
    Digit = Value / Divisor;        // find digit: integer divide
    if (Digit != 0)
      HadADigit = true;             // flag if non zero so all trailing digits added
    if (HadADigit)                  // if 1st non zero or all subsequent
    {
      *dest++ = Digit + VASCII0;    // add to output
      DigitCount++;
    }
    Value -= (Digit * Divisor);     // get remainder from divide
    Divisor = Divisor / 10;         // ready for next digit
  }
//
// if we need a decimal point, add it now. Also if there hasn't been a preceiding digit
// (i.e. number was like 0.3) add the zero
//
  if (AddDP)
  {
    if (HadADigit == false)
    {
      *dest++ = '0';
      DigitCount++;
    }
    *dest++ = '.';
  DigitCount++;
  }
  *dest++ = Value + VASCII0;
  DigitCount++;
//
// finally terminate with a 0
//
  *dest++ = 0;
  return DigitCount;
}



//
// set foreground and background of bargraphs to set power scale
//
void SetBargraphImages(void)
{
  byte Image;
  char Str[20];
  char Str2[5];

  Str2[0]=0;                                            // empty the string
  if(GDisplayPageInUse == 2)
  {
    Image = GPowerForeground[GDisplayScaleInUse];       // foreground image number
    mysprintf(Str2, Image, false);
    strcpy(Str,"p2j0.ppic=");
    strcat(Str, Str2);
    sendCommand(Str);
    Image = GPowerBackground[GDisplayScaleInUse];       // background image number
    mysprintf(Str2, Image, false);
    strcpy(Str,"p2j0.bpic=");
    strcat(Str, Str2);
    sendCommand(Str);
  }
}


//
// set background for meter display to set the power scale
//
void SetMeterImages(void)
{
  byte Image;
  char Str[20];
  char Str2[5];

  Str2[0]=0;                                            // empty the string
  if(GDisplayPageInUse == 4)
  {
    Image = GMeterPicture[GDisplayScaleInUse];       // foreground image number
    mysprintf(Str2, Image, false);
    strcpy(Str,"p4z0.picc=");
    strcat(Str, Str2);
    sendCommand(Str);
  }
}



//
// set background for crossed needle display to set the power scale
//
void SetCrossedNeedleImages(void)
{
  byte Image;
  if(GDisplayPageInUse == 1)
  {
    Image = GCrossedNeedlePicture[GDisplayScaleInUse];       // foreground image number
    p1Axes.setPic(Image);
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// touch event handlers: PAGE change
//

//
// page 1 display button callback
// enter page 2
//
void p1DisplayBtnPushCallback(void *ptr)
{
  GDisplayPage = ePowerBargraphPage;
  page2.show();
  EEWritePage(2);
  GInitialisePage = true;
}


//
// page 2 display button callback
// enter page 3
//
void p2DisplayBtnPushCallback(void *ptr)
{
  GDisplayPage = eLogBargraphPage;
  page3.show();
  EEWritePage(3);
  GInitialisePage = true;
}


//
// page 3 display button callback
// enter page 4
//
void p3DisplayBtnPushCallback(void *ptr)
{
  GDisplayPage = eMeterPage;
  page4.show();
  EEWritePage(4);
  GInitialisePage = true;
}


//
// page 4 display button callback
// enter page 5
//
void p4DisplayBtnPushCallback(void *ptr)
{
  GDisplayPage = eEngineeringPage;
  page5.show();
  EEWritePage(5);
  GInitialisePage = true;
}


//
// page 5 display button callback
// enter page 1
//
void p5DisplayBtnPushCallback(void *ptr)
{
  GDisplayPage = eCrossedNeedlePage;
  page1.show();
  EEWritePage(1);
  GInitialisePage = true;
}


//
// touch event - SCALE pushbutton
// change the stored display scale
//
void ScaleBtnPushCallback(void *ptr)              // display scale pushbutton
{
  if(GDisplayScaleInUse++ >= VMAXSCALESETTING)      // increment or wrap
    GDisplayScaleInUse = 0;
  EEWriteScale(GDisplayScaleInUse);                 // store to EEPROM so we start with the same

//
// now set display pic accordingly
//
  if(GDisplayPageInUse == 1)
  {
    SetCrossedNeedleImages();
    GInitialisePage = true;
  }
  else if(GDisplayPageInUse == 2)
    SetBargraphImages();
  else if(GDisplayPageInUse == 4)
    SetMeterImages();
}


//
// touch event - peak/normal button
//
void P1PeakBtnPushCallback(void *ptr)             // peak/normal display button
{
  uint32_t State;

  p1PeakBtn.getValue(&State);
  if(State == 0)
  {
    p1PeakBtn.setText("Average");
    GPeakDisplayInUse = false;
  }
  else
  {
    p1PeakBtn.setText("Peak");
    GPeakDisplayInUse = true;
  }
  EEWritePeak(GPeakDisplayInUse);                 // store to EEPROM so we start with the same
}


//
// touch event - peak/normal button
//
void P2PeakBtnPushCallback(void *ptr)             // peak/normal display button
{
  uint32_t State;

  p2PeakBtn.getValue(&State);
  if(State == 0)
  {
    p2PeakBtn.setText("Average");
    GPeakDisplayInUse = false;
  }
  else
  {
    p2PeakBtn.setText("Peak");
    GPeakDisplayInUse = true;
  }
  EEWritePeak(GPeakDisplayInUse);                 // store to EEPROM so we start with the same
}



//
// touch event - peak/normal button
//
void P4PeakBtnPushCallback(void *ptr)             // peak/normal display button
{
  uint32_t State;

  p4PeakBtn.getValue(&State);
  if(State == 0)
  {
    p4PeakBtn.setText("Average");
    GPeakDisplayInUse = false;
  }
  else
  {
    p4PeakBtn.setText("Peak");
    GPeakDisplayInUse = true;
  }
  EEWritePeak(GPeakDisplayInUse);                 // store to EEPROM so we start with the same
}



//
// display initialise
//
// nexinit performs the following:
//    dbSerialBegin(9600);
//    nexSerial.begin(Speed);
//    sendCommand("");
//    sendCommand("bkcmd=1");
//    ret1 = recvRetCommandFinished();
//    sendCommand("page 0");
//    ret2 = recvRetCommandFinished();
//    return ret1 && ret2;
//

void DisplayInit(void)
{
  char Str[10];
//
// set baud rate & register event callback functions
//  
  nexInit(115200);
  p1ScaleBtn.attachPush(ScaleBtnPushCallback);
  p2ScaleBtn.attachPush(ScaleBtnPushCallback);
  p1PeakBtn.attachPush(P1PeakBtnPushCallback);
  p2PeakBtn.attachPush(P2PeakBtnPushCallback);
  p4ScaleBtn.attachPush(ScaleBtnPushCallback);
  p4PeakBtn.attachPush(P4PeakBtnPushCallback);
  p1DisplayBtn.attachPush(p1DisplayBtnPushCallback);
  p2DisplayBtn.attachPush(p2DisplayBtnPushCallback);
  p3DisplayBtn.attachPush(p3DisplayBtnPushCallback);
  p4DisplayBtn.attachPush(p4DisplayBtnPushCallback);
  p5DisplayBtn.attachPush(p5DisplayBtnPushCallback);
  GDisplayPage = eSplashPage;

  mysprintf(Str, SWVERSION, false);
  p0SWVersion.setText(Str);
  GSplashCountdown = VFIVESECONDS;                  // ticks to stay in splash page
  GUpdateItem = 0;
}


//
// display tick
// this is responsible for drawing the display in a mode dependent way
//
void DisplayTick(void)
{
  char Str[30];
  char Str2[10];
  int X1, Y1, X2, Y2;
  int Forward, Reverse;
  float X,Y;
  float Angle;
  unsigned long T1;
//
// handle touch display events
//
  nexLoop(nex_listen_list);
  Str2[0] = 0;                                      //empty the string
  digitalWrite(VPINDEBUGSCOPE, HIGH); // Led on
//
// display dependent processing
//
  switch(GDisplayPage)
  {
    case  eSplashPage:                              // startup splash - nothing to add to display
      if(GSplashCountdown-- <= 0)
      {
        if(GDisplayPageInUse == 5)                  // choose the operating page from eeprom stored value
        {
          sendCommand("bkcmd=1");
          page5.show();
          GDisplayPage = eEngineeringPage;
          GInitialisePage = true;
        }
        else if(GDisplayPageInUse == 4)
        {
          page4.show();
          GDisplayPage = eMeterPage;
          GInitialisePage = true;
        }
        else if(GDisplayPageInUse == 3)
        {
          page3.show();
          GDisplayPage = eLogBargraphPage;
          GInitialisePage = true;
        }
        else if(GDisplayPageInUse == 2)
        {
          page2.show();
          GDisplayPage = ePowerBargraphPage;
          GInitialisePage = true;
        }
        else
        {
          page1.show();
          GDisplayPage = eCrossedNeedlePage;
          GInitialisePage = true;
        }
      }
      break;

///////////////////////////////////////////////////

    case  eCrossedNeedlePage:                         // crossed needle VSWR page display
      GUpdateMeterTicks++;                            // update ticks since last updated
      if(GInitialisePage == true)                     // load background pics
      {
        SetCrossedNeedleImages();                     // get correct display scales
        GInitialisePage = false;
        GDisplayedForward = -100;                     // set illegal display angles
        GDisplayedReverse = -100;
        GCrossedNeedleRedrawing = false;
      }
      else
      {
//
// first see what the angles should be, and if they are different from what's drawn.
// we only trest this is we are NOT already redrawing the display.
// redraw once a second even if no change
//
        if(!GCrossedNeedleRedrawing)
        {
          Forward = GetCrossedNeedleDegrees(true, GPeakDisplayInUse);
          Reverse = GetCrossedNeedleDegrees(false, GPeakDisplayInUse);
          if((Forward != GDisplayedForward) || (Reverse != GDisplayedReverse) || (GUpdateMeterTicks >= 50))
          {
            GCrossedNeedleRedrawing = true;           // if changed, set need to redraw display and required angles
            GDisplayedForward = Forward;
            GDisplayedReverse = Reverse;
            GUpdateItem = 0;                          // set to do 1st stage
          }
        }

//
// then if we need to update display, get on and drawe it in sections
//
        if(GCrossedNeedleRedrawing)
        {
          GUpdateMeterTicks = 0;
          switch(GUpdateItem++)
          {
            case 0:                                     // erase image
              sendCommand("ref 1");
              break;
  
            case 1:                                     // draw reverse power line
              X1 = VXNEEDLEREVX1;
              Y1 = VXNEEDLEY1;
              Angle = (float)GDisplayedReverse * M_PI / 180.0;
              X = X1 + VNEEDLESIZE * cos(Angle);
              X2 = (int)X;
              Y = Y1 - VNEEDLESIZE * sin(Angle);
              Y2 = (int)Y;
// get text commands
              strcpy(Str, "line ");             // line
              mysprintf(Str2, X1, false);
              strcat(Str, Str2);
              strcat(Str, ",");                 // line X1,
              mysprintf(Str2, Y1, false);
              strcat(Str, Str2);
              strcat(Str, ",");                 // line X1,Y1,
              mysprintf(Str2, X2, false);
              strcat(Str, Str2);
              strcat(Str, ",");                 // line X1,Y1,X2
              mysprintf(Str2, Y2, false);
              strcat(Str, Str2);
              strcat(Str, ",BLUE");             // line X1,Y1,X2,Y2,BLUE
              sendCommand(Str);
              break;

            case 2:                                     // draw forward power line
              X1 = VXNEEDLEFWDX1;
              Y1 = VXNEEDLEY1;
              Angle = (float)GDisplayedForward * M_PI / 180.0;
              X = X1 - VNEEDLESIZE * cos(Angle);
              X2 = (int)X;
              Y = Y1 - VNEEDLESIZE * sin(Angle);
              Y2 = (int)Y;
// get text commands
              strcpy(Str, "line ");             // line
              mysprintf(Str2, X1, false);
              strcat(Str, Str2);
              strcat(Str, ",");                 // line X1,
              mysprintf(Str2, Y1, false);
              strcat(Str, Str2);
              strcat(Str, ",");                 // line X1,Y1,
              mysprintf(Str2, X2, false);
              strcat(Str, Str2);
              strcat(Str, ",");                 // line X1,Y1,X2
              mysprintf(Str2, Y2, false);
              strcat(Str, Str2);
              strcat(Str, ",BLUE");             // line X1,Y1,X2,Y2,BLUE
              sendCommand(Str);
              break;
              
            case 6:                                     // end of dwell after redraw
              GCrossedNeedleRedrawing = false;
              break;
          }
        }   // switch
      }
      break;      // end of crossed needle


///////////////////////////////////////////////////

    case  ePowerBargraphPage:                            // power bargraph page display
      if(GInitialisePage)
      {
        if(GPeakDisplayInUse)
        {
          p2PeakBtn.setValue(1);
          p2PeakBtn.setText("Peak");
        }
        SetBargraphImages();                            // get correct display scales
        GInitialisePage = false;
      }
      else
        switch(GUpdateItem)
        {
          case 0:
            Forward = GetPowerPercent(true, GPeakDisplayInUse);
            p2FwdBar.setValue(Forward);
            break;
  
          case 4:
            Forward = GetVSWRPercent();
            p2VSWRBar.setValue(Forward);
            break;
  
          case 8:
            if(GPeakDisplayInUse)
              Forward = FindPeakPower(true, false);                 // get forward peak power, in watts
            else
              Forward = GetPowerReading(true, false);               // get forward power, in watts
            mysprintf(Str, Forward, false);
            p2FwdPower.setText(Str);
  
            mysprintf(Str, GVSWR, true);
            p2VSWRTxt.setText(Str);
            break;
  
          case 9:
            if(GForwardOverscale != 0)
            {
              p2Overrange.setValue(1);
              GForwardOverscale--;
            }
            else
              p2Overrange.setValue(0);
            break;
        }          
      if (GUpdateItem++ >= 9)
        GUpdateItem = 0;
      break;


///////////////////////////////////////////////////

    case  eLogBargraphPage:                              // dBm bargraph page display
      switch(GUpdateItem)
      {
        case 0:
          Forward = GetLogPowerPercent(true);
          p3FwddBmBar.setValue(Forward);
          break;

        case 4:
          Reverse = GetLogPowerPercent(false);
          p3RevdBmBar.setValue(Reverse);
          break;

        case 8:
          mysprintf(Str, GForwardTenthdBm, true);
          p3FwddBm.setText(Str);
          break;

        case 9:
          mysprintf(Str, GReverseTenthdBm, true);
          p3RevdBm.setText(Str);
          break;
          
        default:
          break;
      }
      if (GUpdateItem++ >= 9)
        GUpdateItem = 0;

      GInitialisePage = true;
      break;


///////////////////////////////////////////////////

    case  eMeterPage:                            // power meter page display
      if(GInitialisePage)
      {
        if(GPeakDisplayInUse)
        {
          p4PeakBtn.setValue(1);
          p4PeakBtn.setText("Peak");
        }
        SetMeterImages();                            // get correct display scales
        GInitialisePage = false;
      }
      else
        switch(GUpdateItem)
        {
          case 0:
            Forward = GetPowerMeterDegrees(true, GPeakDisplayInUse);
            p4Meter.setValue(Forward);
            break;
  
          case 10:
            Forward = GetVSWRPercent();
            p4VSWRBar.setValue(Forward);
            break;
  
        }          
      if (GUpdateItem++ >= 14)
        GUpdateItem = 0;
      break;


///////////////////////////////////////////////////

    case  eEngineeringPage:                         // engineering page with raw ADC values
      switch(GUpdateItem)
      {
        case 0:
          mysprintf(Str, GFwdLineVoltageTenth, true);
          p5FwdVolts.setText(Str);
          break;        
        case 1:
          mysprintf(Str, GRevLineVoltageTenth, true);
          p5RevVolts.setText(Str);
          break;        
        case 2:
          mysprintf(Str, GForwardTenthdBm, true);
          p5FwddBm.setText(Str);
          break;        
        case 3:
          mysprintf(Str, GReverseTenthdBm, true);
          p5RevdBm.setText(Str);
          break;        
        case 4:
          mysprintf(Str, GetPowerReading(true, false), false);
          p5FwdPower.setText(Str);
          break;        
        case 5:
          mysprintf(Str, GetPowerReading(false, false), false);
          p5RevPower.setText(Str);
          break;        
        case 6:
          Forward = FindPeakPower(true, false);
          mysprintf(Str, Forward, false);
          p5FwdPeak.setText(Str);
          break;        
        case 7:
          Reverse = FindPeakPower(false, false);
          mysprintf(Str, Reverse, false);
          p5RevPeak.setText(Str);
          break;        
        case 8:
          mysprintf(Str, GVSWR, true);
          p5VSWR.setText(Str);
          break;        
      }
//      if (GUpdateItem++ >= VMAXENGITEM)
        GUpdateItem = 0;

      GInitialisePage = false;
      break;
  }
  digitalWrite(VPINDEBUGSCOPE, LOW); // Led on
}
