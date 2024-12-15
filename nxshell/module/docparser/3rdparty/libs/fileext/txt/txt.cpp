/**
 * @brief     TXT files into HTML сonverter
 * @package   txt
 * @file      txt.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright adhocore (https://github.com/adhocore/htmlup)
 * @date      01.08.2016 -- 29.01.2018
 */
#include <fstream>
#include <sstream>

#include "txt.hpp"

namespace txt {
Txt::Txt(const std::string &fileName)
    : FileExtension(fileName) {}

int Txt::convert(bool addStyle, bool extractImages, char mergingMode)
{
    std::string line;
    std::ifstream inputFile(m_fileName);
    while (getline(inputFile, line))
        m_text += line + '\n';
    inputFile.close();

    return 0;
}

}   // End namespace
