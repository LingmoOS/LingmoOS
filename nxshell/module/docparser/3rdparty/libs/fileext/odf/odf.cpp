/**
 * @brief     ODG files into HTML converter
 * @package   odg
 * @file      odg.cpp
 * @author    dmryutov (hujianzhong@gmail.com)
 * @date     2021-09-14
 */
#include <algorithm>
#include <fstream>
#include <string.h>

#include "odf.hpp"

#include <iostream>

namespace odf {

Odf::Odf(const std::string& fileName)
    : FileExtension(fileName)
{
    m_cmdHandlers["table:table"] = &Odf::parseODFXMLTable;
    m_cmdHandlers["text:p"] = &Odf::parseODFXMLPara;
    m_cmdHandlers["text:a"] = &Odf::parserODFXMLUrl;
}

int  Odf::convert(bool addStyle, bool extractImages, char mergingMode)
{
	pugi::xml_document tree;
	Ooxml::extractFile(m_fileName, "content.xml", tree);
    m_text = parseXmlData(tree);

    return 0;
}

std::string Odf::xmlLocateName(const pugi::xml_node &node)
{
    const char* p = strchr(node.name(), ':');
    return p ? p + 1 : node.name();
}

void Odf::parseODFXMLTable(Odf *parser, pugi::xml_node &node, std::string &text)
{
    std::string cellStr;
    for (const auto &rowNode : node.children("table:table-row")) {
        cellStr.clear();
        for (auto &cellNode : rowNode.children("table:table-cell")) {
            cellStr += parser->parseXmlData(cellNode) + '\n';
        }
        text += cellStr;
    }
}

void Odf::parseODFXMLPara(Odf *parser, pugi::xml_node &node, std::string &text)
{
    text += parser->parseXmlData(node) + '\n';
}

void Odf::parserODFXMLUrl(Odf *parser, pugi::xml_node &node, std::string &text)
{
    text += parser->parseXmlData(node) + '\n';
}

std::string Odf::parseXmlData(const pugi::xml_node &node)
{
    auto curNode = node.first_child();
    std::string text;
    do {
        if (!executeCommand(curNode, text)) {
            if (curNode.first_child()) {
                curNode = curNode.first_child();
                continue;
            }
        }

        if (curNode.next_sibling())
            curNode = curNode.next_sibling();
        else {
            // Borland C++ workaround
            while (!curNode.next_sibling() && curNode != node && !curNode.parent().empty()) {
                curNode = curNode.parent();
            }

            if (curNode != node)
                curNode = curNode.next_sibling();
        }
    } while (curNode && curNode != node);

    return text;
}

bool Odf::executeCommand(pugi::xml_node &node, std::string &text)
{
    const auto &it = m_cmdHandlers.find(node.name());
    if (it != m_cmdHandlers.end()) {
        (*it->second)(this, node, text);
        return true;
    } else {
        std::string value = node.value();
        if (!value.empty())
            text += value;
    }

    return false;
}

}  // End namespace
