/**
 * @brief     Excel files (xls/xlsx) into HTML сonverter
 * @package   excel
 * @file      sheet.hpp
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

#include "book.hpp"
#include "format.hpp"


namespace excel {

class Book;
class XF;
class XFColor;
class Hyperlink;
class MSObj;
class MSTxo;
class Note;
class Cell;

/**
 * @class Colinfo
 * @brief
 *     Width and default formatting information that applies to one or more columns in a sheet
 * @details
 *     Here is default hierarchy for width, according to the OOo docs:
 *     In BIFF3, if a COLINFO record is missing for a column, width specified in record DEFCOLWIDTH.
 *     In BIFF4-BIFF7, width set in this COLINFO record is only used, if the corresponding bit for
 *     this column is cleared in GCW record, otherwise column width set in DEFCOLWIDTH record.
 *     In BIFF8, if COLINFO record is missing for column, width specified in record STANDARDWIDTH.
 *     If this STANDARDWIDTH record is also missing, column width of record DEFCOLWIDTH is used.
 */
class Colinfo {
public:
	Colinfo() = default;

	/** Width of the column in 1/256 of width of zero character, using default font */
	unsigned short m_width = 0;
	/** XF index for formatting empty cells */
	int m_xfIndex = -1;
	/** If column is hidden */
	bool m_isHidden = false;
	/** If column is collapsed */
	bool m_isCollapsed = false;
	/** 1-bit flag whose purpose is unknown, but is often seen set to 1 */
	bool m_bitFlag = false;
	/** Outline level of the column (0 = no outline) */
	int m_outlineLevel = 0;
};


/**
 * @class Rowinfo
 * @brief
 *     Height and default formatting information that applies to row in a sheet
 */
class Rowinfo {
public:
	Rowinfo() = default;

	/** Height of the row, in twips. One twip == 1/20 of a point */
	int m_height = 0;
	/**
	 * If row has default height
	 * Value | Description
	 * :---: | -----------
	 * False | Row has custom height
	 * True  | Row has default height
	 */
	bool m_hasDefaultHeight = false;
	/**
	 * Height mismatch
	 * Value | Description
	 * :---: | -----------
	 * False | Row height and default font height are equal
	 * True  | Row height and default font height do not match
	 */
	bool m_isHeightMismatch = false;
	/** If row is hidden (manually, or by filter or outline group) */
	bool m_isHidden = false;
	/**
	 * If row has default XF index
	 * Value | Description
	 * :---: | -----------
	 * False | Ignore xfIndex attribute
	 * True  | xfIndex attribute is usable
	 */
	bool m_hasDefaultXfIndex = false;
	/** Index to default XF record for empty cells in this row */
	int m_xfIndex = 0;
	/** Outline level of the row (0 to 7) */
	int m_outlineLevel = 0;
	/**
	 * If outline group starts or ends here (depending on where outline buttons are located)
	 * and is collapsed
	 */
	bool m_isOutlineGroupStartsEnds = false;
	/**
	 * This flag is set if the upper border of at least one cell in this row or if the lower
	 * border of at least one cell in row above is formatted with a thick line style.
	 * Thin and medium line styles are not taken into account
	 */
	bool m_hasAdditionalSpaceAbove = false;
	/**
	 * This flag is set if the lower border of at least one cell in this row or if the upper
	 * border of at least one cell in row below is formatted with a medium or thick line style.
	 * Thin line styles are not taken into account
	 */
	bool m_hasAdditionalSpaceBelow = false;
};


/**
 * @class Sheet
 * @brief
 *     Contains data for one worksheet
 * @note
 *     Negative values for row/column indexes and slice positions are supported in expected fashion
 */
class Sheet {
public:
	/**
	 * @param[in] book
	 *     Pointer to parent Book object
	 * @param[in] position
	 *     Sheet record start position
	 * @param[in] name
	 *     Sheet name
	 * @param[in] number
	 *     Sheet number
	 * @param[in] table
	 *     Result HTML table
	 * @since 1.0
	 */
	Sheet(Book* book, int position, const std::string& name,
          size_t number, std::string& text);

	/**
	 * @brief
	 *     Read SHEET data
	 * @since 1.0
	 */
	void read();

	/**
	 * @brief
	 *     Add cell data to table
	 * @param[in] rowIndex
	 *     Row index
	 * @param[in] colIndex
	 *     Column index
	 * @param[in] value
	 *     Cell value
	 * @param[in] xfIndex
	 *     XF index
	 * @since 1.0
	 */
	void putCell(int rowIndex, int colIndex, const std::string& value, int xfIndex);

    void append(const std::string &value);

	/**
	 * @brief
	 *     Add missing cells to table
	 * @since 1.0
	 */
	void tidyDimensions();

	/** Pointer to parent BOOK object */
	Book* m_book;
	/** Result HTML table */
    std::string& m_sheetContent;
	/** Sheet name */
	std::string m_name;
	/** Sheet number */
	size_t m_number;
	/** Number of rows in sheet */
	int m_rowCount = 0;
	/**
	 * Nominal number of columns in sheet. It is one more than maximum column index found,
	 * ignoring trailing empty cells
	 */
	int m_colCount = 0;
	/**
	 * Map from column index to COLINFO object. Often there is an entry in COLINFO records
	 * for all column indexes in `range(257)`
	 */
	std::unordered_map<int, Colinfo> m_colinfoMap;
	/**
	 * Map from row index to ROWINFO object. It is possible to have missing entries - at least
	 * one source of XLS files doesn't bother writing ROW records
	 */
	std::unordered_map<int, Rowinfo> m_rowinfoMap;
	/**
	 * List of address ranges of cells containing column labels. These are set up
	 * in Excel by `Insert` -> `Name` -> `Labels` -> `Columns`
	 */
	std::vector<std::vector<int>> m_colLabelRanges;
	/** List of address ranges of cells containing row labels */
	std::vector<std::vector<int>> m_rowLabelRanges;
	/**
	 * List of address ranges of cells which have been merged. These are set up
	 * in Excel by `Format` -> `Cells` -> `Alignment`, then ticking "Merge cells" box.
	 * Upper limits are exclusive: i.e. `[2, 3, 7, 9]` only spans two cells
	 */
	std::vector<std::vector<int>> m_mergedCells;
	/** Table parts style list in format: `(row1, col1, row2, col2, styleId)` */
	std::vector<std::vector<int>> m_tableParts;
	/** Max row count */
	long int m_maxRowCount;
	/** Max column count */
	int m_maxColCount = 256;
	/** Dimension row count as per DIMENSIONS record */
	int m_dimensionRowCount = 0;
	/** Dimension column count as per DIMENSIONS record */
	int m_dimensionColCount = 0;
	/** First full row index */
	int m_firstFullRowIndex = -1;
	/**
	 * Default column width from DEFCOLWIDTH record, else `None`. From the OOo docs:
	 * Column width in characters, using width of zero character from default font (first FONT
	 * record in file). Excel adds some extra space to the default width, depending on default font
	 * and default font size. Algorithm how to exactly calculate resulting column width is not known.
	 * Example:
	 * @code Default width of 8 set in this record results in column width of 8.43 using Arial
	 * font with a size of 10 points. @endcode
	 */
	unsigned short m_defaultColWidth = 0;
	/**
	 * Default column width from STANDARDWIDTH record, else `None`. From the OOo docs:
	 * Default width of columns in 1/256 of width of zero character, using default font
	 * (first FONT record in the file)
	 */
	unsigned short m_standardWidth;
	/**
	 * Default value to be used for row if there is no ROW record for that row. From
	 * optional `DEFAULTROWHEIGHT` record
	 */
	int m_defaultRowHeight;
	/**
	 * Default row height mismatch
	 * Value | Description
	 * :---: | -----------
	 * False | Default row height and default font height are equal
	 * True  | Default row height and default font height do not match
	 */
	bool m_isDefaultRowHeightMismatch;
	/** If default row is hidden */
	bool m_isDefaultRowHidden;
	/** If cells have default additional space above */
	bool m_hasDefaultAdditionalSpaceAbove;
	/** If cells have default additional space below */
	bool m_hasDefaultAdditionalSpaceBelow;
	/** List of HYPERLINK objects corresponding to HLINK records found in worksheet */
	std::vector<Hyperlink> m_hyperlinkList;
	/**
	 * Mapping from `(rowIndex, colIndex)` to item in #m_hyperlinkList. Cells not covered by
	 * hyperlink are not mapped. It is possible using the Excel UI to set up a hyperlink that
	 * covers larger-than-1x1 rectangle of cells. Hyperlink rectangles may overlap (Excel doesn't
	 * check). When a multiply-covered cell is clicked on, hyperlink that is activated (and the one
	 * that is mapped here) is the last in @ref Sheet.m_hyperlinkList
	 */
	std::map<std::pair<int, int>, Hyperlink> m_hyperlinkMap;
	/**
	 * Mapping from `(rowIndex, colIndex)` to NOTE object. Cells not containing
	 * a note ("comment") are not mapped
	 */
	std::map<std::pair<int, int>, Note> m_cellNoteMap;
	/**
	 * Contains data for one cell. Cell objects have three attributes: `type` is an int, `value`
	 * (which depends on `type`) and `xfIndex`.
	 * The following table describes types of cells and how their values are represented
	 *   Type symbol   | Type number | Value
	 * --------------- | :---------: | -----
	 * XL_CELL_EMPTY   |      0      | Empty string
	 * XL_CELL_TEXT    |      1      | Unicode string
	 * XL_CELL_NUMBER  |      2      | Float
	 * XL_CELL_DATE    |      3      | Float
	 * XL_CELL_BOOLEAN |      4      | Bool: 0 => False; 1 => True
	 * XL_CELL_ERROR   |      5      | Int representing internal Excel codes
	 * XL_CELL_BLANK   |      6      | Empty string. Only when m_addStyle = true
	 */
	std::vector<std::vector<std::string>> m_cellValues;
	std::vector<std::vector<int>> m_cellTypes;
	std::vector<std::vector<int>> m_cellXfIndexes;
	/**
	 * Visibility of sheet
	 * Value | Description
	 * :---: | -----------
	 *   0   | Visible
	 *   1   | Hidden (can be unhidden by user - `Format` -> `Sheet` -> `Unhide`)
	 *   2   | "very hidden" (can be unhidden only by VBA macro)
	 */
	int m_visibility = 0;
	/** First visible row index */
	unsigned short m_firstVisibleRowIndex = 0;
	/** First visible column index */
	unsigned short m_firstVisibleColIndex = 0;
	/** Grid line color. Pre-BIFF8 */
	std::vector<unsigned char> m_gridlineColor {0, 0, 0};
	/** Grid line color index */
	unsigned short m_gridlineColorIndex = 0x40;
	/**
	 * Sheet flags
	 * Bit |  Mask | Contents
	 * :-: | ----- | --------
	 *  0  | 0001H | 0 = Show formula results 1 = Show formulas
	 *  1  | 0002H | 0 = Do not show grid lines 1 = Show grid lines
	 *  2  | 0004H | 0 = Do not show sheet headers 1 = Show sheet headers
	 *  3  | 0008H | 0 = Panes are not frozen 1 = Panes are frozen (freeze)
	 *  4  | 0010H | 0 = Show zero values as empty cells 1 = Show zero values
	 *  5  | 0020H | 0 = Manual grid line color 1 = Automatic grid line color
	 *  6  | 0040H | 0 = Columns from left to right 1 = Columns from right to left
	 *  7  | 0080H | 0 = Do not show outline symbols 1 = Show outline symbols
	 *  8  | 0100H | 0 = Keep splits if pane freeze is removed 1 = Remove splits if pane freeze is removed
	 *  9  | 0200H | 0 = Sheet not selected 1 = Sheet selected (BIFF5-BIFF8)
	 * 10  | 0400H | 0 = Sheet not visible 1 = Sheet visible (BIFF5-BIFF8)
	 * 11  | 0800H | 0 = Show in normal view 1 = Show in page break preview (BIFF8)
	 */
	bool m_showFormula            = false;
	bool m_showGridLine           = true;
	bool m_showSheetHeader        = true;
	bool m_isFrozenPanes          = false;
	bool m_showZeroValue          = true;
	bool m_automaticGridLineColor = true;
	bool m_columnsRightToLeft     = false;
	bool m_showOutlineSymbol      = true;
	bool m_removeSplits           = false;
	/** If sheet is selected. Multiple sheets can be selected, but only one can be active */
	bool m_isSheetSelected = 0;
	/**
	 * Should really be called "sheetActive" and is 1 when this sheet is sheet displayed when file
	 * is open. More than likely only one sheet should ever be set as visible. This would correspond
	 * to Book's sheetActive attribute, but that doesn't exist as WINDOW1 records aren't currently
	 * processed. The real thing is the visibility attribute from BOUNDSHEET record
	 */
	bool m_isSheetVisible = 0;
	/**
	 * Mapping of `(rowIndex, colIndex)` to list of `(offset, fontIndex)` tuples. Offset defines
	 * where in string the font begins to be used. Offsets are expected to be in ascending order.
	 * If first offset != zero, the meaning is that cell's XF font should be used from offset 0.
	 * This is a sparse mapping. There is no entry for cells that are not formatted with rich text
	 */
	std::map<std::pair<int, int>, std::vector<std::pair<unsigned short, unsigned short>>> m_richtextRunlistMap;
	/** Number of columns in left pane (frozen panes) */
	unsigned short m_vertSplitPos = 0;
	/** Number of rows in top pane (frozen panes) */
	unsigned short m_horzSplitPos = 0;
	/** Index of first visible row in bottom frozen/split pane */
	unsigned short m_horzSplitFirstVisible = 0;
	/** Index of first visible column in right frozen/split pane */
	unsigned short m_vertSplitFirstVisible = 0;
	/** Frozen panes: ignore it. Split panes: explanation and diagrams in OOo docs */
	char m_splitActivePane = 0;
	/** Boolean specifying if PANE record was present */
	bool m_hasPaneRecord = false;
	/**
	 * List of horizontal page breaks in this sheet. Breaks are tuples in the form
	 * `(index of row after break, start column index, end column index)`
	 */
	std::vector<std::vector<int>> m_horizontalPageBreaks;
	/**
	 * List of vertical page breaks in this sheet. Breaks are tuples in the form
	 * `(index of column after break, start row index, end row index)`
	 */
	std::vector<std::vector<int>> m_verticalPageBreaks;
	/**
	 * 256-element tuple corresponding to contents of GCW record for this sheet. If no such record,
	 * treat as all bits zero. Applies to BIFF4-7 only
	 */
	std::vector<int> m_gcw;
	/**
	 * Values calculated to predict mag factors that will actually be used by Excel to display
	 * worksheet. Pass these values to when writing XLS files.
	 * Warning 1: Behaviour of OOo Calc and Gnumeric has been observed to differ from Excel's.
	 * Warning 2: A value of zero means almost exactly what it says. Your sheet will be displayed
	 * as very tiny speck on the screen
	 */
	int m_cookedPageBreakPreviewMagFactor = 60;
	int m_cookedNormalViewMagFactor       = 100;
	/** Values (if any) actually stored on the XLS file */
	int  m_cachedPageBreakPreviewMagFactor = 0;  // Default (60%), from WINDOW2 record
	int  m_cachedNormalViewMagFactor       = 0;  // Default (100%), from WINDOW2 record
	bool m_showPageBreakPreview = 0;
	int  m_sclMagFactor = -1; // From SCL record
	int  m_ixfe         = 0;  // BIFF2 only

private:
	/**
	 * @brief
	 *     Get record string content
	 * @param[in] data
	 *     Binary data
	 * @return
	 *     Record string content
	 * @since 1.0
	 */
	std::string stringRecordContent(const std::string& data);

	/**
	 * @brief
	 *     Get fixed BIFF 2 XF index
	 * @param[in] cellAttributes
	 *     Cell attributes
	 * @param[in] rowIndex
	 *     Row index
	 * @param[in] colIndex
	 *     Column index
	 * @param[in] trueXfIndex
	 *     True XF index
	 * @return
	 *     XF index
	 * @since 1.0
	 */
	int fixedXfIndexB2(const std::string& cellAttributes, int trueXfIndex = -1);

	/**
	 * @brief
	 *     Insert new BIFF 2.0 XF
	 * @param[in] cellAttributes
	 *     Cell attributes
	 * @param[in] isStyle
	 *     If cell attribute is style
	 * @return
	 *     XF index
	 * @since 1.0
	 */
	int insertXfB20(const std::string& cellAttributes, bool isStyle = false);

	/**
	 * @brief
	 *     Get fake XF from BIFF 2.0 cell attribute
	 * @param[out] xf
	 *     Parent XF
	 * @param[in] cellAttributes
	 *     Cell attributes
	 * @param[in] isStyle
	 *     If cell attribute is style
	 * @since 1.0
	 */
	void fakeXfFromCellAttrB20(XF& xf, const std::string& cellAttributes, bool isStyle = false);

	/**
	 * @brief
	 *     Get null-terminated unicode string
	 * @param[in] buf
	 *     Binary data
	 * @param[in] offset
	 *     Record start position
	 * @return
	 *     Unicode string
	 * @since 1.0
	 */
	std::string getNullTerminatedUnicode(const std::string& buf, int& offset) const;

	/**
	 * @brief
	 *     Read HYPERLINK record
	 * @param[in] data
	 *     Binary data
	 * @since 1.0
	 */
	void handleHyperlink(const std::string& data);

	/**
	 * @brief
	 *     Read QUICKTIP record
	 * @param[in] data
	 *     Binary data
	 * @since 1.0
	 */
	void handleQuicktip(const std::string& data);

	/**
	 * @brief
	 *     Process MSObj object
	 * @param[in] data
	 *     Binary data
	 * @param[out] msObj
	 *     MSObj object
	 * @since 1.0
	 */
	void handleMSObj(const std::string& data, MSObj& msObj);

	/**
	 * @brief
	 *     Process MSTxo object
	 * @param[in] data
	 *     Binary data
	 * @param[out] msTxo
	 *     MSTxo object
	 * @since 1.0
	 */
	void handleMSTxo(const std::string& data, MSTxo& msTxo);

	/**
	 * @brief
	 *     Process NOTE record
	 * @param[in] data
	 *     Binary data
	 * @param[in] msTxos
	 *     List of MSTxo objects
	 * @since 1.0
	 */
	void handleNote(const std::string& data, std::unordered_map<unsigned short, MSTxo>& msTxos);

	/**
	 * @brief
	 *     Update coocked factors
	 * @details
	 *     Cached values are used ONLY for non-active view mode. When user switches to non-active
	 *     view mode, if cached value for that mode is not valid, Excel pops up a window which says:
	 *     "The number must be between 10 and 400. Try again by entering a number in this range."
	 *     When user hits OK, it drops into non-active view mode but uses the magn from active mode
	 * @since 1.0
	 */
	void updateCookedFactors();

	/**
	 * @brief
	 *     Unpack range of cells address list and update position
	 * @param[out] outputList
	 *     Array in which address will be saved
	 * @param[in] data
	 *     Binary data
	 * @param[in] pos
	 *     Record start position
	 * @param[in] addressSize
	 *     Address size
	 * @since 1.0
	 */
	void unpackCellRangeAddressListUpdatePos(std::vector<std::vector<int>>& outputList,
											 const std::string& data, int& pos, int addressSize = 6) const;

	/**
	 * @brief
	 *     Unpack RK record data
	 * @param[in] data
	 *     Binary data
	 * @return
	 *     RD data
	 * @since 1.0
	 */
	double unpackRK(const std::string& data) const;

	/**
	 * @brief
	 *     Add style to cell node in HTML-tree
	 * @param[out] node
	 *     Cell node in HTML-tree
	 * @param[in] xf
	 *     Parent XF
	 * @param[in] rowIndex
	 *     Row index
	 * @param[in] colIndex
	 *     Column index
	 * @since 1.0
	 */
	void addCellStyle(pugi::xml_node& node, const XF& xf, int rowIndex, int colIndex);

	/**
	 * @brief
	 *     Add style to table row
	 * @param[out] node
	 *     Cell node in HTML-tree
	 * @param[in] rowIndex
	 *     Row index
	 * @since 1.0
	 */
	void addRowStyle(pugi::xml_node& node, int rowIndex);

	/**
	 * @brief
	 *     Add style to table column
	 * @param[out] node
	 *     Cell node in HTML-tree
	 * @param[in] colIndex
	 *     Column index
	 * @since 1.0
	 */
	void addColStyle(pugi::xml_node& node, int colIndex);

	/**
	 * @brief
	 *     Get object color from XFColor record
	 * @param[in] xf
	 *     Parent XFColor object
	 * @return
	 *     Object color like `rgb(red, green, blue)`
	 * @since 1.0
	 */
	std::string getColor(const XFColor& color) const;

	/**
	 * @brief
	 *     Get table color value
	 * @param[out] style
	 *     Result style value
	 * @param[in] colorMap
	 *     Color map
	 * @param[in] colorIndex
	 *     Color index
	 * @since 1.0
	 */
	void getTableColor(std::string& style, const std::vector<std::string>& colorMap, int colorIndex) const;

	/** Sheet record start position */
	int m_position;
	//** Highest rowIndex containing a non-empty cell */
	//int m_maxDataRowIndex = -1;
	//** Highest colIndex containing a non-empty cell */
	//int m_maxDataColIndex = -1;
	/** Maps cell attributes to XF index. BIFF2.0 only */
	std::unordered_map<std::string, int> m_cellAttributesToXfIndex;
};


/**
 * @class Hyperlink
 * @brief
 *     Contains attributes of hyperlink
 */
class Hyperlink {
public:
	Hyperlink() = default;

	/** Index of first row */
	unsigned short m_firstRowIndex = 0;
	/** Index of last row */
	unsigned short m_lastRowIndex = 0;
	/** Index of first column */
	unsigned short m_firstColIndex = 0;
	/** Index of last column */
	unsigned short m_lastColIndex = 0;
	/** Type of hyperlink: unicode string, 'url', 'unc', 'local file', 'workbook', 'unknown' */
	std::string m_type;
	/**
	 * URL or file-path, depending on type. Unicode string, except in the rare case of
	 * a local but non-existent file with non-ASCII characters in name, in which case
	 * only the "8.3" filename is available
	 */
	std::string m_url;
	/**
	 * Description. It is displayed in cell, and should be identical to cell value. Unicode string
	 * or `None`. It seems impossible NOT to have description created by Excel UI
	 */
	std::string m_description;
	/**
	 * Target frame. Unicode string. No cases of this have been seen in the wild.
	 * It seems impossible to create one in Excel UI
	 */
	std::string m_target;
	/**
	 * The piece after the "#" in "http://docs.python.org/library#struct_module",
	 * or `Sheet1!A1:Z99` part when type is "workbook"
	 */
	std::string m_textmark;
	/** Text of "quick tip" displayed when cursor hovers over hyperlink */
	std::string m_quicktip;
};


/**
 * @class MSObj
 * @brief
 *     Represents MSObj object
 */
class MSObj {
public:
	MSObj() = default;

	/** If object is NULL */
	bool m_isNull = false;
	/** Object type */
	unsigned short m_type = 0;
	/** Object id */
	unsigned short m_id = -1;
	/** If object is locked */
	bool m_isLocked = false;
	/** If object is printable */
	bool m_isPrintable = false;
	/** Automatic filter options */
	int m_autoFilter = 0;
	/** Automatic fill options */
	int m_autoFill = 0;
	/** Automatic line options */
	int m_autoLine = 0;
	/** Scrollbar parameters */
	int m_scrollbarFlag = 0;
	unsigned short m_scrollbarValue = 0;
	unsigned short m_scrollbarMin   = 0;
	unsigned short m_scrollbarMax   = 0;
	unsigned short m_scrollbarInc   = 0;
	unsigned short m_scrollbarPage  = 0;
};


/**
 * @class MSTxo
 * @brief
 *     Represents MSTxo object
 */
class MSTxo {
public:
	MSTxo() = default;

	/** If object is NULL */
	bool m_isNull = false;
	/** Object rotation */
	unsigned short m_rotation = 0;
	/** If is not empty */
	unsigned short m_isNotEmpty = -1;
	/** Object formula */
	std::string m_formula;
	/** Object text */
	std::string m_text;
	/** Object horizontal alignment */
	int m_horzAlign = 0;
	/** Object vertical alignment */
	int m_vertAlign = 0;
	/** Text options */
	int m_lockText = 0;
	int m_justLast = 0;
	int m_secretEdit = 0;
	/** List List of `(offsetInString, fontIndex)` tuples. The first offset should always be 0 */
	std::vector<std::pair<unsigned short, unsigned short>> m_richtextRunlist;
};


/**
 * @class Note
 * @brief
 *     Represents user "comment" or "note" object
 */
class Note {
public:
	Note() = default;

	/** Current object id */
	unsigned short m_objectId = 0;
	/** Author of note */
	std::string m_author;
	/** Text of note */
	std::string m_text;
	/** If note is always shown */
	bool m_isShown = 0;
	/** If containing row is hidden */
	bool m_isRowHidden = false;
	/** If containing column is hidden */
	bool m_isColHidden = false;
	/** Row index */
	int m_rowIndex = 0;
	/** Column index */
	int m_colIndex = 0;
	/** List of `(offsetInString, fontIndex)` tuples. The first offset should always be 0 */
	std::vector<std::pair<unsigned short, unsigned short>> m_richtextRunlist;
};

}  // End namespace
