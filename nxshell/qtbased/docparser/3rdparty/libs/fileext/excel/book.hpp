/**
 * @brief     Excel files (xls/xlsx) into HTML сonverter
 * @package   excel
 * @file      book.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright python-excel (https://github.com/python-excel/xlrd)
 * @date      02.12.2016 -- 28.01.2018
 */
#pragma once

#include <map>
#include <string>
#include <vector>
#include <unordered_map>
#include <pugixml.hpp>

#include "fileext/cfb/cfb.hpp"

#include "formula.hpp"
#include "sheet.hpp"
#include "frmt.hpp"


namespace excel {

class Font;
class XFBorder;
class XFBackground;
class XF;
class Name;
class Sheet;
class Operand;

/**
 * @class Book
 * @brief
 *     Excel Workbook data
 */
class Book: public cfb::Cfb {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @param htmlTree
	 *     Result HTML tree
	 * @param addStyle
	 *     Should read and add styles to HTML-tree
	 * @param mergingMode
	 *     Colspan/rowspan processing mode
	 * @since 1.0
	 */
    Book(const std::string& fileName, std::string& text, bool addStyle);

	/**
	 * @brief
	 *     Read XLS WorkBook
	 * @throw std::logic_error
	 *     Can't determine file's BIFF version
	 * @throw std::invalid_argument
	 *     BIFF version %1 is not supported
	 * @since 1.0
	 */
	void openWorkbookXls();

	/**
	 * @brief
	 *     Get sheet write access
	 * @since 1.0
	 */
	void handleWriteAccess(const std::string& data);

	/**
	 * @brief
	 *     Read records parts
	 * @param[out] code
	 *     Record type
	 * @param[out] length
	 *     Record length
	 * @param[out] data
	 *     Record content
	 * @param[in] condition
	 *     Reading condition (return empty record)
	 * @since 1.0
	 */
	void getRecordParts(unsigned short& code, unsigned short& length,
						std::string& data, int condition = -1);

	/**
	 * @brief
	 *     Get encoding from stream data
	 * @since 1.0
	 */
	void getEncoding();

	/**
	 * @brief
	 *     Read binary string
	 * @param[in] data
	 *     Binary data
	 * @param[in] pos
	 *     Record size information start position
	 * @param[in] length
	 *     Record size information length
	 * @return
	 *     UTF-8 text
	 * @since 1.0
	 */
	std::string unpackString(const std::string& data, int pos, int length = 1) const;

	/**
	 * @brief
	 *     Read binary string and update position
	 * @param[in] data
	 *     Binary data
	 * @param[in,out] pos
	 *     Record size information start position
	 * @param[in] length
	 *     Record size information length
	 * @param[in] knownLength
	 *     Record size
	 * @return
	 *     UTF-8 text
	 * @since 1.0
	 */
	std::string unpackStringUpdatePos(const std::string& data, int& pos, int length = 1,
									  int knownLength = 0) const;

	/**
	 * @brief
	 *     Convert binary text to UTF-8
	 * @param[in] data
	 *     Binary data
	 * @param[in] pos
	 *     Record size information start position
	 * @param[in] length
	 *     Record size information length
	 * @return
	 *     UTF-8 text
	 * @since 1.0
	 */
	std::string unpackUnicode(const std::string& data, int pos, int length = 2) const;

	/**
	 * @brief
	 *     Convert binary text to UTF-8 and update position
	 * @param[in] data
	 *     Binary data
	 * @param[in,out] pos
	 *     Record size information start position
	 * @param[in] length
	 *     Record size information length
	 * @param[in] knownLength
	 *     Record size
	 * @return
	 *     UTF-8 text
	 * @since 1.0
	 */
	std::string unpackUnicodeUpdatePos(const std::string& data, int& pos, int length = 2,
									   int knownLength = 0) const;

	/** Result HTML tree */
    std::string& m_contentText;
    /** Should read and add styles to HTML-tree */
    const bool m_addStyle;
	/** Current position in the stream  */
	int m_position = 0;
	/**
	 * Version of BIFF (Binary Interchange File Format). Used to create the file.
	 * Latest is 8.0, introduced with Excel 97. Earliest supported by this module: 2.0
	 */
	unsigned char m_biffVersion = 0;
	/** The number of worksheets in workbook */
	size_t m_sheetCount;
	/** All strings in workbook */
	std::vector<std::string> m_sharedStrings;
	/** Sheet list */
	std::vector<Sheet> m_sheetList;
	/** Sheets names list */
	std::vector<std::string> m_sheetNames;
	/** Sheet visibility. From BOUNDSHEET record */
	std::vector<unsigned char> m_sheetVisibility;
	/** Maps an allSheets index to a calc-sheets index (or -1) */
	std::vector<int> m_sheetMap;
	/** Information about external sheets */
	std::vector<std::vector<int>> m_externalSheetInfo;
	/** External sheet types */
	std::vector<int> m_externalSheetTypes;
	/** SUPBOOK local index */
	int m_supbookLocalIndex = 0;
	/** SUPBOOK ADDIN index */
	int m_supbookAddinIndex = 0;
	/** List containing object for each NAME record in the workbook */
	std::vector<Name> m_nameObjList;
	/** Maps an NAME records to scope index */
	std::map<std::pair<std::string, int>, Name> m_nameScopeMap;
	/** NAME record names map */
	std::map<std::string, std::vector<Name>> m_nameMap;
	/** List of FONT records */
	std::vector<Font> m_fontList;
	/** List of XF records */
	std::vector<XF> m_xfList;
	/** XF records count */
	int m_xfCount = 0;
	/** IF XF records are finalized */
	bool m_xfEpilogueDone = false;
	/**
	 * List of FORMAT records in the order that they appear in the input file.
	 * It doesn't contain built-in formats
	 */
	std::vector<Format> m_formatList;
	/** Maps an XF formatKey to FORMAT. Used for all visual rendering purposes */
	std::unordered_map<int, Format> m_formatMap;
	/**
	 * Provides access via name to `(builtIn, xfIndex)` - extended format information for
	 * built-in/user-defined styles
	 * Value | Description
	 * :---: | -----------
	 *   0   | User-defined
	 *   1   | Built-in style
	 * Known built-in names are:
	 * - Normal
	 * - RowLevel_1 to RowLevel_7
	 * - ColLevel_1 to ColLevel_7
	 * - Comma
	 * - Currency
	 * - Percent
	 * - "Comma [0]"
	 * - "Currency [0]"
	 * - Hyperlink
	 * - "Followed Hyperlink"
	 */
	std::unordered_map<std::string, std::pair<bool, int>> m_styleNameMap;
	/**
	 * Provides definitions like `(red, green, blue)` for color indexes (`0x7FFF` maps to `None`)
	 * This is what you need if you want to render cells on screen or in PDF file
	 */
	std::unordered_map<int, std::vector<unsigned char>> m_colorMap;
	/** List of used color indexes */
	std::unordered_map<int, int> m_colorIndexUsed;
	/**
	 * If user has changed any of colors in standard palette, XLS file will contain `PALETTE`
	 * record with 56 (16 for Excel 4.0 and earlier) RGB values in it.
	 * This is what you need if you are writing an output XLS file
	 */
	std::vector<std::vector<int>> m_paletteRecord;
	/** List of already seen richtext records */
	std::unordered_map<size_t, std::vector<std::pair<unsigned short, unsigned short>>> m_richtextRunlistMap;
	/** Number of FORMAT records seen so far */
	int m_actualFormatCount = 0;
	/** Number of built-in FORMAT records. Unknown as yet. BIFF 3, 4S, 4W */
	int m_builtinFormatCount = -1;
	/** List of document properties (XLSX only) */
	std::unordered_map<std::string, std::string> m_properties;
	/** Maps XF index to XL type */
	std::map<int, int> m_xfIndexXlTypeMap = {{0, 0}};
	/** List of cell's border objects */
	std::vector<XFBorder> m_borderList;
	/** List of cell's background objects */
	std::vector<XFBackground> m_backgroundList;
	/**
	 * Shows which date system was when this file was last saved
	 * Value | Description
	 * :---: | -----------
	 *   0   | 1900 system (the Excel for Windows default)
	 *   1   | 1904 system (the Excel for Macintosh default)
	 */
	unsigned short m_dateMode = 0;
	/**
	 * An integer denoting the character set used for strings in this file.
	 * For BIFF 8 and later, this will be 1200, meaning Unicode; more precisely, UTF_16_LE
	 * For earlier versions, this is used to derive the appropriate encoding.
	 * Example:
	 * @code `1252 -> 'cp1252'`, `10000 -> 'mac_roman'` @endcode
	 */
	unsigned short m_codePage = 0;
	/**
	 * A tuple containing the telephone country code for:
	 * Value | Description
	 * :---: | -----------
	 *   0   | User-interface setting when the file was created
	 *   1   | Regional settings
	 * This information may give a clue to the correct encoding for an unknown codepage.
	 * Example:
	 * @code `(1, 61)` -> `(USA, Australia)` @endcode
	 */
	std::pair<unsigned short, unsigned short> m_countries {0, 0};
	/** What (if anything) is recorded as the name of the last user to save the file */
	std::string m_userName;
	/** Encoding that was derived from the codepage */
	std::string m_encoding;

private:
	/**
	 * @brief
	 *     Get BIFF version
	 * @param[in] streamSign
	 *     Stream signature
	 * @return
	 *     BIFF version
	 * @throw std::invalid_argument
	 *     Unsupported format, or corrupt file: Expected BOF record
	 * @throw std::invalid_argument
	 *     Unsupported format, or corrupt file: Invalid length %1 for BOF record type %2
	 * @throw std::invalid_argument
	 *     Unsupported format, or corrupt file: Incomplete BOF record[2]
	 * @throw std::logic_error
	 *     Workspace file -- no spreadsheet data
	 * @throw std::logic_error
	 *     BOF not workbook/worksheet
	 * @since 1.0
	 */
	int getBiffVersion(int streamSign);

	/**
	 * @brief
	 *     Get worksheet when BIFF version <= 40 (No workbook globals)
	 * @since 1.0
	 */
	void getFakeGlobalsSheet();

	/**
	 * @brief
	 *     Read workbook globals
	 * @since 1.0
	 */
	void parseGlobals();

	/**
	 * @brief
	 *     Get sheet by id
	 * @param[in] sheetId
	 *     Sheet id
	 * @param[in] shouldUpdatePos
	 *     Should update stream position
	 * @since 1.0
	 */
	void getSheet(size_t sheetId, bool shouldUpdatePos = true);

	/**
	 * @brief
	 *     Read SST (Shared Strings Table) data
	 * @param[in] data
	 *     Binary data
	 * @since 1.0
	 */
	void handleSst(const std::string& data);

	/**
	 * @brief
	 *     Read Boundsheet data
	 * @param[in] data
	 *     Binary data
	 * @since 1.0
	 */
	void handleBoundsheet(const std::string& data);

	/**
	 * @brief
	 *     Read External names
	 * @param[in] data
	 *     Binary data
	 * @since 1.0
	 */
	void handleExternalName(const std::string& data);

	/**
	 * @brief
	 *     Read External sheet
	 * @param[in] data
	 *     Binary data
	 * @throw std::logic_error
	 *     Missing CONTINUE after EXTERNSHEET record
	 * @since 1.0
	 */
	void handleExternalSheet(std::string& data);

	/**
	 * @brief
	 *     Read SHEETHDR record (is followed by (BOF ... EOF) substream). BIFF 4W special
	 * @param[in] data
	 *     Binary data
	 * @since 1.0
	 */
	void handleSheethdr(const std::string& data);

	/**
	 * @brief
	 *     Read SUPBOOK (EXTERNALBOOK in OOo docs)
	 * @param[in] data
	 *     Binary data
	 * @since 1.0
	 */
	void handleSupbook(const std::string& data);

	/**
	 * @brief
	 *     Read NAME records
	 * @param[in] data
	 *     Binary data
	 * @since 1.0
	 */
	void handleName(const std::string & data);

	/**
	 * @brief
	 *     Initialize format info. Needs to be done once per sheet for BIFF 4W
	 * @since 1.0
	 */
	void initializeFormatInfo();

	/**
	 * @brief
	 *     Unpack SST (Shared Strings Table) to @ref m_sharedStrings
	 * @param[in] dataTable
	 *     SST data
	 * @param[in] stringCount
	 *     String count in SST
	 * @since 1.0
	 */
	void unpackSst(const std::vector<std::string>& dataTable, int stringCount);

	/**
	 * @brief
	 *     Finalize NAME records
	 * @since 1.0
	 */
	void namesEpilogue();

	/** Workbook stream content */
	std::string m_workBook;
	/** Stream start position */
	int m_base = 0;
	/** Sheets absolute position in the stream */
	std::vector<int> m_sheetAbsolutePos;
	/** Sheets id and name relation */
	std::unordered_map<std::string, int> m_sheetIdFromName;
	/** Sheet records offset */
	int m_sheetOffset = 0;
	/** External sheet count */
	int m_externalSheetCount = 0;
	/** External sheets id and name relation */
	std::unordered_map<int, std::string> m_externalSheetNameFromId;
	/** Supbook types */
	std::vector<int> m_supbookTypes;
	/** SUPBOOK record count */
	int m_supbookCount = 0;
	/** ADDIN functions names */
	std::vector<std::string> m_addinFuncNames;
	/** SHEETHDR record count. BIFF 4W only */
	int m_sheethdrCount = 0;
	/** If raw user name */
	bool m_isRawUserName = false;
};


/**
 * @class Name
 * @brief
 *     Information relating to a named reference, formula, macro, ...
 */
class Name {
public:
	/**
	 * @param[in] book
	 *     Pointer to parent Book object
	 * @since 1.0
	 */
	Name(Book* book);

	/**
	 * @brief
	 *     Operator `<` overload
	 * @since 1.0
	 */
	bool operator < (const Name& name) const;

	/** Pointer to parent Book object */
	Book* m_book;
	/**
	 * If formula is hidden
	 * Value | Description
	 * :---: | -----------
	 * False | Visible
	 * True  | Hidden
	 */
	bool m_isHidden = false;
	/**
	 * Function type. Relevant only if macro == 1
	 * Value | Description
	 * :---: | -----------
	 * False | Command macro
	 * True  | Function macro
	 */
	bool m_function = false;
	/**
	 * Macro type. Relevant only if macro == 1
	 * Value | Description
	 * :---: | -----------
	 * False | Sheet macro
	 * True  | VisualBasic macro
	 */
	bool m_vbasic = false;
	/**
	 * Formula name
	 * Value | Description
	 * :---: | -----------
	 * False | Standard name
	 * True  | Macro name
	 */
	bool m_macro = false;
	/**
	 * Formula complexity
	 * Value | Description
	 * :---: | -----------
	 * False | Simple formula
	 * True  | Complex formula (array formula or user defined)
	 */
	bool m_isComplex = false;
	/**
	 * Formula name type
	 * Value | Description
	 * :---: | -----------
	 * False | User-defined name
	 * True  | Built-in name
	 */
	bool m_builtIn = false;
	/** Function group. Relevant only if macro == 1 */
	bool m_functionGroup = false;
	/**
	 * If data is binary
	 * Value | Description
	 * :---: | -----------
	 * False | Formula definition
	 * True  | Binary data
	 */
	bool m_isBinary = false;
	/** Index of this object in @ref Book.m_nameObjList */
	size_t m_nameIndex = 0;
	/** Object name (Unicode string). If built-in, decoded as per OOo docs */
	std::string m_name;
	/** 8-bit string */
	std::string m_rawFormula;
	/**
	 * Name scope visibility
	 * Value | Description
	 * :---: | -----------
	 *  -1   | Name is global (visible in all calculation sheets)
	 *  -2   | Name belongs to a macro sheet or VBA sheet
	 *  -3   | Name is invalid
	 *   0 <= scope < sheetCount | Name is local to the sheet whose index is scope
	 */
	int m_scope = -1;
	//** Result of evaluating the formula, if any */
	//Operand m_result;
	/** Current sheet index */
	int m_excelSheetIndex;
	/** External sheet index */
	int m_externalSheetIndex;
	/** if formula is evaluated */
	bool m_evaluated;
	/** Stack of formula operations */
	std::vector<Operand> m_stack;
	/** If has relations */
	bool m_hasRelation = false;
	/** If has errors */
	bool m_hasError = false;
	/** Function flags */
	unsigned short m_optionFlags;
	/** VisualBasic formula length */
	unsigned short m_basicFormulaLength;
	/** Formula text */
	std::string m_formulaText;
};

}  // End namespace
