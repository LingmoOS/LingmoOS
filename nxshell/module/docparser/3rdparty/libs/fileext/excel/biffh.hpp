/**
 * @brief     Excel files (xls/xlsx) into HTML сonverter
 * @package   excel
 * @file      biffh.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright python-excel (https://github.com/python-excel/xlrd)
 * @date      02.12.2016 -- 18.10.2017
 */
#pragma once

#include <vector>
#include <unordered_map>


namespace excel {

/** Classification that has been inferred from format string */
enum {
	FUN,  ///< Unknown
	FDT,  ///< Date
	FNU,  ///< Number
	FGE,  ///< General
	FTX   ///< Text
};
/** Cell types */
enum {
	XL_CELL_EMPTY,   ///< Empty
	XL_CELL_TEXT,    ///< Text
	XL_CELL_NUMBER,  ///< Number
	XL_CELL_DATE,    ///< Date
	XL_CELL_BOOLEAN, ///< Boolean
	XL_CELL_ERROR,   ///< Error
	XL_CELL_BLANK    ///< Blank. For use in debugging, gathering stats, etc
};
/** Record codes */
enum {
	XL_RK2                = 0x7e,
	XL_ARRAY              = 0x0221,
	XL_ARRAY2             = 0x0021,
	XL_BLANK              = 0x0201,
	XL_BLANK_B2           = 0x01,
	XL_BOF                = 0x809,
	XL_BOOLERR            = 0x205,
	XL_BOOLERR_B2         = 0x5,
	XL_BOUNDSHEET         = 0x85,
	XL_BUILTINFMTCOUNT    = 0x56,
	XL_CF                 = 0x01B1,
	XL_CODEPAGE           = 0x42,
	XL_COLINFO            = 0x7D,
	XL_COLUMNDEFAULT      = 0x20,  ///< BIFF2 only
	XL_COLWIDTH           = 0x24,  ///< BIFF2 only
	XL_CONDFMT            = 0x01B0,
	XL_CONTINUE           = 0x3c,
	XL_COUNTRY            = 0x8C,
	XL_DATEMODE           = 0x22,
	XL_DEFAULTROWHEIGHT   = 0x0225,
	XL_DEFCOLWIDTH        = 0x55,
	XL_DIMENSION          = 0x200,
	XL_DIMENSION2         = 0x0,
	XL_EFONT              = 0x45,
	XL_EOF                = 0x0a,
	XL_EXTERNNAME         = 0x23,
	XL_EXTERNSHEET        = 0x17,
	XL_EXTSST             = 0xff,
	XL_FEAT11             = 0x872,
	XL_FILEPASS           = 0x2f,
	XL_FONT               = 0x31,
	XL_FONT_B3B4          = 0x231,
	XL_FORMAT             = 0x41e,
	XL_FORMAT2            = 0x1E,  ///< BIFF2, BIFF3
	XL_FORMULA            = 0x6,
	XL_FORMULA3           = 0x206,
	XL_FORMULA4           = 0x406,
	XL_GCW                = 0xab,
	XL_HLINK              = 0x01B8,
	XL_QUICKTIP           = 0x0800,
	XL_HORIZONTALBREAKS   = 0x1b,
	XL_INDEX              = 0x20b,
	XL_INTEGER            = 0x2,  ///< BIFF2 only
	XL_IXFE               = 0x44,  ///< BIFF2 only
	XL_LABEL              = 0x204,
	XL_LABEL_B2           = 0x04,
	XL_LABELRANGES        = 0x15f,
	XL_LABELSST           = 0xfd,
	XL_LEFTMARGIN         = 0x26,
	XL_TOPMARGIN          = 0x28,
	XL_RIGHTMARGIN        = 0x27,
	XL_BOTTOMMARGIN       = 0x29,
	XL_HEADER             = 0x14,
	XL_FOOTER             = 0x15,
	XL_HCENTER            = 0x83,
	XL_VCENTER            = 0x84,
	XL_MERGEDCELLS        = 0xE5,
	XL_MSO_DRAWING        = 0x00EC,
	XL_MSO_DRAWING_GROUP  = 0x00EB,
	XL_MSO_DRAWING_SELECT = 0x00ED,
	XL_MULRK              = 0xbd,
	XL_MULBLANK           = 0xbe,
	XL_NAME               = 0x18,
	XL_NOTE               = 0x1c,
	XL_NUMBER             = 0x203,
	XL_NUMBER_B2          = 0x3,
	XL_OBJ                = 0x5D,
	XL_PAGESETUP          = 0xA1,
	XL_PALETTE            = 0x92,
	XL_PANE               = 0x41,
	XL_PRINTGRIDLINES     = 0x2B,
	XL_PRINTHEADERS       = 0x2A,
	XL_RK                 = 0x27e,
	XL_ROW                = 0x208,
	XL_ROW_B2             = 0x08,
	XL_RSTRING            = 0xd6,
	XL_SCL                = 0x00A0,
	XL_SHEETHDR           = 0x8F,  ///< BIFF4W only
	XL_SHEETPR            = 0x81,
	XL_SHEETSOFFSET       = 0x8E,  ///< BIFF4W only
	XL_SHRFMLA            = 0x04bc,
	XL_SST                = 0xfc,
	XL_STANDARDWIDTH      = 0x99,
	XL_STRING             = 0x207,
	XL_STRING_B2          = 0x7,
	XL_STYLE              = 0x293,
	XL_SUPBOOK            = 0x1AE,  ///< aka EXTERNALBOOK in OOo docs
	XL_TABLEOP            = 0x236,
	XL_TABLEOP2           = 0x37,
	XL_TABLEOP_B2         = 0x36,
	XL_TXO                = 0x1b6,
	XL_UNCALCED           = 0x5e,
	XL_UNKNOWN            = 0xffff,
	XL_VERTICALPAGEBREAKS = 0x1a,
	XL_WINDOW2            = 0x023E,
	XL_WINDOW2_B2         = 0x003E,
	XL_WRITEACCESS        = 0x5C,
	XL_WSBOOL             = XL_SHEETPR,
	XL_XF                 = 0xe0,
	XL_XF2                = 0x0043,  ///< BIFF2 version of XF record
	XL_XF3                = 0x0243,  ///< BIFF3 version of XF record
	XL_XF4                = 0x0443,  ///< BIFF4 version of XF record
};
/** Cell type from format type */
const std::unordered_map<int, int> CELL_TYPE_FROM_FORMAT_TYPE {
	{FNU, XL_CELL_NUMBER},
	{FUN, XL_CELL_NUMBER},
	{FGE, XL_CELL_NUMBER},
	{FDT, XL_CELL_DATE},
	{FTX, XL_CELL_NUMBER}  // Yes, number can be formatted as text
};
/** BOF codes */
const std::vector<int> BOF_CODES = {
	0x0809, 0x0409, 0x0209, 0x0009
};
/** Error text from code */
const std::unordered_map<int, std::string> ERROR_TEXT_FROM_CODE {
	{0x00, "#NULL!"},  // Intersection of two cell ranges is empty
	{0x07, "#DIV/0!"}, // Division by zero
	{0x0F, "#VALUE!"}, // Wrong type of operand
	{0x17, "#REF!"},   // Illegal or deleted cell reference
	{0x1D, "#NAME?"},  // Wrong function or range name
	{0x24, "#NUM!"},   // Value range overflow
	{0x2A, "#N/A"}     // Argument or function not available
};
/** Error code from text */
const std::unordered_map<std::string, int> ERROR_CODE_FROM_TEXT {
	{"#NULL!",  0x00},  // Intersection of two cell ranges is empty
	{"#DIV/0!", 0x07},  // Division by zero
	{"#VALUE!", 0x0F},  // Wrong type of operand
	{"#REF!",   0x17},  // Illegal or deleted cell reference
	{"#NAME?",  0x1D},  // Wrong function or range name
	{"#NUM!",   0x24},  // Value range overflow
	{"#N/A",    0x2A}   // Argument or function not available
};

}  // End namespace