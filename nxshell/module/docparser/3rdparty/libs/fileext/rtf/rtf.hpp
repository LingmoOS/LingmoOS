/**
 * @brief     RTF files into HTML сonverter
 * @package   rtf
 * @file      rtf.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright lvu (https://github.com/lvu/rtf2html)
 * @version   1.1
 * @date      31.07.2016 -- 18.10.2017
 */
#pragma once

#include <string>
#include <vector>
#include <pugixml.hpp>

#include "fileext/fileext.hpp"


/**
 * @namespace rtf
 * @brief
 *     RTF files into HTML сonverter
 */
namespace rtf {

/**
 * @class Rtf
 * @brief
 *     RTF files into HTML сonverter
 */
class Rtf: public fileext::FileExtension {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
	Rtf(const std::string& fileName);

	/** Destructor */
	virtual ~Rtf() = default;

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
	/**
	 * @brief
	 *     Skip rtf groups
	 * @param[in] it
	 *     Begining of group iterator
	 * @since 1.0
	 */
	void skipGroup(std::string::iterator& it) const;

	/**
	 * @brief
	 *     Convert hex code to text
	 * @param[in] it
	 *     Begining of code iterator
	 * @return
	 *     Decoded string
	 * @since 1.0
	 */
	std::string codeToText(std::string::iterator& it);

	/** List of HTML-nodes */
	std::vector<pugi::xml_node> m_nodeList;
	/** if iterator in list */
	bool m_isUl = false;
	/** if iterator in bullet list */
	bool m_inBullet = false;
};

}  // End namespace
