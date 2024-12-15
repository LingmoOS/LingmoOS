/**
 * @brief     Excel files (xls/xlsx) into HTML сonverter
 * @package   excel
 * @file      excel.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright python-excel (https://github.com/python-excel/xlrd)
 * @version   1.1.1
 * @date      02.12.2016 -- 18.10.2017
 */
#pragma once

#include <string>
#include <vector>

#include "fileext/fileext.hpp"


/**
 * @namespace excel
 * @brief
 *     Excel files (xls/xlsx) into HTML сonverter
 */
namespace excel {

/**
 * @class Excel
 * @brief
 *     Excel files (xls/xlsx) into HTML сonverter
 */
class Excel: public fileext::FileExtension {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @param[in] extension
	 *     File extension
	 * @since 1.0
	 */
	Excel(const std::string& fileName, const std::string& extension);

	/** Destructor */
	virtual ~Excel() = default;

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
	/** Input file extension (xls/xlsx) */
	const std::string m_extension;
};

}  // End namespace
