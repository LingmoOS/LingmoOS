/**
 * @brief     Excel files (xls/xlsx) into HTML сonverter
 * @package   excel
 * @file      xlsx.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright python-excel (https://github.com/python-excel/xlrd)
 * @date      02.12.2016 -- 18.10.2017
 */
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <pugixml.hpp>

#include "fileext/ooxml/ooxml.hpp"

#include "book.hpp"
#include "format.hpp"


namespace excel {

/**
 * @class Xlsx
 * @brief
 *     Wrapper for XLSX format
 */
class Xlsx: public ooxml::Ooxml {
public:
	/**
	 * @param[in] book
	 *     Pointer to parent Book object
	 * @since 1.0
	 */
	Xlsx(Book* book);

	/**
	 * @brief
	 *     Read XLSX WorkBook
	 * @since 1.0
	 */
	void openWorkbookXlsx();

	/** Pointer to parent Book object */
	Book* m_book;
};


/**
 * @class X12General
 * @brief
 *     Base class for document components
 */
class X12General: public ooxml::Ooxml {
public:
	/**
	 * @param[in] book
	 *     Pointer to parent Book object
	 * @since 1.0
	 */
	X12General(Book* book);

	/**
	 * @brief
	 *     Get node text
	 * @param[in] node
	 *     Node in XML-tree
	 * @return
	 *     Node text
	 * @since 1.0
	 */
	std::string getNodeText(const pugi::xml_node& node);

	/**
	 * @brief
	 *     Get text from `is` or `is` nodes
	 * @param[in] node
	 *     Node in XML-tree
	 * @return
	 *     Node text
	 * @since 1.0
	 */
	std::string getTextFromSiIs(const pugi::xml_node& node);

	/**
	 * @brief
	 *     Convert hex string to color
	 * @param[out] colorMap
	 *     Array in which color will be saved
	 * @param[in] color
	 *     Hex string
	 * @param[in] offset
	 *     Start position in hex string
	 * @since 1.0
	 */
	void hexToColor(std::vector<unsigned char>& colorMap, const std::string& color, int offset = 0);

	/** Pointer to parent Book object */
	Book* m_book;
};


/**
 * @class X12Book
 * @brief
 *     Excel Workbook data
 */
class X12Book: public X12General {
public:
	/**
	 * @param book
	 *     Pointer to parent Book object
	 * @since 1.0
	 */
	X12Book(Book* book);

	/**
	 * @brief
	 *     Read SST (Shared Strings Table) data
	 * @since 1.0
	 */
	void handleSst();

	/**
	 * @brief
	 *     Read relations data
	 * @since 1.0
	 */
	void handleRelations();

	/**
	 * @brief
	 *     Read properties data
	 * @since 1.0
	 */
	void handleProperties();

	/**
	 * @brief
	 *     Read main stream data
	 * @since 1.0
	 */
	void handleStream();

private:
	/**
	 * @brief
	 *     Read defined names data
	 * @param[in] node
	 *     Node in XML-tree
	 * @since 1.0
	 */
	void handleDefinedNames(const pugi::xml_node& node);

	/**
	 * @brief
	 *     Read sheet data
	 * @param[in] node
	 *     Node in XML-tree
	 * @since 1.0
	 */
	void handleSheet(const pugi::xml_node& node);

	/**
	 * @brief
	 *     Create name map
	 * @since 1.0
	 */
	void createNameMap();

	/** Map relation id to path */
	std::unordered_map<std::string, std::string> m_relIdToPath;
	/** Map relation id to type */
	std::unordered_map<std::string, std::string> m_relIdToType;
	/** Sheet target list */
	std::vector<std::string> m_sheetTargets;
	/** Sheet id list */
	std::vector<int> m_sheetIds;
};


/**
 * @class X12Sheet
 * @brief
 *     Contains data for one worksheet
 */
class X12Sheet: public X12General {
public:
	/**
	 * @param[in] book
	 *     Pointer to parent Book object
	 * @param[in] sheet
	 *     Reference to SHEET object
	 * @since 1.0
	 */
	X12Sheet(Book* book, Sheet& sheet);

	/**
	 * @brief
	 *     Read relations data
	 * @param[in] fileName
	 *     XML data file name
	 * @since 1.0
	 */
	void handleRelations(const std::string& fileName);

	/**
	 * @brief
	 *     Read main stream data
	 * @param[in] fileName
	 *     XML data file name
	 * @since 1.0
	 */
	void handleStream(const std::string& fileName);

	/**
	 * @brief
	 *     Read comments/notes data
	 * @param[in] fileName
	 *     XML data file name
	 * @since 1.0
	 */
	void handleComments(const std::string& fileName);

	/**
	 * @brief
	 *     Get drawing relationship map from `xl/drawings/_rels/drawingN.xml.rels`
	 * @param[in] sheetIndex
	 *     Sheet index
	 * @since 1.1
	 */
	void getDrawingRelationshipMap(int sheetIndex);

	/**
	 * @brief
	 *     Read images data
	 * @param[in] sheetIndex
	 *     Sheet index
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.1
	 */
	void handleImages(int sheetIndex, pugi::xml_node& htmlNode);

	/** Map relation id to path */
	std::unordered_map<std::string, std::string> m_relIdToPath;
	/** Map relation id to type */
	std::unordered_map<std::string, std::string> m_relIdToType;

private:
	/**
	 * @brief
	 *     Read column (COLINFO) data
	 * @param[in] node
	 *     Node in XML-tree
	 * @since 1.0
	 */
	void handleCol(const pugi::xml_node& node);

	/**
	 * @brief
	 *     Read row (Cell + ROWINFO) data
	 * @param[in] node
	 *     Node in XML-tree
	 * @since 1.0
	 */
	void handleRow(const pugi::xml_node& node);

	/**
	 * @brief
	 *     Get sheet dimensions. Example: "A1:Z99" or just "A1"
	 * @param[in] node
	 *     Node in XML-tree
	 * @since 1.0
	 */
	void handleDimensions(const pugi::xml_node& node);

	/**
	 * @brief
	 *     Get merged cells
	 * @param[in] node
	 *     Node in XML-tree
	 * @since 1.0
	 */
	void handleMergedCells(const pugi::xml_node& node);

	/**
	 * @brief
	 *     Get table parts information
	 * @param[in] node
	 *     Node in XML-tree
	 * @since 1.0
	 */
	void handleTableParts(const pugi::xml_node& node);

	/**
	 * @brief
	 *     Convert cell name to row/column indexes
	 * @details
	 *     Example:
	 *     @code `A<row number>` => `0`, `Z` =>`25`, `AA` => `26` @endcode
	 * @param[in] cellName
	 *     Cell name
	 * @param[out] rowIndex
	 *     Row index
	 * @param[out] colIndex
	 *     Column index
	 * @param[in] noCol
	 *     If there was no column marker
	 * @since 1.0
	 */
	void cellNameToIndex(const std::string& cellName, int& rowIndex,int& colIndex,
						 bool noCol = false);

	/**
	 * @brief
	 *     Get image size and update `img` tag
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.1
	 */
	void getImageSize(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) const;

	/** Reference to SHEET object */
	Sheet& m_sheet;
	/** Drawing relationship map */
	std::unordered_map<std::string, std::string> m_drawingRelationshipMap;
	/** Row index */
	int m_rowIndex = -1;
};


/**
 * @class X12Styles
 * @brief
 *     Contains style data
 */
class X12Styles: public X12General {
public:
	/**
	 * @param[in] book
	 *     Pointer to parent Book object
	 * @since 1.0
	 */
	X12Styles(Book* book);

	/**
	 * @brief
	 *     Read theme data
	 * @since 1.0
	 */
	void handleTheme();

	/**
	 * @brief
	 *     Read main stream data
	 * @since 1.0
	 */
	void handleStream();

private:
	/**
	 * @brief
	 *     Read number format record
	 * @param[in] node
	 *     Node in XML-tree
	 * @since 1.0
	 */
	void handleNumFormat(const pugi::xml_node& node);

	/**
	 * @brief
	 *     Read FONT data
	 * @param[in] node
	 *     Node in XML-tree
	 * @since 1.0
	 */
	void handleFont(const pugi::xml_node& node, int fontIndex);

	/**
	 * @brief
	 *     Read BORDER data
	 * @param[in] node
	 *     Node in XML-tree
	 * @since 1.0
	 */
	void handleBorder(const pugi::xml_node& node);

	/**
	 * @brief
	 *     Read BACKGROUND data
	 * @param[in] node
	 *     Node in XML-tree
	 * @since 1.0
	 */
	void handleBackground(const pugi::xml_node& node);

	/**
	 * @brief
	 *     Read XF data
	 * @param[in] node
	 *     Node in XML-tree
	 * @since 1.0
	 */
	void handleXf(const pugi::xml_node& node);

	/**
	 * @brief
	 *     Get color information from node
	 * @param[in] node
	 *     Node in XML-tree
	 * @param[in] color
	 *     Reference to parent XFColor object
	 * @since 1.0
	 */
	void extractColor(const pugi::xml_node& node, XFColor& color);

	/** Used XF records count */
	std::vector<int> m_xfCount = {0, 0};
	/** If formatted string is date */
	std::unordered_map<int, bool> m_isDateFormat;
};

}  // End namespace
