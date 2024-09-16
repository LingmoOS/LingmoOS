/**
 * @brief     PDF files into HTML сonverter
 * @package   pdf
 * @file      pdf.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright Alex Rembish (https://github.com/rembish/TextAtAnyCost)
 * @version   1.0
 * @date      06.08.2017 -- 18.10.2017
 */
#pragma once

#include <string>
#include "fileext/fileext.hpp"


/**
 * @namespace pdf
 * @brief
 *     PDF files into HTML сonverter
 */
namespace pdf {

/**
 * @class Pdf
 * @brief
 *     PDF files into HTML сonverter
 */
class Pdf: public fileext::FileExtension {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
	Pdf(const std::string& fileName);

	/** Destructor */
	virtual ~Pdf() = default;

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
