/**
 * @brief     RTF files into HTML сonverter
 * @package   rtf
 * @file      formatting.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright lvu (https://github.com/lvu/rtf2html)
 * @date      31.07.2016 -- 18.10.2017
 */
#include <unordered_map>

#include "tools.hpp"

#include "formatting.hpp"


namespace rtf {

// Color
bool Color::operator==(const Color& obj) const {
	return (m_red   == obj.m_red   &&
			m_green == obj.m_green &&
			m_blue  == obj.m_blue);
}

bool Color::operator!=(const Color& obj) const {
	return !(*this == obj);
}

Color& Color::operator=(const Color& obj) {
	m_red   = obj.m_red;
	m_green = obj.m_green;
	m_blue  = obj.m_blue;
	return *this;
}

std::string Color::toString() const {
	return "rgb("+ std::to_string(m_red) +", "+
			std::to_string(m_green) +", "+ std::to_string(m_blue) +")";
}


// Font
bool Font::operator==(const Font& obj) const {
	return (m_family == obj.m_family &&
			m_name   == obj.m_name);
}

bool Font::operator!=(const Font& obj) const {
	return !(*this == obj);
}

Font& Font::operator=(const Font& obj) {
	m_family  = obj.m_family;
	m_name    = obj.m_name;
	m_charset = obj.m_charset;
	return *this;
}


// Formatting
bool Formatting::operator==(const Formatting& obj) const {
	return (m_isBold          == obj.m_isBold       &&
			m_isItalic        == obj.m_isItalic     &&
			m_isUnderlined    == obj.m_isUnderlined &&
			m_isStruckOut     == obj.m_isStruckOut  &&
			//m_isOutlined    == obj.m_isOutlined   &&
			m_isSub           == obj.m_isSub        &&
			m_isSup           == obj.m_isSup        &&
			m_fontSize        == obj.m_fontSize     &&
			m_font            == obj.m_font         &&
			m_fontColor       == obj.m_fontColor    &&
			m_backgroundColor == obj.m_backgroundColor &&
			m_horizontalAlign == obj.m_horizontalAlign &&
			m_verticalAlign   == obj.m_verticalAlign);
}

bool Formatting::operator!=(const Formatting& obj) const {
	return !(*this == obj);
}

Formatting& Formatting::operator=(const Formatting& obj) {
	m_isBold          = obj.m_isBold;
	m_isItalic        = obj.m_isItalic;
	m_isUnderlined    = obj.m_isUnderlined;
	m_isStruckOut     = obj.m_isStruckOut;
	//m_isOutlined    = obj.m_isOutlined;
	m_isSub           = obj.m_isSub;
	m_isSup           = obj.m_isSup;
	m_parInTable      = obj.m_parInTable;
	m_listLevel       = obj.m_listLevel;
	m_fontSize        = obj.m_fontSize;
	m_font            = obj.m_font;
	m_fontColor       = obj.m_fontColor;
	m_backgroundColor = obj.m_backgroundColor;
	m_horizontalAlign = obj.m_horizontalAlign;
	m_verticalAlign   = obj.m_verticalAlign;
	return *this;
}


// HtmlText public:
HtmlText::HtmlText(const Formatting& format, bool addStyle)
: m_format(format), m_addStyle(addStyle) {
	m_nodeList.emplace_back(m_tree);
}

void HtmlText::addSubtree(pugi::xml_node& node) const {
	std::string style;
	std::string nodeName = node.name();
	auto parentNode = m_nodeList[0].child("parent");

	if (parentNode.attribute("style"))
		style += parentNode.attribute("style").value();
	if (tools::xmlChildrenCount(parentNode) == 1) {
		style += parentNode.child("span").attribute("style").value();
		parentNode = parentNode.child("span");
	}

	if (nodeName.empty())
		parentNode = m_nodeList[0];
	else if (!style.empty())
		node.append_attribute("style") = style.c_str();

	for (const auto& child : parentNode)
		node.append_copy(child);
}

void HtmlText::clearText() {
	m_text.clear();
	m_formatStack.clear();

	m_nodeList.clear();
	m_tree.reset();
	m_nodeList.emplace_back(m_tree);
}

void HtmlText::close() {
	if (!m_text.empty()) {
		m_nodeList.back().append_child(pugi::node_pcdata).set_value(m_text.c_str());
		m_text.clear();
	}
}

// HtmlText private:
void HtmlText::addStyle(pugi::xml_node& node, const std::string& styleName,
						bool oldStyle, bool newStyle)
{
	if (oldStyle != newStyle) {
		close();
		if (oldStyle) {
			node = node.append_child(styleName.c_str());
			m_nodeList.emplace_back(node);
		}
		else {
			m_nodeList.pop_back();
			node = m_nodeList.back();
		}
	}
}

}  // End namespace
