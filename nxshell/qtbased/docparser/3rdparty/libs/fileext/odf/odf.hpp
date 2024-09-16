/**
 * @brief     ODT files into HTML сonverter
 * @package   odt
 * @file      odt.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @version   1.0
 * @date      12.08.2017 -- 18.10.2017
 */
#pragma once

#include <string>
#include <map>
#include <pugixml.hpp>

#include "fileext/fileext.hpp"
#include "fileext/ooxml/ooxml.hpp"


/**
 * @namespace odf
 * @brief
 *     ODF files into HTML сonverter
 */
namespace odf {
/**
 * @class Odf
 * @brief
 *     ODF files into HTML сonverter
 */
class Odf: public fileext::FileExtension, public ooxml::Ooxml {
public:
    typedef void (*CommandHandler)(Odf *, pugi::xml_node &, std::string &);
    typedef std::vector<std::string> svector;
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
    Odf(const std::string& fileName);

	/** Destructor */
    virtual ~Odf() = default;

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
    std::string xmlLocateName(const pugi::xml_node &node);
    std::string parseXmlData(const pugi::xml_node &node);
    bool executeCommand(pugi::xml_node &node, std::string &text);

    static void parseODFXMLTable(Odf *parser, pugi::xml_node &node, std::string &text);
    static void parseODFXMLPara(Odf *parser, pugi::xml_node &node, std::string &text);
    static void parserODFXMLUrl(Odf *parser, pugi::xml_node &node, std::string &text);

private:
    std::map<std::string, CommandHandler> m_cmdHandlers;
};

}  // End namespace
