/**
 * @brief     DOCX files into HTML сonverter
 * @package   docx
 * @file      docx.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright PolicyStat (https://github.com/PolicyStat/docx2html)
 * @date      12.07.2016 -- 18.10.2017
 */
#include <algorithm>
#include <fstream>
#include <string.h>

#include "docx.hpp"


namespace docx {

/** Header tags map */
const std::unordered_map<std::string, std::string> HEADER_LIST {
	{"heading 1",  "h1"},
	{"heading 2",  "h2"},
	{"heading 3",  "h3"},
	{"heading 4",  "h4"},
	{"heading 5",  "h5"},
	{"heading 6",  "h6"},
	{"heading 7",  "h6"},
	{"heading 8",  "h6"},
	{"heading 9",  "h6"},
	{"heading 10", "h6"}
};
/** List type map */
const std::unordered_map<std::string, std::string> LIST_TYPE {
	{"decimal",      "decimal"},
	{"decimalZero",  "decimal-leading-zero"},
	{"upperRoman",   "upper-roman"},
	{"lowerRoman",   "lower-roman"},
	{"upperLetter",  "upper-alpha"},
	{"lowerLetter",  "lower-alpha"},
	{"ordinal",      "decimal"},
	{"cardinalText", "decimal"},
	{"ordinalText",  "decimal"}
};
/** Only these tags contain text that we care about (e.g. don't care about delete tags) */
const std::vector<std::string> CONTENT_TAGS {
	"w:r",
	"w:hyperlink",
	"w:ins",
	"w:smartTag"
};
/** Horizontal align map */
const std::unordered_map<std::string, std::string> HORZ_ALIGN {
	{"left",   "left"},
	{"center", "center"},
	{"right",  "right"},
	{"both",   "justify"},
};
/** Vertical align map */
const std::unordered_map<std::string, std::string> VERT_ALIGN {
	{"top",         "top"},
	{"center",      "middle"},
	{"bottom",      "bottom"},
	{"justify",     "middle"},
	{"distributed", "middle"}
};
/** Border type map */
const std::unordered_map<std::string, std::string> BORDER_TYPE {
	{"",                 "none"},
	{"single",           "solid"},
	{"thin",             "solid"},
	{"medium",           "solid"},
	{"dashed",           "dashed"},
	{"dotted",           "dotted"},
	{"thick",            "solid"},
	{"double",           "double"},
	{"hair",             "dotted"},
	{"mediumDashed",     "dashed"},
	{"dashDot",          "dashed"},
	{"mediumDashDot",    "dashed"},
	{"dashDotDot",       "dotted"},
	{"mediumDashDotDot", "dotted"},
	{"slantDashDot",     "dashed"}
};
/** Border size map */
const std::unordered_map<std::string, int> BORDER_SIZE {
	{"",                 1},
	{"single",           1},
	{"thin",             1},
	{"medium",           2},
	{"dashed",           1},
	{"dotted",           1},
	{"thick",            3},
	{"double",           1},
	{"hair",             1},
	{"mediumDashed",     2},
	{"dashDot",          1},
	{"mediumDashDot",    2},
	{"dashDotDot",       1},
	{"mediumDashDotDot", 2},
	{"slantDashDot",     3}
};
/** Border position list */
const std::vector<std::string> BORDER_LIST {
	"top",
	"left",
	"right",
	"bottom"
};

// public:
Docx::Docx(const std::string& fileName, int maxLen)
    : FileExtension(fileName)
    , m_maxLen(maxLen) {}

int Docx::convert(bool addStyle, bool extractImages, char mergingMode) {
	getNumberingMap();
	getStyleMap();
	getRelationshipMap();

	pugi::xml_document tree;
	Ooxml::extractFile(m_fileName, "word/document.xml", tree);

	for (const auto& node : tree.child("w:document").child("w:body")) {
		// Lists are handled specific => could double visit certain elements. Keep track
		// of visited elements and skip any that have been visited already
		std::string nodeName = node.name();
		if (nodeName == "w:sectPr" ||
			find(m_visitedNodeList.begin(), m_visitedNodeList.end(), node) !=
				m_visitedNodeList.end()
		)
			continue;
		std::string headerValue = isHeader(node);
		if (!headerValue.empty()) {
            getParagraphText(node);
		}
		else if (nodeName == "w:p") {
			// Certain `p` tags denoted as `Title` tags. Strip out them
			auto pStyle = node.select_node(".//w:pStyle");
			std::string isTitle = pStyle.node().attribute("w:val").value();
			if (isTitle == "Title")
				continue;
			// Parse out the needed info from node
			if (isLi(node)) {
                buildList(node);
			}
			// Handle generic `p` tag
			else {
                getParagraphText(node);
			}
		}
		else if (nodeName == "w:tbl") {
            buildTable(node);
			continue;
		}
		m_visitedNodeList.emplace_back(node);
	}
    return 0;
}


// private:
void Docx::getNumberingMap() {
	pugi::xml_document tree;
	Ooxml::extractFile(m_fileName, "word/numbering.xml", tree);

	std::unordered_map<std::string, std::string> numIdList;
	// Each list type is assigned an abstractNumber that defines how lists should look
	for (const auto& node : tree.select_nodes("//w:num")) {
		auto nd = node.node();
		std::string abstractNumber = nd.child("w:abstractNumId").attribute("w:val").value();
		numIdList[abstractNumber] = nd.attribute("w:numId").value();
	}

	for (const auto& node : tree.select_nodes("//w:abstractNum")) {
		auto nd = node.node();
		std::string abstractNumId = nd.attribute("w:abstractNumId").value();
		// If we find abstractNumber that is not being used in document => ignore it
		if (numIdList.find(abstractNumId) == numIdList.end())
			continue;
		// Get level of abstract number
		for (const auto& child : nd.children("w:lvl")) {
			// Based on list type and ilvl (indentation level) store needed style
			m_numberingMap[numIdList[abstractNumId]].emplace_back(
				child.child("w:numFmt").attribute("w:val").value()
			);
		}
	}
}

void Docx::getStyleMap() {
	pugi::xml_document tree;
	Ooxml::extractFile(m_fileName, "word/styles.xml", tree);

	// This is a partial document and actual H1 is the document title, which
	// will be displayed elsewhere
	for (const auto& node : tree.select_nodes("//w:style")) {
		auto nd = node.node();
		std::unordered_map<std::string, std::string> style {
			{"header",    ""},
			{"font_size", ""},
			{"based_on",  ""}
		};
		// Get header info
		auto name = nd.child("w:name");
		if (!name)
			continue;
		std::string value = name.attribute("w:val").value();
		transform(value.begin(), value.end(), value.begin(), ::tolower);
		if (HEADER_LIST.find(value) != HEADER_LIST.end())
			style["header"] = HEADER_LIST.at(value);

		// Get size info
		auto rPr = nd.child("w:rPr");
		if (!rPr)
			continue;
		auto size = rPr.child("w:sz");
		if (size)
			style["font_size"] = size.attribute("w:val").value();

		// Get based on info
		auto basedOn = nd.child("w:basedOn");
		if (basedOn)
			style["based_on"] = basedOn.attribute("w:val").value();

		std::string styleId = nd.attribute("w:styleId").value();
		m_styleMap[styleId] = style;
	}
}

void Docx::getRelationshipMap() {
	pugi::xml_document tree;
	Ooxml::extractFile(m_fileName, "word/_rels/document.xml.rels", tree);

	for (const auto& node : tree.child("Relationships")) {
		auto id = node.attribute("Id").value();
		if (id)
			m_relationshipMap[id] = node.attribute("Target").value();
	}
}


std::string Docx::isHeader(const pugi::xml_node& node) const {
	if (isTopLevel(node))
		return "h2";
	std::string el_isNaturalHeader = isNaturalHeader(node);
	if (!el_isNaturalHeader.empty())
		return el_isNaturalHeader;
	if (hasIndentationLevel(node))
		return "";
	if (strcmp(node.name(), "w:tbl") == 0)
		return "";
	return "";
}

std::string Docx::isNaturalHeader(const pugi::xml_node& node) const {
	auto pPr = node.child("w:pPr");
	if (!pPr)
		return "";
	auto pStyle = pPr.child("w:pStyle");
	if (!pStyle)
		return "";
	std::string styleId = pStyle.attribute("w:val").value();
    if (m_styleMap.find(styleId) != m_styleMap.end() && m_styleMap.at(styleId).at("header") != "false")
		return m_styleMap.at(styleId).at("header");
	return "";
}

std::string Docx::getNumberingId(const pugi::xml_node& node) const {
	auto numId = node.select_node(".//w:numId");
	return numId.node().attribute("w:val").value();
}

bool Docx::hasIndentationLevel(const pugi::xml_node& node) const {
	auto ilvl = node.select_nodes(".//w:numPr/w:ilvl");
	return (ilvl.end() - ilvl.begin() != 0);
}

// Paragraph
void Docx::getParagraphText(const pugi::xml_node& xmlNode) {
    if (m_maxLen > 0 && m_text.size() >= m_maxLen)
        return;

    std::string text;
	for (const auto& child : xmlNode) {
		std::string childName = child.name();
		if (find(CONTENT_TAGS.begin(), CONTENT_TAGS.end(), childName) != CONTENT_TAGS.end()) {
			// Hyperlinks and insert tags need to be handled differently than r and smart tags
			if (childName == "w:r")
                text += getElementText(child);
			else if (childName == "w:hyperlink")
                buildHyperlink(child);
			else
                getParagraphText(child);
		}
	}

    m_text += text + '\n';
}

std::string Docx::getElementText(const pugi::xml_node& xmlNode) {
    std::string elementText;
	for (const auto& child : xmlNode) {
		std::string childName = child.name();
		if (childName == "w:t") {
			// Generate string data that for this particular `t` tag
			std::string text = child.child_value();
			if (text.empty())
                return elementText;

			// Wrap text with any modifiers it might have (bold/italics/underlined)
            elementText += text;
		}
	}

    return elementText;
}

// Hyperlink
void Docx::buildHyperlink(const pugi::xml_node& xmlNode) {
	// If we have hyperlink we need to get relationship id
	auto hyperlinkId = xmlNode.attribute("r:id").value();

	// Once we have hyperlinkId then we need to replace hyperlink tag with its child run tags
    if (m_relationshipMap.find(hyperlinkId) != m_relationshipMap.end()) {
        getParagraphText(xmlNode);
	}
}

// Table
void Docx::buildTable(const pugi::xml_node& xmlNode) {
	for (const auto& child : xmlNode.children("w:tr")) {
        buildTr(child);
	}
}

void Docx::buildTr(const pugi::xml_node& xmlNode) {
	for (const auto& child : xmlNode.children("w:tc")) {
		if (find(m_visitedNodeList.begin(), m_visitedNodeList.end(), child) != m_visitedNodeList.end())
			continue;
		// Keep track of m_visitedNodeList
		m_visitedNodeList.emplace_back(xmlNode);

		// vMerge is what docx uses to denote that table cell is part of rowspan. First
		// cell has vMerge - start of rowspan, and vMerge will be denoted with `restart`.
		// If it is anything other than restart then it is continuation of another rowspan
		auto vMerge = child.child("w:tcPr").child("w:vMerge");
		std::string vMergeVal = vMerge.attribute("w:val").value();
        if (vMerge && vMergeVal != "restart")
			continue;

		// Loop through each and build list of all content
		for (const auto& tdContent : child) {
			// Since we are doing look-a-heads in this loop we need to check already visited nodes
			if (find(m_visitedNodeList.begin(), m_visitedNodeList.end(), tdContent) !=
                m_visitedNodeList.end())
				continue;

			std::string tdContentName = tdContent.name();
			// Check to see if it is list or regular paragraph
			// If it is a list, create list and update m_visitedNodeList
			if (isLi(tdContent)) {
                buildList(tdContent);
            } else if (tdContentName == "w:tbl") {
                buildTable(tdContent);
			}
			// Do nothing
			else if (tdContentName == "w:tcPr") {
				m_visitedNodeList.push_back(tdContent);
				continue;
			}
			else {
                getParagraphText(tdContent);
			}
		}
	}
}

// List
void Docx::buildList(const pugi::xml_node& xmlNode) {
	// Need to keep track of current indentation level
	int currentIndentationLevel = -1;
	// Need to keep track of current list id
	std::string currentListId = "-1";
	// Need to keep track of list that new `li` tags should be added too
	pugi::xml_node node;

	// Get consecutive `li` tags that have content
	std::vector<pugi::xml_node> liNodes;
	getListNodes(xmlNode, liNodes);

	for (const auto& li : liNodes) {
		if (!isLi(li)) {
			// Get content and visited nodes
            buildNonListContent(li);
			m_visitedNodeList.emplace_back(li);
			continue;
		}

		int ilvl = getIndentationLevel(li);
		std::string numId = getNumberingId(li);
        if (m_numberingMap.find(numId) == m_numberingMap.end())
            continue;

        const auto &list = m_numberingMap[numId];
        if (ilvl + 1 > list.size())
            continue;

        std::string listType = list[ilvl];
		if (listType.empty())
			listType = "decimal";

		// If ilvl is greater than current indentation level or list id is changing then we have
		// first li tag in nested list. Need to create new list object and update all variables
		if (ilvl > currentIndentationLevel || numId != currentListId) {
			currentIndentationLevel = ilvl;
			currentListId           = numId;
		}

		// Add li element to tree
        getParagraphText(li);

		m_visitedNodeList.push_back(li);
	}
}

void Docx::getListNodes(const pugi::xml_node& node, std::vector<pugi::xml_node>& liNodes) const {
	liNodes.emplace_back(node);
	std::string currentNumId  = getNumberingId(node);
	int startIndentationLevel = getIndentationLevel(node);
	for (auto li = node; li; li = li.next_sibling()) {
		if (!li.child_value())
			continue;

		// Stop lists if come across list item that should be heading
		if (isTopLevel(li))
			break;
		bool isListItem = isLi(li);
		if (isListItem && (startIndentationLevel > getIndentationLevel(li)))
			break;

		std::string numId = getNumberingId(li);
		// Not `p` tag or list item
		if (numId.empty() || numId == "-1") {
			liNodes.emplace_back(li);
			continue;
		}
		// If list id of next tag is different that previous that means new list (not nested)
		if (currentNumId != numId) {
			break;
		}
		if (isListItem && isLastLi(li, currentNumId)) {
			liNodes.emplace_back(li);
			break;
		}
		liNodes.emplace_back(li);
	}
}

int Docx::getIndentationLevel(const pugi::xml_node& node) const {
	auto ilvl = node.select_node(".//w:ilvl");
	if (!ilvl)
		return -1;
	return ilvl.node().attribute("w:val").as_int();
}

bool Docx::isLi(const pugi::xml_node& node) const {
	if (!isHeader(node).empty())
		return false;
	return hasIndentationLevel(node);
}

bool Docx::isTopLevel(const pugi::xml_node& node) const {
	int ilvl = getIndentationLevel(node);
	if (ilvl != 0)
		return false;

    auto numId = getNumberingId(node);
    if (m_numberingMap.find(numId) == m_numberingMap.end())
        return false;

    return (m_numberingMap.at(numId)[ilvl] == "upperRoman");
}

bool Docx::isLastLi(const pugi::xml_node& node, const std::string& currentNumId) const {
	for (auto li = node; li; li = li.next_sibling()) {
		if (!isLi(li))
			continue;

		if (currentNumId != getNumberingId(li))
			return true;
		// If here, we have found another list item in current list, so `li` is not last
		return false;
	}
	// If we run out of element this must be last list item
	return true;
}

void Docx::buildNonListContent(const pugi::xml_node& xmlNode) {
	std::string nodeName = xmlNode.name();
	if (nodeName == "w:tbl") {
        buildTable(xmlNode);
    } else if (nodeName == "w:p") {
        getParagraphText(xmlNode);
	}
}

}  // End namespace
