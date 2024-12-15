/**
 * @brief     TXT files into HTML сonverter
 * @package   txt
 * @file      txt.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright adhocore (https://github.com/adhocore/htmlup)
 * @version   1.1
 * @date      01.08.2016 -- 18.10.2017
 */
#pragma once

#include <string>
#include <vector>

#include "fileext/fileext.hpp"

namespace txt {

class Txt: public fileext::FileExtension {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
	Txt(const std::string& fileName);

	/** Destructor */
	virtual ~Txt() = default;

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
};

}  // End namespace
