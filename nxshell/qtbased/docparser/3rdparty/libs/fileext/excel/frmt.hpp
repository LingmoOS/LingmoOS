/**
 * @brief     Excel files (xls/xlsx) into HTML сonverter
 * @package   excel
 * @file      frmt.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright python-excel (https://github.com/python-excel/xlrd)
 * @date      02.12.2016 -- 18.10.2017
 */
#pragma once

#include <string>


namespace excel {

/**
 * @class Format
 * @brief
 *     Number format information from FORMAT record
 */
class Format {
public:
	Format() = default;

	/**
	 * @param[in] formatKey
	 *     Key into @ref Book::m_formatMap
	 * @param[in] type
	 *     String type
	 * @param[in] formatString
	 *     Format string
	 * @since 1.0
	 */
	Format(unsigned short formatKey, unsigned char type, std::string formatString);

	/** Key into @ref Book::m_formatMap */
	unsigned short m_formatKey = 0;
	/**
	 * Classification that has been inferred from format string. Currently, this is used
	 * only to distinguish between numbers and dates
	 * Value | Description
	 * :---: | -----------
	 *   0   | FUN (Unknown)
	 *   1   | FDT (Date)
	 *   2   | FNU (Number)
	 *   3   | FGE (General)
	 *   4   | FTX (Text)
	 */
	unsigned char m_type = FUN;
	/** Format string */
	std::string m_formatString;
};

}  // End namespace