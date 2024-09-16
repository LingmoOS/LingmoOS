/**
 * @brief   CSV files into HTML сonverter
 * @package csv
 * @file    csv.hpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @version 1.0
 * @date    03.04.2017 -- 18.10.2017
 */
#pragma once

#include <fstream>
#include <string>

#include "fileext/fileext.hpp"


/**
 * @namespace csv
 * @brief
 *     CSV files into HTML сonverter
 */
namespace csv {

/**
 * @class Csv
 * @brief
 *     CSV files into HTML сonverter
 */
class Csv: public fileext::FileExtension {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @param[in] delimiter
	 *     Column delimiter
	 * @param[in] newline
	 *     Row delimiter
	 * @param[in] quote
	 *     Quote symbol
	 * @since 1.0
	 */
	Csv(const std::string& fileName, const char& delimiter = ',',
		const char& newline = '\n', const char& quote = '"');

	/** Destructor */
	virtual ~Csv() = default;

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
	 *     Automatically detect delimiter
	 * @param[in] ifstream
	 *     File stream
	 * @since 1.0
	 */
	void detectDelimiter(std::ifstream& file);

	/**
	 * @brief
	 *     Clear cell
	 * @param[in] cell
	 *     Cell value
	 * @return
	 *     Cleared cell value
	 * @since 1.0
	 */
	std::string clearCell(std::string& cell) const;

	/** Column delimiter */
	char m_delimiter;
	/** Row delimiter */
	char m_newline;
	/** Quote symbol */
	char m_quote;
};

}  // End namespace
