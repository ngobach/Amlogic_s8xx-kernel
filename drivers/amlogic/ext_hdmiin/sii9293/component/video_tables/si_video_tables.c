//------------------------------------------------------------------------------
// Project: HDMI Repeater
// Copyright (C) 2002-2013, Silicon Image, Inc.  All rights reserved.
//
// No part of this work may be reproduced, modified, distributed, transmitted,
// transcribed, or translated into any language or computer format, in any form
// or by any means without written permission of: Silicon Image, Inc.,
// 1140 East Arques Avenue, Sunnyvale, California 94085
//------------------------------------------------------------------------------

#include "si_video_tables.h"


// Note: maximum audio frequency for 8 channels is not exact: see PixClc=54MHz

ROM const videoMode_t VideoModeTable[NMB_OF_VIDEO_MODES + 1] =
{
  //{VIC, HVIC {H/V active}, {H/V  total},{H/V blank),{HVSyncO}, {HVSyncW},Fh , Fv , Fpix, Pclk, I/P , HPol, VPol, syst, repetition                  ,audio},
  // CEA-861D video modes
	{1,0  , 0, {640 , 480 }, {800 , 525 }, {160, 45}, {16 , 10}, {96 , 2}, 31 , 60 , 25 ,  2518, PROG, NEG,  NEG,  0   , RP1                         ,  48},
	// 1, 480p60
	{2,3  , 0, {720 , 480 }, {858 , 525 }, {138, 45}, {16 ,  9}, {62 , 6}, 31 , 60 , 27 ,  2700, PROG, NEG,  NEG,  NTSC, RP1                         ,  48},
	// 2, 720p60
	{0,4  , 0, {1280, 720 }, {1650, 750 }, {370, 30}, {110,  5}, {40 , 5}, 45 , 60 , 74 ,  7425, PROG, POS,  POS,  NTSC, RP1                         , 192},
	// 3, 1080i60
	{0,5  , 0, {1920, 1080}, {2200, 1125}, {280, 22}, {88 ,  2}, {44 , 5}, 34 , 60 , 74 ,  7425, INTL, POS,  POS,  NTSC, RP1                         , 192},
	// 4, 480i60
	{6,7  , 0, {1440, 480 }, {1716, 525 }, {276, 22}, {38 ,  4}, {124, 3}, 16 , 60 , 27 ,  2700, INTL, NEG,  NEG,  NTSC,     RP2                     ,  48},
	{8,9  , 0, {1440, 240 }, {1716, 262 }, {276, 22}, {38 ,  4}, {124, 3}, 16 , 60 , 27 ,  2700, PROG, NEG,  NEG,  NTSC,     RP2                     ,  48},
	{8,9  , 0, {1440, 240 }, {1716, 263 }, {276, 23}, {38 ,  5}, {124, 3}, 16 , 60 , 27 ,  2700, PROG, NEG,  NEG,  NTSC,     RP2                     ,  48},
	{10,11, 0, {2880, 480 }, {3432, 525 }, {552, 22}, {76 ,  4}, {248, 3}, 16 , 60 , 54 ,  5400, INTL, NEG,  NEG,  NTSC,         RP4|RP5|RP7|RP8|RP10,  96},
	{12,13, 0, {2880, 240 }, {3432, 262 }, {552, 22}, {76 ,  4}, {248, 3}, 16 , 60 , 54 ,  5400, PROG, NEG,  NEG,  NTSC,         RP4|RP5|RP7|RP8|RP10,  96},
	{12,13, 0, {2880, 240 }, {3432, 263 }, {552, 23}, {76 ,  5}, {248, 3}, 16 , 60 , 54 ,  5400, PROG, NEG,  NEG,  NTSC,         RP4|RP5|RP7|RP8|RP10,  96},
	{14,15, 0, {1440, 480 }, {1716, 525 }, {276, 45}, {32 ,  9}, {124, 6}, 31 , 60 , 54 ,  5400, PROG, NEG,  NEG,  NTSC, RP1|RP2                     ,  96},
	// 11, 1080p60
	{0,16,  0, {1920, 1080}, {2200, 1125}, {280, 45}, {88 ,  4}, {44 , 5}, 67 , 60 , 148, 14850, PROG, POS,  POS,  NTSC, RP1                         , 192},
	// 12, 576p50
	{17,18, 0, {720 , 576 }, {864 , 625 }, {144, 49}, {12 ,  5}, {64 , 5}, 31 , 50 , 27 ,  2700, PROG, NEG,  NEG,  PAL , RP1                         ,  48},
	// 13, 720p50
	{0,19,  0, {1280, 720 }, {1980, 750 }, {700, 30}, {440,  5}, {40 , 5}, 38 , 50 , 74 ,  7425, PROG, POS,  POS,  PAL , RP1                         , 192},
	// 14, 1080i50
	{0,20,  0, {1920, 1080}, {2640, 1125}, {720, 22}, {528,  2}, {44 , 5}, 28 , 50 , 74 ,  7425, INTL, POS,  POS,  PAL , RP1                         , 192},
	// 15, 576i50
	{21,22, 0, {1440, 576 }, {1728, 625 }, {288, 24}, {24 ,  2}, {126, 3}, 16 , 50 , 27 ,  2700, INTL, NEG,  NEG,  PAL ,     RP2                     ,  48},
	{23,24, 0, {1440, 288 }, {1728, 312 }, {288, 24}, {24 ,  2}, {126, 3}, 16 , 50 , 27 ,  2700, PROG, NEG,  NEG,  PAL ,     RP2                     ,  48},
	{23,24, 0, {1440, 288 }, {1728, 313 }, {288, 25}, {24 ,  3}, {126, 3}, 16 , 49 , 27 ,  2700, PROG, NEG,  NEG,  PAL ,     RP2                     ,  48},
	{23,24, 0, {1440, 288 }, {1728, 314 }, {288, 26}, {24 ,  4}, {126, 3}, 16 , 49 , 27 ,  2700, PROG, NEG,  NEG,  PAL ,     RP2                     ,  48},
	{25,26, 0, {2880, 576 }, {3456, 625 }, {576, 24}, {48 ,  2}, {252, 3}, 16 , 50 , 54 ,  5400, INTL, NEG,  NEG,  PAL ,         RP4|RP5|RP7|RP8|RP10,  96},
	{27,28, 0, {2880, 288 }, {3456, 312 }, {576, 24}, {48 ,  2}, {252, 3}, 16 , 50 , 54 ,  5400, PROG, NEG,  NEG,  PAL ,         RP4|RP5|RP7|RP8|RP10,  96},
	{27,28, 0, {2880, 288 }, {3456, 313 }, {576, 25}, {48 ,  3}, {252, 3}, 16 , 49 , 54 ,  5400, PROG, NEG,  NEG,  PAL ,         RP4|RP5|RP7|RP8|RP10,  96},
	{27,28, 0, {2880, 288 }, {3456, 314 }, {576, 26}, {48 ,  4}, {252, 3}, 16 , 49 , 54 ,  5400, PROG, NEG,  NEG,  PAL ,         RP4|RP5|RP7|RP8|RP10,  96},
	{29,30, 0, {1440, 576 }, {1728, 625 }, {288, 49}, {24 ,  5}, {128, 5}, 31 , 50 , 54 ,  5400, PROG, NEG,  POS,  PAL , RP1|RP2                     ,  96}, // H-neg, V-pos
	// 24, 1080p50
	{0,31,  0, {1920, 1080}, {2640, 1125}, {720, 45}, {528,  4}, {44 , 5}, 56 , 50 , 148, 14850, PROG, POS,  POS,  0   , RP1                         , 192},
	// 25, 1080p24
	{0,32,  0, {1920, 1080}, {2750, 1125}, {830, 45}, {638,  4}, {44 , 5}, 27 , 24 , 74 ,  7425, PROG, POS,  POS,  0   , RP1                         , 192},
	{0,33,  0, {1920, 1080}, {2640, 1125}, {720, 45}, {528,  4}, {44 , 5}, 28 , 25 , 74 ,  7425, PROG, POS,  POS,  0   , RP1                         , 192},
	// 27, 1080p30
	{0,34,  0, {1920, 1080}, {2200, 1125}, {280, 45}, {88 ,  4}, {44 , 5}, 34 , 30 , 74 ,  7425, PROG, POS,  POS,  0   , RP1                         , 192},
	{35,36, 0, {2880, 480 }, {3432, 525 }, {552, 45}, {96 ,  9}, {248, 6}, 31 , 60 , 108, 10800, PROG, NEG,  NEG,  NTSC, RP1|RP2|RP4                 , 192},
	{37,38, 0, {2880, 576 }, {3456, 625 }, {576, 49}, {48 ,  5}, {256, 5}, 31 , 50 , 108, 10800, PROG, NEG,  NEG,  PAL , RP1|RP2|RP4                 , 192},
	{0,39,  0, {1920, 1080}, {2304, 1250}, {384, 85}, {32 , 23}, {168, 5}, 31 , 50 , 72 ,  7200, INTL, POS,  NEG,  PAL , RP1                         , 192}, // H-pos, V-neg, 1,2 blanks are same = 85
	{0,40,  0, {1920, 1080}, {2640, 1125}, {720, 22}, {528,  2}, {44 , 5}, 56 , 100, 148, 14850, INTL, POS,  POS,  PAL , RP1                         , 192},
	{0,41,  0, {1280, 720 }, {1980, 750 }, {700, 30}, {440,  5}, {40 , 5}, 75 , 100, 148, 14850, PROG, POS,  POS,  PAL , RP1                         , 192},
	{42,43, 0, {720 , 576 }, {864 , 625 }, {144, 49}, {12 ,  5}, {64 , 5}, 63 , 100, 54 ,  5400, PROG, NEG,  NEG,  PAL , RP1                         ,  96},
	{44,45, 0, {1440, 576 }, {1728, 625 }, {288, 24}, {24 ,  2}, {126, 3}, 31 , 100, 54 ,  5400, INTL, NEG,  NEG,  PAL ,     RP2                     ,  96},
	{0,46,  0, {1920, 1080}, {2200, 1125}, {280, 22}, {88 ,  2}, {44 , 5}, 68 , 120, 148, 14850, INTL, POS,  POS,  NTSC, RP1                         , 192},
	{0,47,  0, {1280, 720 }, {1650, 750 }, {370, 30}, {110,  5}, {40 , 5}, 90 , 120, 148, 14850, PROG, POS,  POS,  NTSC, RP1                         , 192},
	{48,49, 0, {720 , 480 }, {858 , 525 }, {138, 45}, {16 ,  9}, {62 , 6}, 63 , 120, 54 ,  5400, PROG, NEG,  NEG,  NTSC, RP1                         ,  96},
	{50,51, 0, {1440, 480 }, {1716, 525 }, {276, 22}, {38 ,  4}, {124, 3}, 32 , 120, 54 ,  5400, INTL, NEG,  NEG,  NTSC,     RP2                     ,  96},
	{52,53, 0, {720 , 576 }, {864 , 625 }, {144, 49}, {12 ,  5}, {64 , 5}, 125, 200, 108, 10800, PROG, NEG,  NEG,  PAL , RP1                         , 192},
	{54,55, 0, {1440, 576 }, {1728, 625 }, {288, 24}, {24 ,  2}, {126, 3}, 63 , 200, 108, 10800, INTL, NEG,  NEG,  PAL ,     RP2                     , 192},
	{56,57, 0, {720 , 480 }, {858 , 525 }, {138, 45}, {16 ,  9}, {62 , 6}, 126, 240, 108, 10800, PROG, NEG,  NEG,  NTSC, RP1                         , 192},
	{58,59, 0, {1440, 480 }, {1716, 525 }, {276, 22}, {38 ,  4}, {124, 3}, 63 , 240, 108, 10800, INTL, NEG,  NEG,  NTSC,     RP2                     , 192},

    // CEA-861E video modes
	{ 0,60, 0, {1280, 720 }, {3300, 750 }, {2020,30}, {1760, 5}, { 40, 5}, 79 , 24 , 59 ,  5940, PROG, POS,  POS,  0   , RP1                         , 192},
	{ 0,61, 0, {1280, 720 }, {3960, 750 }, {2680,30}, {2420, 5}, { 40, 5}, 99 , 25 , 74 ,  7425, PROG, POS,  POS,  PAL , RP1                         , 192},
	{ 0,62, 0, {1280, 720 }, {3300, 750 }, {2020,30}, {1760, 5}, { 40, 5}, 91 , 30 , 74 ,  7425, PROG, POS,  POS,  NTSC, RP1                         , 192},
	{ 0,63, 0, {1920, 1080}, {2200, 1125}, {280, 45}, {  88, 4}, { 44, 5},264 , 120, 297, 29700, PROG, POS,  POS,  NTSC, RP1                         , 192},
	{ 0,64, 0, {1920, 1080}, {2640, 1125}, {720, 45}, { 528, 4}, { 44, 5},264 , 100, 297, 29700, PROG, POS,  POS,  PAL , RP1                         , 192},

	// HDMI 1.4a video modes
    {0,0  , 1, {3840, 2160}, {4400, 2250}, {560, 90}, {176 , 8}, {88 ,10}, 132,  30, 297, 29700, PROG, POS,  POS,  NTSC, RP1                         ,  96},
    {0,0  , 2, {3840, 2160}, {5280, 2250}, {1440,90}, {1056, 8}, {88 ,10}, 132,  25, 297, 29700, PROG, POS,  POS,  PAL , RP1                         , 192},
    {0,0  , 3, {3840, 2160}, {5500, 2250}, {1660,90}, {1276, 8}, {88 ,10}, 132,  24, 297, 29700, PROG, POS,  POS,  0   , RP1                         , 192},
    {0,0  , 4, {4096, 2160}, {5500, 2250}, {1404,90}, {1020, 8}, {88 ,10}, 132,  24, 297, 29700, PROG, POS,  POS,  0   , RP1                         , 192}, // SMPTE (cannot be distinguished from #3)

    {0,0  , 0, {0   , 0   }, {0   , 0   }, {0  , 0 }, {0  ,  0}, {0  , 0}, 0  , 0  , 0  ,     0, 0   , 0  ,  0  ,  0   , 0                           ,   0}
};

// It is not clear from CEA861C, where it is said at one place that formats 10...13 and 25...28 could have
// repetition factors 4,5,7,8,10, but at the table 12 on page 76 it is said that the
// factor could be any in 1...10 range...



ROM const uint8_t ceaVicToVideoTableIndex[LAST_KNOWN_CEA_VIC + 1] =
{
	NMB_OF_CEA861_VIDEO_MODES, //no VIC (dummy, to start real indexes from 1, but not from 0)
	0,  //VIC=1
	1,  //VIC=2
	1,  //VIC=3
	2,  //VIC=4
	3,  //VIC=5
	4,  //VIC=6
	4,  //VIC=7
	5,  //VIC=8  Possible indexes: 5,6
	5,  //VIC=9  Possible indexes: 5,6
	7,  //VIC=10
	7,  //VIC=11
	8,  //VIC=12 Possible indexes: 8,9
	8,  //VIC=13 Possible indexes: 8,9
	10, //VIC=14
	10, //VIC=15
	11, //VIC=16
	12, //VIC=17
	12, //VIC=18
	13, //VIC=19
	14, //VIC=20
	15, //VIC=21
	15, //VIC=22
	16, //VIC=23 Possible indexes: 16,17,18
	16, //VIC=24 Possible indexes: 16,17,18
	19, //VIC=25
	19, //VIC=26
	20, //VIC=27 Possible indexes: 20,21,22
	20, //VIC=28 Possible indexes: 20,21,22
	23, //VIC=29
	23, //VIC=30
	24, //VIC=31
	25, //VIC=32
	26, //VIC=33
	27, //VIC=34
	28, //VIC=35
	28, //VIC=36
	29, //VIC=37
	29, //VIC=38
	30, //VIC=39
	31, //VIC=40
	32, //VIC=41
	33, //VIC=42
	33, //VIC=43
	34, //VIC=44
	34, //VIC=45
	35, //VIC=46
	36, //VIC=47
	37, //VIC=48
	37, //VIC=49
	38, //VIC=50
	38, //VIC=51
	39, //VIC=52
	39, //VIC=53
	40, //VIC=54
	40, //VIC=55
	41, //VIC=56
	41, //VIC=57
	42, //VIC=58
	42, //VIC=59
	43, //VIC=60
	44, //VIC=61
	45, //VIC=62
	46, //VIC=63
	47, //VIC=64
};


ROM const uint8_t  hdmiVicToVideoTableIndex[LAST_KNOWN_HDMI_VIC + 1] =
{
    // Not a VIC (dummy, to start real indexes from 1, but not from 0)
    NMB_OF_VIDEO_MODES,

    NMB_OF_CEA861_VIDEO_MODES + 0,  // HDMI VIC=1
    NMB_OF_CEA861_VIDEO_MODES + 1,  // HDMI VIC=2
    NMB_OF_CEA861_VIDEO_MODES + 2,  // HDMI VIC=3
    NMB_OF_CEA861_VIDEO_MODES + 3,  // HDMI VIC=4
};

ROM const videoMode_t VideoModeTableOther[NMB_OF_VIDEO_OTHER_MODES+1] =
{
//{VIC, HVIC {H/V active}, {H/V  total},{H/V blank),{HVSyncO}, {HVSyncW},Fh , Fv , Fpix, Pclk, I/P , HPol, VPol, syst, repetition                  ,audio},
// CEA-861D video modes
{
// 640*480 P 60hz
.Vic4x3 = 0,
.Vic16x9 = 0,
.HdmiVic = 0,
.Active.H = 640,
.Active.V = 480,
.Total.H = 800,
.Total.V = 525,
.Blank.H = 160, // right_border=8, front_porch=8, sync=96,back_porch=40,left_border=8
.Blank.V = 45, // bottom_border=8, front_porch = 2, sync=2, back_porch=25, top_border=8
.SyncOffset.H = 16,
.SyncOffset.V = 10,
.SyncWidth.H = 96,
.SyncWidth.V = 2,
.HFreq = 31, // 31.469 kHz
.VFreq = 60, // 59.940 Hz
.PixFreq = 25, // 25.175 MHz
.PixClk = 2517, // 2517.5 10kHz
.Interlaced = PROG,
.HPol = NEG,
.VPol = NEG,
.NtscPal = NTSC,
.Repetition = RP1,
.MaxAudioSR8Ch = 0
},

{
// 800*600 P 60hz
.Vic4x3 = 0,
.Vic16x9 = 0,
.HdmiVic = 0,
.Active.H = 800,
.Active.V = 600,
.Total.H = 1056,
.Total.V = 628,
.Blank.H = 256, // right_border=0, front_porch=40, sync=128,back_porch=88,left_border=0
.Blank.V = 28, // bottom_border=0, front_porch = 1, sync=4, back_porch=23, top_border=0
.SyncOffset.H = 40,
.SyncOffset.V = 1,
.SyncWidth.H = 128,
.SyncWidth.V = 4,
.HFreq = 38, // 37.879 kHz
.VFreq = 60, // 60.317 Hz
.PixFreq = 40, // 40.000 MHz
.PixClk = 4000, // 4000 10kHz
.Interlaced = PROG,
.HPol = POS,
.VPol = POS,
.NtscPal = NTSC,
.Repetition = RP1,
.MaxAudioSR8Ch = 0
},

{
// 1024*768 P 60hz
.Vic4x3 = 0,
.Vic16x9 = 0,
.HdmiVic = 0,
.Active.H = 1024,
.Active.V = 768,
.Total.H = 1344,
.Total.V = 806,
.Blank.H = 320, // right_border=0, front_porch=24, sync=136,back_porch=160,left_border=0
.Blank.V = 38, // bottom_border=0, front_porch = 3, sync=6, back_porch=29, top_border=0
.SyncOffset.H = 24,
.SyncOffset.V = 3,
.SyncWidth.H = 136,
.SyncWidth.V = 6,
.HFreq = 48, // 48.363 kHz
.VFreq = 60, // 60.004 Hz
.PixFreq = 65, // 65.000 MHz
.PixClk = 6500, // 6500 10kHz
.Interlaced = PROG,
.HPol = NEG,
.VPol = NEG,
.NtscPal = NTSC,
.Repetition = RP1,
.MaxAudioSR8Ch = 0
},

{
// 1152*864 P 60hz
.Vic4x3 = 0,
.Vic16x9 = 0,
.HdmiVic = 0,
.Active.H = 1152,
.Active.V = 864,
.Total.H = 1600,
.Total.V = 900,
.Blank.H = 448, // right_border=0, front_porch=64, sync=128,back_porch=256,left_border=0
.Blank.V = 36, // bottom_border=0, front_porch = 1, sync=3, back_porch=32, top_border=0
.SyncOffset.H = 64,
.SyncOffset.V = 1,
.SyncWidth.H = 128,
.SyncWidth.V = 3,
.HFreq = 54, // 54.000 kHz = 900*60
.VFreq = 60, // 60.000 Hz
.PixFreq = 86, // 86.400 MHz = 1600*900*60
.PixClk = 8640, // 8640 10kHz
.Interlaced = PROG,
.HPol = NEG,
.VPol = NEG,
.NtscPal = NTSC,
.Repetition = RP1,
.MaxAudioSR8Ch = 0
},

{
// 1280*768 P 60hz
.Vic4x3 = 0,
.Vic16x9 = 0,
.HdmiVic = 0,
.Active.H = 1280,
.Active.V = 768,
.Total.H = 1664,
.Total.V = 798,
.Blank.H = 384, // right_border=0, front_porch=64, sync=128,back_porch=192,left_border=0
.Blank.V = 30, // bottom_border=0, front_porch = 3, sync=7, back_porch=20, top_border=0
.SyncOffset.H = 64,
.SyncOffset.V = 3,
.SyncWidth.H = 128,
.SyncWidth.V = 7,
.HFreq = 48, // 47.776 kHz
.VFreq = 60, // 59.870 Hz
.PixFreq = 79, // 79.500 MHz
.PixClk = 7950, // 7950 10kHz
.Interlaced = PROG,
.HPol = NEG,
.VPol = POS,
.NtscPal = NTSC,
.Repetition = RP1,
.MaxAudioSR8Ch = 0
},

{
// 1280*800 P 60hz
.Vic4x3 = 0,
.Vic16x9 = 0,
.HdmiVic = 0,
.Active.H = 1280,
.Active.V = 800,
.Total.H = 1680,
.Total.V = 831,
.Blank.H = 400, // right_border=0, front_porch=72, sync=128,back_porch=200,left_border=0
.Blank.V = 31, // bottom_border=0, front_porch = 3, sync=6, back_porch=22, top_border=0
.SyncOffset.H = 72,
.SyncOffset.V = 3,
.SyncWidth.H = 128,
.SyncWidth.V = 6,
.HFreq = 50, // 49.702 kHz
.VFreq = 60, // 59.810 Hz
.PixFreq = 83, // 83.500 MHz
.PixClk = 8350, // 8350 10kHz
.Interlaced = PROG,
.HPol = NEG,
.VPol = POS,
.NtscPal = NTSC,
.Repetition = RP1,
.MaxAudioSR8Ch = 0
},

{
// 1280*960 P 60hz
.Vic4x3 = 0,
.Vic16x9 = 0,
.HdmiVic = 0,
.Active.H = 1280,
.Active.V = 960,
.Total.H = 1800,
.Total.V = 1000,
.Blank.H = 520, // right_border=0, front_porch=96, sync=112,back_porch=312,left_border=0
.Blank.V = 40, // bottom_border=0, front_porch = 1, sync=3, back_porch=36, top_border=0
.SyncOffset.H = 96,
.SyncOffset.V = 1,
.SyncWidth.H = 112,
.SyncWidth.V = 3,
.HFreq = 60, // 60.000 kHz
.VFreq = 60, // 60.000 Hz
.PixFreq = 108, // 108.000 MHz
.PixClk = 10800, // 10800 10kHz
.Interlaced = PROG,
.HPol = POS,
.VPol = POS,
.NtscPal = NTSC,
.Repetition = RP1,
.MaxAudioSR8Ch = 0
},

{
// 1280*1024 P 60hz
.Vic4x3 = 0,
.Vic16x9 = 0,
.HdmiVic = 0,
.Active.H = 1280,
.Active.V = 1024,
.Total.H = 1688,
.Total.V = 1066,
.Blank.H = 408, // right_border=0, front_porch=48, sync=112,back_porch=248,left_border=0
.Blank.V = 42, // bottom_border=0, front_porch = 1, sync=3, back_porch=38, top_border=0
.SyncOffset.H = 48,
.SyncOffset.V = 1,
.SyncWidth.H = 112,
.SyncWidth.V = 3,
.HFreq = 64, // 63.981 kHz
.VFreq = 60, // 60.020 Hz
.PixFreq = 108, // 108.00 MHz
.PixClk = 10800, // 10800 10kHz
.Interlaced = PROG,
.HPol = POS,
.VPol = POS,
.NtscPal = NTSC,
.Repetition = RP1,
.MaxAudioSR8Ch = 0
},

{
// 1366*768 P 60hz
.Vic4x3 = 0,
.Vic16x9 = 0,
.HdmiVic = 0,
.Active.H = 1366,
.Active.V = 768,
.Total.H = 1792,
.Total.V = 798,
.Blank.H = 426, // right_border=0, front_porch=70, sync=143,back_porch=213,left_border=0
.Blank.V = 30, // bottom_border=0, front_porch = 3, sync=3, back_porch=24, top_border=0
.SyncOffset.H = 70,
.SyncOffset.V = 3,
.SyncWidth.H = 143,
.SyncWidth.V = 3,
.HFreq = 48, // 47.712 kHz
.VFreq = 60, // 59.790 Hz
.PixFreq = 85, // 85.500 MHz
.PixClk = 8550, // 8550 10kHz
.Interlaced = PROG,
.HPol = POS,
.VPol = POS,
.NtscPal = NTSC,
.Repetition = RP1,
.MaxAudioSR8Ch = 0
},

{
// 1600*1200 P 60hz
.Vic4x3 = 0,
.Vic16x9 = 0,
.HdmiVic = 0,
.Active.H = 1600,
.Active.V = 1200,
.Total.H = 2160,
.Total.V = 1250,
.Blank.H = 560, // right_border=0, front_porch=64, sync=192,back_porch=304,left_border=0
.Blank.V = 50, // bottom_border=0, front_porch = 1, sync=3, back_porch=46, top_border=0
.SyncOffset.H = 64,
.SyncOffset.V = 1,
.SyncWidth.H = 192,
.SyncWidth.V = 3,
.HFreq = 75, // 75.000 kHz
.VFreq = 60, // 60.000 Hz
.PixFreq = 162, // 162.000 MHz
.PixClk = 16200, // 16200 10kHz
.Interlaced = PROG,
.HPol = POS,
.VPol = POS,
.NtscPal = NTSC,
.Repetition = RP1,
.MaxAudioSR8Ch = 0
},

{0,0  , 0, {0   , 0   }, {0   , 0   }, {0  , 0 }, {0  ,  0}, {0  , 0}, 0  , 0  , 0  ,     0, 0   , 0  ,  0  ,  0   , 0  ,   0}
};


