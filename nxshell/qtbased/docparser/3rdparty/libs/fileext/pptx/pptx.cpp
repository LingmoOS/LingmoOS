/**
 * @brief     pptx files into HTML сonverter
 * @package   pptx
 * @file      pptx.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright PolicyStat (https://github.com/PolicyStat/pptx2html)
 * @date      12.07.2016 -- 18.10.2017
 */
#include <algorithm>
#include <fstream>
#include <iostream>

#include "pptx.hpp"


namespace pptx {

class TreeWalker : public pugi::xml_tree_walker {
public:
    virtual bool for_each(pugi::xml_node &node) override {
        if (node.type() != pugi::node_element)
            return true;

        std::string nodeName = node.name();
        if (nodeName == "a:t")
            content += node.child_value();
        else if (nodeName == "a:p")
            content += '\n';

        return true;
    }

    std::string content;
};

// public:
Pptx::Pptx(const std::string& fileName)
    : FileExtension(fileName) {}

int Pptx::convert(bool addStyle, bool extractImages, char mergingMode) {
    pugi::xml_document presentationDoc;
    Ooxml::extractFile(m_fileName, "ppt/presentation.xml", presentationDoc);
    const auto &numNode = presentationDoc.child("p:presentation").child("p:sldIdLst");
    int pageNum = std::distance(numNode.begin(), numNode.end());

    pugi::xml_document tree;
    for (int i = 1; i <= pageNum && i < 2500; ++i) {
        std::string xmlName = "ppt/slides/slide" + std::to_string(i) + ".xml";
        Ooxml::extractFile(m_fileName, xmlName, tree);
        TreeWalker walker;
        tree.traverse(walker);
        m_text += walker.content;
    }

    return 0;
}

}  // End namespace
