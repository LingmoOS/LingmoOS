/**
 * @brief     PPT files into HTML сonverter
 * @package   ppt
 * @file      ppt.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright Alex Rembish (https://github.com/rembish/TextAtAnyCost)
 * @date      05.08.2017 -- 10.02.2018
 */
#include <list>
#include <regex>
#include <unordered_map>
#include <iostream>

#include "tools.hpp"

#include "ppt.hpp"


namespace ppt {

enum RecordType {
    RT_CSTRING = 0xFBA,
    RT_DOCUMENT = 0x03E8,
    RT_DRAWING = 0x040C,
    RT_END_DOCUMENT_ATOM = 0x03EA,
    RT_LIST = 0x07D0,
    RT_MAIN_MASTER = 0x03F8,
    RT_SLIDE_BASE = 0x03EC,
    RT_SLIDE_LIST_WITH_TEXT = 0x0FF0,
    RT_SLIDE = 0x03EE,
    RT_TEXT_BYTES_ATOM = 0x0FA8,
    RT_TEXT_CHARS_ATOM = 0x0FA0,
    OFFICE_ART_CLIENT_TEXTBOX = 0xF00D,
    OFFICE_ART_DG_CONTAINER = 0xF002,
    OFFICE_ART_SPGR_CONTAINER = 0xF003,
    OFFICE_ART_SP_CONTAINER = 0xF004
};

const std::regex HEADER_MASK("(\xA8|\xA0)\x0F", std::regex::icase);
/** Inline style */
const std::string STYLE = "div{font-family: monospace;font-size: 13px}.slide{margin-bottom: 20px;"
						  "padding-bottom: 10px;border-bottom: 1px solid #ddd}.slide-number{"
						  "font-weight: bold;font-size: 15px;margin-bottom: 10px}.slide-title{"
						  "font-weight: bold;font-size: 13px;margin-bottom: 10px}";

bool utf16_unichar_has_4_bytes(unsigned int ch)
{
    return (ch & 0xFC00) == 0xD800;
}

std::string unichar_to_utf8(unsigned int unichar)
{
    //unichars which have only two bytes are represented by 1, 2 or 3 bytes in utf8.
    char out[4];
    if (unichar < 0x80)
    {
        out[0] = (char)unichar;
        return std::string(out, 1);
    }
    else if (unichar < 0x800)
    {
        out[0] = 0xC0 | (unichar >> 6);
        out[1] = 0x80 | (unichar & 0x0000003F);
        return std::string(out, 2);
    }
    else if (unichar < 0x10000)
    {
        out[0] = 0xE0 | (unichar >> 12);
        out[1] = 0x80 | ((unichar & 0x00000FFF) >> 6);
        out[2] = 0x80 | (unichar & 0x0000003F);
        return std::string(out, 3);
    }
    else
    {
        //4-bytes length unichars (code points from 0x10000 to 0x10FFFF) have 4-bytes length characters equivalent in utf8
        unsigned short second_byte = unichar & 0x000003FF;
        unichar = (unichar & 0x03FF0000) >> 6;
        unichar = (unichar | second_byte) + 0x10000;
        out[0] = 0xF0 | (unichar >> 18);
        out[1] = 0x80 | ((unichar & 0x0003FFFF) >> 12);
        out[2] = 0x80 | ((unichar & 0x00000FFF) >> 6);
        out[3] = 0x80 | (unichar & 0x0000003F);
        return std::string(out, 4);
    }
    return "";
}

// public:
Ppt::Ppt(const std::string& fileName)
	: FileExtension(fileName), Cfb(fileName) {}

int  Ppt::convert(bool addStyle, bool extractImages, char mergingMode) {
	Cfb::parse();
    std::string ppdStream = getStream("PowerPoint Document");
    if (ppdStream.empty())
        return 3;
	Cfb::clear();
    parsePPT(ppdStream);
    return 0;
}

void Ppt::parsePPT(const std::string &ppd)
{
    size_t offset = 0;
    size_t surplusSize = 0;
    std::vector<unsigned char> rec(8);
    while (offset < ppd.size()) {
        surplusSize = ppd.size() - offset;
        if (surplusSize < 8) {
            parseRecord(ppd, offset, RT_END_DOCUMENT_ATOM, 0);
            return;
        }

        rec.assign(ppd.begin() + offset, ppd.begin() + offset + 8);
        if (rec.size() < 8)
            break;

        auto type = getRecordType(rec.begin() + 2);
        auto len  = getRecordLength(rec.begin() + 4);
        offset += 8;
        parseRecord(ppd, offset, type, len);
    }
}

void Ppt::parseRecord(const std::string &ppd, size_t &offset, int recType, ulong recLen)
{
    switch (recType) {
    case RT_CSTRING:
    case RT_TEXT_CHARS_ATOM: {
        std::vector<unsigned char> buf(2);
        ulong textLen = recLen / 2;
        if (textLen * 2 > ppd.size() - offset)
            textLen = (ppd.size() - offset) / 2;

        for (int i = 0; i < textLen; ++i) {
            auto u = readByte<unsigned short>(ppd, offset, 2);
            offset += 2;
            if (u == 0x0D || u == 0x0B) {
                m_text += '\n';
            } else {
                if (utf16_unichar_has_4_bytes(u) && ++i < textLen) {
                    auto b = readByte<unsigned short>(ppd, offset, 2);
                    offset += 2;
                    u = (u << 16 | b);
                }
                m_text += unichar_to_utf8(u);
            }
        }
        m_text += '\n';
        break;
    }
    case RT_TEXT_BYTES_ATOM: {
        ulong textLen = recLen;
        if (textLen > ppd.size() - offset)
            textLen = ppd.size() - offset;

        for (int i = 0; i < textLen; ++i) {
            auto u = readByte<unsigned short>(ppd, offset, 1);
            ++offset;
            if (u == 0x0B || u == 0x0D)
                m_text += '\n';
            else
                m_text += unichar_to_utf8(u);
        }
        m_text += '\n';
        break;
    }
    case OFFICE_ART_SP_CONTAINER:
    case OFFICE_ART_CLIENT_TEXTBOX:
    case OFFICE_ART_SPGR_CONTAINER:
    case OFFICE_ART_DG_CONTAINER:
    case RT_DRAWING:
    case RT_SLIDE:
    case RT_LIST:
    case RT_DOCUMENT:
    case RT_SLIDE_BASE:
    case RT_SLIDE_LIST_WITH_TEXT:
        break;
    case RT_END_DOCUMENT_ATOM:
    case RT_MAIN_MASTER:
    default:
        ulong len = recLen;
        if (offset + len > ppd.size())
            len = ppd.size() - offset;
        offset += len;
    }
}

// private:
unsigned int Ppt::getRecordLength(std::vector<unsigned char>::const_iterator buffer) const
{
    return (unsigned long)(*buffer) | ((unsigned long)(*(buffer + 1 )) << 8L) | ((unsigned long)(*(buffer + 2)) << 16L) | ((unsigned long)(*(buffer + 3)) << 24L);
}

unsigned short Ppt::getRecordType(std::vector<unsigned char>::const_iterator buffer) const
{
    return (unsigned short int)(*buffer) | ((unsigned short int)(*(buffer + 1)) << 8);
}

void Ppt::addParagraph(const std::string& text, pugi::xml_node& htmlNode) const {
	/*text = html_entity_decode(iconv("windows-1251", "utf-8", text), ENT_QUOTES, "UTF-8");*/
	auto node = htmlNode.append_child("p");
//	node.append_child(pugi::node_pcdata).set_value(text.c_str());
    std::cout << text << std::endl;
}

}  // End namespace
