#include "monitor.h"

T_EdidAudioData tAudioData;
T_EdidVideoData tVideoData;
T_Vsdb1Content tVsdb1;
T_Vsdb2Content tVsdb2;
unsigned char aHdrContent[2];
vector<E_Timing> aSupportTimings;



// Reference CEA-861-F <A.4 Example EDID Detailed Timing Descriptors>
static const unsigned int aDtd18Byte[][19] = {
    {HDMI_VIC_640X480P60,      0xD5, 0x09, 0x80, 0xA0, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x60, 0xA2, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18}, //640X480P60
    {HDMI_VIC_720X480I60_4_3,  0x8C, 0x0A, 0xA0, 0x14, 0x51, 0xF0, 0x16, 0x00, 0x26, 0x7C, 0x43, 0x00, 0x13, 0x8E, 0x21, 0x00, 0x00, 0x98}, //480I60 4:3
    {HDMI_VIC_720X480I60_16_9, 0x8C, 0x0A, 0xA0, 0x14, 0x51, 0xF0, 0x16, 0x00, 0x26, 0x7C, 0x43, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x98}, //480I60 16:9
    {HDMI_VIC_720X576I50_4_3,  0x8C, 0x0A, 0xA0, 0x20, 0x51, 0x20, 0x18, 0x10, 0x18, 0x7E, 0x23, 0x00, 0x13, 0x8E, 0x21, 0x00, 0x00, 0x98}, //576I50 4:3
    {HDMI_VIC_720X576I50_16_9, 0x8C, 0x0A, 0xA0, 0x20, 0x51, 0x20, 0x18, 0x10, 0x18, 0x7E, 0x23, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x98}, //576I50 16:9
    {HDMI_VIC_720X480P60_4_3,  0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 0x13, 0x8E, 0x21, 0x00, 0x00, 0x18}, //480P60 4:3
    {HDMI_VIC_720X480P60_16_9, 0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18}, //480P60 16:9
    {HDMI_VIC_720X576P50_4_3,  0x8C, 0x0A, 0xD0, 0x90, 0x20, 0x40, 0x31, 0x20, 0x0C, 0x40, 0x55, 0x00, 0x13, 0x8E, 0x21, 0x00, 0x00, 0x18}, //576P50 4:3
    {HDMI_VIC_720X576P50_16_9, 0x8C, 0x0A, 0xD0, 0x90, 0x20, 0x40, 0x31, 0x20, 0x0C, 0x40, 0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18}, //576P50 16:9
    {HDMI_VIC_1280X720P50,     0x01, 0x1D, 0x00, 0xBC, 0x52, 0xD0, 0x1E, 0x20, 0xB8, 0x28, 0x55, 0x40, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E}, //720P50
    {HDMI_VIC_1280X720P60,     0x01, 0x1D, 0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20, 0x6E, 0x28, 0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E}, //720P60
    {HDMI_VIC_1920X1080I50,    0x01, 0x1D, 0x80, 0xD0, 0x72, 0x1C, 0x16, 0x20, 0x10, 0x2C, 0x25, 0x80, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x9E}, //1080I50
    {HDMI_VIC_1920X1080I60,    0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20, 0x58, 0x2C, 0x25, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x9E}, //1080I60
    {HDMI_VIC_1920X1080P30,    0x01, 0x1D, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C, 0x45, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x1E}, //1080P30
    {HDMI_VIC_1920X1080P50,    0x02, 0x3A, 0x80, 0xD0, 0x72, 0x38, 0x2D, 0x40, 0x10, 0x2C, 0x45, 0x80, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x1E}, //1080P50
    {HDMI_VIC_1920X1080P60,    0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C, 0x45, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x1E}, //1080P60
    {HDMI_VIC_3840X2160P30,    0x04, 0x74, 0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80, 0xB0, 0x58, 0x8A, 0x00, 0x55, 0x50, 0x21, 0x00, 0x00, 0x1E}, //4K30
    {HDMI_VIC_3840X2160P60,    0x08, 0xE8, 0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80, 0xB0, 0x58, 0x8A, 0x00, 0x55, 0x50, 0x21, 0x00, 0x00, 0x1E}, //4K60
    {HDMI_VIC_4096X2160P30,    0x04, 0x74, 0x00, 0x30, 0x01, 0x70, 0x5A, 0x80, 0x58, 0x58, 0x8A, 0x00, 0x55, 0x50, 0x21, 0x00, 0x00, 0x1E}, //4096X2160P30
    {HDMI_VIC_4096X2160P60,    0x08, 0xE8, 0x00, 0x30, 0x01, 0x70, 0x5A, 0x80, 0x58, 0x58, 0x8A, 0x00, 0x55, 0x50, 0x21, 0x00, 0x00, 0x1E}, //4096X2160P60

    {VESA_800X600P60,          0xA0, 0x0F, 0x20, 0x00, 0x31, 0x58, 0x1C, 0x20, 0x28, 0x80, 0x14, 0x00, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E}, //800X600P60
    {VESA_1024X768P60,         0x64, 0x19, 0x00, 0x40, 0x41, 0x00, 0x26, 0x30, 0x18, 0x88, 0x36, 0x00, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E}, //1024X768P60
    {VESA_1280X768P60,         0x0E, 0x1F, 0x00, 0x80, 0x51, 0x00, 0x1E, 0x30, 0x40, 0x80, 0x37, 0x00, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E}, //1280X768P60
    {VESA_1280X800P60,         0x9E, 0x20, 0x00, 0x90, 0x51, 0x20, 0x1F, 0x30, 0x48, 0x80, 0x36, 0x00, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E}, //1280X800P60
    {VESA_1280X960P60,         0x30, 0x2A, 0x00, 0x08, 0x52, 0xC0, 0x28, 0x30, 0x60, 0x70, 0x13, 0x00, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E}, //1280X960P60
    {VESA_1280X1024P60,        0x30, 0x2A, 0x00, 0x98, 0x51, 0x00, 0x2A, 0x40, 0x30, 0x70, 0x13, 0x00, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E}, //1280X1024P60
    {VESA_1360X768P60,         0x66, 0x21, 0x50, 0xB0, 0x51, 0x00, 0x1B, 0x30, 0x40, 0x70, 0x36, 0x00, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E}, //1360X768P60
    {VESA_1366X768P60,         0x66, 0x21, 0x56, 0xAA, 0x51, 0x00, 0x1E, 0x30, 0x46, 0x8F, 0x33, 0x00, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E}, //1366X768P60
    {VESA_1400X1050P60,        0x8F, 0x2F, 0x78, 0xD0, 0x51, 0x1A, 0x27, 0x40, 0x58, 0x90, 0x34, 0x00, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E}, //1400X1050P60
    {VESA_1440X900P60,         0x9A, 0x29, 0xA0, 0xD0, 0x51, 0x84, 0x22, 0x30, 0x50, 0x98, 0x36, 0x00, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E}, //1440X900P60
    {VESA_1600X900P60,         0x30, 0x2A, 0x40, 0xC8, 0x60, 0x84, 0x64, 0x30, 0x18, 0x50, 0x13, 0x00, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E}, //1600X900P60
    {VESA_1600X1200P60,        0x48, 0x3F, 0x40, 0x30, 0x62, 0xB0, 0x32, 0x40, 0x40, 0xC0, 0x13, 0x00, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E}, //1600X1200P60
    {VESA_1680X1050P60,        0x21, 0x39, 0x90, 0x30, 0x62, 0x1A, 0x27, 0x40, 0x68, 0xB0, 0x36, 0x00, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E}, //1680X1050P60
    {VESA_1920X1200P60,        0x7D, 0x4B, 0x80, 0xA0, 0x72, 0xB0, 0x2D, 0x40, 0x88, 0xC8, 0x36, 0x00, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E}, //1920X1200P60
    {VESA_2560X1600P60,        0x22, 0x88, 0x00, 0xB0, 0xA3, 0x40, 0x3A, 0x60, 0xC0, 0x18, 0x36, 0x10, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E}, //2560X1600P60
};

static const T_TimingMap aTimingTable[] = {
    //VIC                       Name                   Hact  Vact  I/P Htotal Vtotal VFreq PClock
    { HDMI_VIC_1920X1080P24,    "1920x1080p@24",     { 1920, 1080, 0,  2750,  1125,  24,   74250,  }, },
    { HDMI_VIC_1920X1080P25,    "1920x1080p@25",     { 1920, 1080, 0,  2640,  1125,  25,   74250,  }, },
    { HDMI_VIC_1920X1080P30,    "1920x1080p@30",     { 1920, 1080, 0,  2200,  1125,  30,   74250,  }, },
    { HDMI_VIC_3840X2160P24,    "3840x2160p@24",     { 3840, 2160, 0,  5500,  2250,  24,   297000, }, },
    { HDMI_VIC_3840X2160P25,    "3840x2160p@25",     { 3840, 2160, 0,  5280,  2250,  25,   297000, }, },
    { HDMI_VIC_3840X2160P30,    "3840x2160p@30",     { 3840, 2160, 0,  4400,  2250,  30,   297000, }, },
    { HDMI_VIC_4096X2160P24,    "4096x2160p@24",     { 4096, 2160, 0,  5500,  2250,  24,   297000, }, },
    { HDMI_VIC_4096X2160P25,    "4096x2160p@25",     { 4096, 2160, 0,  5280,  2250,  25,   297000, }, },
    { HDMI_VIC_4096X2160P30,    "4096x2160p@30",     { 4096, 2160, 0,  4400,  2250,  30,   297000, }, },
    { HDMI_VIC_720X576P50_4_3,  "720x576p@50 4:3",   { 720,  576,  0,  864,   625,   50,   27000,  }, },
    { HDMI_VIC_720X576P50_16_9, "720x576p@50 16:9",  { 720,  576,  0,  864,   625,   50,   27000,  }, },
    { HDMI_VIC_1280X720P50,     "1280x720p@50",      { 1280, 720,  0,  1980,  750,   50,   74250,  }, },
    { HDMI_VIC_1920X1080I50,    "1920x1080i@50",     { 1920, 540,  1,  2640,  562,   50,   74250,  }, },
    { HDMI_VIC_720X576I50_4_3,  "720x576i@50 4:3",   { 720,  288,  1,  864,   312,   50,   27000,  }, },
    { HDMI_VIC_720X576I50_4_3,  "720x576i@50 4:3",   { 1440, 288,  1,  1728,  312,   50,   27000,  }, },
    { HDMI_VIC_720X576I50_16_9, "720x576i@50 16:9",  { 1440, 288,  1,  1728,  312,   50,   27000,  }, },
    { HDMI_VIC_1920X1080P50,    "1920x1080p@50",     { 1920, 1080, 0,  2640,  1125,  50,   148500, }, },
    { HDMI_VIC_3840X2160P50,    "3840x2160p@50",     { 3840, 2160, 0,  5280,  2250,  50,   594000, }, },
    { HDMI_VIC_4096X2160P50,    "4096x2160p@50",     { 4096, 2160, 0,  5280,  2250,  50,   594000, }, },
    { HDMI_VIC_640X480P60,      "640x480p@60",       { 640,  480,  0,  800,   525,   60,   25175,  }, },
    { HDMI_VIC_720X480P60_4_3,  "720x480p@60 4:3",   { 720,  480,  0,  858,   525,   60,   27000,  }, },
    { HDMI_VIC_720X480P60_16_9, "720x480p@60 16:9",  { 720,  480,  0,  858,   525,   60,   27000,  }, },
    { HDMI_VIC_1280X720P60,     "1280x720p@60",      { 1280, 720,  0,  1650,  750,   60,   74250,  }, },
    { HDMI_VIC_1920X1080I60,    "1920x1080i@60",     { 1920, 540,  1,  2200,  562,   60,   74250,  }, },
    { HDMI_VIC_720X480I60_4_3,  "720x480i@60 4:3",   { 720,  240,  1,  858,   262,   60,   27000,  }, },
    { HDMI_VIC_720X480I60_4_3,  "720x480i@60 4:3",   { 1440, 240,  1,  1716,  262,   60,   27000,  }, },
    { HDMI_VIC_720X480I60_16_9, "720x480i@60 16:9",  { 1440, 240,  1,  1716,  262,   60,   27000,  }, },
    { HDMI_VIC_1920X1080P60,    "1920x1080p@60",     { 1920, 1080, 0,  2200,  1125,  60,   148500, }, },
    { HDMI_VIC_3840X2160P60,    "3840x2160p@60",     { 3840, 2160, 0,  4400,  2250,  60,   594000, }, },
    { HDMI_VIC_4096X2160P60,    "4096x2160p@60",     { 4096, 2160, 0,  4400,  2250,  60,   594000, }, },

    //VESA                      Name                   Hact  Vact  I/P Htotal Vtotal VFreq PClock
    { VESA_800X600P60,          "800x600p@60",       { 800,  600,  0,  1056,  628,   60,   40000,  }, },
    { VESA_1024X768P60,         "1024x768p@60",      { 1024, 768,  0,  1344,  806,   60,   65000,  }, },
    { VESA_1280X768P60,         "1280x768p@60",      { 1280, 768,  0,  1644,  798,   60,   79500,  }, },
    { VESA_1280X800P60,         "1280x800p@60",      { 1280, 800,  0,  1680,  831,   60,   83500,  }, },
    { VESA_1280X960P60,         "1280x960p@60",      { 1280, 960,  0,  1800,  1000,  60,   108000, }, },
    { VESA_1280X1024P60,        "1280x1024p@60",     { 1280, 1024, 0,  1688,  1066,  60,   108000, }, },
    { VESA_1360X768P60,         "1360x768p@60",      { 1360, 768,  0,  1792,  795,   60,   85500,  }, },
    { VESA_1366X768P60,         "1366x768p@60",      { 1366, 768,  0,  1792,  798,   60,   85588,  }, },
    { VESA_1400X1050P60,        "1400x1050p@60",     { 1400, 1050, 0,  1864,  1089,  60,   121750, }, },
    { VESA_1440X900P60,         "1440x900p@60",      { 1440, 900,  0,  1904,  934,   60,   106500, }, },
    { VESA_1600X900P60,         "1600x900p@60",      { 1600, 900,  0,  1800,  1000,  60,   108000, }, },
    { VESA_1600X1200P60,        "1600x1200p@60",     { 1600, 1200, 0,  2160,  1250,  60,   162000, }, },
    { VESA_1680X1050P60,        "1680x1050p@60",     { 1680, 1050, 0,  2240,  1089,  60,   146250, }, },
    { VESA_1920X1200P60,        "1920x1200p@60",     { 1920, 1200, 0,  2592,  1245,  60,   193250, }, },
    { VESA_2560X1600P60,        "2560x1600p@60",     { 2560, 1600, 0,  3504,  1658,  60,   348500, }, },
};

void AddTiming(E_Timing i_eTiming)
{
    vector<E_Timing>::iterator it = find(aSupportTimings.begin(), aSupportTimings.end(), i_eTiming);
    if (it == aSupportTimings.end())
        aSupportTimings.push_back(i_eTiming);
}

const char *ConvertTiming2Name(int i_nVic)
{
    for (int i = 0; i < sizeof(aTimingTable) / sizeof(aTimingTable[0]); i++)
        if (i_nVic == aTimingTable[i].s32Vic)
            return aTimingTable[i].pName;
    return "None";
}

// int DtdPrint(unsigned char *i_pDtd)
// {
//     int htotal = 0;
//     int vtotal = 0;
//     htotal = H_ACTIVE + H_BLANKING;
//     vtotal = V_ACTIVE + V_BLANKING;

//     ; ///printf(("\tMode \t\"%dx%d\"", H_ACTIVE, V_ACTIVE);
//     ; ///printf(("\t# vfreq %3.3fHz, hfreq %6.3fkHz\n", (double)PIXEL_CLOCK / ((double)vtotal * (double)htotal), (double)PIXEL_CLOCK / (double)(htotal * 1000));
//     ; ///printf(("\t\t DotClock\t%f\n", (double)PIXEL_CLOCK / 1000000.0);
//     ; ///printf(("\t\t HTimings\t%u %u %u %u\n", H_ACTIVE, H_FRONT_PORCH, H_SYNC_WIDTH, htotal);
//     ; ///printf(("\t\t VTimings\t%u %u %u %u\n", V_ACTIVE, V_FRONT_PORCH, V_SYNC_WIDTH, vtotal);

//     if (INTERLACED || (SYNC_TYPE == SYNC_SEPARATE))
//         ; ///printf(("\t\t Sync Profile\t%s\"%sHSync\" \"%sVSync\"\n", INTERLACED ? "\"Interlace\" " : "", HSYNC_POSITIVE ? "+" : "-", VSYNC_POSITIVE ? "+" : "-");

//     ; ///printf(("\tEndMode\n");
//     return 0;
// }

int Dtd2Vic(unsigned char *i_pTable)
{
    int s32Vic = 0;
    unsigned char u8Valid = 0;

    for (int i = 0; i < sizeof(aDtd18Byte) / sizeof(aDtd18Byte[0]); i++) {
        u8Valid = 0;
        if ((i_pTable[0] == aDtd18Byte[i][1]) && (i_pTable[1] == aDtd18Byte[i][2])) { // pixel clock
            for (int j = 2; j < 12; j++) {
                if (i_pTable[j] == aDtd18Byte[i][j + 1])
                    u8Valid += 1;
            }
            if (u8Valid == 10) {
                s32Vic = aDtd18Byte[i][0];
                break;
            }
        }
    }
    return s32Vic;
}

// void DtdParse(unsigned char *i_pDtd)
// {
//     int i = 0;
//     int s32Vic = 0;

//     s32Vic = Dtd2Vic(i_pDtd);
//     AddTiming((E_Timing)s32Vic);
//     ; ///printf(("\tDTD support timing: %s\n", ConvertTiming2Name(s32Vic));
// }

void AudioParse(unsigned char *i_pTable, unsigned char i_u8Off)
{
    unsigned char u8Len = 0;

    ; ///printf(("\nAudio Data Block\n");

    u8Len = i_pTable[i_u8Off] & 0x1F;
    tAudioData.u8Count = u8Len / 3;
    if (tAudioData.u8Count > 10)
        tAudioData.u8Count = 10;
    for (int i = 0; i < tAudioData.u8Count; i++) {
        tAudioData.aSAD[i].eAudioFormat = (E_AudioFormat)((i_pTable[i_u8Off + 1 + 3 * i] >> 3) & 0x0F);
        tAudioData.aSAD[i].u8MaxChannels = (i_pTable[i_u8Off + 1 + 3 * i] & 0x07) + 1;
        tAudioData.aSAD[i].u8SampleRate = i_pTable[i_u8Off + 1 + 3 * i + 1];
        tAudioData.aSAD[i].u8Byte3 = i_pTable[i_u8Off + 1 + 3 * i + 2];

        ; ///printf(("\tSAD%d support audio format: %s, max channels: %d\n",           i + 1, AUDIO_FORMAT(tAudioData.aSAD[i].eAudioFormat), tAudioData.aSAD[i].u8MaxChannels);
        ; ///printf(("\tSAD%d support sample rate:", i + 1);
        for (int j = 0; j < 8; j++)
            if (tAudioData.aSAD[i].u8SampleRate & (1 << j))
                ; ///printf((" %s", SAMPLE_RATE(1 << j));
        ; ///printf(("\n");
        ; ///printf(("\tSAD%d byte3: 0x%02X\n", i + 1, tAudioData.aSAD[i].u8Byte3);
    }
}

void VideoParse(unsigned char *i_pTable, unsigned char i_u8Off)
{
    int i = 0;
    unsigned char u8Len = 0;
    unsigned char u8Tmp;

    ; ///printf(("\nVideo Data Block\n");

    u8Len = i_pTable[i_u8Off] & 0x1F;
    tVideoData.u8Count = u8Len;

    for (i = 0; i < u8Len; i++) {
        tVideoData.aVic[i] =  i_pTable[i_u8Off + 1 + i];
        u8Tmp = tVideoData.aVic[i];
        if (((u8Tmp >= 1) && (u8Tmp <= 64)) || ((u8Tmp >= 129) && (u8Tmp <= 192))) { // remove native bit
            u8Tmp &= 0x7F;
            tVideoData.aVic[i] = u8Tmp;
        }
        AddTiming((E_Timing)(tVideoData.aVic[i]));
        ; ///printf(("\t%s\n", ConvertTiming2Name(tVideoData.aVic[i]));
    }
}

void Vsdb1Parse(unsigned char *i_pTable, unsigned char i_u8Off)
{
    int i = 0;
    unsigned char u8NextOffset = 0;
    unsigned char u83dOrderLen = 0;
    unsigned char u8RemainingBytes = 0;
    unsigned char u82dVicOrder = 0;
    unsigned char u8Len = 0;
    unsigned char u83dStructure = 0;
    unsigned char u8Tmp = 0;
    unsigned short u16Tmp = 0;

    ; ///printf(("\nH14b VSDB\n");

    u8Len = i_pTable[i_u8Off] & 0x1F;
    tVsdb1.u8PhyAddr0 = i_pTable[i_u8Off + 4];
    tVsdb1.u8PhyAddr1 = i_pTable[i_u8Off + 5];
    ; ///printf(("\tCEC phyaddr: 0x%02x 0x%02x\n", tVsdb1.u8PhyAddr0, tVsdb1.u8PhyAddr1);

    if (u8Len > 5) {
        tVsdb1.u8DcSupport = i_pTable[i_u8Off + 6];
        if (tVsdb1.u8DcSupport & 0x08) {
            ; ///printf(("\tsupport deep color type:");
            if (tVsdb1.u8DcSupport & 0x10) ; ///printf((" %s", DEEP_COLOR(HDMI_DEEP_COLOR_10BIT));
            if (tVsdb1.u8DcSupport & 0x20) ; ///printf((" %s", DEEP_COLOR(HDMI_DEEP_COLOR_12BIT));
            if (tVsdb1.u8DcSupport & 0x40) ; ///printf((" %s", DEEP_COLOR(HDMI_DEEP_COLOR_16BIT));
        }
        ; ///printf(("\n");
        if (u8Len == 6) {
            return;
        }

        tVsdb1.u32MaxTmdsClock = i_pTable[i_u8Off + 7] * 5;
        ; ///printf(("\tsupport max tmds clock: %dMHz\n", tVsdb1.u32MaxTmdsClock);
        if (u8Len == 7) {
            return;
        }

        tVsdb1.u8CnAndPresent = i_pTable[i_u8Off + 8];
        if (tVsdb1.u8CnAndPresent & 0x80) { // Latency_Fields_Present
            tVsdb1.u8VideoLatency = i_pTable[i_u8Off + 9];
            tVsdb1.u8AudioLatency = i_pTable[i_u8Off + 10];
            u8NextOffset += 2;
            ; ///printf(("\tVideo Latency: %dms\n", tVsdb1.u8VideoLatency);
            ; ///printf(("\tAudio Latency: %dms\n", tVsdb1.u8AudioLatency);
        }
        if (tVsdb1.u8CnAndPresent & 0x40) { // I_Latency_Fields_Present
            tVsdb1.u8IVideoLatency = i_pTable[i_u8Off + 11];
            tVsdb1.u8IAudioLatency = i_pTable[i_u8Off + 12];
            u8NextOffset += 2;
            ; ///printf(("\tI Video Latency: %dms\n", tVsdb1.u8IVideoLatency);
            ; ///printf(("\tI Audio Latency: %dms\n", tVsdb1.u8IAudioLatency);
        }
        if (tVsdb1.u8CnAndPresent & 0x20) { // HDMI_Video_present
            tVsdb1.u8ImageSizeAnd3dPresent = i_pTable[i_u8Off + 9 + u8NextOffset];
            tVsdb1.u8ImageSizeFlag = (tVsdb1.u8ImageSizeAnd3dPresent & 0x18) >> 3;
            tVsdb1.u8HdmiVicLen = (i_pTable[i_u8Off + 10 + u8NextOffset] & 0xE0) >> 5;
            tVsdb1.u8Hdmi3dLen = i_pTable[i_u8Off + 10 + u8NextOffset] & 0x1F;

            ; ///printf(("\tHDMI 4K VIC len: %d\n", tVsdb1.u8HdmiVicLen);
            for (i = 0; i < tVsdb1.u8HdmiVicLen; i++) {
                tVsdb1.u8Hdmi4kVic[i] = i_pTable[i_u8Off + 11 + u8NextOffset + i];
                if (tVsdb1.u8Hdmi4kVic[i] == HDMI_4K_VIC_3840X2160P30) AddTiming(HDMI_VIC_3840X2160P30);
                if (tVsdb1.u8Hdmi4kVic[i] == HDMI_4K_VIC_3840X2160P25) AddTiming(HDMI_VIC_3840X2160P25);
                if (tVsdb1.u8Hdmi4kVic[i] == HDMI_4K_VIC_3840X2160P24) AddTiming(HDMI_VIC_3840X2160P24);
                if (tVsdb1.u8Hdmi4kVic[i] == HDMI_4K_VIC_4096X2160P24) AddTiming(HDMI_VIC_4096X2160P24);
                ; ///printf(("\t\t %s\n", HDMI_4K_VIC(tVsdb1.u8Hdmi4kVic[i]));
            }
            u8NextOffset += tVsdb1.u8HdmiVicLen;

            if (tVsdb1.u8Hdmi3dLen > 0) {
                tVsdb1.u8Support3D = 1;
                u8Tmp = tVsdb1.u8ImageSizeAnd3dPresent;
                if (((u8Tmp & 0x60) == 0x20) || ((u8Tmp & 0x60) == 0x40)) { // 3D_Multi_present
                    tVsdb1.u83dStructAll15_8 = i_pTable[i_u8Off + 11 + u8NextOffset];
                    tVsdb1.u83dStructAll7_0 = i_pTable[i_u8Off + 11 + u8NextOffset + 1];
                    u16Tmp = (tVsdb1.u83dStructAll15_8 << 8) + tVsdb1.u83dStructAll7_0;
                    ; ///printf(("\t3D support format:\n");
                    for (i = 0; i < 16; i++) {
                        if (u16Tmp & (1 << i)) {
                            ; ///printf(("\t\t %s\n", HDMI_3D_FORMAT(1 << i));
                        }
                    }
                    u8NextOffset += 2;
                }
                if ((u8Tmp & 0x60) == 0x40) { // 3D_Multi_present
                    unsigned char u83dVicLen = 0;
                    u16Tmp = (i_pTable[i_u8Off + 11 + u8NextOffset] << 8) + i_pTable[i_u8Off + 11 + u8NextOffset + 1];
                    ; ///printf(("\t3D support VIC:\n");
                    for (i = 0; i < 16; i++) {
                        if (u16Tmp & (1 << i)) {
                            tVsdb1.u83dVicSupport[u83dVicLen] = tVideoData.aVic[i];
                            ; ///printf(("\t\t %d-%s\n", tVsdb1.u83dVicSupport[u83dVicLen], ConvertTiming2Name(tVsdb1.u83dVicSupport[u83dVicLen]));
                            u83dVicLen++;
                        }
                    }
                    tVsdb1.u83dVicSupportLen = u83dVicLen;
                    u8NextOffset += 2;
                }
                u8RemainingBytes = u8Len - 10 - u8NextOffset;
                for (i = 0; i < u8RemainingBytes; i++) {
                    u82dVicOrder = (i_pTable[i_u8Off + 11 + u8NextOffset + i] & 0xF0) >> 4;
                    u83dStructure = i_pTable[i_u8Off + 11 + u8NextOffset + i] & 0x0F;
                    tVsdb1.u163dOrder[u83dOrderLen] = (u83dStructure << 8) + tVideoData.aVic[u82dVicOrder];
                    ; ///printf(("\t3D order: %04x\n", tVsdb1.u163dOrder[u83dOrderLen]);
                    if (u83dStructure == 0x08) i++;
                    u83dOrderLen++;
                }
                tVsdb1.u83dOrderLen = u83dOrderLen;
            }
        }
    }
}

void Vsdb2Parse(unsigned char *i_pTable, unsigned char i_u8Off)
{
    unsigned char u8Tmp;

    ; ///printf(("\nHF-VSDB\n");

    tVsdb2.u32MaxTmdsClock = i_pTable[i_u8Off + 5] * 5;
    ; ///printf(("\tsupport max tmds clock: %dMHz\n", tVsdb2.u32MaxTmdsClock);

    u8Tmp = i_pTable[i_u8Off + 6];
    tVsdb2.u8ScdcPresent = (u8Tmp & 0x80) >> 7;
    tVsdb2.u8RrCapable = (u8Tmp & 0x40) >> 6;
    tVsdb2.u8Lte340mScramble = (u8Tmp & 0x08) >> 3;
    tVsdb2.u8IndependentView = (u8Tmp & 0x04) >> 2;
    tVsdb2.u8DualView = (u8Tmp & 0x02) >> 1;
    tVsdb2.u83dOsd = u8Tmp & 0x01;

    u8Tmp = i_pTable[i_u8Off + 7];
    tVsdb2.u8Dc16Bit420 = (u8Tmp & 0x04) >> 2;
    tVsdb2.u8Dc12Bit420 = (u8Tmp & 0x02) >> 1;
    tVsdb2.u8Dc10Bit420 = u8Tmp & 0x01;
    if (tVsdb2.u8Dc16Bit420)
        ; ///printf(("\tsupport DC_48bit_420\n");
    if (tVsdb2.u8Dc12Bit420)
        ; ///printf(("\tsupport DC_36bit_420\n");
    if (tVsdb2.u8Dc10Bit420)
        ; ///printf(("\tsupport DC_30bit_420\n");
}

void ExtTagParse(unsigned char *i_pTable, unsigned char i_u8Off)
{
    int i = 0;
    int j = 0;
    unsigned char u8Tag = 0;
    unsigned char u8Len = 0;
    unsigned char u8Tmp = 0;
    unsigned char u8Vic = 0;

    u8Tag = i_pTable[i_u8Off + 1];
    u8Len = i_pTable[i_u8Off] & 0x1F;

    if (u8Tag == CEA_EXT_TAG_CDB) { // Colorimetry Data Block
        ; ///printf(("\nColorimetry Data Block\n");

        u8Tmp = i_pTable[i_u8Off + 2];
        ; ///printf(("\tsupport colorimetry:\n");
        for (i = 0; i < 8; i++) {
            if (u8Tmp & (1 << i))
                ; ///printf(("\t\t %s\n", HDMI_COLORIMETRY(1 << i));
        }

        u8Tmp = i_pTable[i_u8Off + 3];
        ; ///printf(("\tsupport metadata profiles:\n");
        if (u8Tmp & 0x01) ; ///printf(("\t\t %s\n", "Metadata Profile 0");
        if (u8Tmp & 0x02) ; ///printf(("\t\t %s\n", "Metadata Profile 1");
        if (u8Tmp & 0x04) ; ///printf(("\t\t %s\n", "Metadata Profile 2");
        if (u8Tmp & 0x08) ; ///printf(("\t\t %s\n", "Metadata Profile 3");
    } else if (u8Tag == CEA_EXT_TAG_HDR) { // HDR
        ; ///printf(("\nHDR Static Metadata Data Block\n");

        // support HDR
        aHdrContent[0] = i_pTable[i_u8Off + 2];
        aHdrContent[1] = i_pTable[i_u8Off + 3];

        ; ///printf(("\tsupport EOTF:\n");
        if (aHdrContent[0] & 0x01)
            ; ///printf(("\t\t %s\n", "Traditional SDR");
        if (aHdrContent[0] & 0x02)
            ; ///printf(("\t\t %s\n", "Traditional HDR");
        if (aHdrContent[0] & 0x04)
            ; ///printf(("\t\t %s\n", "SMPTE ST 2084 [2]");
        if (aHdrContent[0] & 0x08)
            ; ///printf(("\t\t %s\n", "Hybrid Log-Gamma(HLG)");

        ; ///printf(("\tsupport static metadata descriptor:\n");
        if (aHdrContent[1] & 0x01)
            ; ///printf(("\t\t %s\n", "Type 1");
    } else if (u8Tag == CEA_EXT_TAG_Y420VDB) { // only support 420
        ; ///printf(("\nYCbCr 4:2:0 Video Data Block\n");
        ; ///printf(("\tonly support YCbCr 4:2:0 VICs:\n");
        for (i = 0; i < u8Len - 1; i++)
            ; ///printf(("\t\t %s\n", ConvertTiming2Name(i_pTable[i_u8Off + 2 + i]));
    } else if (u8Tag == CEA_EXT_TAG_Y420CMDB) { // support 420
        ; ///printf(("\nYCbCr 4:2:0 Capability Map Data Block\n");
        ; ///printf(("\tsupport YCbCr 4:2:0 VICs:\n");
        for (i = 0; i < u8Len - 1; i++) {
            u8Tmp = i_pTable[i_u8Off + 2 + i]; // bitmap table
            if (u8Tmp)
                for (j = 0; j < 8; j++)
                    if (u8Tmp & (1 << j)) { // bitmap compare
                        u8Vic = tVideoData.aVic[i * 8 + j]; // real vic
                        ; ///printf(("\t\t %s\n", ConvertTiming2Name(u8Vic));
                    }
        }
    }
}

int ParseHdmiEdidBlock0(hwNode &nn, unsigned char *i_pEdid)
{
    int i = 0;
    int j = 0;
    unsigned char *block;
    unsigned char DisplayDescriptorFlag[2] = {0x00, 0x00};
    char ManufacturerName[4] = {0};
    unsigned short ProductCode;
    unsigned int SerialNumber;
    unsigned char WeekManufacturer;
    unsigned int YearManufacturer;
    char ProductName[13] = {0};
    unsigned short Sum = 0;
    unsigned char Checksum = 0;
    int DetailTimingBlockType = UNKNOW_DESCRIPTOR;

    ; ///printf(("\nVendor and Product Information\n");
    ManufacturerName[0] = (i_pEdid[0x08] >> 2) + 'A' - 1;
    ManufacturerName[1] = (((i_pEdid[0x08] << 3) | (i_pEdid[0x09] >> 5)) & 0x1f) + 'A' - 1;
    ManufacturerName[2] = (i_pEdid[0x09] & 0x1f) + 'A' - 1;
    ; ///printf(("\tManufacturer Name: %c%c%c\n", ManufacturerName[0], ManufacturerName[1], ManufacturerName[2]);
    nn.setVendor(string(ManufacturerName));
    ProductCode = i_pEdid[0x0B] << 8 | i_pEdid[0x0A];
    ; ///printf(("\tProduct Code: %04X\n", ProductCode);
    nn.setConfig("ProductCode", ProductCode);
    SerialNumber = i_pEdid[0x0F] << 24 | i_pEdid[0x0E] << 16 | i_pEdid[0x0D] << 8 | i_pEdid[0x0C];
    ; ///printf(("\tSerial Number: %08X\n", SerialNumber);
    nn.setConfig("SerialNumber", SerialNumber);
    WeekManufacturer = i_pEdid[0x10];
    ; ///printf(("\tWeek of Manufacture: %d\n", WeekManufacturer);
    nn.setConfig("WeekManufacturer", WeekManufacturer);
    YearManufacturer = (i_pEdid[0x11] + 1990);
    ; ///printf(("\tYear of Manufacture: %d\n", YearManufacturer);
    nn.setConfig("YearManufacturer", YearManufacturer);

    ; ///printf(("\nEDID Structure\n");
    ; ///printf(("\tVersion no.: %d\n", i_pEdid[0x12]);
    ; ///printf(("\tRevision no.: %d\n", i_pEdid[0x13]);

    ; ///printf(("\nBasic Display Parameters and Features\n");
    ; ///printf(("\tVideo Input Signal Type: %s\n", (i_pEdid[0x14] >> 7) ? "Digital" : "Analog");
    if (i_pEdid[0x14] >> 7)
        // Digital
        ; ///printf(("\tInterface signal compatible with VESA DFP 1.X: %s\n", (i_pEdid[0x14] & 0x01) ? "Compatible" : "Not Compatible");
    ; ///printf(("\tMax Horz Size (in cm): %d\n", i_pEdid[0x15]);
    ; ///printf(("\tMax Vert Size (in cm): %d\n", i_pEdid[0x16]);
    if (i_pEdid[0x17] == 0xFF)
        ; ///printf(("\tGamma Value: Not defined\n");
    else
        ; ///printf(("\tGamma Value: %d.%d\n", (i_pEdid[0x17] + 100) / 100, (i_pEdid[0x17] + 100) % 100);

    ; ///printf(("\tFeature Support (DPMS)\n");
    ; ///printf(("\t\t Standby Mode: %s\n", (i_pEdid[0x18] & 0x80) ? "Supported" : "Not Supported");
    ; ///printf(("\t\t Suspend Mode: %s\n", (i_pEdid[0x18] & 0x40) ? "Supported" : "Not Supported");
    ; ///printf(("\t\t Active Off Mode: %s\n", (i_pEdid[0x18] & 0x20) ? "Supported" : "Not Supported");
    ; ///printf(("\t\t Display Type: ");
    switch ((i_pEdid[0x18] & 0x18) >> 3) {
    case 0: ; ///printf(("Monochrome display\n");
        break;
    case 1: ; ///printf(("RGB color display\n");
        break;
    case 2: ; ///printf(("Non-RGB multicolor display\n");
        break;
    case 3: ; ///printf(("Undefined display\n");
        break;
    }
    ; ///printf(("\t\t Color Space: %s\n", (i_pEdid[0x18] & 0x04)       ? "sRGB" : "Alternate");
    ; ///printf(("\t\t Preferred Timing: %s\n", (i_pEdid[0x18] & 0x02)  ? "1st DTD" : "Non indicated");
    ; ///printf(("\t\t GTF Timing: %s\n", (i_pEdid[0x18] & 0x01)        ? "Supported" : "Not Supported");

    ; ///printf(("\nColor Characteristic\n");
    ; ///printf(("\tRed_x: 0.%.3d\n", ((i_pEdid[0x1B] << 2)  | ((i_pEdid[0x19] >> 6) & 0x03)) * 1000 / 1024);
    ; ///printf(("\tRed_y: 0.%.3d\n", ((i_pEdid[0x1C] << 2)  | ((i_pEdid[0x19] >> 4) & 0x03)) * 1000 / 1024);
    ; ///printf(("\tGreen_x: 0.%.3d\n", ((i_pEdid[0x1D] << 2)  | ((i_pEdid[0x19] >> 2) & 0x03)) * 1000 / 1024);
    ; ///printf(("\tGreen_y: 0.%.3d\n", ((i_pEdid[0x1E] << 2) | (i_pEdid[0x19] & 0x03)) * 1000 / 1024);
    ; ///printf(("\tBlue_x: 0.%.3d\n", ((i_pEdid[0x1F] << 2)  | ((i_pEdid[0x1A] >> 6) & 0x03)) * 1000 / 1024);
    ; ///printf(("\tBlue_y: 0.%.3d\n", ((i_pEdid[0x20] << 2)  | ((i_pEdid[0x1A] >> 4) & 0x03)) * 1000 / 1024);
    ; ///printf(("\tWhite_x: 0.%.3d\n", ((i_pEdid[0x21] << 2)  | ((i_pEdid[0x1A] >> 2) & 0x03)) * 1000 / 1024);
    ; ///printf(("\tWhite_y: 0.%.3d\n", ((i_pEdid[0x22] << 2)  | (i_pEdid[0x1A] & 0x03)) * 1000 / 1024);

    ; ///printf(("\nEstablished Timings\n");

                if (i_pEdid[0x23] & 0x80)    nn.addCapability("720x400P70");
                if (i_pEdid[0x23] & 0x40)    nn.addCapability("720x400P88");
                if (i_pEdid[0x23] & 0x20)    nn.addCapability("640x480P60"); 
                if (i_pEdid[0x23] & 0x10)    nn.addCapability("640x480P67");
                if (i_pEdid[0x23] & 0x08)    nn.addCapability("640x480P72");
                if (i_pEdid[0x23] & 0x04)    nn.addCapability("640x480P75");
                if (i_pEdid[0x23] & 0x02)    nn.addCapability("800x600P56");
                if (i_pEdid[0x23] & 0x01)    nn.addCapability("800x600P60");
                if (i_pEdid[0x24] & 0x80)    nn.addCapability("800x600P72");
                if (i_pEdid[0x24] & 0x40)    nn.addCapability("800x600P75");
                if (i_pEdid[0x24] & 0x20)    nn.addCapability("832x624P75");
                if (i_pEdid[0x24] & 0x10)    nn.addCapability("1024x768P87");
                if (i_pEdid[0x24] & 0x08)    nn.addCapability("1024x768P60");
                if (i_pEdid[0x24] & 0x04)    nn.addCapability("1024x768P70");
                if (i_pEdid[0x24] & 0x02)    nn.addCapability("1024x768P75");
                if (i_pEdid[0x24] & 0x01)    nn.addCapability("1280x1024P75");
                if (i_pEdid[0x25] & 0x80)    nn.addCapability("1152x870P75");

                ; ///printf(("\nStandard Timings\n");
                for (i = 0; i < 8; i++) {
                    ; ///printf(("\tStandard Timing %d\n", i + 1);
                    if ((i_pEdid[0x26 + i * 2] == 0x01) && (i_pEdid[0x26 + i * 2 + 1] == 0x01))
                        ; ///printf(("\t\t Unused\n");
                    else {
                        unsigned int HActive = (i_pEdid[0x26 + i * 2] + 31) * 8;
                        unsigned int VActive = 0;
                        unsigned char u8Rate = 0;
                        ; ///printf(("\t\t Horizontal active pixels: %d\n", HActive);
                        ; ///printf(("\t\t Image Aspect ratio: ");
                        switch ((i_pEdid[0x26 + i * 2 + 1] & 0xC0) >> 6) {
                        case 0: ; ///printf(("16:10\n"); VActive = HActive * 10 / 16;
                            break;
                        case 1: ; ///printf(("4:3\n");   VActive = HActive * 3 / 4;
                            break;
                        case 2: ; ///printf(("5:4\n");   VActive = HActive * 4 / 5;
                            break;
                        case 3: ; ///printf(("16:9\n");  VActive = HActive * 9 / 16;
                            break;
                        }
                        u8Rate = (i_pEdid[0x26 + i * 2 + 1] & 0x3F) + 60;
                        ; ///printf(("\t\t Refresh Rate: %d Hz\n", u8Rate);

                        if (u8Rate == 60) { // only care 60Hz
                            int j = 0;
                            for (j = 0; j < sizeof(aTimingTable) / sizeof(aTimingTable[0]); j++) {
                                if ((HActive == aTimingTable[j].tTimingInfo.u32HActive)
                                        && (VActive == aTimingTable[j].tTimingInfo.u32VActive)
                                        && (u8Rate == aTimingTable[j].tTimingInfo.u32VFreq)) {
                                    AddTiming((E_Timing)(aTimingTable[j].s32Vic));
                                    break;
                                }
                            }
                        }
                    }
                }

                block = i_pEdid + BLOCK0_DTD_START;

                for (i = 0; i < BLOCK0_TOTAL_DTD; i++, block += DTD_SIZE) {
                    if (memcmp(DisplayDescriptorFlag, block, 2) == 0) {
                        if (block[2] != 0)
                            DetailTimingBlockType = UNKNOW_DESCRIPTOR;
                        else
                            DetailTimingBlockType = block[3];
                    } else
                        DetailTimingBlockType = DETAILED_TIMING_DEFINITION;


                    ; ///printf(("\nBlock0 Detailed Timing Descriptor %d\n", i + 1);
                    switch (DetailTimingBlockType) {
                    case DETAILED_TIMING_DEFINITION:
                        // DtdParse(block);
                        // DtdPrint(block);
                        break;
                    case MONITOR_NAME:
                        for (j = 5; j < DTD_SIZE; j++) {
                            ProductName[j - 5] = block[j];
                            if (block[j] == 0x0A) { // end string
                                ProductName[j - 5] = 0x00;
                                break;
                            }
                        }
                        ; ///printf(("\tMonitor name: %s\n", ProductName);
                        nn.setProduct(ProductName);
                        break;
                    case RANGE_LIMITS:
                        ; ///printf(("\tMin. Vertical rate (Hz): %d\n", block[5]);
                        ; ///printf(("\tMax. Vertical rate (Hz): %d\n", block[6]);
                        ; ///printf(("\tMin. Horizontal rate (KHz): %d\n", block[7]);
                        ; ///printf(("\tMax. Horizontal rate (KHz): %d\n", block[8]);
                        ; ///printf(("\tMax. Supported Pixel Clock rate (KHz): %d\n", block[9] * 10);
                        if (block[10] == 0x00)
                            ; ///printf(("\tDefault GTF supported\n");
                        else if (block[10] == 0x02)
                            ; ///printf(("\tSecondary GTF supported\n");
                        break;
                    case PRODUCT_SERIAL_NUMBER:
                        ; ///printf(("\tProduct Serial Number: ");
                        for (int k = 0; k < 13; k++) {
                            ; ///printf(("%c", block[k + 5]);
                            if (block[k + 5] == 0x0A) break;
                        }
                        break;
                    default:
                        break;
                    }
                }

                ; ///printf(("\nExtension Flag\n");
                ; ///printf(("\tExtension Flag: %d\n", i_pEdid[0x7E]);

                for (i = 0; i < EDID_LENGTH - 1; i++)
                    Sum += i_pEdid[i];

                Checksum = 0x100 - (Sum & 0xFF);
                ; ///printf(("\nChecksum\n");
                ; ///printf(("\tEDID Block0 checksum: %X\n", Checksum);

                return 1;
            }

            // unsigned char ParseHdmiEdidBlock1(unsigned char *i_pEdid) {
            //     int i = 0;
            //     int j = 0;
            //     unsigned char u8Offset = 0;
            //     unsigned char u8Len = 0;
            //     unsigned char u8CeaType = 0;
            //     unsigned char aDtd[18] = {0};
            //     unsigned char u8DtdOffset = 0;
            //     unsigned char u8Tmp = 0;
            //     unsigned char u8SpeakerInfo = 0;

            //     u8DtdOffset = i_pEdid[2];
            //     u8Offset = 4;
            //     u8Len = (i_pEdid[4] & 0x1F);
            //     for (;;) {
            //         u8CeaType = (i_pEdid[u8Offset] & 0xE0) >> 5;
            //         switch (u8CeaType) {
            //         case CEA_TAG_ADB:   AudioParse(i_pEdid, u8Offset);  break;
            //         case CEA_TAG_VDB:   VideoParse(i_pEdid, u8Offset);  break;
            //         case CEA_TAG_VSDB:
            //             if ((i_pEdid[u8Offset + 1] == 0x03) && (i_pEdid[u8Offset + 2] == 0x0C) && (i_pEdid[u8Offset + 3] == 0x00))
            //                 Vsdb1Parse(i_pEdid, u8Offset);
            //             else if ((i_pEdid[u8Offset + 1] == 0xD8) && (i_pEdid[u8Offset + 2] == 0x5D) && (i_pEdid[u8Offset + 3] == 0xC4))
            //                 Vsdb2Parse(i_pEdid, u8Offset);
            //             break;
            //         case CEA_TAG_SPK:
            //             u8SpeakerInfo = i_pEdid[u8Offset + 1];
            //             ; ///printf(("\nSpeaker Allocation Data Block\n");
            //             ; ///printf(("\tspeaker info: 0x%02X\n", u8SpeakerInfo);
            //             break;
            //         case CEA_TAG_EXT:
            //             ExtTagParse(i_pEdid, u8Offset);
            //             break;
            //         default:
            //             break;
            //         }
            //         u8Offset += (u8Len + 1);
            //         u8Len = (i_pEdid[u8Offset] & 0x1F);
            //         if (u8Offset >= u8DtdOffset)
            //             break;
            //     }

            //     u8Tmp = EDID_LENGTH - u8DtdOffset;
            //     u8Tmp = u8Tmp / DTD_SIZE;
            //     for (i = 0; i < u8Tmp; i++) {
            //         ; ///printf(("\nBlock1 Detailed Timing Descriptor %d\n", i + 1);
            //         for (j = 0; j < DTD_SIZE; j++)
            //             aDtd[j] = i_pEdid[u8DtdOffset + j + i * DTD_SIZE];
            //         DtdParse(aDtd);
            //         DtdPrint(aDtd);
            //     }
            //     return 1;
            // }

            // int test_demo(int argc, char *argv[]) {
            //     // int i = 0;
            //     // ParseHdmiEdidBlock0(aTestEdid);
            //     // ParseHdmiEdidBlock1(aTestEdid + EDID_LENGTH);

            //     // ; ///printf(("\nTotal support %ld timings:\n", aSupportTimings.size());
            //     // for (i = 0; i < aSupportTimings.size(); i++)
            //     //     ; ///printf(("\t%s\n", ConvertTiming2Name(aSupportTimings[i]));

            //     return 0;
            // }
