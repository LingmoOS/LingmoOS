/**
 * @brief     Excel files (xls/xlsx) into HTML сonverter
 * @package   excel
 * @file      formatting.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright python-excel (https://github.com/python-excel/xlrd)
 * @date      02.12.2016 -- 28.01.2018
 */
#include <regex>

#include "biffh.hpp"

#include "format.hpp"


namespace excel {

/** Standard for US English locale format code types */
const std::unordered_map<int, int> STD_FORMAT_CODE_TYPES {
	{0,  FGE}, {1,  FNU}, {2,  FNU}, {3,  FNU}, {4,  FNU},
	{5,  FNU}, {6,  FNU}, {7,  FNU}, {8,  FNU}, {9,  FNU},
	{10, FNU}, {11, FNU}, {12, FNU}, {13, FNU}, {14, FDT},
	{15, FDT}, {16, FDT}, {17, FDT}, {18, FDT}, {19, FDT},
	{20, FDT}, {21, FDT}, {22, FDT}, {27, FDT}, {28, FDT},
	{29, FDT}, {30, FDT}, {31, FDT}, {32, FDT}, {33, FDT},
	{34, FDT}, {35, FDT}, {36, FDT}, {37, FNU}, {38, FNU},
	{39, FNU}, {40, FNU}, {41, FNU}, {42, FNU}, {43, FNU},
	{44, FNU}, {45, FDT}, {46, FDT}, {47, FDT}, {48, FNU},
	{49, FTX}, {50, FDT}, {51, FDT}, {52, FDT}, {53, FDT},
	{54, FDT}, {55, FDT}, {56, FDT}, {57, FDT}, {58, FDT},
	{59, FNU}, {60, FNU}, {61, FNU}, {62, FNU}, {67, FNU},
	{68, FNU}, {69, FNU}, {70, FNU}, {71, FDT}, {72, FDT},
	{73, FDT}, {74, FDT}, {75, FDT}, {76, FDT}, {77, FDT},
	{78, FDT}, {79, FDT}, {80, FDT}, {81, FDT}
};
/** Standard for US English locale format string */
const std::unordered_map<int, std::string> STD_FORMAT_STRINGS {
	{0x00, "General"},
	{0x01, "0"},
	{0x02, "0.00"},
	{0x03, "#,##0"},
	{0x04, "#,##0.00"},
	{0x05, "$#,##0_);($#,##0)"},
	{0x06, "$#,##0_);[Red]($#,##0)"},
	{0x07, "$#,##0.00_);($#,##0.00)"},
	{0x08, "$#,##0.00_);[Red]($#,##0.00)"},
	{0x09, "0%"},
	{0x0a, "0.00%"},
	{0x0b, "0.00E+00"},
	{0x0c, "# \?/\?"},
	{0x0d, "# \?\?/\?\?"},
	{0x0e, "m/d/yy"},
	{0x0f, "d-mmm-yy"},
	{0x10, "d-mmm"},
	{0x11, "mmm-yy"},
	{0x12, "h:mm AM/PM"},
	{0x13, "h:mm:ss AM/PM"},
	{0x14, "h:mm"},
	{0x15, "h:mm:ss"},
	{0x16, "m/d/yy h:mm"},
	{0x25, "#,##0_);(#,##0)"},
	{0x26, "#,##0_);[Red](#,##0)"},
	{0x27, "#,##0.00_);(#,##0.00)"},
	{0x28, "#,##0.00_);[Red](#,##0.00)"},
	{0x29, "_(* #,##0_);_(* (#,##0);_(* \"-\"_);_(@_)"},
	{0x2a, "_($* #,##0_);_($* (#,##0);_($* \"-\"_);_(@_)"},
	{0x2b, "_(* #,##0.00_);_(* (#,##0.00);_(* \"-\"\?\?_);_(@_)"},
	{0x2c, "_($* #,##0.00_);_($* (#,##0.00);_($* \"-\"\?\?_);_(@_)"},
	{0x2d, "mm:ss"},
	{0x2e, "[h]:mm:ss"},
	{0x2f, "mm:ss.0"},
	{0x30, "##0.0E+0"},
	{0x31, "@"}
};
/** Non date formats */
const std::unordered_map<std::string, int> NON_DATE_FORMATS {
	{"0.00E+00", 1},
	{"##0.0E+0", 1},
	{"General",  1},
	{"GENERAL",  1},  // OOo Calc 1.1.4 does this
	{"general",  1},  // pyExcelerator 0.6.3 does this
	{"@",        1}
};
/** Built-in style name list */
const std::vector<std::string> BUILIT_STYLE_NAMES {
	"Normal",
	"RowLevel_",
	"ColLevel_",
	"Comma",
	"Currency",
	"Percent",
	"Comma [0]",
	"Currency [0]",
	"Hyperlink",
	"Followed Hyperlink"
};
/** Skip charecters dictionary */
const std::unordered_map<char, int> SKIP_CHAR_DICT {
	{' ', 1}, {'$', 1}, {')', 1}, {'(', 1},
	{'+', 1}, {'-', 1}, {'/', 1}, {':', 1}
};
/** Date charecters dictionary */
const std::unordered_map<char, int> DATE_CHAR_DICT {
	{'y', 5}, {'Y', 5}, {'m', 5}, {'M', 5},
	{'d', 5}, {'D', 5}, {'h', 5}, {'H', 5},
	{'s', 5}, {'S', 5}
};
/** Number charecters dictionary */
const std::unordered_map<char, int> NUMBER_CHAR_DICT = {
	{'0', 5}, {'#', 5}, {'?', 5}
};
/** BIFF 5 default palette */
const std::vector<std::vector<unsigned char>> DEFAULT_PALETTE_B5 {
	{  0,   0,   0}, {255, 255, 255}, {255,   0,   0}, {  0, 255,   0},
	{  0,   0, 255}, {255, 255,   0}, {255,   0, 255}, {  0, 255, 255},
	{128,   0,   0}, {  0, 128,   0}, {  0,   0, 128}, {128, 128,   0},
	{128,   0, 128}, {  0, 128, 128}, {192, 192, 192}, {128, 128, 128},
	{153, 153, 255}, {153,  51, 102}, {255, 255, 204}, {204, 255, 255},
	{102,   0, 102}, {255, 128, 128}, {  0, 102, 204}, {204, 204, 255},
	{  0,   0, 128}, {255,   0, 255}, {255, 255,   0}, {  0, 255, 255},
	{128,   0, 128}, {128,   0,   0}, {  0, 128, 128}, {  0,   0, 255},
	{  0, 204, 255}, {204, 255, 255}, {204, 255, 204}, {255, 255, 153},
	{153, 204, 255}, {255, 153, 204}, {204, 153, 255}, {227, 227, 227},
	{ 51, 102, 255}, { 51, 204, 204}, {153, 204,   0}, {255, 204,   0},
	{255, 153,   0}, {255, 102,   0}, {102, 102, 153}, {150, 150, 150},
	{  0,  51, 102}, { 51, 153, 102}, {  0,  51,   0}, { 51,  51,   0},
	{153,  51,   0}, {153,  51, 102}, { 51,  51, 153}, { 51,  51,  51}
};
/** BIFF 2 default palette */
const std::vector<std::vector<unsigned char>> DEFAULT_PALETTE_B2(DEFAULT_PALETTE_B5.begin(),
																 DEFAULT_PALETTE_B5.begin() + 16);
/** BIFF 8 default palette */
const std::vector<std::vector<unsigned char>> DEFAULT_PALETTE_B8 {  // {red, green, blue}
	{  0,   0,   0}, {255, 255, 255}, {255,   0,   0}, {  0, 255,   0}, // 0
	{  0,   0, 255}, {255, 255,   0}, {255,   0, 255}, {  0, 255, 255}, // 4
	{128,   0,   0}, {  0, 128,   0}, {  0,   0, 128}, {128, 128,   0}, // 8
	{128,   0, 128}, {  0, 128, 128}, {192, 192, 192}, {128, 128, 128}, // 12
	{153, 153, 255}, {153,  51, 102}, {255, 255, 204}, {204, 255, 255}, // 16
	{102,   0, 102}, {255, 128, 128}, {  0, 102, 204}, {204, 204, 255}, // 20
	{  0,   0, 128}, {255,   0, 255}, {255, 255,   0}, {  0, 255, 255}, // 24
	{128,   0, 128}, {128,   0,   0}, {  0, 128, 128}, {  0,   0, 255}, // 28
	{  0, 204, 255}, {204, 255, 255}, {204, 255, 204}, {255, 255, 153}, // 32
	{153, 204, 255}, {255, 153, 204}, {204, 153, 255}, {255, 204, 153}, // 36
	{ 51, 102, 255}, { 51, 204, 204}, {153, 204,   0}, {255, 204,   0}, // 40
	{255, 153,   0}, {255, 102,   0}, {102, 102, 153}, {150, 150, 150}, // 44
	{  0,  51, 102}, { 51, 153, 102}, {  0,  51,   0}, { 51,  51,   0}, // 48
	{153,  51,   0}, {153,  51, 102}, { 51,  51, 153}, { 51,  51,  51}  // 52
};
/** XLS Default palette */
const std::unordered_map<int, std::vector<std::vector<unsigned char>>> DEFAULT_PALETTE {
	{80, DEFAULT_PALETTE_B8},
	{70, DEFAULT_PALETTE_B5},
	{50, DEFAULT_PALETTE_B5},
	{45, DEFAULT_PALETTE_B2},
	{40, DEFAULT_PALETTE_B2},
	{30, DEFAULT_PALETTE_B2},
	{21, DEFAULT_PALETTE_B2},
	{20, DEFAULT_PALETTE_B2}
};
/** Format bracketed text regex */
const std::regex FORMAT_BRACKETED_TEXT("\\[[^\\]]*\\]");

// Formatting public:
Formatting::Formatting(Book* book)
	: m_book(book) {}

void Formatting::initializeBook() {
	m_book->m_xfEpilogueDone = false;
	if (!m_book->m_addStyle)
		return;
	// Add 8 invariant colors
	for (int i = 0; i < 8; ++i)
		m_book->m_colorMap[i] = DEFAULT_PALETTE_B8[i];
	// Add default palette depending on the version
	auto& defaultPalette = DEFAULT_PALETTE.at(m_book->m_biffVersion);
	int paletteSize = static_cast<int>(defaultPalette.size());
	for (int i = 0; i < paletteSize; ++i)
		m_book->m_colorMap[i+8] = defaultPalette[i];
	// Add the specials -- None means the RGB value is not known
	// System window text color for border lines
	//m_book->m_colorMap[paletteSize+8]   = {255, 255, 255};
	// System window background color for pattern background
	//m_book->m_colorMap[paletteSize+8+1] = {255, 255, 255};
	// System ToolTip text color (used in note objects)
	m_book->m_colorMap[0x51]   = {255, 255, 255};
	// 32767, system window text color for fonts
	m_book->m_colorMap[0x7FFF] = {255, 255, 255};
}

void Formatting::handleFont(const std::string& data) {
	if (!m_book->m_addStyle)
		return;
	if (m_book->m_encoding.empty())
		m_book->getEncoding();

	int size = static_cast<int>(m_book->m_fontList.size());
	if (size == 4) {
		Font f;
		f.m_name      = "Dummy Font";
		f.m_fontIndex = size;
		m_book->m_fontList.push_back(f);
		size += 1;
	}
	Font f;
	f.m_fontIndex = size;
	unsigned short flags;
	if (m_book->m_biffVersion >= 50) {
		f.m_height        = m_book->readByte<unsigned short>(data, 0, 2);
		flags             = m_book->readByte<unsigned short>(data, 2, 2);
		f.m_color.m_index = m_book->readByte<unsigned short>(data, 4, 2);
		f.m_weight        = m_book->readByte<unsigned short>(data, 6, 2);
		f.m_escapement    = m_book->readByte<unsigned short>(data, 8, 2);
		f.m_underlineType = m_book->readByte<unsigned char>(data, 10, 1);
		f.m_family        = m_book->readByte<unsigned char>(data, 11, 1);
		f.m_characterSet  = m_book->readByte<unsigned char>(data, 12, 1);
		f.m_isBold        = flags  & 1;
		f.m_isItalic      = (flags & 2)  >> 1;
		f.m_isUnderlined  = (flags & 4)  >> 2;
		f.m_isStruckOut   = (flags & 8)  >> 3;
		f.m_isOutlined    = (flags & 16) >> 4;
		f.m_isShadowed    = (flags & 32) >> 5;
		if (m_book->m_biffVersion >= 80)
			f.m_name = m_book->unpackUnicode(data, 14, 1);
		else
			f.m_name = m_book->unpackString(data, 14, 1);
	}
	else if (m_book->m_biffVersion >= 30) {
		f.m_height        = m_book->readByte<unsigned short>(data, 0, 2);
		flags             = m_book->readByte<unsigned short>(data, 2, 2);
		f.m_color.m_index = m_book->readByte<unsigned short>(data, 4, 2);
		f.m_isBold        = flags  & 1;
		f.m_isItalic      = (flags & 2)  >> 1;
		f.m_isUnderlined  = (flags & 4)  >> 2;
		f.m_isStruckOut   = (flags & 8)  >> 3;
		f.m_isOutlined    = (flags & 16) >> 4;
		f.m_isShadowed    = (flags & 32) >> 5;
		f.m_name          = m_book->unpackString(data, 6, 1);
		f.m_weight        = f.m_isBold ? 700 : 400;
		f.m_escapement    = 0;  // None
		f.m_underlineType = f.m_isUnderlined;  // None or Single
		f.m_family        = 0;  // Unknown / don't care
		f.m_characterSet  = 1;  // System default (0 means "ANSI Latin")
	}
	else {  // BIFF2
		f.m_height        = m_book->readByte<unsigned short>(data, 0, 2);
		flags             = m_book->readByte<unsigned short>(data, 2, 2);
		f.m_color.m_index = 0x7FFF;  // "System window text color"
		f.m_isBold        = flags  & 1;
		f.m_isItalic      = (flags & 2) >> 1;
		f.m_isUnderlined  = (flags & 4) >> 2;
		f.m_isStruckOut   = (flags & 8) >> 3;
		f.m_isOutlined    = 0;
		f.m_isShadowed    = 0;
		f.m_name          = m_book->unpackString(data, 4, 1);
		f.m_weight        = f.m_isBold ? 700 : 400;
		f.m_escapement    = 0;  // None
		f.m_underlineType = f.m_isUnderlined;  // None or Single
		f.m_family        = 0;  // Unknown / don't care
		f.m_characterSet  = 1;  // System default (0 means "ANSI Latin")
	}
	m_book->m_fontList.push_back(f);
}

void Formatting::handleFormat(const std::string& data, int recordType) {
	if (!m_book->m_addStyle)
		return;
	int bv = m_book->m_biffVersion;
	if (recordType == XL_FORMAT2)
		bv = std::min(bv, 30);
	if (!m_book->m_encoding.empty())
		m_book->getEncoding();

	int position = 2;
	unsigned short formatKey;
	if (bv >= 50)
		formatKey = m_book->readByte<unsigned short>(data, 0, 2);
	else {
		formatKey = m_book->m_actualFormatCount;
		if (bv <= 30)
			position = 0;
	}
	m_book->m_actualFormatCount += 1;

	std::string unistrg;
	if (bv >= 80)
		unistrg = m_book->unpackUnicode(data, 2);
	else
		unistrg = m_book->unpackString(data, position, 1);

	int type = isDateFormattedString(unistrg) ? FDT : FGE;
	Format format(formatKey, type, unistrg);

	m_book->m_formatMap.emplace(formatKey, format);
	m_book->m_formatList.push_back(format);
}

void Formatting::handleXf(const std::string& data) {
	if (!m_book->m_addStyle)
		return;

	XF xf;
	xf.m_alignment.m_indentLevel    = 0;
	xf.m_alignment.m_isShrinkToFit  = 0;
	xf.m_alignment.m_textDirection  = 0;
	xf.m_border.m_diagUp            = false;
	xf.m_border.m_diagDown          = false;
	xf.m_border.m_diagColor.m_index = 0;
	xf.m_border.m_diagLineStyle     = 0;  // No line

	// Fill in known standard formats, i.e. do this once before process first XF record
	if (m_book->m_biffVersion >= 50 && !m_book->m_xfCount) {
		for (const auto& x : STD_FORMAT_CODE_TYPES) {
			if (m_book->m_formatMap.find(x.first) == m_book->m_formatMap.end()) {
				// Note: many standard format codes (mostly CJK date formats) have format strings that
				// vary by locale. Type (date or numeric) is recorded but formatString will be `None`
				const int& type = STD_FORMAT_CODE_TYPES.at(x.first);
				m_book->m_formatMap.emplace(x.first, Format(x.first, type,
															STD_FORMAT_STRINGS.at(x.first)));
			}
		}
	}
	if (m_book->m_biffVersion >= 80) {
		xf.m_fontIndex               = m_book->readByte<unsigned short>(data, 0, 2);
		xf.m_formatKey               = m_book->readByte<unsigned short>(data, 2, 2);
		unsigned short parTypeFlags  = m_book->readByte<unsigned short>(data, 4, 2);
		unsigned char parAlign1      = m_book->readByte<unsigned char>(data, 6, 1);
		xf.m_alignment.m_rotation    = m_book->readByte<unsigned char>(data, 7, 1);
		unsigned char parAlign2      = m_book->readByte<unsigned char>(data, 8, 1);
		unsigned char parUsed        = m_book->readByte<unsigned char>(data, 9, 1);
		unsigned int borderBackgr1   = m_book->readByte<unsigned int>(data, 10, 4);
		int borderBackgr2            = m_book->readByte<int>(data, 14, 4);
		unsigned short borderBackgr3 = m_book->readByte<unsigned short>(data, 18, 2);

		xf.m_protection.m_isCellLocked    = (parTypeFlags & 0x01) >> 0;
		xf.m_protection.m_isFormulaHidden = (parTypeFlags & 0x02) >> 1;
		// Following is not in OOo docs, but is mentioned in Gnumeric source
		xf.m_isStyle          = (parTypeFlags & 0x0004) >> 2;
		xf.m_lotusPrefix      = (parTypeFlags & 0x0008) >> 3;  // Meaning is not known
		xf.m_parentStyleIndex = (parTypeFlags & 0xFFF0) >> 4;

		xf.m_alignment.m_horizontalAlign = (parAlign1 & 0x07) >> 0;  // parAlign1
		xf.m_alignment.m_isTextWrapped   = (parAlign1 & 0x08) >> 3;
		xf.m_alignment.m_verticalAlign   = (parAlign1 & 0x70) >> 4;
		xf.m_alignment.m_indentLevel     = (parAlign2 & 0x0f) >> 0;  // parAlign2
		xf.m_alignment.m_isShrinkToFit   = (parAlign2 & 0x10) >> 4;
		xf.m_alignment.m_textDirection   = (parAlign2 & 0xC0) >> 6;

		unsigned char flags = parUsed >> 2;
		xf.m_formatFlag     = (flags >> 0) & 1;
		xf.m_fontFlag       = (flags >> 1) & 1;
		xf.m_alignmentFlag  = (flags >> 2) & 1;
		xf.m_borderFlag     = (flags >> 3) & 1;
		xf.m_backgroundFlag = (flags >> 4) & 1;
		xf.m_protectionFlag = (flags >> 5) & 1;

		xf.m_border.m_leftLineStyle       = (borderBackgr1 & 0x0000000f) >> 0;  // borderBackgr1
		xf.m_border.m_rightLineStyle      = (borderBackgr1 & 0x000000f0) >> 4;
		xf.m_border.m_topLineStyle        = (borderBackgr1 & 0x00000f00) >> 8;
		xf.m_border.m_bottomLineStyle     = (borderBackgr1 & 0x0000f000) >> 12;
		xf.m_border.m_leftColor.m_index   = (borderBackgr1 & 0x007f0000) >> 16;
		xf.m_border.m_rightColor.m_index  = (borderBackgr1 & 0x3f800000) >> 23;
		xf.m_border.m_diagDown            = (borderBackgr1 & 0x40000000) >> 30;
		xf.m_border.m_diagUp              = (borderBackgr1 & 0x80000000) >> 31;
		xf.m_border.m_topColor.m_index    = (borderBackgr2 & 0x0000007F) >> 0;  // borderBackgr2
		xf.m_border.m_bottomColor.m_index = (borderBackgr2 & 0x00003F80) >> 7;
		xf.m_border.m_diagColor.m_index   = (borderBackgr2 & 0x001FC000) >> 14;
		xf.m_border.m_diagLineStyle       = (borderBackgr2 & 0x01E00000) >> 21;

		xf.m_background.m_fillPattern             = (borderBackgr2 & 0xFC000000) >> 26;  // borderBackgr2
		xf.m_background.m_patternColor.m_index    = (borderBackgr3 & 0x007F) >> 0;  // borderBackgr3
		xf.m_background.m_backgroundColor.m_index = (borderBackgr3 & 0x3F80) >> 7;
	}
	else if (m_book->m_biffVersion >= 50) {
		xf.m_fontIndex              = m_book->readByte<unsigned short>(data, 0, 2);
		xf.m_formatKey              = m_book->readByte<unsigned short>(data, 2, 2);
		unsigned short parTypeFlags = m_book->readByte<unsigned short>(data, 4, 2);
		unsigned char parAlign1     = m_book->readByte<unsigned char>(data, 6, 1);
		unsigned char orientUsed    = m_book->readByte<unsigned char>(data, 7, 1);
		unsigned int borderBackgr1  = m_book->readByte<unsigned int>(data, 8, 4);
		int borderBackgr2           = m_book->readByte<int>(data, 12, 4);

		xf.m_protection.m_isCellLocked    = (parTypeFlags & 0x01) >> 0;
		xf.m_protection.m_isFormulaHidden = (parTypeFlags & 0x02) >> 1;

		xf.m_isStyle          = (parTypeFlags & 0x0004) >> 2;
		xf.m_lotusPrefix      = (parTypeFlags & 0x0008) >> 3;  // Meaning is not known
		xf.m_parentStyleIndex = (parTypeFlags & 0xFFF0) >> 4;

		xf.m_alignment.m_horizontalAlign = (parAlign1 & 0x07) >> 0;
		xf.m_alignment.m_isTextWrapped   = (parAlign1 & 0x08) >> 3;
		xf.m_alignment.m_verticalAlign   = (parAlign1 & 0x70) >> 4;

		int orientation[]         = {0, 255, 90, 180};
		xf.m_alignment.m_rotation = orientation[orientUsed & 0x03];

		unsigned char flags = orientUsed >> 2;
		xf.m_formatFlag     = (flags >> 0) & 1;
		xf.m_fontFlag       = (flags >> 1) & 1;
		xf.m_alignmentFlag  = (flags >> 2) & 1;
		xf.m_borderFlag     = (flags >> 3) & 1;
		xf.m_backgroundFlag = (flags >> 4) & 1;
		xf.m_protectionFlag = (flags >> 5) & 1;

		xf.m_border.m_bottomLineStyle     = (borderBackgr1 & 0x01C00000) >> 22;  // borderBackgr1
		xf.m_border.m_bottomColor.m_index = (borderBackgr1 & 0xFE000000) >> 25;
		xf.m_border.m_topLineStyle        = (borderBackgr2 & 0x00000007) >> 0;  // borderBackgr2
		xf.m_border.m_leftLineStyle       = (borderBackgr2 & 0x00000038) >> 3;
		xf.m_border.m_rightLineStyle      = (borderBackgr2 & 0x000001C0) >> 6;
		xf.m_border.m_topColor.m_index    = (borderBackgr2 & 0x0000FE00) >> 9;
		xf.m_border.m_leftColor.m_index   = (borderBackgr2 & 0x007F0000) >> 16;
		xf.m_border.m_rightColor.m_index  = (borderBackgr2 & 0x3F800000) >> 23;

		xf.m_background.m_patternColor.m_index    = (borderBackgr1 & 0x0000007F) >> 0;
		xf.m_background.m_backgroundColor.m_index = (borderBackgr1 & 0x00003F80) >> 7;
		xf.m_background.m_fillPattern             = (borderBackgr1 & 0x003F0000) >> 16;
	}
	else if (m_book->m_biffVersion >= 40) {
		xf.m_fontIndex              = m_book->readByte<unsigned char>(data, 0, 1);
		xf.m_formatKey              = m_book->readByte<unsigned char>(data, 1, 1);
		unsigned short parTypeFlags = m_book->readByte<unsigned short>(data, 2, 2);
		unsigned char alignOrient   = m_book->readByte<unsigned char>(data, 4, 1);
		unsigned char parUsed       = m_book->readByte<unsigned char>(data, 5, 1);
		unsigned short background34 = m_book->readByte<unsigned short>(data, 6, 2);
		unsigned int border34       = m_book->readByte<unsigned int>(data, 8, 4);

		xf.m_protection.m_isCellLocked    = (parTypeFlags & 0x01) >> 0;
		xf.m_protection.m_isFormulaHidden = (parTypeFlags & 0x02) >> 1;

		xf.m_isStyle          = (parTypeFlags & 0x0004) >> 2;
		xf.m_lotusPrefix      = (parTypeFlags & 0x0008) >> 3;  // Meaning is not known
		xf.m_parentStyleIndex = (parTypeFlags & 0xFFF0) >> 4;

		xf.m_alignment.m_horizontalAlign = (alignOrient & 0x07) >> 0;
		xf.m_alignment.m_isTextWrapped   = (alignOrient & 0x08) >> 3;
		xf.m_alignment.m_verticalAlign   = (alignOrient & 0x30) >> 4;

		int orientation[]         = {0, 255, 90, 180};
		xf.m_alignment.m_rotation = orientation[(alignOrient & 0xC0) >> 6];

		unsigned char flags = parUsed >> 2;
		xf.m_formatFlag     = (flags >> 0) & 1;
		xf.m_fontFlag       = (flags >> 1) & 1;
		xf.m_alignmentFlag  = (flags >> 2) & 1;
		xf.m_borderFlag     = (flags >> 3) & 1;
		xf.m_backgroundFlag = (flags >> 4) & 1;
		xf.m_protectionFlag = (flags >> 5) & 1;

		xf.m_border.m_topLineStyle        = (border34 & 0x00000007) >> 0;
		xf.m_border.m_topColor.m_index    = (border34 & 0x000000F8) >> 3;
		xf.m_border.m_leftLineStyle       = (border34 & 0x00000700) >> 8;
		xf.m_border.m_leftColor.m_index   = (border34 & 0x0000F800) >> 11;
		xf.m_border.m_bottomLineStyle     = (border34 & 0x00070000) >> 16;
		xf.m_border.m_bottomColor.m_index = (border34 & 0x00F80000) >> 19;
		xf.m_border.m_rightLineStyle      = (border34 & 0x07000000) >> 24;
		xf.m_border.m_rightColor.m_index  = (border34 & 0xF8000000) >> 27;

		xf.m_background.m_fillPattern             = (background34 & 0x003F) >> 0;
		xf.m_background.m_patternColor.m_index    = (background34 & 0x07C0) >> 6;
		xf.m_background.m_backgroundColor.m_index = (background34 & 0xF800) >> 11;
	}
	else if (m_book->m_biffVersion == 30) {
		xf.m_fontIndex              = m_book->readByte<unsigned char>(data, 0, 1);
		xf.m_formatKey              = m_book->readByte<unsigned char>(data, 1, 1);
		unsigned char protectType   = m_book->readByte<unsigned char>(data, 2, 1);
		unsigned char parUsed       = m_book->readByte<unsigned char>(data, 3, 1);
		unsigned short parAlign     = m_book->readByte<unsigned short>(data, 4, 2);
		unsigned short background34 = m_book->readByte<unsigned short>(data, 6, 2);
		unsigned int border34       = m_book->readByte<unsigned int>(data, 8, 4);

		xf.m_protection.m_isCellLocked    = (protectType & 0x01) >> 0;
		xf.m_protection.m_isFormulaHidden = (protectType & 0x02) >> 1;

		xf.m_isStyle          = (protectType & 0x0004) >> 2;  // protectType
		xf.m_lotusPrefix      = (protectType & 0x0008) >> 3;  // Meaning is not known
		xf.m_parentStyleIndex = (parAlign & 0xFFF0) >> 4;  // parAlign

		xf.m_alignment.m_horizontalAlign = (parAlign & 0x07) >> 0;
		xf.m_alignment.m_isTextWrapped   = (parAlign & 0x08) >> 3;
		xf.m_alignment.m_verticalAlign   = 2;  // Bottom
		xf.m_alignment.m_rotation        = 0;

		unsigned char flags = parUsed >> 2;
		xf.m_formatFlag     = (flags >> 0) & 1;
		xf.m_fontFlag       = (flags >> 1) & 1;
		xf.m_alignmentFlag  = (flags >> 2) & 1;
		xf.m_borderFlag     = (flags >> 3) & 1;
		xf.m_backgroundFlag = (flags >> 4) & 1;
		xf.m_protectionFlag = (flags >> 5) & 1;

		xf.m_border.m_topLineStyle        = (border34 & 0x00000007) >> 0;
		xf.m_border.m_topColor.m_index    = (border34 & 0x000000F8) >> 3;
		xf.m_border.m_leftLineStyle       = (border34 & 0x00000700) >> 8;
		xf.m_border.m_leftColor.m_index   = (border34 & 0x0000F800) >> 11;
		xf.m_border.m_bottomLineStyle     = (border34 & 0x00070000) >> 16;
		xf.m_border.m_bottomColor.m_index = (border34 & 0x00F80000) >> 19;
		xf.m_border.m_rightLineStyle      = (border34 & 0x07000000) >> 24;
		xf.m_border.m_rightColor.m_index  = (border34 & 0xF8000000) >> 27;

		xf.m_background.m_fillPattern             = (background34 & 0x003F) >> 0;
		xf.m_background.m_patternColor.m_index    = (background34 & 0x07C0) >> 6;
		xf.m_background.m_backgroundColor.m_index = (background34 & 0xF800) >> 11;
	}
	else if (m_book->m_biffVersion == 21) {
		// Warning: incomplete treatment; formatting_info not fully supported. Probably need
		// to offset incoming BIFF2 XF[n] to BIFF8-like XF[n+16], and create XF[0:16] like
		// the standard ones in BIFF8 *AND* add 16 to all XF references in cell records
		xf.m_fontIndex = m_book->readByte<unsigned char>(data, 0, 1);
		unsigned char format_etc = m_book->readByte<unsigned char>(data, 1, 1);
		unsigned char halign_etc = m_book->readByte<unsigned char>(data, 2, 1);
		xf.m_formatKey = format_etc & 0x3F;

		xf.m_protection.m_isCellLocked    = (format_etc & 0x40) >> 6;
		xf.m_protection.m_isFormulaHidden = (format_etc & 0x80) >> 7;

		xf.m_parentStyleIndex = 0;  // ???????????

		xf.m_alignment.m_horizontalAlign = (halign_etc & 0x07) >> 0;
		xf.m_alignment.m_verticalAlign   = 2;  // Bottom
		xf.m_alignment.m_rotation        = 0;

		xf.m_border.m_leftLineStyle       = (halign_etc & 0x08) ? 1 : 0;  // 1 - thin
		xf.m_border.m_leftColor.m_index   = (halign_etc & 0x08) ? 8 : 0;  // 8 - black
		xf.m_border.m_rightLineStyle      = (halign_etc & 0x10) ? 1 : 0;
		xf.m_border.m_rightColor.m_index  = (halign_etc & 0x10) ? 8 : 0;
		xf.m_border.m_topLineStyle        = (halign_etc & 0x20) ? 1 : 0;
		xf.m_border.m_topColor.m_index    = (halign_etc & 0x20) ? 8 : 0;
		xf.m_border.m_bottomLineStyle     = (halign_etc & 0x40) ? 1 : 0;
		xf.m_border.m_bottomColor.m_index = (halign_etc & 0x40) ? 8 : 0;

		xf.m_formatFlag     = true;
		xf.m_fontFlag       = true;
		xf.m_alignmentFlag  = true;
		xf.m_borderFlag     = true;
		xf.m_backgroundFlag = true;
		xf.m_protectionFlag = true;

		xf.m_background.m_fillPattern = (halign_etc & 0x80) ? 17 : 0;
		xf.m_background.m_patternColor.m_index    = 8;  // Black
		xf.m_background.m_backgroundColor.m_index = 9;  // White
	}
	else {
		throw std::logic_error("programmer stuff-up: bv=" + std::to_string(m_book->m_biffVersion));
	}

	xf.m_xfIndex = static_cast<int>(m_book->m_xfList.size());
	int cellType = XL_CELL_NUMBER;
	if (m_book->m_formatMap.find(xf.m_formatKey) != m_book->m_formatMap.end()) {
		unsigned char type = m_book->m_formatMap[xf.m_formatKey].m_type;
		cellType = CELL_TYPE_FROM_FORMAT_TYPE.at(type);
	}
	m_book->m_xfIndexXlTypeMap[xf.m_xfIndex] = cellType;

	if (m_book->m_formatMap.find(xf.m_formatKey) == m_book->m_formatMap.end())
		xf.m_formatKey = 0;

	m_book->m_xfList.push_back(xf);
	m_book->m_xfCount++;
}

void Formatting::handlePalette(const std::string& data) {
	if (!m_book->m_addStyle)
		return;
	unsigned short colorCount = m_book->readByte<unsigned short>(data, 0, 2);
	//int expectedColorCount  = (m_book->m_biffVersion >= 50) ? 56 : 16;
	int expectedSize = 4 * colorCount + 2;
	int actualSize   = static_cast<int>(data.size());
	int tolerance    = 4;
	if (expectedSize > actualSize || actualSize > expectedSize + tolerance)
		throw std::logic_error(
			"PALETTE record: expected size "+ std::to_string(expectedSize) +
			", actual size "+ std::to_string(actualSize)
		);

	// Color will be 0xbbggrr. IOW, red is at the little end
	for (int i = 0; i < colorCount; ++i) {
		int color = m_book->readByte<int>(data, 4*i + 2, 4);
		unsigned char red   = color & 0xff;
		unsigned char green = (color >> 8)  & 0xff;
		unsigned char blue  = (color >> 16) & 0xff;
		m_book->m_paletteRecord.push_back({red, green, blue});
		m_book->m_colorMap[8+i] = {red, green, blue};
	}
}

void Formatting::handleStyle(const std::string& data) {
	if (!m_book->m_addStyle)
		return;
	unsigned short flagAndXfx = m_book->readByte<unsigned short>(data, 0, 2);
	unsigned char  builtinId  = m_book->readByte<unsigned char>(data, 2, 1);
	unsigned char  level      = m_book->readByte<unsigned char>(data, 3, 1);
	unsigned short xfIndex    = flagAndXfx & 0x0fff;

	bool builtIn;
	std::string name;
	// Erroneous record (doesn't have built-in bit set)
	if (data == "\0\0\0\0" && m_book->m_styleNameMap.find("Normal") == m_book->m_styleNameMap.end()) {
		builtIn = true;
		xfIndex = 0;
		name    = "Normal";
	}
	// Built-in style
	else if (flagAndXfx & 0x8000) {
		builtIn = true;
		name = BUILIT_STYLE_NAMES[builtinId];
		if (1 <= builtinId && builtinId <= 2)
			name += std::to_string(level + 1);
	}
	// User-defined style
	else {
		builtIn   = false;
		builtinId = 0;
		level     = 0;
		if (m_book->m_biffVersion >= 80)
			name = m_book->unpackUnicode(data, 2, 2);
		else
			name = m_book->unpackString(data, 2, 1);
	}
	m_book->m_styleNameMap[name] = {builtIn, xfIndex};
}

void Formatting::xfEpilogue() {
	if (!m_book->m_addStyle)
		return;

	m_book->m_xfEpilogueDone = true;
	size_t xfCount = m_book->m_xfList.size();

	for (size_t i = 0; i < xfCount; ++i) {
		XF& xf = m_book->m_xfList[i];

		int cellType = XL_CELL_TEXT;
		if (m_book->m_formatMap.find(xf.m_formatKey) != m_book->m_formatMap.end()) {
			unsigned char type = m_book->m_formatMap[xf.m_formatKey].m_type;
			cellType = CELL_TYPE_FROM_FORMAT_TYPE.at(type);
		}
		m_book->m_xfIndexXlTypeMap[xf.m_xfIndex] = cellType;

		// Now for some assertions
		if (!m_book->m_addStyle || xf.m_isStyle)
			continue;
		if (xf.m_parentStyleIndex < 0 || xf.m_parentStyleIndex >= static_cast<int>(xfCount))
			xf.m_parentStyleIndex = 0;
	}
}

void Formatting::paletteEpilogue() {
	for (const auto& font : m_book->m_fontList) {
		if (font.m_fontIndex == 4 || font.m_color.m_index == 0x7fff)  // Missing font record
			continue;
		if (m_book->m_colorMap.find(font.m_color.m_index) != m_book->m_colorMap.end())
			m_book->m_colorIndexUsed[font.m_color.m_index] = 1;
	}
}

bool Formatting::isDateFormattedString(const std::string& format) {
	int state = 0;
	std::string str;
	std::string ch = "\\_*";

	for (const auto& c : format) {
		if (state == 0) {
			if (c == '"')
				state = 1;
			else if (find(ch.begin(), ch.end(), c) == ch.end())
				state = 2;
			else if (SKIP_CHAR_DICT.find(c) == SKIP_CHAR_DICT.end())
				str += c;
		}
		else if (state == 1) {
			if (c == '"')
				state = 0;
		}
		// Ignore char after backslash, underscore or asterisk
		else if (state == 2) {
			state = 0;
		}
	}

	str = std::regex_replace(str, FORMAT_BRACKETED_TEXT, "");
	if (NON_DATE_FORMATS.find(str) != NON_DATE_FORMATS.end())
		return false;

	state = 0;
	//int gotSep  = 0;
	int dateCount = 0;
	int numCount  = 0;
	for (const auto& c : str) {
		if (DATE_CHAR_DICT.find(c) == DATE_CHAR_DICT.end())
			dateCount += DATE_CHAR_DICT.at(c);
		else if (NUMBER_CHAR_DICT.find(c) == NUMBER_CHAR_DICT.end())
			numCount += NUMBER_CHAR_DICT.at(c);
		//else if (c == ';')
		//	got_sep = 1;
	}

	if (dateCount && !numCount)
		return true;
	if (numCount && !dateCount)
		return false;
	return (dateCount > numCount);
}

int Formatting::getNearestColorIndex(std::unordered_map<int, std::vector<unsigned char>>& colorMap,
									 std::vector<unsigned char>& rgb)
{
	int bestMetric = 196608;  // 3*256*256
	int bestColor  = 0;
	for (auto const & map : colorMap) {
		if (map.second.empty())
			continue;
		int metric = 0;
		for (size_t i = 0; i < rgb.size(); ++i)
			metric += (rgb[i] - map.second[i]) * (rgb[i] - map.second[i]);
		if (metric < bestMetric) {
			bestMetric = metric;
			bestColor  = map.first;
			if (metric == 0)
				break;
		}
	}
	return bestColor;
}


// Format public:
Format::Format(unsigned short formatKey, unsigned char type, std::string formatString)
: m_formatKey(formatKey), m_type(type), m_formatString(formatString) {}

}  // End namespace
