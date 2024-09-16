/**
 * @brief     Excel files (xls/xlsx) into HTML сonverter
 * @package   excel
 * @file      book.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright python-excel (https://github.com/python-excel/xlrd)
 * @date      02.12.2016 -- 29.01.2018
 */
#include <fstream>

#include "encoding/encoding.hpp"
#include "tools.hpp"

#include "biffh.hpp"
#include "formula.hpp"
#include "sheet.hpp"

#include "book.hpp"

namespace excel {

/** Supbook types */
enum {
	SUPBOOK_UNK,      ///< Unknown
	SUPBOOK_INTERNAL, ///< Internal
	SUPBOOK_EXTERNAL, ///< Extarnal
	SUPBOOK_ADDIN,    ///< Addin
	SUPBOOK_DDEOLE    ///< DDE OLE
};
const int XL_WORKBOOK_GLOBALS = 0x5;
const int XL_WORKBOOK_GLOBALS_4W = 0x100;
const int XL_WORKSHEET = 0x10;
const int XL_BOUNDSHEET_WORKSHEET = 0x00;
/** BIFF supported versions */
const std::vector<int> SUPPORTED_VERSIONS {80, 70, 50, 45, 40, 30, 21, 20};
/** BOF length list */
const std::unordered_map<int, int> BOF_LENGTH {
	{0x0809, 8},
	{0x0409, 6},
	{0x0209, 6},
	{0x0009, 4}
};
/** Get built-in name from code */
const std::unordered_map<std::string, std::string> BUILTIN_NAME_FROM_CODE {
	{"Consolidate_Area", "\x00"},
	{"Auto_Open",        "\x01"},
	{"Auto_Close",       "\x02"},
	{"Extract",          "\x03"},
	{"Database",         "\x04"},
	{"Criteria",         "\x05"},
	{"Print_Area",       "\x06"},
	{"Print_Titles",     "\x07"},
	{"Recorder",         "\x08"},
	{"Data_Form",        "\x09"},
	{"Auto_Activate",    "\x0A"},
	{"Auto_Deactivate",  "\x0B"},
	{"Sheet_Title",      "\x0C"},
	{"_FilterDatabase",  "\x0D"}
};
/** Get encoding from codepage */
const std::unordered_map<int, std::string> ENCODING_FROM_CODEPAGE {
	{1200,  "UTF-16LE"},
	{10000, "MacRoman"},
	{10006, "MacGreek"},
	{10007, "MacCyrillic"},
	{10029, "MacLatin2"},
	{10079, "MacIceland"},
	{10081, "MacTurkish"},
	{32768, "MacRoman"},
	{32769, "CP1252"}
};
/** BIFF text version */
const std::unordered_map<int, std::string> BIFF_TEXT {
	{0,  "(not BIFF)"},
	{20, "2.0"},
	{21, "2.1"},
	{30, "3"},
	{40, "4S"},
	{45, "4W"},
	{50, "5"},
	{70, "7"},
	{80, "8"},
	{85, "8X"}
};

// Book public:
Book::Book(const std::string& fileName, std::string &text, bool addStyle)
: Cfb(fileName), m_contentText(text), m_addStyle(addStyle) {}

void Book::openWorkbookXls() {
	// Read CFB part
	Cfb::parse();
	m_workBook = getStream("Workbook");
	if (m_workBook.empty())
		return;
	Cfb::clear();

	m_biffVersion = getBiffVersion(XL_WORKBOOK_GLOBALS);
	if (!m_biffVersion)
		throw std::logic_error("Can't determine file's BIFF version");
	if (find(SUPPORTED_VERSIONS.begin(), SUPPORTED_VERSIONS.end(), m_biffVersion) ==
		SUPPORTED_VERSIONS.end()
	)
		throw std::invalid_argument("BIFF version "+ BIFF_TEXT.at(m_biffVersion) +" is not supported");

	if (m_biffVersion <= 40) {
		// No workbook globals, only 1 worksheet
		getFakeGlobalsSheet();
	}
	else if (m_biffVersion == 45) {
		// Worksheet(s) embedded in global stream
		parseGlobals();
	}
	else {
		parseGlobals();
		m_sheetList.clear();
		size_t sheetCount = m_sheetNames.size();
		for (size_t i = 0; i < sheetCount; ++i)
			getSheet(i);
	}
	m_sheetCount = m_sheetList.size();

	// Release resources
	m_workBook.clear();
	m_sharedStrings.clear();
	m_richtextRunlistMap.clear();
	m_workBook.shrink_to_fit();
	m_sharedStrings.shrink_to_fit();
}

void Book::handleWriteAccess(const std::string& data) {
	std::string str;
	if (m_biffVersion < 80) {
		if (m_encoding.empty()) {
			m_isRawUserName = true;
			m_userName      = data;
			return;
		}
		str = unpackString(data, 0, 1);
	}
	else
		str = unpackUnicode(data, 0, 2);
	m_userName = tools::rtrim(str);
}

void Book::getRecordParts(unsigned short& code, unsigned short& length,
						  std::string& data, int condition)
{
	int pos = m_position;
	code    = readByte<unsigned short>(m_workBook, pos,   2);
	length  = readByte<unsigned short>(m_workBook, pos+2, 2);

	if (condition != -1 && code != condition) {
		data   = "";
		code   = 0;
		length = 0;
		return;
	}
	pos += 4;
	data = m_workBook.substr(pos, length);
	m_position = pos + length;
}

void Book::getEncoding() {
	if (!m_codePage) {
		if (m_biffVersion < 80)
			m_encoding = "ascii";
		else
			m_codePage = 1200;  // utf16le
	}
	else {
		if (ENCODING_FROM_CODEPAGE.find(m_codePage) != ENCODING_FROM_CODEPAGE.end())
			m_encoding = ENCODING_FROM_CODEPAGE.at(m_codePage);
		else if (300 <= m_codePage && m_codePage <= 1999)
			m_encoding = "cp" + std::to_string(m_codePage);
		else
			m_encoding = "unknown_codepage_" + std::to_string(m_codePage);
	}
	if (m_isRawUserName) {
		m_userName      = tools::rtrim(unpackString(m_userName, 0, 1));
		m_isRawUserName = false;
	}
}

std::string Book::unpackString(const std::string& data, int pos, int length) const {
	std::string result = data.substr(pos + length, readByte<int>(data, pos, length));
	return encoding::decode(result, m_encoding);
}

std::string Book::unpackStringUpdatePos(const std::string& data, int& pos,
										int length, int knownLength) const
{
	int charCount;
	if (knownLength) {
		// On a NAME record, the length byte is detached from the front of the string
		charCount = knownLength;
	}
	else {
		charCount = readByte<int>(data, pos, length);
		pos   += length;
	}
	pos += charCount;
	std::string result = data.substr(pos - charCount, charCount);
	return encoding::decode(result, m_encoding);
}

std::string Book::unpackUnicode(const std::string& data, int pos, int length) const {
	unsigned short charCount = readByte<int>(data, pos, length);
	// Ambiguous whether 0-length string should have an "options" byte. Avoid crash if missing
	if (!charCount)
		return "";

	pos += length;
	std::string result;
	char options = data[pos];
	pos += 1;
	if (options & 0x08)
		pos += 2;
	if (options & 0x04)
		pos += 4;
	if (options & 0x01) {
		// Uncompressed UTF-16-LE
		result = data.substr(pos, 2*charCount);
		result = encoding::decode(result, "UTF-16LE");
	}
	else {
		// Note: this is COMPRESSED (not ASCII) encoding! Merely returning the raw bytes would
		// work OK 99.99% of time if local codepage was cp1252 - however this would rapidly go
		// pear-shaped for other codepages so return Unicode
		result = data.substr(pos, charCount);
		result = encoding::decode(result, "ISO-8859-1");
	}
	return result;
}

std::string Book::unpackUnicodeUpdatePos(const std::string& data, int& pos,
										 int length, int knownLength) const
{
	int charCount;
	if (knownLength)
		// On a NAME record, the length byte is detached from the front of the string
		charCount = knownLength;
	else {
		charCount = readByte<int>(data, pos, length);
		pos += length;
	}
	// Zero-length string with no options byte
	if (!charCount && data.substr(pos).empty())
		return "";

	std::string result;
	unsigned short rt = 0;
	char options  = data[pos];
	char phonetic = options & 0x04;
	char richtext = options & 0x08;
	int  size = 0;
	pos += 1;
	if (richtext) {
		rt = readByte<unsigned short>(data, pos, 2);
		pos += 2;
	}
	if (phonetic) {
		size = readByte<int>(data, pos, 4);
		pos += 4;
	}
	if (options & 0x01) {
		// Uncompressed UTF-16-LE
		result = data.substr(pos, 2*charCount);
		result = encoding::decode(result, "UTF-16LE");
		pos += 2 * charCount;
	}
	else {
		// Note: this is COMPRESSED (not ASCII!) encoding!!!
		result = data.substr(pos, charCount);
		result = encoding::decode(result, "ISO-8859-1");
		pos += charCount;
	}
	if (richtext)
		pos += 4 * rt;
	if (phonetic)
		pos += size;
	return result;
}


// Book private:
int Book::getBiffVersion(int streamSign) {
	unsigned short signature = readByte<unsigned short>(m_workBook, m_position, 2);
	unsigned short length    = readByte<unsigned short>(m_workBook, m_position + 2, 2);
	//int savpos  = m_position;
	m_position += 4;

	if (find(BOF_CODES.begin(), BOF_CODES.end(), signature) == BOF_CODES.end())
		throw std::invalid_argument("Unsupported format, or corrupt file: Expected BOF record");
	if (length < 4 || length > 20)
		throw std::invalid_argument(
			"Unsupported format, or corrupt file: Invalid length (" +
			std::to_string(length) +") for BOF record type " + std::to_string(signature)
		);

	std::string padding(std::max(0, BOF_LENGTH.at(signature) - length), '\0');
	std::string data = m_workBook.substr(m_position, length);
	if (data.size() < length)
		throw std::invalid_argument("Unsupported format, or corrupt file: Incomplete BOF record[2]");

	m_position  += length;
	data        += padding;
	int version  = 0;
	int version1 = signature >> 8;
	unsigned short version2   = readByte<unsigned short>(data, 0, 2);
	unsigned short streamType = readByte<unsigned short>(data, 2, 2);

	if (version1 == 0x08) {
		unsigned short build = readByte<unsigned short>(data, 4, 2);
		unsigned short year  = readByte<unsigned short>(data, 6, 2);

		if (version2 == 0x0600) {
			version = 80;
		}
		else if (version2 == 0x0500) {
			if (year < 1994 || (build == 2412 || build == 3218 || build == 3321))
				version = 50;
			else
				version = 70;
		}
		else {
			// Dodgy one, created by a 3rd-party tool
			std::unordered_map<int, int> code {
				{0x0000, 21}, {0x0007, 21},
				{0x0200, 21}, {0x0300, 30},
				{0x0400, 40}
			};
			version = code[version2];
		}
	}
	else if (version1 == 0x04)
		version = 40;
	else if (version1 == 0x02)
		version = 30;
	else if (version1 == 0x00)
		version = 21;
	if (version == 40 && streamType == XL_WORKBOOK_GLOBALS_4W)
		version = 45;

	bool gotGlobals = (
		streamType == XL_WORKBOOK_GLOBALS ||
		(version == 45 && streamType == XL_WORKBOOK_GLOBALS_4W)
	);
	if ((streamSign == XL_WORKBOOK_GLOBALS && gotGlobals) || streamType == streamSign)
		return version;
	if (version < 50 && streamType == XL_WORKSHEET)
		return version;
	if (version >= 50 && streamType == 0x0100)
		throw std::logic_error("Workspace file -- no spreadsheet data");
	throw std::logic_error("BOF not workbook/worksheet");
}

void Book::getFakeGlobalsSheet() {
	Formatting formatting(this);
	formatting.initializeBook();

	m_sheetNames       = {"Sheet 1"};
	m_sheetAbsolutePos = {0};
	m_sheetVisibility  = {0};  // One sheet, visible
    m_sheetList.emplace_back(Sheet(this, m_position, "Sheet 1", 0, m_contentText));
	size_t sheetCount = m_sheetNames.size();
	for (size_t i = 0; i < sheetCount; ++i)
		getSheet(i);
}

void Book::parseGlobals() {
	// No need to position, just start reading (after the BOF)
	Formatting formatting(this);
	formatting.initializeBook();

	while (true) {
		unsigned short code;
		unsigned short length;
		std::string    data;
		getRecordParts(code, length, data);

		if (code == XL_SST)
			handleSst(data);
		else if (code == XL_FONT || code == XL_FONT_B3B4)
			formatting.handleFont(data);
		else if (code == XL_FORMAT)  // XL_FORMAT2 is BIFF <= 3.0, can't appear in globals
			formatting.handleFormat(data);
		else if (code == XL_XF)
			formatting.handleXf(data);
		else if (code == XL_BOUNDSHEET)
			handleBoundsheet(data);
		else if (code == XL_DATEMODE)
			m_dateMode = readByte<unsigned short>(data, 0, 2);
		else if (code == XL_CODEPAGE) {
			m_codePage = readByte<unsigned short>(data, 0, 2);
			getEncoding();
		}
		else if (code == XL_COUNTRY) {
			m_countries = {
				readByte<unsigned short>(data, 0, 2),
				readByte<unsigned short>(data, 2, 2)
			};
		}
		else if (code == XL_EXTERNNAME)
			handleExternalName(data);
		else if (code == XL_EXTERNSHEET)
			handleExternalSheet(data);
		else if (code == XL_WRITEACCESS)
			handleWriteAccess(data);
		else if (code == XL_SHEETSOFFSET)
			m_sheetOffset = readByte<int>(data, 0, 4);
		else if (code == XL_SHEETHDR)
			handleSheethdr(data);
		else if (code == XL_SUPBOOK)
			handleSupbook(data);
		else if (code == XL_NAME)
			handleName(data);
		else if (code == XL_PALETTE)
			formatting.handlePalette(data);
		else if (code == XL_STYLE)
			formatting.handleStyle(data);
		else if (code == XL_EOF) {
			formatting.xfEpilogue();
			namesEpilogue();
			formatting.paletteEpilogue();
			if (m_encoding.empty())
				getEncoding();
			return;
		}
	}
}

void Book::getSheet(size_t sheetId, bool shouldUpdatePos) {
	if (shouldUpdatePos)
		m_position = m_sheetAbsolutePos[sheetId];
	getBiffVersion(XL_WORKSHEET);

	// Add sheet information
//	auto div = m_htmlTree.append_child("div");
//	div.append_attribute("id") = ("tabC"+ std::to_string(sheetId+1)).c_str();
//    auto table = m_htmlTree;

    m_sheetList.emplace_back(Sheet(this, m_position, m_sheetNames[sheetId], sheetId, m_contentText));
    m_sheetList.back().read();
}

void Book::handleSst(const std::string& data) {
	std::vector<std::string> stringList = {data};
	while (true) {
		unsigned short code;
		unsigned short length;
		std::string    data;
		getRecordParts(code, length, data, XL_CONTINUE);

		if (!code)
			break;
		stringList.emplace_back(data);
	}
	unpackSst(stringList, readByte<int>(data, 4, 4));
}

void Book::handleBoundsheet(const std::string& data) {
	getEncoding();
	std::string   sheetName;
	unsigned char visibility;
	unsigned char sheetType;
	int           absolutePos;

	if (m_biffVersion == 45) {  // BIFF4W
		// Not documented in OOo docs. In fact, the only data is the name of the sheet
		sheetName  = unpackString(data, 0, 1);
		visibility = 0;
		sheetType  = XL_BOUNDSHEET_WORKSHEET;
		// Note:
		// (a) This won't be used
		// (b) It's the position of the SHEETHDR record
		// (c) Add 11 to get to the worksheet BOF record
		if (m_sheetAbsolutePos.size() == 0)
			absolutePos = m_sheetOffset + m_base;
		else
			absolutePos = -1;  // Unknown
	}
	else {
		int offset  = readByte<int>(data, 0, 4);
		visibility  = readByte<unsigned char>(data, 4, 1);
		sheetType   = readByte<unsigned char>(data, 5, 1);
		absolutePos = offset + m_base;  // Because global BOF is always at position 0 in the stream
		if (m_biffVersion < 80)
			sheetName = unpackString(data, 6, 1);
		else
			sheetName = unpackUnicode(data, 6, 1);
	}

	if (sheetType != XL_BOUNDSHEET_WORKSHEET)
		m_sheetMap.push_back(-1);
	else {
		int size = static_cast<int>(m_sheetNames.size());
		m_sheetMap.push_back(size);
		m_sheetNames.push_back(sheetName);
		m_sheetAbsolutePos.push_back(absolutePos);
		m_sheetVisibility.push_back(visibility);
		m_sheetIdFromName[sheetName] = size;
	}
}

void Book::handleExternalName(const std::string& data) {
	if (m_biffVersion >= 80) {
		int pos = 6;
		std::string name = unpackUnicodeUpdatePos(data, pos, 1);
		if (m_supbookTypes.back() == SUPBOOK_ADDIN)
			m_addinFuncNames.push_back(name);
	}
}

void Book::handleExternalSheet(std::string& data) {
	getEncoding();  // If CODEPAGE record is missing/out of order/wrong
	m_externalSheetCount++;
	if (m_biffVersion >= 80) {
		unsigned short numRefs = readByte<unsigned short>(data, 0, 2);
		while (data.size() < numRefs*6 + 2) {
			unsigned short code;
			unsigned short length;
			std::string    data2;
			getRecordParts(code, length, data2);
			if (code != XL_CONTINUE)
				throw std::logic_error("Missing CONTINUE after EXTERNSHEET record");
			data += data2;
		}
		int pos = 2;
		for (int k = 0; k < numRefs; ++k) {
			m_externalSheetInfo.push_back({
				readByte<unsigned short>(data, pos,   2),
				readByte<unsigned short>(data, pos+2, 2),
				readByte<unsigned short>(data, pos+4, 2)
			});
			pos += 6;
		}
	}
	else {
		unsigned char size = readByte<unsigned char>(data, 0, 1);
		unsigned char type = readByte<unsigned char>(data, 1, 1);
		if (type == 3)
			m_externalSheetNameFromId[m_externalSheetCount] = data.substr(2, size);
		if (type < 1 || type > 4)
			type = 0;
		m_externalSheetTypes.push_back(type);
	}
}

void Book::handleSheethdr(const std::string& data) {
	getEncoding();
	int sheetLength         = readByte<int>(data, 0, 4);
	//std::string sheetName = unpackString(data, 4, 1);
	int bofPosition         = m_position;
	m_sheethdrCount++;

	initializeFormatInfo();
	getSheet(m_sheethdrCount, false);
	m_position = bofPosition + sheetLength;
}

void Book::handleSupbook(const std::string& data) {
	m_supbookTypes.push_back(-1);
	unsigned short sheetCount = readByte<unsigned short>(data, 0, 2);
	m_supbookCount++;
	if (data.substr(2, 2) == "\x01\x04") {
		m_supbookTypes.back() = SUPBOOK_INTERNAL;
		m_supbookLocalIndex  = m_supbookCount - 1;
		return;
	}
	if (data.substr(0, 4) == "\x01\x00\x01\x3A") {
		m_supbookTypes.back() = SUPBOOK_ADDIN;
		m_supbookAddinIndex   = m_supbookCount - 1;
		return;
	}

	int pos = 2;
	std::string url = unpackUnicodeUpdatePos(data, pos, 2);
	if (sheetCount == 0) {
		m_supbookTypes.back() = SUPBOOK_DDEOLE;
		return;
	}
	m_supbookTypes.back() = SUPBOOK_EXTERNAL;
	std::vector<std::string> sheetNames;
	for (int x = 0; x < sheetCount; ++x) {
		try {
			sheetNames.emplace_back(unpackUnicodeUpdatePos(data, pos, 2));
		}
		catch (...) {
			break;
		}
	}
}

void Book::handleName(const std::string& data) {
	if (m_biffVersion < 50)
		return;
	getEncoding();

	unsigned short optionFlags             = readByte<unsigned short>(data, 0, 2);
	//unsigned char  kbShortcut            = readByte<unsigned char>(data, 2, 1);
	unsigned char  nameLength              = readByte<unsigned char>(data, 3, 1);
	unsigned short formulaLength           = readByte<unsigned short>(data, 4, 2);
	unsigned short externalSheetIndex      = readByte<unsigned short>(data, 6, 2);
	unsigned short sheetIndex              = readByte<unsigned short>(data, 8, 2);
	//unsigned char  menuTextLength        = readByte<unsigned char>(data, 10, 1);
	//unsigned char  descriptionTextLength = readByte<unsigned char>(data, 11, 1);
	//unsigned char  helpTextLength        = readByte<unsigned char>(data, 12, 1);
	//unsigned char  statusTextLength      = readByte<unsigned char>(data, 13, 1);

	m_nameObjList.emplace_back(Name(this));
	Name& nobj                = m_nameObjList.back();
	nobj.m_nameIndex          = m_nameObjList.size() - 1;
	nobj.m_optionFlags        = optionFlags;
	nobj.m_isHidden           = (optionFlags & 1) >> 0;
	nobj.m_function           = (optionFlags & 2) >> 1;
	nobj.m_vbasic             = (optionFlags & 4) >> 2;
	nobj.m_macro              = (optionFlags & 8) >> 3;
	nobj.m_isComplex          = (optionFlags & 0x10) >> 4;
	nobj.m_builtIn            = (optionFlags & 0x20) >> 5;
	nobj.m_functionGroup      = (optionFlags & 0xFC0) >> 6;
	nobj.m_isBinary           = (optionFlags & 0x1000) >> 12;
	nobj.m_externalSheetIndex = externalSheetIndex;
	nobj.m_excelSheetIndex    = sheetIndex;
	nobj.m_basicFormulaLength = formulaLength;
	nobj.m_evaluated          = 0;
	nobj.m_scope              = -5;  // Patched up in the names_epilogue() method

	std::string internalName;
	int pos = 14;
	if (m_biffVersion < 80)
		internalName = unpackStringUpdatePos(data, pos, 1, nameLength);
	else
		internalName = unpackUnicodeUpdatePos(data, pos, 2, nameLength);

	if (!nobj.m_builtIn)
		nobj.m_name = internalName;
    else if (BUILTIN_NAME_FROM_CODE.find(internalName) != BUILTIN_NAME_FROM_CODE.end())
		nobj.m_name = BUILTIN_NAME_FROM_CODE.at(internalName);
	else
		nobj.m_name = "??Unknown??";
	nobj.m_rawFormula = data.substr(pos);
}

void Book::initializeFormatInfo() {
	m_formatMap.clear();
	m_formatList.clear();
	m_xfCount           = 0;
	m_actualFormatCount = 0;  // Number of FORMAT records seen so far
	m_xfEpilogueDone    = 0;
	m_xfIndexXlTypeMap  = {{0, XL_CELL_NUMBER}};
	m_xfList.clear();
	m_fontList.clear();
}

void Book::unpackSst(const std::vector<std::string>& dataTable, int stringCount) {
	std::string data  = dataTable[0];
	int dataIndex     = 0;
	size_t dataSize   = dataTable.size();
	size_t dataLength = data.size();
	int pos           = 8;
	m_sharedStrings.clear();
	if (m_addStyle)
		m_richtextRunlistMap.clear();

	for (int i = 0; i < stringCount; ++i) {
		unsigned short charCount = readByte<unsigned short>(data, pos, 2);
		char options       = data[pos + 2];
		int  richTextCount = 0;
		int  phoneticSize  = 0;
		pos += 3;
		if (options & 0x08) {  // Richtext
			richTextCount = readByte<unsigned short>(data, pos, 2);
			pos += 2;
		}
		if (options & 0x04) {  // Phonetic
			phoneticSize = readByte<int>(data, pos, 4);
			pos += 4;
		}
		std::string result;
		int gotChars = 0;
		while (true) {
			int charsNeed = charCount - gotChars;
			int charsAvailable;
            std::string text;
			if (options & 0x01) {
				// Uncompressed UTF-16
				charsAvailable = std::min(((int)dataLength - pos) >> 1, charsNeed);
                text    = data.substr(pos, 2*charsAvailable);
                text    = encoding::decode(text, "UTF-16LE");
				pos    += 2*charsAvailable;
			}
			else {
				// Note: this is COMPRESSED (not ASCII!) encoding!!!
				charsAvailable = std::min((int)dataLength - pos, charsNeed);
                text    = data.substr(pos, charsAvailable);
                text    = encoding::decode(text, "ISO-8859-1");
				pos    += charsAvailable;
			}
            result += text;
			gotChars += charsAvailable;
			if (gotChars == charCount)
				break;
			dataIndex += 1;
			data = dataTable[dataIndex];
			dataLength = data.size();
			options = data[0];
			pos = 1;
		}

		if (richTextCount) {
			std::vector<std::pair<unsigned short, unsigned short>> runs;
			for (int j = 0; j < richTextCount; ++j) {
				if (pos == static_cast<int>(dataLength)) {
					pos        = 0;
					dataIndex += 1;
					data       = dataTable[dataIndex];
					dataLength = data.size();
				}
				runs.emplace_back(readByte<unsigned short>(data, pos,   2),
								  readByte<unsigned short>(data, pos+2, 2));
				pos += 4;
			}
			if (m_addStyle)
				m_richtextRunlistMap[m_sharedStrings.size()] = runs;
		}

		pos += phoneticSize;  // Size of the phonetic stuff to skip
		if (pos >= static_cast<int>(dataLength)) {
			// Adjust to correct position in next record
			pos -= static_cast<int>(dataLength);
			dataIndex++;
			if (dataIndex < static_cast<int>(dataSize)) {
				data       = dataTable[dataIndex];
				dataLength = data.size();
			}
		}
		m_sharedStrings.push_back(result);
	}
}

void Book::namesEpilogue() {
	size_t nameCount = m_nameObjList.size();
	for (size_t i = 0; i < nameCount; ++i) {
		Name& name = m_nameObjList[i];
		int internalSheetIndex = -3;
		// Convert from excelSheetIndex to scope. Done here because in BIFF7 and earlier
		// the BOUNDSHEET records come after the NAME records
		if (m_biffVersion >= 80) {
			int sheetIndex = name.m_excelSheetIndex;
			if (sheetIndex == 0)
				internalSheetIndex = -1;  // Global
			else if (1 <= sheetIndex && sheetIndex <= static_cast<int>(m_sheetMap.size())) {
				internalSheetIndex = m_sheetMap[sheetIndex-1];
				if (internalSheetIndex == -1)  // Maps to a macro or VBA sheet
					internalSheetIndex = -2;  // Valid sheet reference but not useful
			}
			else
				internalSheetIndex = -3;  // Invalid
		}
		else if (50 <= m_biffVersion && m_biffVersion <= 70) {
			int sheetIndex = name.m_externalSheetIndex;
			if (sheetIndex == 0)
				internalSheetIndex = -1;  // Global
			else {
				std::string sheetName = m_externalSheetNameFromId[sheetIndex];
				if (m_sheetIdFromName.find(sheetName) == m_sheetIdFromName.end())
					internalSheetIndex = m_sheetIdFromName[sheetName];
				else
					internalSheetIndex = -2;
			}
		}
		name.m_scope = static_cast<int>(internalSheetIndex);
	}

	Formula formula(this);
	for (int i = 0; i < (int)nameCount; ++i) {
		Name& name = m_nameObjList[i];
		// Parse the formula
		if (name.m_macro || name.m_isBinary || name.m_evaluated)
			continue;
		formula.evaluateFormula(name, i);
	}

	// Build some dicts for access to the name objects
	m_nameScopeMap.clear();
	m_nameMap.clear();
	std::map<std::string, std::vector<std::pair<Name, int>>> nameMap;
	for (int i = 0; i < (int)nameCount; ++i) {
		Name& name = m_nameObjList[i];
		std::string nameName = name.m_name;
		std::transform(nameName.begin(), nameName.end(), nameName.begin(), ::tolower);

		std::pair<std::string, int> key {nameName, name.m_scope};
		m_nameScopeMap.erase(key);
		m_nameScopeMap.emplace(key, name);

		nameMap[nameName].emplace_back(name, i);
	}
	for (auto & map : nameMap) {
		std::sort(map.second.begin(), map.second.end());
		for (const auto& obj : map.second)
			m_nameMap[map.first].emplace_back(obj.first);
	}
}


// Name public:
Name::Name(Book* book)
	: m_book(book) {}

bool Name::operator < (const Name& name) const {
	return m_scope < name.m_scope;
}

}  // End namespace
