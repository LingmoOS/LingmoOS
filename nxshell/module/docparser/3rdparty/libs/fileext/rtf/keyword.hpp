/**
 * @brief     RTF files into HTML сonverter
 * @package   rtf
 * @file      keyword.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright lvu (https://github.com/lvu/rtf2html)
 * @date      31.07.2016 -- 18.10.2017
 */
#pragma once

#include <string>


namespace rtf {

/**
 * @class Keyword
 * @brief
 *     Keyword information
 */
class Keyword {
public:
	/**
	 * @details
	 *     Iterator must point after backslash starting the keyword. After construction,
	 *     iterator points at char following the keyword.
	 * @param[in] it
	 *     Begining of keyword iterator
	 * @since 1.0
	 */
	Keyword(std::string::iterator& it);

	/** Keyword name */
	std::string m_name;
	/** If char is control */
	bool m_isControlChar;
	/** Control char value */
	char m_controlChar;
	/** Parameter value */
	int m_parameter;
};

}  // End namespace