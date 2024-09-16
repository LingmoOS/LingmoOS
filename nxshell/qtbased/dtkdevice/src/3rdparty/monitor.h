#ifndef __MONITOR_H__
#define __MONITOR_H__

#include "hw.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <algorithm>    // std::find + transform

using namespace std;

#define EDID_LENGTH 128
#define BLOCK0_DTD_START 0x36
#define BLOCK0_TOTAL_DTD 4
#define DTD_SIZE 18

#define UPPER_NIBBLE(x) (((128|64|32|16) & (x)) >> 4)
#define LOWER_NIBBLE(x) ((1|2|4|8) & (x))
#define COMBINE_HI_4LO(hi, lo) ((((unsigned)hi) << 4) | (unsigned)lo)
#define COMBINE_HI_8LO(hi, lo) ((((unsigned)hi) << 8) | (unsigned)lo)
#define PIXEL_CLOCK_LO     (unsigned)i_pDtd[ 0 ]
#define PIXEL_CLOCK_HI     (unsigned)i_pDtd[ 1 ]
#define PIXEL_CLOCK        (COMBINE_HI_8LO( PIXEL_CLOCK_HI,PIXEL_CLOCK_LO )*10000)
#define H_ACTIVE_LO        (unsigned)i_pDtd[ 2 ]
#define H_BLANKING_LO      (unsigned)i_pDtd[ 3 ]
#define H_ACTIVE_HI        UPPER_NIBBLE( (unsigned)i_pDtd[ 4 ] )
#define H_ACTIVE           COMBINE_HI_8LO( H_ACTIVE_HI, H_ACTIVE_LO )
#define H_BLANKING_HI      LOWER_NIBBLE( (unsigned)i_pDtd[ 4 ] )
#define H_BLANKING         COMBINE_HI_8LO( H_BLANKING_HI, H_BLANKING_LO )
#define V_ACTIVE_LO        (unsigned)i_pDtd[ 5 ]
#define V_BLANKING_LO      (unsigned)i_pDtd[ 6 ]
#define V_ACTIVE_HI        UPPER_NIBBLE( (unsigned)i_pDtd[ 7 ] )
#define V_ACTIVE           COMBINE_HI_8LO( V_ACTIVE_HI, V_ACTIVE_LO )
#define V_BLANKING_HI      LOWER_NIBBLE( (unsigned)i_pDtd[ 7 ] )
#define V_BLANKING         COMBINE_HI_8LO( V_BLANKING_HI, V_BLANKING_LO )
#define H_SYNC_OFFSET_LO   (unsigned)i_pDtd[ 8 ]
#define H_SYNC_WIDTH_LO    (unsigned)i_pDtd[ 9 ]
#define V_SYNC_OFFSET_LO   UPPER_NIBBLE( (unsigned)i_pDtd[ 10 ] )
#define V_SYNC_WIDTH_LO    LOWER_NIBBLE( (unsigned)i_pDtd[ 10 ] )
#define V_SYNC_WIDTH_HI    ((unsigned)i_pDtd[ 11 ] & (1|2))
#define V_SYNC_OFFSET_HI   (((unsigned)i_pDtd[ 11 ] & (4|8)) >> 2)
#define H_SYNC_WIDTH_HI    (((unsigned)i_pDtd[ 11 ] & (16|32)) >> 4)
#define H_SYNC_OFFSET_HI   (((unsigned)i_pDtd[ 11 ] & (64|128)) >> 6)
#define V_SYNC_WIDTH       COMBINE_HI_4LO( V_SYNC_WIDTH_HI, V_SYNC_WIDTH_LO )
#define V_FRONT_PORCH      COMBINE_HI_4LO( V_SYNC_OFFSET_HI, V_SYNC_OFFSET_LO )
#define H_SYNC_WIDTH       COMBINE_HI_4LO( H_SYNC_WIDTH_HI, H_SYNC_WIDTH_LO )
#define H_FRONT_PORCH      COMBINE_HI_4LO( H_SYNC_OFFSET_HI, H_SYNC_OFFSET_LO )
#define H_SIZE_LO          (unsigned)i_pDtd[ 12 ]
#define V_SIZE_LO          (unsigned)i_pDtd[ 13 ]
#define H_SIZE_HI          UPPER_NIBBLE( (unsigned)i_pDtd[ 14 ] )
#define V_SIZE_HI          LOWER_NIBBLE( (unsigned)i_pDtd[ 14 ] )
#define H_SIZE             COMBINE_HI_8LO( H_SIZE_HI, H_SIZE_LO )
#define V_SIZE             COMBINE_HI_8LO( V_SIZE_HI, V_SIZE_LO )
#define H_BORDER           (unsigned)i_pDtd[ 15 ]
#define V_BORDER           (unsigned)i_pDtd[ 16 ]
#define FLAGS              (unsigned)i_pDtd[ 17 ]
#define INTERLACED         (FLAGS&128)
#define SYNC_TYPE          (FLAGS&3<<3)  /* bits 4,3 */
#define SYNC_SEPARATE      (3<<3)
#define HSYNC_POSITIVE     (FLAGS & 4)
#define VSYNC_POSITIVE     (FLAGS & 2)

//HDMI Resolution
typedef enum {
    HDMI_VIC_VESA_TIMING     = 0,
    HDMI_VIC_640X480P60      = 1,
    HDMI_VIC_720X480P60_4_3  = 2,
    HDMI_VIC_720X480P60_16_9 = 3,
    HDMI_VIC_1280X720P60     = 4,
    HDMI_VIC_1920X1080I60    = 5,
    HDMI_VIC_720X480I60_4_3  = 6,
    HDMI_VIC_720X480I60_16_9 = 7,
    HDMI_VIC_1920X1080P60    = 16,
    HDMI_VIC_720X576P50_4_3  = 17,
    HDMI_VIC_720X576P50_16_9 = 18,
    HDMI_VIC_1280X720P50     = 19,
    HDMI_VIC_1920X1080I50    = 20,
    HDMI_VIC_720X576I50_4_3  = 21,
    HDMI_VIC_720X576I50_16_9 = 22,
    HDMI_VIC_1920X1080P50    = 31,
    HDMI_VIC_1920X1080P24    = 32,
    HDMI_VIC_1920X1080P25    = 33,
    HDMI_VIC_1920X1080P30    = 34,
    HDMI_VIC_3840X2160P24    = 93,
    HDMI_VIC_3840X2160P25    = 94,
    HDMI_VIC_3840X2160P30    = 95,
    HDMI_VIC_3840X2160P50    = 96,
    HDMI_VIC_3840X2160P60    = 97,
    HDMI_VIC_4096X2160P24    = 98,
    HDMI_VIC_4096X2160P25    = 99,
    HDMI_VIC_4096X2160P30    = 100,
    HDMI_VIC_4096X2160P50    = 101,
    HDMI_VIC_4096X2160P60    = 102,

    VESA_800X600P60          = 1000, // Fixme
    VESA_1024X768P60,
    VESA_1280X768P60,
    VESA_1280X800P60,
    VESA_1280X960P60,
    VESA_1280X1024P60,
    VESA_1360X768P60,
    VESA_1366X768P60,
    VESA_1400X1050P60,
    VESA_1440X900P60,
    VESA_1600X900P60,
    VESA_1600X1200P60,
    VESA_1680X1050P60,
    VESA_1920X1200P60,
    VESA_2560X1600P60,
} E_Timing;

typedef enum {
    HDMI_4K_VIC_3840X2160P30 = 1,
    HDMI_4K_VIC_3840X2160P25 = 2,
    HDMI_4K_VIC_3840X2160P24 = 3,
    HDMI_4K_VIC_4096X2160P24 = 4
} E_Hdmi4kVic;

typedef enum {
    HDMI_3D_FORMAT_FRAME_PACKING          = 0x01,
    HDMI_3D_FORMAT_FIELD_ALTERNATIVE      = 0x02,
    HDMI_3D_FORMAT_LINE_ALTERNATIVE       = 0x04,
    HDMI_3D_FORMAT_SIDE_BY_SIDE_FULL      = 0x08,
    HDMI_3D_FORMAT_L_DEPTH                = 0x10,
    HDMI_3D_FORMAT_L_DEPTH_GRAPHICS       = 0x20,
    HDMI_3D_FORMAT_TOP_AND_BOTTOM         = 0x40,
    HDMI_3D_FORMAT_SIDE_BY_SIDE_HALF_HORZ = 0x100,
    HDMI_3D_FORMAT_SIDE_BY_SIDE_HALF_ALL  = 0x8000
} E_Hdmi3dFormat;

typedef enum {
    AUDIO_FORMAT_NONE = 0,
    AUDIO_FORMAT_PCM,
    AUDIO_FORMAT_AC3,
    AUDIO_FORMAT_MPEG1,
    AUDIO_FORMAT_MP3,
    AUDIO_FORMAT_MPEG2,
    AUDIO_FORMAT_AAC,
    AUDIO_FORMAT_DTS,
    AUDIO_FORMAT_ATRAC,
    AUDIO_FORMAT_ONE_BIT_AUDIO,
    AUDIO_FORMAT_DOLBY,
    AUDIO_FORMAT_DTS_HD,
    AUDIO_FORMAT_MAT,
    AUDIO_FORMAT_DST,
    AUDIO_FORMAT_WMA_PRO,
} E_AudioFormat;

typedef enum {
    AUDIO_SAMPLE_RATE_32K  = 0x01, // 32K
    AUDIO_SAMPLE_RATE_44K  = 0x02, // 44.1K
    AUDIO_SAMPLE_RATE_48K  = 0x04, // 48K
    AUDIO_SAMPLE_RATE_88K  = 0x08, // 88.2K
    AUDIO_SAMPLE_RATE_96K  = 0x10, // 96K
    AUDIO_SAMPLE_RATE_176K = 0x20, // 176.4K
    AUDIO_SAMPLE_RATE_192K = 0x40, // 192K
} E_AudioSampleRate;

typedef enum {
    AUDIO_SAMPLE_SIZE_16BIT = 0x01,
    AUDIO_SAMPLE_SIZE_20BIT = 0x02,
    AUDIO_SAMPLE_SIZE_24BIT = 0x04,
} E_AudioSampleSize;

typedef enum {
    CEA_TAG_ADB  = 1, // Audio Data Block
    CEA_TAG_VDB  = 2, // Video Data Block
    CEA_TAG_VSDB = 3, // Vendor-Specific Data Block
    CEA_TAG_SPK  = 4, // Speaker Allocation Data Block
    CEA_TAG_DTC  = 5, // VESA Display Transfer Characteristic Data Block
    CEA_TAG_EXT  = 7, // Use Extended Tag
} E_CeaDataBlockTag;

typedef enum {
    CEA_EXT_TAG_VCDB     = 0,  // Video Capability Data Block
    CEA_EXT_TAG_VSVDB    = 1,  // Vendor-Specific Video Data Block
    CEA_EXT_TAG_DDDB     = 2,  // VESA Display Device Data Block
    CEA_EXT_TAG_VTDB     = 3,  // VESA Video Timing Block Extension
    CEA_EXT_TAG_CDB      = 5,  // Colorimetry Data Block
    CEA_EXT_TAG_HDR      = 6,  // HDR Static Metadata Data Block
    CEA_EXT_TAG_VFPDB    = 13, // Video Format Preference Data Block
    CEA_EXT_TAG_Y420VDB  = 14, // YCBCR 4:2:0 Video Data Block
    CEA_EXT_TAG_Y420CMDB = 15, // YCBCR 4:2:0 Capability Map Data Block
    CEA_EXT_TAG_VSADB    = 17, // Vendor-Specific Audio Data Block
    CEA_EXT_TAG_IDB      = 32, // InfoFrame Data Block
} E_CeaExtDataBlockTag;

typedef enum {
    HDMI_DEEP_COLOR_8BIT = 0,
    HDMI_DEEP_COLOR_10BIT,
    HDMI_DEEP_COLOR_12BIT,
    HDMI_DEEP_COLOR_16BIT
} E_HdmiDeepColorType;

typedef enum {
    HDMI_XV_YCC601    = 0x01,
    HDMI_XV_YCC709    = 0x02,
    HDMI_S_YCC601     = 0x04,
    HDMI_ADOBE_YCC601 = 0x08,
    HDMI_ADOBE_RGB    = 0x10,
    HDMI_BT2020_C_YCC = 0x20,
    HDMI_BT2020_YCC   = 0x40,
    HDMI_BT2020_RGB   = 0x80
} E_HdmiColorimetry;

typedef enum {
    PRODUCT_SERIAL_NUMBER          = 0xFF,
    ASCII_DATA_STRING              = 0xFE,
    RANGE_LIMITS                   = 0xFD,
    MONITOR_NAME                   = 0xFC,
    COLOR_POINT_DATA               = 0xFB,
    STANDART_TIMING                = 0xFA,
    DISPLAY_COLOR_MANAGEMENT       = 0xF9,
    CVT_TIMING_CODES               = 0xF8,
    ESTABLISHED_TIMINGS_III        = 0xF7,
    DUMMY_DESCRIPTOR               = 0x10,
    MANUFACTURER_SPECIFIED_DISPLAY = 0x0F,
    DETAILED_TIMING_DEFINITION     = 0x00,
    UNKNOW_DESCRIPTOR              = -1,
} E_DisplayDescriptor;

#define AUDIO_FORMAT(i) ( \
                          (i == AUDIO_FORMAT_PCM)           ? "PCM" : \
                          (i == AUDIO_FORMAT_AC3)           ? "AC3" : \
                          (i == AUDIO_FORMAT_MPEG1)         ? "MPEG1" : \
                          (i == AUDIO_FORMAT_MP3)           ? "MP3" : \
                          (i == AUDIO_FORMAT_MPEG2)         ? "MPEG2" : \
                          (i == AUDIO_FORMAT_AAC)           ? "AAC" : \
                          (i == AUDIO_FORMAT_DTS)           ? "DTS" : \
                          (i == AUDIO_FORMAT_ATRAC)         ? "ATRAC" : \
                          (i == AUDIO_FORMAT_ONE_BIT_AUDIO) ? "ONE_BIT_AUDIO" : \
                          (i == AUDIO_FORMAT_DOLBY)         ? "DOLBY" : \
                          (i == AUDIO_FORMAT_DTS_HD)        ? "DTS_HD" : \
                          (i == AUDIO_FORMAT_MAT)           ? "MAT" : \
                          (i == AUDIO_FORMAT_DST)           ? "DST" : \
                          (i == AUDIO_FORMAT_WMA_PRO)       ? "WMA_PRO" : "None")

#define SAMPLE_RATE(i) ( \
                         (i == AUDIO_SAMPLE_RATE_32K)  ? "32K" : \
                         (i == AUDIO_SAMPLE_RATE_44K)  ? "44.1K" : \
                         (i == AUDIO_SAMPLE_RATE_48K)  ? "48K" : \
                         (i == AUDIO_SAMPLE_RATE_88K)  ? "88.2K" : \
                         (i == AUDIO_SAMPLE_RATE_96K)  ? "96K" : \
                         (i == AUDIO_SAMPLE_RATE_176K) ? "176.4K" : \
                         (i == AUDIO_SAMPLE_RATE_192K) ? "192K" : "None")

#define DEEP_COLOR(i) ( \
                        (i == HDMI_DEEP_COLOR_8BIT)  ? "8BIT" : \
                        (i == HDMI_DEEP_COLOR_10BIT) ? "10BIT" : \
                        (i == HDMI_DEEP_COLOR_12BIT) ? "12BIT" : \
                        (i == HDMI_DEEP_COLOR_16BIT) ? "16BIT" : "None")

#define HDMI_4K_VIC(i) ( \
                         (i == HDMI_4K_VIC_3840X2160P30) ? "3840x2160p@30" : \
                         (i == HDMI_4K_VIC_3840X2160P25) ? "3840x2160p@25" : \
                         (i == HDMI_4K_VIC_3840X2160P24) ? "3840x2160p@24" : \
                         (i == HDMI_4K_VIC_4096X2160P24) ? "4096x2160p@24" : "None")

#define HDMI_3D_FORMAT(i) ( \
                            (i == HDMI_3D_FORMAT_FRAME_PACKING)          ? "FramePacking" : \
                            (i == HDMI_3D_FORMAT_FIELD_ALTERNATIVE)      ? "FieldAlternative" : \
                            (i == HDMI_3D_FORMAT_LINE_ALTERNATIVE)       ? "LineAlternative" : \
                            (i == HDMI_3D_FORMAT_SIDE_BY_SIDE_FULL)      ? "SideBySide(Full)" : \
                            (i == HDMI_3D_FORMAT_L_DEPTH)                ? "L+Depth" : \
                            (i == HDMI_3D_FORMAT_L_DEPTH_GRAPHICS)       ? "L+Depth+Graphics" : \
                            (i == HDMI_3D_FORMAT_TOP_AND_BOTTOM)         ? "TopAndBottom" : \
                            (i == HDMI_3D_FORMAT_SIDE_BY_SIDE_HALF_HORZ) ? "SideBySide(Half Horz)" : \
                            (i == HDMI_3D_FORMAT_SIDE_BY_SIDE_HALF_ALL)  ? "SideBySide(Half All)" : "None")

#define HDMI_COLORIMETRY(i) ( \
                              (i == HDMI_XV_YCC601)    ? "XVYCC-601" : \
                              (i == HDMI_XV_YCC709)    ? "XVYCC-709" : \
                              (i == HDMI_S_YCC601)     ? "sYCC-601" : \
                              (i == HDMI_ADOBE_YCC601) ? "AdobeYCC-601" : \
                              (i == HDMI_ADOBE_RGB)    ? "AdobeRGB" : \
                              (i == HDMI_BT2020_C_YCC) ? "BT.2020-cYCC" : \
                              (i == HDMI_BT2020_YCC)   ? "BT.2020-YCC" : \
                              (i == HDMI_BT2020_RGB)   ? "BT.2020-RGB" : "None")

typedef struct {
    E_AudioFormat eAudioFormat;
    unsigned char u8MaxChannels;
    unsigned char u8SampleRate; // Byte2
    unsigned char u8Byte3; // Byte3
} T_AudioDescriptor;

typedef struct {
    // count of Short Audio Descriptors
    unsigned char u8Count;
    // up to 10 Short Audio Descriptors
    T_AudioDescriptor aSAD[10];
} T_EdidAudioData;

typedef struct {
    // count of Short Video Descriptors
    unsigned char u8Count;
    // up to 32 Short Video Descriptors
    unsigned char aVic[32];
} T_EdidVideoData;

typedef struct {
    unsigned int  u32MaxTmdsClock;
    unsigned char u8ScdcPresent;
    unsigned char u8RrCapable;
    unsigned char u8Lte340mScramble;
    unsigned char u8IndependentView;
    unsigned char u8DualView;
    unsigned char u83dOsd;
    unsigned char u8Dc16Bit420;
    unsigned char u8Dc12Bit420;
    unsigned char u8Dc10Bit420;
} T_Vsdb2Content;

typedef struct {
    unsigned char u8PhyAddr0;
    unsigned char u8PhyAddr1;
    unsigned char u8DcSupport;
    unsigned int  u32MaxTmdsClock;
    unsigned char u8CnAndPresent;
    unsigned char u8VideoLatency;
    unsigned char u8AudioLatency;
    unsigned char u8IVideoLatency;
    unsigned char u8IAudioLatency;
    unsigned char u8HdmiVicLen;
    unsigned char u8Hdmi3dLen;
    unsigned char u8Hdmi4kVic[4];
    unsigned char u8ImageSizeAnd3dPresent;
    unsigned char u8ImageSizeFlag;
    unsigned char u8Support3D;
    unsigned char u83dStructAll15_8;
    unsigned char u83dStructAll7_0;
    unsigned char u83dVicSupportLen;
    unsigned char u83dVicSupport[16];
    unsigned char u83dOrderLen;
    unsigned short u163dOrder[16];
} T_Vsdb1Content;

typedef struct {
    unsigned int u32HActive;
    unsigned int u32VActive;
    unsigned int u32ScanMode; // 0:progressive, 1:interlace
    unsigned int u32HTotal;
    unsigned int u32VTotal;
    unsigned int u32VFreq;
    unsigned int u32PixelClock;
} T_TimingInfo;

typedef struct {
    int s32Vic; // 0 for VESA timing
    const char *pName;
    T_TimingInfo tTimingInfo;
} T_TimingMap;

int ParseHdmiEdidBlock0(hwNode &nn, unsigned char *i_pEdid);


#endif // __MONITOR_H__
