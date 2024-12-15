/**
 * @brief     RTF files into HTML сonverter
 * @package   rtf
 * @file      formatting.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright lvu (https://github.com/lvu/rtf2html)
 * @date      31.07.2016 -- 18.10.2017
 */
#pragma once

#include <deque>
#include <string>
#include <vector>
#include <pugixml.hpp>


namespace rtf {

/**
 * @class Color
 * @brief
 *     Color information for font, background, borders
 */
class Color {
public:
	Color() = default;

	/** Equality operators */
	bool operator==(const Color& obj) const;
	bool operator!=(const Color& obj) const;

	/** Assign operator */
	Color& operator=(const Color& obj);

	/**
	 * @brief
	 *     Convert color to CSS string
	 * @return
	 *     CSS color string
	 * @since 1.0
	 */
	std::string toString() const;

	/** Color RED component */
	int m_red   = -1;
	/** Color GREEN component */
	int m_green = -1;
	/** Color BLUE component */
	int m_blue  = -1;
};


/**
 * @class Font
 * @brief
 *     Font information
 */
class Font {
public:
	Font() = default;

	/** Equality operators */
	bool operator==(const Font& obj) const;
	bool operator!=(const Font& obj) const;

	/** Assign operator */
	Font& operator=(const Font& obj);

	/** Font family */
	enum fontFamily {
		FF_NONE,       ///< None (unknown or don't care)
		FF_SERIF,      ///< Roman (variable width, serifed)
		FF_SANS_SERIF, ///< Swiss (variable width, sans-serifed)
		FF_MONOSPACE,  ///< Modern (fixed width, serifed or sans-serifed)
		FF_CURSIVE,    ///< Script (cursive)
		FF_FANTASY     ///< Decorative (specialised, e.g. Old English, Fraktur)
	};
	fontFamily m_family = FF_NONE;
	/** Font name */
	std::string m_name;
	/**
	 * Character set values
	 * Value | Description
	 * :---: | -----------
	 *  0    | ANSI Latin
	 *  1    | System default
	 *  2    | Symbol
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
	int m_charset = 0;
};


/**
 * @class Formatting
 * @brief
 *     Text formatting information
 */
class Formatting {
public:
	Formatting() = default;

	/** Equality operators */
	bool operator==(const Formatting& obj) const;
	bool operator!=(const Formatting& obj) const;

	/** Assign operator */
	Formatting& operator=(const Formatting& obj);

	/**
	 * If text is bold
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
	 * If text is underlined
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
	 * If text is subscript
	 * Value | Description
	 * :---: | -----------
	 * False | Normal text
	 * True  | Characters are subscript
	 */
	bool m_isSub = false;
	/**
	 * If text is superscript
	 * Value | Description
	 * :---: | -----------
	 * False | Normal text
	 * True  | Characters superscript
	 */
	bool m_isSup = false;
	/** Font size */
	int m_fontSize = 0;
	/** Font object (name, family) */
	Font m_font;
	/** Font color */
	Color m_fontColor;
	/** Background color */
	Color m_backgroundColor;
	/** Paragraph horizontal alignment */
	std::string m_horizontalAlign;
	/** Paragraph vertical alignment */
	std::string m_verticalAlign;
	/** If paragraph in table */
	bool m_parInTable = false;
	/** List level */
	int m_listLevel = 0;
};


/**
 * @class HtmlText
 * @brief
 *     Parts of HTML text before adding to tree
 */
class HtmlText {
public:
	HtmlText(const Formatting& format, bool addStyle);

	/**
	 * @brief
	 *     Add text and insert style tags
	 * @tparam T
	 *     Input string type
	 * @param[in] str
	 *     Input string
	 * @since 1.0
	 */
	template <typename T>
	void add(T str);

	/**
	 * @brief
	 *     Copy nodes to main tree
	 * @param[in,out] node
	 *     HTML-node
	 * @since 1.0
	 */
	void addSubtree(pugi::xml_node& node) const;

	/**
	 * @brief
	 *     Clear text and element stack
	 * @since 1.0
	 */
	void clearText();

	/**
	 * @brief
	 *     Close element stack
	 * @since 1.0
	 */
	void close();

private:
	/**
	 * @brief
	 *     Add style tags
	 * @param[out] node
	 *     HTML-node
	 * @param[in] styleName
	 *     Style tag name
	 * @param[in] oldStyle
	 *     Old style
	 * @param[in] newStyle
	 *     New style
	 * @since 1.0
	 */
	void addStyle(pugi::xml_node& node, const std::string& styleName, bool oldStyle, bool newStyle);

	/** Text current formatting information */
	const Formatting& m_format;
	/** Formatting stack */
	std::deque<Formatting> m_formatStack;
	/** HTML text */
	std::string m_text;
	/** HTML-tree */
	pugi::xml_document m_tree;
	/** HTML-nodes stack */
	std::vector<pugi::xml_node> m_nodeList;
	/** Should read and add styles to HTML-tree */
	bool m_addStyle;
};


template <typename T>
void HtmlText::add(T str) {
	if (!m_addStyle) {
		m_text += str;
		return;
	}

	Formatting lastFormat;
	if (!m_formatStack.empty()) {
		int count = 0;
		std::deque<Formatting>::reverse_iterator it;
		for (it = m_formatStack.rbegin(); it != m_formatStack.rend(); ++it) {
			if (*it == m_format)
				break;
			++count;
		}

		if (count == 0) {
			m_text += str;
			return;
		}
		if (it != m_formatStack.rend()) {
			while (count--) {
				close();
				m_nodeList.pop_back();
				m_formatStack.pop_back();
			}
			m_text += str;
			return;
		}
		lastFormat = m_formatStack.back();
	}

	// Get parent node
	auto node = m_nodeList.back();
	std::string nodeName = node.name();
	if (nodeName.empty()) {
		node = node.append_child("parent");
		m_nodeList.emplace_back(node);
		nodeName = "parent";
	}

	std::unordered_map<std::string, std::string> parentStyleMap;
	std::unordered_map<std::string, std::string> elementStyleMap;
	// Get parent style
	if (!m_format.m_horizontalAlign.empty())
		parentStyleMap["text-align"] = m_format.m_horizontalAlign;
	if (!m_format.m_verticalAlign.empty())
		parentStyleMap["vertical-align"] = m_format.m_verticalAlign;
	// Get element style
	if (!m_format.m_font.m_name.empty())
		elementStyleMap["font-family"] = "'"+ m_format.m_font.m_name +"'";
	if (m_format.m_fontSize)
		elementStyleMap["font-size"] = std::to_string(m_format.m_fontSize) +"px";
	if (m_format.m_fontColor.m_red != -1)
		elementStyleMap["color"] = m_format.m_fontColor.toString();
	if (m_format.m_backgroundColor.m_red != -1)
		elementStyleMap["background"] = m_format.m_backgroundColor.toString();

	// Set parent style
	std::string style;
	for (const auto& sm : parentStyleMap)
		style += sm.first + ":" + sm.second + "; ";
	if (!style.empty() && nodeName == "parent")
		node.append_attribute("style") = style.c_str();

	// Set element style
	style.clear();
	for (const auto& sm : elementStyleMap)
		style += sm.first + ":" + sm.second + "; ";
	if (m_format.m_fontSize        == lastFormat.m_fontSize  &&
		m_format.m_font            == lastFormat.m_font      &&
		m_format.m_fontColor       == lastFormat.m_fontColor &&
		m_format.m_backgroundColor == lastFormat.m_backgroundColor &&
		m_format.m_horizontalAlign == lastFormat.m_horizontalAlign &&
		m_format.m_verticalAlign   == lastFormat.m_verticalAlign
	) {
		node = m_nodeList.back();
	}
	else if (!style.empty()) {
		node = node.append_child("span");
		node.append_attribute("style") = style.c_str();

		m_nodeList.emplace_back(node);
	}


	addStyle(node, "b",   m_format.m_isBold,       lastFormat.m_isBold);
	addStyle(node, "i",   m_format.m_isItalic,     lastFormat.m_isItalic);
	addStyle(node, "u",   m_format.m_isUnderlined, lastFormat.m_isUnderlined);
	addStyle(node, "s",   m_format.m_isStruckOut,  lastFormat.m_isStruckOut);
	addStyle(node, "sub", m_format.m_isSub,        lastFormat.m_isSub);
	addStyle(node, "sup", m_format.m_isSup,        lastFormat.m_isSup);

	m_formatStack.emplace_back(m_format);
	m_text += str;
}

}  // End namespace
