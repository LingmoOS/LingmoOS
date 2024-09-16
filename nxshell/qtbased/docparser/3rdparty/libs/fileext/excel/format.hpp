/**
 * @brief     Excel files (xls/xlsx) into HTML сonverter
 * @package   excel
 * @file      format.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright python-excel (https://github.com/python-excel/xlrd)
 * @date      02.12.2016 -- 18.10.2017
 */
#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "biffh.hpp"
#include "book.hpp"
#include "frmt.hpp"

namespace excel {

class Book;

/**
 * @class Formatting
 * @brief
 *     Workbook formatting information
 */
class Formatting {
public:
	/**
	 * @param[in] book
	 *     Pointer to parent Book object
	 * @since 1.0
	 */
	Formatting(Book* book);

	/**
	 * @brief
	 *     Initialize formatting for workbook
	 * @since 1.0
	 */
	void initializeBook();

	/**
	 * @brief
	 *     Read FONT record
	 * @param[in] data
	 *     Binary data
	 * @since 1.0
	 */
	void handleFont(const std::string& data);

	/**
	 * @brief
	 *     Read FORMAT record
	 * @param[in] data
	 *     Binary data
	 * @param[in] recordType
	 *     Record type
	 * @since 1.0
	 */
	void handleFormat(const std::string& data, int recordType = XL_FORMAT);

	/**
	 * @brief
	 *     Read XF record
	 * @param[in] data
	 *     Binary data
	 * @throw std::logic_error
	 *     Programmer stuff-up: bv=%1
	 * @since 1.0
	 */
	void handleXf(const std::string& data);

	/**
	 * @brief
	 *     Read palette and color records
	 * @param[in] data
	 *     Binary data
	 * @throw std::logic_error
	 *     PALETTE record: expected size %1, actual size %2
	 * @since 1.0
	 */
	void handlePalette(const std::string& data);

	/**
	 * @brief
	 *     Read style indexes records
	 * @param[in] data
	 *     Binary data
	 * @since 1.0
	 */
	void handleStyle(const std::string& data);

	/**
	 * @brief
	 *     Finalize XF records
	 * @since 1.0
	 */
	void xfEpilogue();

	/**
	 * @brief
	 *     Check color indexes in fonts. FONT records must come before PALETTE record
	 * @since 1.0
	 */
	void paletteEpilogue();

	/**
	 * @brief
	 *     Check if formatted string is date
	 * @details
	 *     Heuristics. Handle backslashed-escaped chars properly (`hh\hmm\mss\s` -> `23h59m59s`).
	 *     Date formats have one or more of ymdhs (caseless) in them. Numeric formats have # and 0.
	 * @param[in] format
	 *     Formatted string
	 * @return
	 *     True if format string is date
	 * @since 1.0
	 */
	static bool isDateFormattedString(const std::string& format);

	/**
	 * @brief
	 *     Find nearest object color index in palette
	 * @details
	 *     Uses Euclidean distance. Used only for pre-BIFF8 `WINDOW2` record.
	 *     Doesn't fast and fancy.
	 * @param[in] colorMap
	 *     Color map
	 * @param[in] rgb
	 *     Object color like `(red, green, blue)`
	 * @return
	 *     Color index in palette
	 * @since 1.0
	 */
	static int getNearestColorIndex(std::unordered_map<int, std::vector<unsigned char>>& colorMap,
									std::vector<unsigned char>& rgb);

	/** Pointer to parent Book object */
	Book* m_book;
};


/**
 * @class XFColor
 * @brief
 *     Color information for font, background, borders
 */
class XFColor {
public:
	XFColor() = default;

	/**
	 * Color representing form
	 * Value | Description
	 * :---: | -----------
	 * False | Color is represented by index in color map
	 * True  | Color is represented in raw RGB form
	 */
	bool m_isRgb = false;
	/** Color index */
	int m_index = -1;
	/**
	 * Color tint
	 * Value | Formula
	 * :---: | -------
	 *  < 0  | color = color * (1 + tint)
	 *  > 0  | color = color * (1 - tint) + (255 - 255 * (1 - tint))
	 */
	double m_tint = 0;
	/** Object color like `(red, green, blue)` */
	std::vector<unsigned char> m_rgb;
};


/**
 * @class Font
 * @brief
 *     Contains details of not only what is normally considered a font, but also several
 *     other display attributes
 * @details
 *     Items correspond to those in the Excel UI's `Format` -> `Cells` -> `Font` tab.
 */
class Font {
public:
	Font() = default;

	/**
	 * If text is bold. Redundant, see "weight" attribute
	 * Value | Description
	 * :---: | -----------
	 * False | Normal text
	 * True  | Characters are bold
	 */
	bool m_isBold = false;
	/**
	 * If text is italic
	 * Value | Description
	 * :---: | -----------
	 * False | Normal text
	 * True  | Characters are italic
	 */
	bool m_isItalic = false;
	/**
	 * Underline types
	 * Value | Description
	 * :---: | -----------
	 *   0   | None
	 *   1   | Single. 0x21 (33) = Single accounting
	 *   2   | Double. 0x22 (34) = Double accounting
	 */
	unsigned char m_underlineType = 0;
	/**
	 * If text is underlined. Redundant
	 * Value | Description
	 * :---: | -----------
	 * False | Normal text
	 * True  | Characters are underlined
	 */
	bool m_isUnderlined = false;
	/**
	 * If text is struck out
	 * Value | Description
	 * :---: | -----------
	 * False | Normal text
	 * True  | Characters are struck out
	 */
	bool m_isStruckOut = false;
	/**
	 * If text is outlined (Macintosh only)
	 * Value | Description
	 * :---: | -----------
	 * False | Normal text
	 * True  | Characters are outlined
	 */
	bool m_isOutlined = false;
	/**
	 * If text is shadowed (Macintosh only)
	 * Value | Description
	 * :---: | -----------
	 * False | Normal text
	 * True  | Characters are shadowed
	 */
	bool m_isShadowed = false;
	/** Font weight (100-1000). Standard values are 400 for normal text and 700 for bold text */
	unsigned short m_weight = 400;
	/**
	 * Character set values
	 * Value | Description
	 * :---: | -----------
	 *   0   | ANSI Latin
	 *   1   | System default
	 *   2   | Symbol
	 *  77   | Apple Roman
	 *  128  | ANSI Japanese Shift-JIS
	 *  129  | ANSI Korean (Hangul)
	 *  130  | ANSI Korean (Johab)
	 *  134  | ANSI Chinese Simplified GBK
	 *  136  | ANSI Chinese Traditional BIG5
	 *  161  | ANSI Greek
	 *  162  | ANSI Turkish
	 *  163  | ANSI Vietnamese
	 *  177  | ANSI Hebrew
	 *  178  | ANSI Arabic
	 *  186  | ANSI Baltic
	 *  204  | ANSI Cyrillic
	 *  222  | ANSI Thai
	 *  238  | ANSI Latin II (Central European)
	 *  255  | OEM Latin I
	 */
	unsigned char m_characterSet = 0;
	/** Font color */
	XFColor m_color;
	/**
	 * Text escapment
	 * Value | Description
	 * :---: | -----------
	 *   1   | Superscript
	 *   2   | Subscript
	 */
	unsigned short m_escapement = 0;
	/**
	 * Font family
	 * Value | Description
	 * :---: | -----------
	 *   0   | None (unknown or don't care)
	 *   1   | Roman (variable width, serifed)
	 *   2   | Swiss (variable width, sans-serifed)
	 *   3   | Modern (fixed width, serifed or sans-serifed)
	 *   4   | Script (cursive)
	 *   5   | Decorative (specialised, e.g. Old English, Fraktur)
	 */
	unsigned char m_family = 0;
	/** Font name */
	std::string m_name;
	/** 0-based font index used to refer to this object. Note that index 4 is never used */
	int m_fontIndex = 0;
	/** Font height (in twips). A twip = 1/20 of a point */
	unsigned short m_height = 0;
};


/**
 * @class XFAlignment
 * @brief
 *     A collection of alignment and similar attributes of XF record
 * @details
 *     Items correspond to those in the Excel UI's `Format` -> `Cells` -> `Alignment` tab.
 */
class XFAlignment {
public:
	XFAlignment() = default;

	/**
	 * Cell horizontal alignment. Section 6.115 (p 214) of OOo docs
	 * Value | Description
	 * :---: | -----------
	 *   0   | General
	 *   1   | Left
	 *   2   | Centred
	 *   3   | Right
	 *   4   | Filled
	 *   5   | Justified (BIFF4-BIFF8X)
	 *   6   | Centred across selection (BIFF4-BIFF8X)
	 *   7   | Distributed (BIFF8X)
	 */
	int m_horizontalAlign = 0;
	/**
	 * Cell vertical alignment. Section 6.115 (p 215) of OOo docs
	 * Value | Description
	 * :---: | -----------
	 *   0   | Top
	 *   1   | Centred
	 *   2   | Bottom
	 *   3   | Justified (BIFF5-BIFF8X)
	 *   4   | Distributed (BIFF8X)
	 */
	int m_verticalAlign = 0;
	/**
	 * Text rotation. Section 6.115 (p 215) of OOo docs.
	 * File versions BIFF7 and earlier use the documented `orientation` attribute
	 */
	unsigned char m_rotation = 0;
	/**
	 * If text is wrapped
	 * Value | Description
	 * :---: | -----------
	 *   0   | Normal text
	 *   1   | Text is wrapped at right margin
	 */
	int m_isTextWrapped = 0;
	/** Indent level */
	int m_indentLevel = 0;
	/**
	 * If font size is shrinked-to-fit
	 * Value | Description
	 * :---: | -----------
	 * False | Normal font size
	 * True  | Shrink font size to fit text into cell
	 */
	bool m_isShrinkToFit = false;
	/**
	 * Text direction
	 * Value | Description
	 * :---: | -----------
	 *   0   | According to context
	 *   1   | Left-to-right
	 *   2   | Right-to-left
	 */
	unsigned char m_textDirection = 0;
};


/**
 * @class XFBorder
 * @brief
 *     A collection of border-related attributes of XF record
 * @details
 *     Items correspond to those in the Excel UI's `Format` -> `Cells` -> `Border` tab
 *     There are five line style attributes; possible values and associated meanings:
 *     Value | Description
 *     :---: | -----------
 *       0   | No line
 *       1   | Thin
 *       2   | Medium
 *       3   | Dashed
 *       4   | Dotted
 *       5   | Thick
 *       6   | Double
 *       7   | Hair
 *       8   | Medium dashed
 *       9   | Thin dash-dotted
 *       10  | Medium dash-dotted
 *       11  | Thin dash-dot-dotted
 *       12  | Medium dash-dot-dotted
 *       13  | Slanted medium dash-dotted
 *     Line styles 8 to 13 appear in BIFF8 files (Excel 97 and later) only
 */
class XFBorder {
public:
	XFBorder() = default;

	/** Top line color */
	XFColor m_topColor;
	/** Bottom line color */
	XFColor m_bottomColor;
	/** Left line color */
	XFColor m_leftColor;
	/** Right line color */
	XFColor m_rightColor;
	/** Diagonal line color */
	XFColor m_diagColor;
	/** Top line style */
	int m_topLineStyle = 0;
	/** Bottom line style */
	int m_bottomLineStyle = 0;
	/** Left line style */
	int m_leftLineStyle = 0;
	/** Right line style */
	int m_rightLineStyle = 0;
	/** Diagonal line style */
	int m_diagLineStyle = 0;
	/** Draw diagonal from top left to bottom right */
	bool m_diagDown = false;
	/** Draw diagonal from bottom left to top right */
	bool m_diagUp = false;
};


/**
 * @class XFBackground
 * @brief
 *     A collection of background-related attributes of XF record
 * @details
 *     Items correspond to those in the Excel UI's `Format` -> `Cells` -> `Patterns` tab.
 */
class XFBackground {
public:
	XFBackground() = default;

	/**
	 * Background fill pattern. Section 3.11 of the OOo docs
	 * Value | Description
	 * :---: | -----------
	 *   0   | None
	 *   1   | Solid
	 *   2   | Medium gray
	 *   3   | Dark gray
	 *   4   | Light gray
	 *   5   | Dark horizontal
	 *   6   | Dark vertical
	 *   7   | Dark down
	 *   8   | Dark up
	 *   9   | Dark grid
	 *   10  | Dark trellis
	 *   11  | Light horizontal
	 *   12  | Light vertical
	 *   13  | Light down
	 *   14  | Light up
	 *   15  | Lightg rid
	 *   16  | Light trellis
	 *   17  | Gray 125
	 *   18  | Gray 0625
	 */
	int m_fillPattern = 0;
	/** Background color */
	XFColor m_backgroundColor;
	/** Pattern color */
	XFColor m_patternColor;
};


/**
 * @class XFProtection
 * @brief
 *     A collection of protection-related attributes of XF record
 * @details
 *     Items correspond to those in the Excel UI's `Format` -> `Cells` -> `Protection` tab.
 * @note
 *     The OOo docs include the "cell or style" bit in this bundle of attributes.
 *     This is incorrect; the bit is used in determining which bundles to use.
 */
class XFProtection {
public:
	XFProtection() = default;

	/**
	 * If cell is locked (only if the sheet is protected)
	 * Value | Description
	 * :---: | -----------
	 * False | Normal cell
	 * True  | Cell is prevented from being changed, moved, resized, or deleted
	 */
	bool m_isCellLocked = false;
	/**
	 * If formula is hidden (only if the sheet is protected)
	 * Value | Description
	 * :---: | -----------
	 * False | Normal formula
	 * True  | Hide formula so that it doesn't appear in formula bar when cell is selected
	 */
	bool m_isFormulaHidden = false;
};


/**
 * @class XF
 * @brief
 *     eXtended Formatting information for cells, rows, columns and styles
 */
class XF {
public:
	XF() = default;

	/**
	 * XF parent type
	 * Value | Description
	 * :---: | -----------
	 * False | Cell XF
	 * False | Style XF
	 */
	bool m_isStyle = false;
	/**
	 * Parent style index
	 * Description | Value
	 * ----------- | :---:
	 * Cell XF     | Index into @ref Book::m_xfList of this XF's style XF
	 * Style XF    | 0xFFF
	 */
	int m_parentStyleIndex = 0;
	/**
	 * Each of flags describes the validity of specific group of attributes
	 * In cell XFs:
	 * Value | Description
	 * :---: | -----------
	 *   0   | Attributes of parent style XF are used, (but only if attributes are valid there)
	 *   1   | Attributes of this XF are used
	 * In style XFs:
	 * Value | Description
	 * :---: | -----------
	 *   0   | Attribute setting is valid
	 *   1   | Attribute should be ignored
	 */
	bool m_formatFlag     = false;
	bool m_fontFlag       = false;
	bool m_alignmentFlag  = false;
	bool m_borderFlag     = false;
	bool m_backgroundFlag = false;
	bool m_protectionFlag = false;
	bool m_lotusPrefix    = false;
	/** XF index in @ref Book::m_xfList */
	int m_xfIndex = 0;
	/** Font index in @ref Book::m_fontList */
	unsigned short m_fontIndex = 0;
	/**
	 * Key into @ref Book::m_formatMap
	 * OOo docs on XF record call this "Index to FORMAT record". It is a key to map.
	 * It is true only for Excel 4.0 and earlier files that the key into formatMap from XF
	 * instance is the same as the index into formatList, and only if index is less than 164
	 */
	unsigned short m_formatKey = 0;
	/** XFAlignment object */
	XFAlignment m_alignment;
	/** XFBorder object */
	XFBorder m_border;
	/** XFBackground object */
	XFBackground m_background;
	/** XFProtection object */
	XFProtection m_protection;
};

}  // End namespace