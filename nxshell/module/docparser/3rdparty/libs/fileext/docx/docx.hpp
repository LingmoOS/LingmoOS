/**
 * @brief     DOCX files into HTML сonverter
 * @package   docx
 * @file      docx.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright PolicyStat (https://github.com/PolicyStat/docx2html)
 * @version   1.1
 * @date      12.07.2016 -- 18.10.2017
 */
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <pugixml.hpp>

#include "fileext/fileext.hpp"
#include "fileext/ooxml/ooxml.hpp"


/**
 * @namespace docx
 * @brief
 *     DOCX files into HTML сonverter
 */
namespace docx {

/**
 * @class Docx
 * @brief
 *     DOCX files into HTML сonverter
 */
class Docx: public fileext::FileExtension, public ooxml::Ooxml {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
    Docx(const std::string& fileName, int maxLen = 0);

	/** Destructor */
	virtual ~Docx() = default;

	/**
	 * @brief
	 *     Convert file to HTML-tree
	 * @param[in] addStyle
	 *     Should read and add styles to HTML-tree
	 * @param[in] extractImages
	 *     True if should extract images
	 * @param[in] mergingMode
	 *     Colspan/rowspan processing mode
	 * @since 1.0
	 */
    int convert(bool addStyle = true, bool extractImages = false, char mergingMode = 0) override;

private:
	/// @name General
	/// @{
	/**
	 * @brief
	 *     Get `word/numbering.xml` content
	 * @details
	 *     Stores how lists should look (unordered, digits, ...).
	 *     Parse it and create map of what each combination should be based on list id and
	 *     level of indentation.
	 * @since 1.0
	 */
	void getNumberingMap();

	/**
	 * @brief
	 *     Get `word/styles.xml` content
	 * @note
	 *     Some things that considered lists are actually supposed
	 *     to be `H` tags (h1, h2, ...). These can be denoted by their styleId.
	 * @since 1.0
	 */
	void getStyleMap();

	/**
	 * @brief
	 *     Get `word/_rels/document.xml.rels` content
	 * @details
	 *     Stores targets to links as well as targets for images.
	 *     Create map based on the relationship id and target.
	 * @since 1.0
	 */
	void getRelationshipMap();

	/**
	 * @brief
	 *     Check if element is header
	 * @param[in] node
	 *     XML-node
	 * @return
	 *     `H` tag if element is header
	 * @since 1.0
	 */
	std::string isHeader(const pugi::xml_node& node) const;

	/**
	 * @brief
	 *     Check if element is natural header
	 * @param[in] node
	 *     XML-node
	 * @return
	 *     `H` tag if element is header
	 * @since 1.0
	 */
	std::string isNaturalHeader(const pugi::xml_node& node) const;

	/**
	 * @brief
	 *     Get numbering id
	 * @details
	 *     numId on `li` tag maps to the numbering map along side ilvl
	 *     to determine what list should look like (unordered, digits, ...).
	 * @param[in] node
	 *     XML-node
	 * @return
	 *     Numbering id
	 * @since 1.0
	 */
	std::string getNumberingId(const pugi::xml_node& node) const;

	/**
	 * @brief
	 *     If element has indentation level
	 * @param[in] node
	 *     XML-node
	 * @return
	 *     True if has indentation level
	 * @since 1.0
	 */
	bool hasIndentationLevel(const pugi::xml_node& node) const;
	/// @}

	/// @name Paragraph
	/// @{
	/**
	 * @brief
	 *     Get paragraph text
	 * @details
	 *     `P` tags are made up of several runs (`r` tags) of text. Takes `p` tag and
	 *     constructs text that should be part of `p` tag.
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
    void getParagraphText(const pugi::xml_node& xmlNode);

	/**
	 * @brief
	 *     Get element text
	 * @details
	 *     It turns out that `r` tags can contain both `t` tags and drawing tags.
	 *     Since we need both, this function will get them in order in which they are found
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
    std::string getElementText(const pugi::xml_node& xmlNode);
	/// @}

	/// @name Hyperlink
	/// @{
	/**
	 * @brief
	 *     Create hyperlink
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
    void buildHyperlink(const pugi::xml_node& xmlNode);
	/// @}

	/// @name Table
	/// @{
	/**
	 * @brief
	 *     Create table with all rows and cells correctly populated
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
    void buildTable(const pugi::xml_node& xmlNode);

	/**
	 * @brief
	 *     Create single tr element, with all tds already populated
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
    void buildTr(const pugi::xml_node& xmlNode);

	/// @name List
	/// @{
	/**
	 * @brief
	 *     Build list structure
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
    void buildList(const pugi::xml_node& xmlNode);

	/**
	 * @brief
	 *     Find consecutive `li` tags that have content that have the same list id
	 * @param[in] node
	 *     XML-node
	 * @param[out] liNodes
	 *     Consecutive `li` tags list
	 * @since 1.0
	 */
	void getListNodes(const pugi::xml_node& node, std::vector<pugi::xml_node>& liNodes) const;

	/**
	 * @brief
	 *     Get indentation level
	 * @details
	 *     ilvl on `li` tag tells the `li` tag at what level of indentation
	 *     this tag. This is used to determine if `li` tag nested or not.
	 * @param[in] node
	 *     XML-node
	 * @return
	 *     Indentation level
	 * @since 1.0
	 */
	int getIndentationLevel(const pugi::xml_node& node) const;

	/**
	 * @brief
	 *     Check if elent is `li`
	 * @details
	 *     Only real distinction between `li` and `p` tags is that `li` has
	 *     attribute `numPr` which holds list id and ilvl (indentation level).
	 * @param[in] node
	 *     XML-node
	 * @return
	 *     If element is `li`
	 * @since 1.0
	 */
	bool isLi(const pugi::xml_node& node) const;

	/**
	 * @brief
	 *     Check if `li` in top level
	 * @details
	 *     If this list is not in the root document (indentation == 0), then it cannot be
	 *     a top level upper roman list.
	 * @param[in] node
	 *     XML-node
	 * @return
	 *     True if list in the root document
	 * @since 1.0
	 */
	bool isTopLevel(const pugi::xml_node& node) const;

	/**
	 * @brief
	 *     Check if `li` is last
	 * @details
	 *     Determine if `li` is last list item for given list.
	 * @param[in] node
	 *     XML-node
	 * @param[in] currentNumId
	 *     Current numId
	 * @return
	 *     True if `li` is last list item
	 * @since 1.0
	 */
	bool isLastLi(const pugi::xml_node& node, const std::string& currentNumId) const;

	/**
	 * @brief
	 *     Build non list content
	 * @param[in] xmlNode
	 *     XML-node
	 * @param[out] htmlNode
	 *     Parent HTML-node
	 * @since 1.0
	 */
    void buildNonListContent(const pugi::xml_node& xmlNode);
	/// @}
    int m_maxLen = 0;
	/** Stores how lists should look (unordered, digits, ...) */
	std::unordered_map<std::string, std::vector<std::string>> m_numberingMap;
	/** Some document style information */
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_styleMap;
	/** Stores targets to links as well as targets for images */
	std::unordered_map<std::string, std::string> m_relationshipMap;
	/** List of visited elements */
	std::vector<pugi::xml_node> m_visitedNodeList;
	/** Stores table border style */
	std::unordered_map<std::string, std::string> m_borderMap;
};

}  // End namespace
