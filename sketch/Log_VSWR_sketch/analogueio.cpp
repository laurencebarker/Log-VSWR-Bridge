/////////////////////////////////////////////////////////////////////////
//
// Log VSWR Bridge Display sketch by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2020
//
// this sketch provides a VSWR bridge display
//
// the code is written for an Arduino Nano Every module
//
// analogueio.cpp
// this file holds the code to read the analogue inputs
/////////////////////////////////////////////////////////////////////////


#include <Arduino.h>
#include "iopins.h"
#include "analogueio.h"

// scale and offset to get of units of dBm
#define VPOWERDBMOFFSET -96.0
#define VPOWERDBMSCALE 0.1253
#define VCOUPLINGCOEFFICIENT 50.0           // couple loss in dB
#define VZo 50.0
#define VHIGHVSWR 9999                      // 999.9
//
// now a big lookup table from ADC reading to coax line voltage
//
const float GLineVoltageTable[] = 
{
  0.00112,0.00114,0.00115,0.00117,0.00119,0.0012,0.00122,0.00124,
  0.00126,0.00128,0.00129,0.00131,0.00133,0.00135,0.00137,0.00139,
  0.00141,0.00143,0.00145,0.00147,0.0015,0.00152,0.00154,0.00156,
  0.00158,0.00161,0.00163,0.00165,0.00168,0.0017,0.00173,0.00175,
  0.00178,0.0018,0.00183,0.00186,0.00188,0.00191,0.00194,0.00197,
  0.002,0.00202,0.00205,0.00208,0.00211,0.00214,0.00218,0.00221,
  0.00224,0.00227,0.00231,0.00234,0.00237,0.00241,0.00244,0.00248,
  0.00251,0.00255,0.00259,0.00262,0.00266,0.0027,0.00274,0.00278,
  0.00282,0.00286,0.0029,0.00295,0.00299,0.00303,0.00308,0.00312,
  0.00317,0.00321,0.00326,0.00331,0.00335,0.0034,0.00345,0.0035,
  0.00355,0.00361,0.00366,0.00371,0.00376,0.00382,0.00388,0.00393,
  0.00399,0.00405,0.00411,0.00416,0.00423,0.00429,0.00435,0.00441,
  0.00448,0.00454,0.00461,0.00467,0.00474,0.00481,0.00488,0.00495,
  0.00502,0.0051,0.00517,0.00525,0.00532,0.0054,0.00548,0.00556,
  0.00564,0.00572,0.0058,0.00589,0.00597,0.00606,0.00615,0.00624,
  0.00633,0.00642,0.00651,0.00661,0.0067,0.0068,0.0069,0.007,
  0.0071,0.00721,0.00731,0.00742,0.00752,0.00763,0.00774,0.00786,
  0.00797,0.00809,0.0082,0.00832,0.00844,0.00857,0.00869,0.00882,
  0.00895,0.00908,0.00921,0.00934,0.00948,0.00962,0.00976,0.0099,
  0.01004,0.01019,0.01033,0.01048,0.01064,0.01079,0.01095,0.01111,
  0.01127,0.01143,0.0116,0.01177,0.01194,0.01211,0.01229,0.01247,
  0.01265,0.01283,0.01302,0.01321,0.0134,0.01359,0.01379,0.01399,
  0.01419,0.0144,0.01461,0.01482,0.01504,0.01526,0.01548,0.0157,
  0.01593,0.01616,0.0164,0.01664,0.01688,0.01712,0.01737,0.01762,
  0.01788,0.01814,0.0184,0.01867,0.01894,0.01922,0.0195,0.01978,
  0.02007,0.02036,0.02065,0.02095,0.02126,0.02157,0.02188,0.0222,
  0.02252,0.02285,0.02318,0.02352,0.02386,0.02421,0.02456,0.02491,
  0.02528,0.02564,0.02602,0.02639,0.02678,0.02717,0.02756,0.02796,
  0.02837,0.02878,0.0292,0.02962,0.03005,0.03049,0.03093,0.03138,
  0.03184,0.0323,0.03277,0.03325,0.03373,0.03422,0.03472,0.03522,
  0.03573,0.03625,0.03678,0.03731,0.03786,0.03841,0.03897,0.03953,
  0.04011,0.04069,0.04128,0.04188,0.04249,0.04311,0.04373,0.04437,
  0.04501,0.04567,0.04633,0.047,0.04769,0.04838,0.04908,0.04979,
  0.05052,0.05125,0.052,0.05275,0.05352,0.0543,0.05509,0.05589,
  0.0567,0.05752,0.05836,0.05921,0.06007,0.06094,0.06182,0.06272,
  0.06363,0.06456,0.0655,0.06645,0.06741,0.06839,0.06939,0.07039,
  0.07142,0.07246,0.07351,0.07458,0.07566,0.07676,0.07787,0.07901,
  0.08015,0.08132,0.0825,0.0837,0.08492,0.08615,0.0874,0.08867,
  0.08996,0.09127,0.09259,0.09394,0.0953,0.09669,0.09809,0.09952,
  0.10096,0.10243,0.10392,0.10543,0.10696,0.10852,0.11009,0.11169,
  0.11331,0.11496,0.11663,0.11833,0.12005,0.12179,0.12356,0.12535,
  0.12718,0.12902,0.1309,0.1328,0.13473,0.13669,0.13867,0.14069,
  0.14273,0.14481,0.14691,0.14905,0.15121,0.15341,0.15564,0.1579,
  0.16019,0.16252,0.16488,0.16728,0.16971,0.17217,0.17468,0.17721,
  0.17979,0.1824,0.18505,0.18774,0.19047,0.19324,0.19604,0.19889,
  0.20178,0.20472,0.20769,0.21071,0.21377,0.21688,0.22003,0.22322,
  0.22647,0.22976,0.2331,0.23648,0.23992,0.24341,0.24694,0.25053,
  0.25417,0.25786,0.26161,0.26541,0.26927,0.27318,0.27715,0.28118,
  0.28526,0.28941,0.29361,0.29788,0.30221,0.3066,0.31105,0.31557,
  0.32016,0.32481,0.32953,0.33432,0.33918,0.3441,0.3491,0.35418,
  0.35932,0.36454,0.36984,0.37521,0.38067,0.3862,0.39181,0.3975,
  0.40328,0.40914,0.41508,0.42111,0.42723,0.43344,0.43974,0.44613,
  0.45261,0.45919,0.46586,0.47263,0.4795,0.48646,0.49353,0.5007,
  0.50798,0.51536,0.52285,0.53044,0.53815,0.54597,0.5539,0.56195,
  0.57012,0.5784,0.58681,0.59533,0.60398,0.61276,0.62166,0.6307,
  0.63986,0.64916,0.65859,0.66816,0.67787,0.68772,0.69771,0.70785,
  0.71813,0.72857,0.73915,0.74989,0.76079,0.77184,0.78306,0.79444,
  0.80598,0.81769,0.82957,0.84163,0.85386,0.86626,0.87885,0.89162,
  0.90458,0.91772,0.93105,0.94458,0.95831,0.97223,0.98636,1.00069,
  1.01523,1.02998,1.04495,1.06013,1.07554,1.09116,1.10702,1.1231,
  1.13942,1.15598,1.17278,1.18982,1.2071,1.22464,1.24244,1.26049,
  1.27881,1.29739,1.31624,1.33537,1.35477,1.37445,1.39442,1.41469,
  1.43524,1.4561,1.47725,1.49872,1.5205,1.54259,1.565,1.58774,
  1.61081,1.63422,1.65796,1.68205,1.70649,1.73129,1.75645,1.78197,
  1.80786,1.83413,1.86078,1.88782,1.91525,1.94308,1.97131,1.99995,
  2.02901,2.0585,2.08841,2.11875,2.14954,2.18077,2.21246,2.24461,
  2.27722,2.31031,2.34388,2.37794,2.41249,2.44754,2.4831,2.51918,
  2.55579,2.59293,2.6306,2.66882,2.7076,2.74695,2.78686,2.82735,
  2.86843,2.91011,2.9524,2.9953,3.03882,3.08297,3.12777,3.17322,
  3.21933,3.2661,3.31356,3.36171,3.41055,3.46011,3.51039,3.56139,
  3.61314,3.66564,3.7189,3.77294,3.82776,3.88338,3.93981,3.99705,
  4.05513,4.11405,4.17383,4.23448,4.29601,4.35843,4.42176,4.48601,
  4.55119,4.61732,4.68441,4.75248,4.82153,4.89159,4.96267,5.03477,
  5.10793,5.18215,5.25745,5.33384,5.41134,5.48997,5.56974,5.65067,
  5.73278,5.81608,5.90058,5.98632,6.0733,6.16155,6.25108,6.34191,
  6.43406,6.52755,6.62239,6.71862,6.81624,6.91528,7.01576,7.11771,
  7.22113,7.32605,7.4325,7.5405,7.65006,7.76122,7.87399,7.9884,
  8.10448,8.22224,8.34171,8.46292,8.58588,8.71064,8.83721,8.96561,
  9.09589,9.22805,9.36214,9.49817,9.63618,9.7762,9.91825,10.06236,
  10.20857,10.35691,10.50739,10.66007,10.81496,10.97211,11.13153,11.29328,
  11.45737,11.62385,11.79275,11.9641,12.13794,12.31431,12.49324,12.67477,
  12.85894,13.04578,13.23534,13.42765,13.62276,13.8207,14.02152,14.22525,
  14.43195,14.64165,14.8544,15.07023,15.28921,15.51136,15.73675,15.96541,
  16.19739,16.43274,16.67151,16.91375,17.15951,17.40885,17.6618,17.91843,
  18.17879,18.44293,18.71091,18.98279,19.25861,19.53845,19.82234,20.11037,
  20.40258,20.69903,20.99979,21.30492,21.61449,21.92855,22.24718,22.57044,
  22.89839,23.23111,23.56867,23.91112,24.25856,24.61104,24.96865,25.33145,
  25.69952,26.07294,26.45179,26.83614,27.22607,27.62167,28.02302,28.43021,
  28.8433,29.2624,29.68759,30.11896,30.5566,31.00059,31.45104,31.90803,
  32.37166,32.84203,33.31923,33.80337,34.29454,34.79285,35.2984,35.81129,
  36.33164,36.85955,37.39513,37.93849,38.48974,39.04901,39.6164,40.19204,
  40.77604,41.36852,41.96962,42.57945,43.19814,43.82582,44.46262,45.10867,
  45.76411,46.42907,47.1037,47.78813,48.4825,49.18696,49.90166,50.62674,
  51.36236,52.10867,52.86582,53.63398,54.41329,55.20393,56.00606,56.81984,
  57.64545,58.48305,59.33282,60.19494,61.06959,61.95695,62.8572,63.77053,
  64.69713,65.63719,66.59092,67.5585,68.54014,69.53605,70.54642,71.57148,
  72.61143,73.66649,74.73689,75.82283,76.92456,78.04229,79.17626,80.32671,
  81.49388,82.67801,83.87934,85.09813,86.33462,87.58909,88.86178,90.15296,
  91.46291,92.79189,94.14018,95.50806,96.89581,98.30373,99.73211,101.18124,
  102.65143,104.14298,105.6562,107.19141,108.74893,110.32908,111.93219,113.55859,
  115.20863,116.88264,118.58097,120.30398,122.05203,123.82547,125.62469,127.45005,
  129.30193,131.18072,133.08681,135.02059,136.98247,138.97286,140.99217,143.04082,
  145.11924,147.22786,149.36712,151.53746,153.73934,155.97321,158.23954,160.5388,
  162.87146,165.23803,167.63898,170.07481,172.54604,175.05318,177.59675,180.17727,
  182.79529,185.45135,188.14601,190.87981,193.65334,196.46718,199.32189,202.21809,
  205.15637,208.13734,211.16163,214.22986,217.34267,220.50072,223.70465,226.95513,
  230.25284,233.59847,236.99272,240.43628,243.92988,247.47424,251.0701,254.71821,
  258.41933,262.17423,265.98369,269.8485,273.76946,277.7474,281.78314,285.87752,
  290.03139,294.24562,298.52108,302.85866,307.25928,311.72383,316.25325,320.84849,
  325.5105,330.24025,335.03872,339.90692,344.84585,349.85654,354.94005,360.09741,
  365.32972,370.63805,376.02351,381.48723,387.03033,392.65398,398.35934,404.1476,
  410.01996,415.97765,422.02191,428.154,434.37518,440.68676,447.09005,453.58638,
  460.1771,466.86359,473.64723,480.52944,487.51165,494.59532,501.78191,509.07293,
  516.46988,523.97432,531.5878,539.3119,547.14823,555.09843,563.16415,571.34707,
  579.64888,588.07132,596.61615,605.28513,614.08007,623.00281,632.05519,641.23911,
  650.55647,660.00922,669.59932,679.32876,689.19958,699.21382,709.37357,719.68094,
  730.13808,740.74717,751.51041,762.43005,773.50834,784.74761,796.15019,807.71845,
  819.4548,831.36168,843.44157,855.69699,868.13048,880.74463,893.54206,906.52545,
  919.69749,933.06092,946.61853,960.37313,974.32759,988.48482,1002.84774,1017.41937,
  1032.20273,1047.20089,1062.41698,1077.85416,1093.51565,1109.4047,1125.52463,1141.87878,
  1158.47056,1175.30343,1192.38088,1209.70647,1227.2838,1245.11654,1263.20839,1281.56312,
  1300.18455,1319.07656,1338.24307,1357.68807,1377.41562,1397.42981,1417.73481,1438.33485,
  1459.23421,1480.43724,1501.94836,1523.77205,1545.91283,1568.37533,1591.16421,1614.28422,
  1637.74017,1661.53694,1685.67949,1710.17283,1735.02206,1760.23237,1785.80898,1811.75723,
  1838.08251,1864.7903,1891.88617,1919.37574,1947.26475,1975.55899,2004.26435,2033.38681,
  2062.93243,2092.90735,2123.31781,2154.17014,2185.47077,2217.2262,2249.44305,2282.12801,
  2315.2879,2348.92961,2383.06014,2417.68659,2452.81618,2488.45621,2524.61409,2561.29736,
  2598.51365,2636.2707,2674.57636,2713.43862,2752.86556,2792.86538,2833.4464,2874.61708  
};



//
// global variables
//
int GFwdPeakADC, GRevPeakADC;                       // forward, reverse peak ADC readings, no scaling
unsigned int  GFwdSumADC, GRevSumADC;               // summed readings for each ADC for averaging
byte GFwdCount, GRevCount;                          // number of summed readings              
bool GIsFwd;                                        // most recent readin was forward ADC
//
// sensor values, as 1DP fixed point integers
//
int GForwardTenthdBm;
int GReverseTenthdBm;
unsigned int GFwdLineVoltageTenth;
unsigned int GRevLineVoltageTenth;
unsigned int GFwdAvgPowerTenth;
unsigned int GRevAvgPowerTenth;
unsigned int GFwdPeakPowerTenth;
unsigned int GRevPeakPowerTenth;
unsigned int GVSWR;                               // set to 9999 if impossible


//
// buffer of forward and reverse power values for peak finding
// a new value written every tick; search all buffer for peak over last N entries
//
#define VSIZEPEAKBUFFER 32
unsigned int GForwardPowerBuffer[VSIZEPEAKBUFFER];          // circular buffer (units tenth of a watt)
unsigned int GReversePowerBuffer[VSIZEPEAKBUFFER];          // circular buffer (units tenth of a watt)
byte GPeakPointer;                                          // pointer into circular buffer

//
// buffer of forward and reverse power values for averaging
// a new value written every tick; average over last N entries
//
#define VSIZEAVGBUFFER 32
unsigned int GForwardAvgPowerBuffer[VSIZEAVGBUFFER];          // circular buffer (units tenth of a watt)
unsigned int GReverseAvgPowerBuffer[VSIZEAVGBUFFER];          // circular buffer (units tenth of a watt)
byte GAvgPointer;                                          // pointer into circular buffer


//
// AnalogueIO initialise
//
void AnalogueIOInit(void)
{
}



//
// fast tick: read one ADC value and store peak value found
//
void AnalogueIOFastTick()
{
  int Reading;
  if(GIsFwd)
  {
    Reading = analogRead(VPINREVPOWERADC);                      // get ADC reading for reverse RF power
    if(Reading > GRevPeakADC)
      GRevPeakADC = Reading;
    GRevSumADC += Reading;                                      // sum the ADC readings so we can average them
    GRevCount++;
  }
  else
  {
    Reading = analogRead(VPINFWDPOWERADC);                      // get ADC reading for forward RF power
    if(Reading > GFwdPeakADC)
      GFwdPeakADC = Reading;
    GFwdSumADC += Reading;                                      // sum the ADC readings so we can average them
    GFwdCount++;
  }
  GIsFwd = !GIsFwd;                                     // other ADC next tick
}


//
// AnalogueIO tick
// read the ADC values then convert to units of dBm
// (timed at ~1ms)
//
void AnalogueIOTick(void)
{
  int SensorReading;
  unsigned int SummedReading;
  byte SummedCount;
  float ScaledReading;
  float FwdVoltage, RevVoltage;
  float Power;

  if(++GPeakPointer >= VSIZEPEAKBUFFER)                             // advance the peak buffer location
    GPeakPointer = 0;
  if(++GAvgPointer >= VSIZEAVGBUFFER)                               // advance the average buffer location
    GAvgPointer = 0;


//
// calculate forward powers
//
  SensorReading = GFwdPeakADC;                                      // get ADC reading for forward RF power
  GFwdPeakADC = 0;
  SummedReading = GFwdSumADC;
  GFwdSumADC = 0;
  SummedCount = GFwdCount;
  GFwdCount = 0;

// find averaged log power reading and store
  SummedReading = SummedReading / SummedCount;                      // averaged ADC reading over the last N samples
  ScaledReading = (float)SummedReading;                             // float avg
  ScaledReading = VPOWERDBMOFFSET + ScaledReading * VPOWERDBMSCALE + VCOUPLINGCOEFFICIENT; // dBm value of line power
  GForwardTenthdBm = (int)(ScaledReading*10.0);

// find average power in W
  FwdVoltage = GLineVoltageTable[SummedReading];
  GFwdLineVoltageTenth = (int)(FwdVoltage * 10.0);
  Power = 10.0 * FwdVoltage * FwdVoltage / VZo;                     // units of 0.1W
  if (Power > 60000)                                                // clip if too big for an unsigned int
    GFwdAvgPowerTenth = 60000;
  else
    GFwdAvgPowerTenth = (int)Power;
  GForwardAvgPowerBuffer[GAvgPointer] = GFwdAvgPowerTenth;

// now find peak power in W  and write to the buffer
  FwdVoltage = GLineVoltageTable[SensorReading];
  GFwdLineVoltageTenth = (int)(FwdVoltage * 10.0);
  Power = 10.0 * FwdVoltage * FwdVoltage / VZo;                     // units of 0.1W
  if (Power > 60000)                                                // clip if too big for an unsigned int
    GFwdPeakPowerTenth = 60000;
  else
    GFwdPeakPowerTenth = (int)Power;
  GForwardPowerBuffer[GPeakPointer] = GFwdPeakPowerTenth;

//
// now calculate the same for reverse powers
//
  SensorReading = GRevPeakADC;                                      // get ADC reading for reverse RF power
  GRevPeakADC = 0;
  SummedReading = GRevSumADC;
  GRevSumADC = 0;
  SummedCount = GRevCount;
  GRevCount = 0;

// find averaged log power reading and store
  SummedReading = SummedReading / SummedCount;                      // averaged ADC reading over the last N samples
  ScaledReading = (float)SummedReading;                             // float avg
  ScaledReading = VPOWERDBMOFFSET + ScaledReading * VPOWERDBMSCALE + VCOUPLINGCOEFFICIENT; // dBm value of line power
  GReverseTenthdBm = (int)(ScaledReading*10.0);

// find average power in W
  RevVoltage = GLineVoltageTable[SummedReading];
  GRevLineVoltageTenth = (int)(RevVoltage * 10.0);
  Power = 10.0 * RevVoltage * RevVoltage / VZo;                     // units of 0.1W
  if (Power > 60000)                                                // clip if too big for an unsigned int
    GRevAvgPowerTenth = 60000;
  else
    GRevAvgPowerTenth = (int)Power;
  GReverseAvgPowerBuffer[GAvgPointer] = GRevAvgPowerTenth;

// now find peak power in W  and write to the buffer
  RevVoltage = GLineVoltageTable[SensorReading];
  GRevLineVoltageTenth = (int)(RevVoltage * 10.0);
  Power = 10.0 * RevVoltage * RevVoltage / VZo;                     // units of 0.1W
  if (Power > 60000)                                                // clip if too big for an unsigned int
    GRevPeakPowerTenth = 60000;
  else
    GRevPeakPowerTenth = (int)Power;
  GReversePowerBuffer[GPeakPointer] = GRevPeakPowerTenth;


//
// finally VSWR
// for low sensor readings, set VSWR=1
//
  if(FwdVoltage < 0.1)
    GVSWR = 10.0;                                 // 10x value
  else if(FwdVoltage == RevVoltage)
    GVSWR = VHIGHVSWR;
  else
    GVSWR = abs(10.0 * (FwdVoltage + RevVoltage) / (FwdVoltage - RevVoltage));
}


//
// find peak power by searching buffer
// returns a power peak value
// 1st parameter true for forward power
// 2nd paramter true for units of tenths of a watt
//
unsigned int FindPeakPower(bool IsFwdPower, bool InTenths)
{
  unsigned int* Ptr;                                                  // buffer pointer
  byte Cntr;
  unsigned int Result = 0;
  
  if(IsFwdPower)
    Ptr = GForwardPowerBuffer;
  else
    Ptr = GReversePowerBuffer;
  for (Cntr = 0; Cntr < VSIZEPEAKBUFFER; Cntr++)
  {
    if(*Ptr > Result)
      Result = *Ptr;
    Ptr++;
  }
  if(!InTenths)                                                      // convert to watts if needed
    Result = Result/10;
  
  return Result;
}



//
// returns an average power value
// 1st parameter true for forward power
// 2nd parameter true for units of tenths of a watt
//
unsigned int GetPowerReading(bool IsFwdPower, bool InTenths)
{
  unsigned int* Ptr;                                                  // buffer pointer
  byte Cntr;
  unsigned long Sum = 0;
  unsigned int Result;
  
  if(IsFwdPower)
    Ptr = GForwardAvgPowerBuffer;
  else
    Ptr = GReverseAvgPowerBuffer;

  for (Cntr = 0; Cntr < VSIZEAVGBUFFER; Cntr++)
    Sum += *Ptr++;

  Result = (unsigned int)(Sum / VSIZEAVGBUFFER);
  if(!InTenths)                                                      // convert to watts if needed
    Result = Result/10;
  
  return Result;
}
