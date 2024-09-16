/**
 * @brief     RTF files into HTML сonverter
 * @package   rtf
 * @file      rtf.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright lvu (https://github.com/lvu/rtf2html)
 * @date      31.07.2016 -- 18.10.2017
 */
#include <algorithm>
#include <codecvt>
#include <iostream>
#include <fstream>
#include <locale>
#include <sstream>
#include <unordered_map>
#include <ctype.h>
#include <list>

#include "formatting.hpp"
#include "keyword.hpp"
#include "table.hpp"

#include "rtf.hpp"


namespace rtf {

// public:
Rtf::Rtf(const std::string& fileName)
    : FileExtension(fileName) {}

int  Rtf::convert(bool addStyle, bool extractImages, char mergingMode) {
    m_addStyle      = addStyle;
    m_extractImages = extractImages;
    m_mergingMode   = mergingMode;

    std::string data;
    std::ifstream inputFile(m_fileName);
    data.assign(std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>());
    inputFile.close();

    bool hasAsterisk = false;
    // List variables
    bool inLi      = false;

    auto dataEnd = data.end();
    for (auto dataIter = data.begin(); dataIter != dataEnd; ) {
        switch (*dataIter) {
        case '\\': {
            Keyword kw(++dataIter);
            if (kw.m_isControlChar) {
                switch (kw.m_controlChar) {
                case '\\': case '{': case '}':
                    m_text += kw.m_controlChar;
                    break;
                case '\'':
                    m_text += codeToText(dataIter);
                    break;
                case '*':
                    // Get access to commented \pict block
                    if (std::string(dataIter, dataIter + 8) != "\\shppict")
                        hasAsterisk = true;
                    break;
                case '~':
                    m_text += "\u00A0";
                    break;
                }
            } else if (hasAsterisk) {// Skip groups with \*
                hasAsterisk   = false;
                skipGroup(dataIter);
            } else {
                std::string& keyword = kw.m_name;
                // Skip such groups
                if (keyword == "filetbl"  || keyword == "stylesheet" || keyword == "header"  ||
                        keyword == "footer"   || keyword == "headerf"    || keyword == "footerf" ||
                        keyword == "object"   || keyword == "info"       || keyword == "colortbl" ||
                        keyword == "pict"     || keyword == "fonttbl") {
                    skipGroup(dataIter);
                } else if (keyword == "line" || keyword == "softline" || keyword == "par" || keyword == "cell") { // Special chars
                    m_text += "\n";
                } else if (keyword == "tab") {
                    if (inLi)
                        m_inBullet = false;
                    else
                        m_text += "\t";
                } else if (keyword == "enspace" || keyword == "emspace") {
                    m_text += "\u00A0";
                } else if (keyword == "endash") {
                    m_text += "\u2013";
                } else if (keyword == "emdash") {
                    m_text += "\u2014";
                } else if (keyword == "bullet") {
                    m_text += "\u2022";
                } else if (keyword == "lquote") {
                    m_text += "\u2018";
                } else if (keyword == "rquote") {
                    m_text += "\u2019";
                } else if (keyword == "ldblquote") {
                    m_text += "\u201C";
                } else if (keyword == "rdblquote") {
                    m_text += "\u201D";
                } else if (keyword == "listtext") {
                    inLi = true;
                    m_inBullet = true;
                } else if (keyword == "pnlvlblt") {
                    m_isUl = true;
                }
            }
            break;
        }
        case '{':
            // Group opening actions
            ++dataIter;
            break;
        case '}':
            // Group closing actions
            ++dataIter;
            break;
        case 13: case 10:
            ++dataIter;
            break;
        case '<':
            m_text += "\u003C";
            ++dataIter;
            break;
        case '>':
            m_text += "\u003E";
            ++dataIter;
            break;
        default:
            if (!m_inBullet) {
                m_text += *dataIter;
            }
            ++dataIter;
        }
    }

    return 0;
}


// private:
void Rtf::skipGroup(std::string::iterator& it) const {
    int count = 1;
    while (count) {
        switch (*it++) {
        case '{':
            count++;
            break;
        case '}':
            count--;
            break;
        case '\\': {
            Keyword kw(it);
            if (!kw.m_isControlChar && kw.m_name == "rkw_bin" && kw.m_parameter > 0)
                std::next(it, kw.m_parameter);
            break;
        }
        }
    }
}

std::string Rtf::codeToText(std::string::iterator& it) {
    // Decode char by hex code
    std::string stmp(1, *it++);
    stmp += *it;
    if (m_inBullet) {
        if (stmp == "b7")
            m_isUl = true;
        return "";
    }

    // Decode \u0000 symbols
    auto br = it - 3;
    std::list<char> codeList;
    while (!isalpha(*--br) && (*br != '\\')) {
        if (isalnum(*br) || *br == '-')
            codeList.push_front(*br);
    }

    if (*br == 'u') {
        std::string code;
        code.assign(codeList.begin(), codeList.end());
        auto arg = strtol(code.c_str(), nullptr, 10);
        if (arg < 0) {
            it++;
            return "";
        }

        std::string output;
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::string bytes = converter.to_bytes(wchar_t(arg));
        for (auto c : bytes) {
            int s = 0;
            std::stringstream ss;

            ss << 0 + (unsigned char)c;
            ss >> s;

            output.push_back(static_cast<unsigned char>(s));
        }
        it++;
        return output;
    }
    it++;

    // Decode \'00 symbols
    int code = strtol(stmp.c_str(), nullptr, 16);
    switch (code) {
    case 147:
        return "\u201C";  // &ldquo;
    case 148:
        return "\u201D";  // &rdquo;
    case 167:
        return "\u00A7";  // &sect;
    default:
//        return std::string(1, (char)code);
        return "";
    }
}

}  // End namespace
