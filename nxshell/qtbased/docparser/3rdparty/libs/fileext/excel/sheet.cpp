/**
 * @brief     Excel files (xls/xlsx) into HTML сonverter
 * @package   excel
 * @file      sheet.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright python-excel (https://github.com/python-excel/xlrd)
 * @date      02.12.2016 -- 10.02.2018
 */
#include "tools.hpp"

#include "biffh.hpp"

#include "sheet.hpp"

#include <iostream>


namespace excel {

/** XL_SHRFMLA types */
const std::vector<int> XL_SHRFMLA_ETC {
	XL_SHRFMLA, XL_ARRAY, XL_TABLEOP, XL_TABLEOP2, XL_ARRAY2, XL_TABLEOP_B2
};
/** Cell horizontal aligment list */
const std::vector<std::string> CELL_HORZ_ALIGN {
	"left", "left", "center", "right", "justify", "justify", "center", "center"
};
/** Cell vertical aligment list */
const std::vector<std::string> CELL_VERT_ALIGN {
	"top", "middle", "bottom", "middle", "middle"
};
/** Cell border type list */
const std::vector<std::string> CELL_BORDER_TYPE {
	"none", "solid", "solid", "dashed", "dotted", "solid", "double", "dotted",
	"dashed", "dashed", "dashed", "dotted", "dotted", "dashed"
};
/** Cell border size list */
const std::vector<int> CELL_BORDER_SIZE {
	1, 1, 2, 1, 1, 3, 1, 1, 2, 1, 2, 1, 2, 3
};

/** Table parts background color map. `type`: {`firstRow`, `evenRow`, `oddRow`} */
const std::unordered_map<int, std::vector<std::string>> TABLE_BACKGROUND {
	// Light
	{101, {"", "D9D9D9", ""}},
	{102, {"", "DDEBF7", ""}},
	{103, {"", "FCE4D6", ""}},
	{104, {"", "EDEDED", ""}},
	{105, {"", "2CC", ""}},
	{106, {"", "D9E1F2", ""}},
	{107, {"", "E2EFDA", ""}},
	{108, {"000000", "", ""}},
	{109, {"5B9BD5", "", ""}},
	{110, {"ED7D31", "", ""}},
	{111, {"A5A5A5", "", ""}},
	{112, {"FFC000", "", ""}},
	{113, {"4472C4", "", ""}},
	{114, {"70AD47", "", ""}},
	{115, {"", "D9D9D9", ""}},
	{116, {"", "DDEBF7", ""}},
	{117, {"", "FCE4D6", ""}},
	{118, {"", "EDEDED", ""}},
	{119, {"", "FFC000", ""}},
	{120, {"", "D9E1F2", ""}},
	{121, {"", "E2EFDA", ""}},
	// Medium
	{201, {"000000", "D9D9D9", ""}},
	{202, {"5B9BD5", "DDEBF7", ""}},
	{203, {"ED7D31", "FCE4D6", ""}},
	{204, {"A5A5A5", "EDEDED", ""}},
	{205, {"FFC000", "2CC", ""}},
	{206, {"4472C4", "D9E1F2", ""}},
	{207, {"70AD47", "E2EFDA", ""}},
	{208, {"000000", "A6A6A6", "D9D9D9"}},
	{209, {"5B9BD5", "BDD7EE", "DDEBF7"}},
	{210, {"ED7D31", "F8CBAD", "FCE4D6"}},
	{211, {"A5A5A5", "DBDBDB", "EDEDED"}},
	{212, {"FFC000", "FFE699", "FFF2CC"}},
	{213, {"4472C4", "B4C6E7", "D9E1F2"}},
	{214, {"70AD47", "C6E0B4", "E2EFDA"}},
	{215, {"000000", "D9D9D9", ""}},
	{216, {"5B9BD5", "D9D9D9", ""}},
	{217, {"ED7D31", "D9D9D9", ""}},
	{218, {"A5A5A5", "D9D9D9", ""}},
	{219, {"FFC000", "D9D9D9", ""}},
	{220, {"4472C4", "D9D9D9", ""}},
	{221, {"70AD47", "D9D9D9", ""}},
	{222, {"D9D9D9", "A6A6A6", "D9D9D9"}},
	{223, {"DDEBF7", "BDD7EE", "DDEBF7"}},
	{224, {"FCE4D6", "F8CBAD", "FCE4D6"}},
	{225, {"EDEDED", "DBDBDB", "EDEDED"}},
	{226, {"FFF2CC", "FFE699", "FFF2CC"}},
	{227, {"D9E1F2", "B4C6E7", "D9E1F2"}},
	{228, {"E2EFDA", "C6E0B4", "E2EFDA"}},
	// Dark
	{301, {"000", "404040", "737373"}},
	{302, {"000", "2F75B5", "5B9BD5"}},
	{303, {"000", "C65911", "ED7D31"}},
	{304, {"000", "7B7B7B", "A5A5A5"}},
	{305, {"000", "BF8F00", "FFC000"}},
	{306, {"000", "305496", "4472C4"}},
	{307, {"000", "548235", "70AD47"}},
	{308, {"000", "A6A6A6", "D9D9D9"}},
	{309, {"ED7D31", "BDD7EE", "DDEBF7"}},
	{310, {"FFC000", "DBDBDB", "EDEDED"}},
	{311, {"70AD47", "B4C6E7", "D9E1F2"}}
};
/** Table parts font color map. `type`: {`firstRow`, `otherRow`} */
const std::unordered_map<int, std::vector<std::string>> TABLE_COLOR {
	// Light
	{102, {"2F75B5", "2F75B5"}},
	{103, {"C65911", "C65911"}},
	{104, {"7B7B7B", "7B7B7B"}},
	{105, {"BF8F00", "BF8F00"}},
	{106, {"305496", "305496"}},
	{107, {"548235", "548235"}},
	{108, {"fff", ""}},
	// Medium
	{201, {"fff", ""}},
	{202, {"fff", ""}},
	{203, {"fff", ""}},
	{204, {"fff", ""}},
	{205, {"fff", ""}},
	{206, {"fff", ""}},
	{207, {"fff", ""}},
	{208, {"fff", ""}},
	// Dark
	{301, {"fff", "fff"}},
	{302, {"fff", "fff"}},
	{303, {"fff", "fff"}},
	{304, {"fff", "fff"}},
	{305, {"fff", "fff"}},
	{306, {"fff", "fff"}},
	{307, {"fff", "fff"}},
	{308, {"fff", ""}},
};


// public:
Sheet::Sheet(Book* book, int position, const std::string& name, size_t number, std::string &text)
: m_book(book), m_sheetContent(text), m_name(name), m_number(number),
  m_maxRowCount((m_book->m_biffVersion >= 80) ? 65536 : 16384), m_position(position) {}

void Sheet::read() {
    try {
        bool isSstRichtext = m_book->m_addStyle && !m_book->m_richtextRunlistMap.empty();
        std::map<std::pair<unsigned short, int>, Rowinfo> rowinfoSharingDict;
        std::unordered_map<unsigned short, MSTxo> msTxos;
        bool eofFound = false;
        int savedObjectId;
        int oldPosition = m_book->m_position;
        m_book->m_position = m_position;
        while (true) {
            unsigned short code;
            unsigned short size;
            std::string    data;
            m_book->getRecordParts(code, size, data);
            if (code == XL_NUMBER) {
                // [:14] in following stmt ignores extraneous rubbish at end of record
                unsigned short rowIndex = m_book->readByte<unsigned short>(data, 0, 2);
                unsigned short colIndex = m_book->readByte<unsigned short>(data, 2, 2);
                unsigned short xfIndex  = m_book->readByte<unsigned short>(data, 4, 2);
                double d = m_book->readByte<double>(data, 6, 8);

                append(std::to_string(d));
                //			putCell(rowIndex, colIndex, std::to_string(d), xfIndex);
            } else if (code == XL_LABELSST) {
                unsigned short rowIndex = m_book->readByte<unsigned short>(data, 0, 2);
                unsigned short colIndex = m_book->readByte<unsigned short>(data, 2, 2);
                unsigned short xfIndex  = m_book->readByte<unsigned short>(data, 4, 2);
                int sstIndex = m_book->readByte<int>(data, 6, 4);

                append(m_book->m_sharedStrings[sstIndex]);
                //			putCell(rowIndex, colIndex, m_book->m_sharedStrings[sstIndex], xfIndex);
                if (isSstRichtext) {
                    auto& runlist = m_book->m_richtextRunlistMap[sstIndex];
                    if (!runlist.empty())
                        m_richtextRunlistMap[{rowIndex, colIndex}] = runlist;
                }
            } else if (code == XL_LABEL) {
                unsigned short rowIndex = m_book->readByte<unsigned short>(data, 0, 2);
                unsigned short colIndex = m_book->readByte<unsigned short>(data, 2, 2);
                unsigned short xfIndex  = m_book->readByte<unsigned short>(data, 4, 2);
                std::string str;

                if (m_book->m_biffVersion < 80)
                    str = m_book->unpackString(data, 6, 2);
                else
                    str = m_book->unpackUnicode(data, 6, 2);
                append(str);
                //			putCell(rowIndex, colIndex, str, xfIndex);
            } else if (code == XL_RSTRING) {
                unsigned short rowIndex = m_book->readByte<unsigned short>(data, 0, 2);
                unsigned short colIndex = m_book->readByte<unsigned short>(data, 2, 2);
                unsigned short xfIndex  = m_book->readByte<unsigned short>(data, 4, 2);
                int pos = 6;
                std::string str;
                std::vector<std::pair<unsigned short, unsigned short>> runlist;

                if (m_book->m_biffVersion < 80) {
                    str = m_book->unpackStringUpdatePos(data, pos, 2);
                    char nrt = data[pos];
                    pos += 1;
                    for (int i = 0; i < nrt; ++i) {
                        runlist.emplace_back(
                                    m_book->readByte<unsigned char>(data, pos,   1),
                                    m_book->readByte<unsigned char>(data, pos+1, 1)
                                    );
                        pos += 2;
                    }
                } else {
                    str = m_book->unpackUnicodeUpdatePos(data, pos, 2);
                    unsigned short nrt = m_book->readByte<unsigned short>(data, pos, 2);
                    pos += 2;
                    for (int i = 0; i < nrt; ++i) {
                        runlist.emplace_back(
                                    m_book->readByte<unsigned short>(data, pos,   2),
                                    m_book->readByte<unsigned short>(data, pos+2, 2)
                                    );
                        pos += 4;
                    }
                }
                append(str);
                //			putCell(rowIndex, colIndex, str, xfIndex);
                m_richtextRunlistMap[{rowIndex, colIndex}] = runlist;
            } else if (code == XL_RK) {
                unsigned short rowIndex = m_book->readByte<unsigned short>(data, 0, 2);
                unsigned short colIndex = m_book->readByte<unsigned short>(data, 2, 2);
                unsigned short xfIndex  = m_book->readByte<unsigned short>(data, 4, 2);
                double d = unpackRK(data.substr(6, 4));

                append(std::to_string(d));
                //			putCell(rowIndex, colIndex, std::to_string(d), xfIndex);
            } else if (code == XL_MULRK) {
                unsigned short rowIndex = m_book->readByte<unsigned short>(data, 0, 2);
                unsigned short firstCol = m_book->readByte<unsigned short>(data, 2, 2);
                unsigned short lastCol  = m_book->readByte<unsigned short>(data, (int)data.size()-2, 2);
                int pos = 4;

                for (int i = firstCol; i <= lastCol; ++i) {
                    unsigned short xfIndex = m_book->readByte<unsigned short>(data, pos, 2);
                    double d = unpackRK(data.substr(pos+2, 4));
                    pos += 6;

                    append(std::to_string(d));
                    //				putCell(rowIndex, i, std::to_string(d), xfIndex);
                }
            } else if (code == XL_ROW) {
                if (!m_book->m_addStyle)
                    continue;

                unsigned short rowIndex = m_book->readByte<unsigned short>(data, 0, 2);
                unsigned short flag1    = m_book->readByte<unsigned short>(data, 6, 2);
                int flag2 = m_book->readByte<int>(data, 12, 4);
                if (!(0 <= rowIndex && rowIndex < m_maxRowCount))
                    continue;

                auto key = std::make_pair(flag1, flag2);
                Rowinfo rowinfo;
                if (rowinfoSharingDict.find(key) == rowinfoSharingDict.end()) {
                    // Using upkbits() is far too slow on a file with 30 sheets each with 10K rows. So:
                    rowinfo.m_height                   = flag1 & 0x7fff;
                    rowinfo.m_hasDefaultHeight         = (flag1 >> 15) & 1;
                    rowinfo.m_outlineLevel             = flag2 & 7;
                    rowinfo.m_isOutlineGroupStartsEnds = (flag2 >> 4)  & 1;
                    rowinfo.m_isHidden                 = (flag2 >> 5)  & 1;
                    rowinfo.m_isHeightMismatch         = (flag2 >> 6)  & 1;
                    rowinfo.m_hasDefaultXfIndex        = (flag2 >> 7)  & 1;
                    rowinfo.m_xfIndex                  = (flag2 >> 16) & 0xfff;
                    rowinfo.m_hasAdditionalSpaceAbove  = (flag2 >> 28) & 1;
                    rowinfo.m_hasAdditionalSpaceBelow  = (flag2 >> 29) & 1;
                    if (!rowinfo.m_hasDefaultXfIndex)
                        rowinfo.m_xfIndex = -1;
                    rowinfoSharingDict[key] = rowinfo;
                } else
                    rowinfo = rowinfoSharingDict[key];
                m_rowinfoMap[rowIndex] = rowinfo;
            } else if (code == 0x0006 || code == 0x0406 || code == 0x0206) {
                unsigned short rowIndex;
                unsigned short colIndex;
                unsigned short xfIndex;
                unsigned short flags;
                std::string result;
                if (m_book->m_biffVersion >= 50) {
                    rowIndex = m_book->readByte<unsigned short>(data, 0, 2);
                    colIndex = m_book->readByte<unsigned short>(data, 2, 2);
                    xfIndex  = m_book->readByte<unsigned short>(data, 4, 2);
                    result   = m_book->readByte<std::string>(data, 6, 8);
                    flags    = m_book->readByte<unsigned short>(data, 14, 2);
                } else if (m_book->m_biffVersion >= 30) {
                    rowIndex = m_book->readByte<unsigned short>(data, 0, 2);
                    colIndex = m_book->readByte<unsigned short>(data, 2, 2);
                    xfIndex  = m_book->readByte<unsigned short>(data, 4, 2);
                    result   = m_book->readByte<std::string>(data, 6, 8);
                    flags    = m_book->readByte<unsigned short>(data, 14, 2);
                }
                // BIFF2
                else {
                    rowIndex = m_book->readByte<unsigned short>(data, 0, 2);
                    colIndex = m_book->readByte<unsigned short>(data, 2, 2);
                    std::string cellAttributes = m_book->readByte<std::string>(data, 4, 3);
                    result   = m_book->readByte<std::string>(data, 7, 8);
                    flags   = m_book->readByte<unsigned char>(data, 15, 1);

                    xfIndex = fixedXfIndexB2(cellAttributes);
                }

                if (result.substr(6, 2) == "\xFF\xFF") {
                    char firstByte = result[0];
                    if (firstByte == 0) {
                        // Need to read next record (STRING)
                        bool gotString = false;
                        // "if flags & 8" applies only to SHRFMLA
                        // Actually there's an optional SHRFMLA or ARRAY etc record to skip over
                        unsigned short code2;
                        unsigned short size2;
                        std::string    data2;

                        m_book->getRecordParts(code2, size2, data2);
                        if (code2 == XL_STRING || code2 == XL_STRING_B2)
                            gotString = true;
                        else if (find(XL_SHRFMLA_ETC.begin(), XL_SHRFMLA_ETC.end(), code2) == XL_SHRFMLA_ETC.end())
                            throw std::logic_error(
                                    "Expected SHRFMLA, ARRAY, TABLEOP* or STRING record; found " +
                                    std::to_string(code2)
                                    );

                        // Now for the STRING record
                        if (!gotString) {
                            m_book->getRecordParts(code2, size2, data2);
                            if (code2 != XL_STRING && code2 != XL_STRING_B2)
                                throw std::logic_error(
                                        "Expected STRING record; found " +
                                        std::to_string(code2)
                                        );
                        }
                        std::string str = stringRecordContent(data2);
                        append(str);
                        //					putCell(rowIndex, colIndex, str, xfIndex);
                    }
                    // Boolean formula result
                    else if (firstByte == 1) {
                        append(std::string(1, result[2]));
                        //					putCell(rowIndex, colIndex, std::string(1, result[2]), xfIndex);
                    }
                    // Error in cell
                    else if (firstByte == 2) {
                        append(std::string(1, result[2]));
                        //					putCell(rowIndex, colIndex, std::string(1, result[2]), xfIndex);
                    }
                    // Empty ... i.e. empty (zero-length) string, NOT an empty cell
                    else if (firstByte == 3) {
                        //					putCell(rowIndex, colIndex, "", xfIndex);
                    }
                    else {
                        throw std::logic_error(
                                    "Unexpected special case (" + std::to_string(firstByte) +
                                    ") in FORMULA"
                                    );
                    }
                }
                // It is a number
                else {
                    double d = m_book->readByte<double>(result, 0, 8);
                    append(std::to_string(d));
                    //				putCell(rowIndex, colIndex, std::to_string(d), xfIndex);
                }
            } else if (code == XL_BOOLERR) {
                unsigned short rowIndex = m_book->readByte<unsigned short>(data, 0, 2);
                unsigned short colIndex = m_book->readByte<unsigned short>(data, 2, 2);
                unsigned short xfIndex  = m_book->readByte<unsigned short>(data, 4, 2);
                unsigned char value     = m_book->readByte<unsigned char>(data, 6, 1);
                //unsigned char hasError  = m_book->readByte<unsigned char>(data, 7, 1);
                // Note: OOo Calc 2.0 writes 9-byte BOOLERR records. OOo docs say 8. Excel writes 8
                //int cellType = hasError ? XL_CELL_ERROR : XL_CELL_BOOLEAN;
                append(std::string(1, value));
                //			putCell(rowIndex, colIndex, std::string(1, value), xfIndex);
            } else if (code == XL_COLINFO) {
                if (!m_book->m_addStyle)
                    continue;

                Colinfo colinfo;
                unsigned short firstColIndex = m_book->readByte<unsigned short>(data, 0, 2);
                unsigned short lastColIndex  = m_book->readByte<unsigned short>(data, 2, 2);
                colinfo.m_width              = m_book->readByte<unsigned short>(data, 4, 2);
                colinfo.m_xfIndex            = m_book->readByte<unsigned char>(data, 6, 1);
                unsigned short flags         = m_book->readByte<unsigned short>(data, 8, 2);
                // #Colinfo.m_width is denominated in 256ths of a character, not in characters
                // Note: 256 instead of 255 is a common mistake. Silently ignore non-existing
                // 257th column in that case
                if (0 > firstColIndex || firstColIndex > lastColIndex || lastColIndex > 256)
                    continue;

                colinfo.m_isHidden     = (flags & 0x0001) >> 0;
                colinfo.m_bitFlag      = (flags & 0x0002) >> 1;
                colinfo.m_outlineLevel = (flags & 0x0700) >> 8;
                colinfo.m_isCollapsed  = (flags & 0x1000) >> 12;

                for (int i = firstColIndex; i <= lastColIndex; ++i) {
                    // Excel does 0 to 256 inclusive
                    if (i > 255)
                        break;
                    m_colinfoMap[i] = colinfo;
                }
            } else if (code == XL_DEFCOLWIDTH) {
                m_defaultColWidth = m_book->readByte<unsigned short>(data, 0, 2);
            } else if (code == XL_STANDARDWIDTH) {
                m_standardWidth = m_book->readByte<unsigned short>(data, 0, 2);
            } else if (code == XL_GCW) {
                // Useless w/o COLINFO
                if (!m_book->m_addStyle)
                    continue;

                std::vector<int> iguff;
                for (int i = 0; i < 8; ++i)
                    iguff.emplace_back(m_book->readByte<int>(data, 2 + i*4, 4));

                m_gcw.clear();
                for (auto& bits : iguff) {
                    for (int i = 0; i < 32; ++i) {
                        m_gcw.push_back(bits & 1);
                        bits >>= 1;
                    }
                }
            } else if (code == XL_BLANK) {
                if (!m_book->m_addStyle)
                    continue;

                unsigned short rowIndex = m_book->readByte<unsigned short>(data, 0, 2);
                unsigned short colIndex = m_book->readByte<unsigned short>(data, 2, 2);
                unsigned short xfIndex  = m_book->readByte<unsigned short>(data, 4, 2);

                //			putCell(rowIndex, colIndex, "", xfIndex);
            } else if (code == XL_MULBLANK) {  // 00BE
                if (!m_book->m_addStyle)
                    continue;

                std::vector<unsigned short> result;
                for (int i = 0; i < (size >> 1); ++i)
                    result.emplace_back(m_book->readByte<unsigned short>(data, 0 + i*2, 2));

                auto mul_last = result.back();
                int pos = 2;
                for (int colx = result[1]; colx < mul_last+1; ++colx) {
                    //				putCell(result[0], colx, "", result[pos]);
                    pos += 1;
                }
            } else if (code == XL_DIMENSION || code == XL_DIMENSION2) {
                // Four zero bytes after some other record
                if (size == 0)
                    continue;
                if (m_book->m_biffVersion < 80) {
                    m_dimensionRowCount = m_book->readByte<unsigned short>(data, 2, 2);
                    m_dimensionColCount = m_book->readByte<unsigned short>(data, 6, 2);
                }
                else {
                    m_dimensionRowCount = m_book->readByte<int>(data, 4, 4);
                    m_dimensionColCount = m_book->readByte<unsigned short>(data, 10, 2);
                }
                m_rowCount = 0;
                m_colCount = 0;

                if (
                        (m_book->m_biffVersion == 21 || m_book->m_biffVersion == 30 || m_book->m_biffVersion == 40) &&
                        !m_book->m_xfList.empty() && !m_book->m_xfEpilogueDone
                        ) {
                    Formatting formatting(m_book);
                    formatting.xfEpilogue();
                }
            }
            else if (code == XL_HLINK) {
                handleHyperlink(data);
            }
            else if (code == XL_QUICKTIP) {
                handleQuicktip(data);
            }
            else if (code == XL_EOF) {
                eofFound = true;
                break;
            }
            else if (code == XL_OBJ) {
                // Handle SHEET-level objects
                MSObj savedObject;
                handleMSObj(data, savedObject);
                savedObjectId = savedObject.m_isNull ? -1 : savedObject.m_id;
            }
            //else if (code == XL_MSO_DRAWING) {
            //	handleMsodrawingetc(code, size, data);
            //}
            else if (code == XL_TXO) {
                MSTxo msTxo;
                handleMSTxo(data, msTxo);
                if (!msTxo.m_isNull && (savedObjectId > 0)) {
                    msTxos[savedObjectId] = msTxo;
                    savedObjectId = -1;
                }
            }
            else if (code == XL_NOTE) {
                handleNote(data, msTxos);
            }
            //else if (code == XL_FEAT11) {
            //	handleFeat11(data);
            //}
            else if (find(BOF_CODES.begin(), BOF_CODES.end(), code) != BOF_CODES.end()) {
                //unsigned short version = m_book->readByte<unsigned short>(data, 0, 2);
                //unsigned short bofType = m_book->readByte<unsigned short>(data, 2, 2);
                unsigned short code2;

                while (true) {
                    m_book->getRecordParts(code2, size, data);
                    if (code2 == XL_EOF)
                        break;
                }
            }
            else if (code == XL_COUNTRY) {
                // Handle country
                m_book->m_countries = {
                    m_book->readByte<unsigned short>(data, 0, 2),
                    m_book->readByte<unsigned short>(data, 2, 2)
                };
            }
            else if (code == XL_LABELRANGES) {
                int pos = 0;
                unpackCellRangeAddressListUpdatePos(m_rowLabelRanges, data, pos, 8);
                unpackCellRangeAddressListUpdatePos(m_colLabelRanges, data, pos, 8);
            }
            //else if (code == XL_ARRAY) {
            //	unsigned short firstRowIndex = m_book->readByte<unsigned short>(data, 0, 2);
            //	unsigned short lastRowIndex  = m_book->readByte<unsigned short>(data, 2, 2);
            //	unsigned char  firstColIndex = m_book->readByte<unsigned char>(data, 4, 1);
            //	unsigned char  lastColIndex  = m_book->readByte<unsigned char>(data, 5, 1);
            //	unsigned char  flags         = m_book->readByte<unsigned char>(data, 6, 1);
            //	unsigned short tokenLength   = m_book->readByte<unsigned short>(data, 12, 2);
            //}
            //else if (code == XL_SHRFMLA) {
            //	unsigned short firstRowIndex = m_book->readByte<unsigned short>(data, 0, 2);
            //	unsigned short lastRowIndex  = m_book->readByte<unsigned short>(data, 2, 2);
            //	unsigned char  firstColIndex = m_book->readByte<unsigned char>(data, 4, 1);
            //	unsigned char  lastColIndex  = m_book->readByte<unsigned char>(data, 5, 1);
            //	unsigned char  formulaCount  = m_book->readByte<unsigned char>(data, 7, 1);
            //	unsigned short tokenLength   = m_book->readByte<unsigned short>(data, 8, 2);
            //}
            else if (code == XL_CONDFMT) {
                if (!m_book->m_addStyle)
                    continue;

                //unsigned short cfCount    = m_book->readByte<unsigned short>(data, 0, 2);
                //unsigned short needRecalc = m_book->readByte<unsigned short>(data, 2, 2);
                //unsigned short rowIndex1  = m_book->readByte<unsigned short>(data, 4, 2);
                //unsigned short rowIndex2  = m_book->readByte<unsigned short>(data, 6, 2);
                //unsigned short colIndex1  = m_book->readByte<unsigned short>(data, 8, 2);
                //unsigned short colIndex2  = m_book->readByte<unsigned short>(data, 10, 2);

                int pos = 12;
                std::vector<std::vector<int>> oList;  // Updated by function
                unpackCellRangeAddressListUpdatePos(oList, data, pos, 8);
            }
            else if (code == XL_CF) {
                if (!m_book->m_addStyle)
                    continue;

                //unsigned char  cfType = m_book->readByte<unsigned char>(data,  0, 1);
                //unsigned char  cmpOp  = m_book->readByte<unsigned char>(data,  1, 1);
                unsigned short size1  = m_book->readByte<unsigned short>(data, 2, 2);
                unsigned short size2  = m_book->readByte<unsigned short>(data, 4, 2);
                int  flags            = m_book->readByte<int>(data, 6, 4);
                bool fontBlock        = (flags >> 26) & 1;
                bool borderBlock      = (flags >> 28) & 1;
                bool paletteBlock     = (flags >> 29) & 1;

                int pos = 12;
                if (fontBlock) {
                    //int fontHeight  = m_book->readByte<int>(data, 64, 4);
                    //int fontOptions = m_book->readByte<int>(data, 68, 4);
                    //unsigned short weight     = m_book->readByte<unsigned short>(data, 72, 2);
                    //unsigned short escapement = m_book->readByte<unsigned short>(data, 74, 2);
                    //unsigned char  underline  = m_book->readByte<unsigned char>(data, 76, 1);
                    //int fontColorIndex = m_book->readByte<int>(data, 80, 4);
                    //int twoBits        = m_book->readByte<int>(data, 88, 4);
                    //int fontEscapment  = m_book->readByte<int>(data, 92, 4);
                    //int fontUnderlying = m_book->readByte<int>(data, 96, 4);

                    //bool fontStyle    = (twoBits > 1) & 1;
                    //bool posture      = (fontOptions > 1) & 1;
                    //bool fontCancel   = (twoBits > 7) & 1;
                    //bool cancellation = (fontOptions > 7) & 1;
                    pos += 118;
                }
                if (borderBlock)
                    pos += 8;
                if (paletteBlock)
                    pos += 4;
                std::string formula1 = data.substr(pos, size1);
                pos += size1;
                std::string formula2 = data.substr(pos, size2);
                pos += size2;
            }
            else if (code == XL_DEFAULTROWHEIGHT) {
                unsigned short bits;
                if (size == 4) {
                    bits = m_book->readByte<unsigned short>(data, 0, 2);
                    m_defaultRowHeight = m_book->readByte<unsigned short>(data, 2, 2);
                }
                else if (size == 2) {
                    bits = 0;
                    m_defaultRowHeight = m_book->readByte<unsigned short>(data, 0, 2);
                }
                else {
                    bits = 0;
                }
                m_isDefaultRowHeightMismatch     = bits & 1;
                m_isDefaultRowHidden             = (bits >> 1) & 1;
                m_hasDefaultAdditionalSpaceAbove = (bits >> 2) & 1;
                m_hasDefaultAdditionalSpaceBelow = (bits >> 3) & 1;
            }
            else if (code == XL_MERGEDCELLS) {
                if (!m_book->m_addStyle)
                    continue;

                int pos = 0;
                unpackCellRangeAddressListUpdatePos(m_mergedCells, data, pos, 8);
            }
            else if (code == XL_WINDOW2) {
                unsigned short options;
                if (m_book->m_biffVersion >= 80 && size >= 14) {
                    options                           = m_book->readByte<unsigned short>(data, 0,  2);
                    m_firstVisibleRowIndex            = m_book->readByte<unsigned short>(data, 2,  2);
                    m_firstVisibleColIndex            = m_book->readByte<unsigned short>(data, 4,  2);
                    m_gridlineColorIndex              = m_book->readByte<unsigned short>(data, 6,  2);
                    m_cachedPageBreakPreviewMagFactor = m_book->readByte<unsigned short>(data, 8,  2);
                    m_cachedNormalViewMagFactor       = m_book->readByte<unsigned short>(data, 10, 2);
                }
                else {
                    options                = m_book->readByte<unsigned short>(data, 0, 2);
                    m_firstVisibleRowIndex = m_book->readByte<unsigned short>(data, 2, 2);
                    m_firstVisibleColIndex = m_book->readByte<unsigned short>(data, 4, 2);
                    m_gridlineColor = {
                        m_book->readByte<unsigned char>(data, 6, 1),
                        m_book->readByte<unsigned char>(data, 7, 1),
                        m_book->readByte<unsigned char>(data, 8, 1),
                    };
                    m_gridlineColorIndex = Formatting::getNearestColorIndex(m_book->m_colorMap, m_gridlineColor);
                }

                m_showFormula            = (options >> 0) & 1;
                m_showGridLine           = (options >> 1) & 1;
                m_showSheetHeader        = (options >> 2) & 1;
                m_isFrozenPanes          = (options >> 3) & 1;
                m_showZeroValue          = (options >> 4) & 1;
                m_automaticGridLineColor = (options >> 5) & 1;
                m_columnsRightToLeft     = (options >> 6) & 1;
                m_showOutlineSymbol      = (options >> 7) & 1;
                m_removeSplits           = (options >> 8) & 1;
                m_isSheetSelected        = (options >> 9) & 1;
                m_isSheetVisible         = (options >> 10) & 1;
                m_showPageBreakPreview   = (options >> 11) & 1;
            }
            else if (code == XL_SCL) {
                unsigned short num = m_book->readByte<unsigned short>(data, 0, 2);
                unsigned short den = m_book->readByte<unsigned short>(data, 2, 2);
                int result = 0;
                if (den)
                    result = (num * 100);
                if (!(10 <= result && result <= 400))
                    result = 100;
                m_sclMagFactor = result;
            }
            else if (code == XL_PANE) {
                m_vertSplitPos          = m_book->readByte<unsigned short>(data, 0, 2);
                m_horzSplitPos          = m_book->readByte<unsigned short>(data, 2, 2);
                m_horzSplitFirstVisible = m_book->readByte<unsigned short>(data, 4, 2);
                m_vertSplitFirstVisible = m_book->readByte<unsigned short>(data, 6, 2);
                m_splitActivePane       = m_book->readByte<unsigned char>(data, 8, 1);
                m_hasPaneRecord         = true;
            }
            else if (code == XL_HORIZONTALBREAKS) {
                if (!m_book->m_addStyle)
                    continue;

                //unsigned short breakCount = m_book->readByte<unsigned short>(data, 0, 2);
                int pos = 2;
                if (m_book->m_biffVersion < 80)
                    while (pos < size) {
                        m_horizontalPageBreaks.push_back({
                                                             m_book->readByte<unsigned short>(data, pos, 2),
                                                             0,
                                                             255
                                                         });
                        pos += 2;
                    }
                else
                    while (pos < size) {
                        m_horizontalPageBreaks.push_back({
                                                             m_book->readByte<unsigned short>(data, pos,   2),
                                                             m_book->readByte<unsigned short>(data, pos+2, 2),
                                                             m_book->readByte<unsigned short>(data, pos+4, 2)
                                                         });
                        pos += 6;
                    }
            }
            else if (code == XL_VERTICALPAGEBREAKS) {
                if (!m_book->m_addStyle)
                    continue;

                //unsigned short breakCount = m_book->readByte<unsigned short>(data, 0, 2);
                int pos = 2;
                if (m_book->m_biffVersion < 80)
                    while (pos < size) {
                        m_verticalPageBreaks.push_back({
                                                           m_book->readByte<unsigned short>(data, pos, 2),
                                                           0,
                                                           65535
                                                       });
                        pos += 2;
                    }
                else
                    while (pos < size) {
                        m_verticalPageBreaks.push_back({
                                                           m_book->readByte<unsigned short>(data, pos,   2),
                                                           m_book->readByte<unsigned short>(data, pos+2, 2),
                                                           m_book->readByte<unsigned short>(data, pos+4, 2)
                                                       });
                        pos += 6;
                    }
            }
            // All of the following are for BIFF <= 4W
            else if (m_book->m_biffVersion <= 45) {
                Formatting formatting(m_book);
                if (code == XL_FORMAT || code == XL_FORMAT2)
                    formatting.handleFormat(data, code);
                else if (code == XL_FONT || code == XL_FONT_B3B4)
                    formatting.handleFont(data);
                else if (code == XL_STYLE) {
                    if (!m_book->m_xfEpilogueDone)
                        formatting.xfEpilogue();
                    formatting.handleStyle(data);
                }
                else if (code == XL_PALETTE)
                    formatting.handlePalette(data);
                else if (code == XL_BUILTINFMTCOUNT)
                    m_book->m_builtinFormatCount = m_book->readByte<unsigned short>(data, 0, 2);
                else if (code == XL_XF4 || code == XL_XF3 || code == XL_XF2)  // N.B. not XL_XF
                    formatting.handleXf(data);
                else if (code == XL_DATEMODE)
                    m_book->m_dateMode = m_book->readByte<unsigned short>(data, 0, 2);
                else if (code == XL_CODEPAGE) {
                    m_book->m_codePage = m_book->readByte<unsigned short>(data, 0, 2);
                    m_book->getEncoding();
                }
                else if (code == XL_WRITEACCESS)
                    m_book->handleWriteAccess(data);
                else if (code == XL_IXFE)
                    m_ixfe = m_book->readByte<unsigned short>(data, 0, 2);
                else if (code == XL_NUMBER_B2) {
                    unsigned short rowIndex    = m_book->readByte<unsigned short>(data, 0, 2);
                    unsigned short colIndex    = m_book->readByte<unsigned short>(data, 2, 2);
                    std::string cellAttributes = m_book->readByte<std::string>(data, 4, 3);
                    double d = m_book->readByte<unsigned short>(data, 7, 4);

                    append(std::to_string(d));
                    //				putCell(rowIndex, colIndex, std::to_string(d), fixedXfIndexB2(cellAttributes));
                }
                else if (code == XL_INTEGER) {
                    unsigned short rowIndex    = m_book->readByte<unsigned short>(data, 0, 2);
                    unsigned short colIndex    = m_book->readByte<unsigned short>(data, 2, 2);
                    std::string cellAttributes = m_book->readByte<std::string>(data, 4, 3);
                    float d = m_book->readByte<unsigned short>(data, 7, 2);

                    append(std::to_string(d));
                    //				putCell(rowIndex, colIndex, std::to_string(d), fixedXfIndexB2(cellAttributes));
                }
                else if (code == XL_LABEL_B2) {
                    unsigned short rowIndex    = m_book->readByte<unsigned short>(data, 0, 2);
                    unsigned short colIndex    = m_book->readByte<unsigned short>(data, 2, 2);
                    std::string cellAttributes = m_book->readByte<std::string>(data, 4, 3);
                    std::string str = m_book->unpackString(data, 7, 1);

                    append(str);
                    //				putCell(rowIndex, colIndex, str, fixedXfIndexB2(cellAttributes));
                }
                else if (code == XL_BOOLERR_B2) {
                    unsigned short rowIndex    = m_book->readByte<unsigned short>(data, 0, 2);
                    unsigned short colIndex    = m_book->readByte<unsigned short>(data, 2, 2);
                    std::string cellAttributes = m_book->readByte<std::string>(data, 4, 3);
                    unsigned char value        = m_book->readByte<unsigned char>(data, 7, 1);
                    //unsigned char hasError   = m_book->readByte<unsigned char>(data, 8, 1);

                    //int cellType = hasError ? XL_CELL_ERROR : XL_CELL_BOOLEAN;
                    append(std::to_string(value));
                    //				putCell(rowIndex, colIndex, std::to_string(value), fixedXfIndexB2(cellAttributes));
                }
                else if (code == XL_BLANK_B2) {
                    if (!m_book->m_addStyle)
                        continue;
                    unsigned short rowIndex    = m_book->readByte<unsigned short>(data, 0, 2);
                    unsigned short colIndex    = m_book->readByte<unsigned short>(data, 2, 2);
                    std::string cellAttributes = m_book->readByte<std::string>(data, 4, 3);

                    //				putCell(rowIndex, colIndex, "", fixedXfIndexB2(cellAttributes));
                }
                else if (code == XL_EFONT) {
                    if (!m_book->m_addStyle)
                        continue;
                    m_book->m_fontList.back().m_color.m_index = m_book->readByte<unsigned short>(data, 0, 2);
                }
                else if (code == XL_ROW_B2) {
                    if (!m_book->m_addStyle)
                        continue;

                    unsigned short rowIndex = m_book->readByte<unsigned short>(data, 0, 2);
                    unsigned short flag1    = m_book->readByte<unsigned short>(data, 6, 2);
                    unsigned char  flag2    = m_book->readByte<unsigned char>(data, 10, 1);

                    if (!(0 <= rowIndex && rowIndex < m_maxRowCount))
                        continue;
                    int xfIndex;
                    // hasDefaultXfIndex is false
                    if (!(flag2 & 1)) {
                        xfIndex = -1;
                    }
                    // Seems XF index in the cellAttributes is dodgy
                    else if (size == 18) {
                        unsigned short xfx = m_book->readByte<unsigned short>(data, 16, 2);
                        xfIndex = fixedXfIndexB2("", xfx);
                    }
                    else {
                        std::string cellAttributes = data.substr(13, 3);
                        xfIndex = fixedXfIndexB2(cellAttributes);
                    }

                    auto key = std::make_pair(flag1, flag2);
                    Rowinfo rowinfo;
                    if (rowinfoSharingDict.find(key) == rowinfoSharingDict.end()) {
                        rowinfo.m_height            = flag1 & 0x7fff;
                        rowinfo.m_hasDefaultHeight  = (flag1 >> 15) & 1;
                        rowinfo.m_hasDefaultXfIndex = flag2 & 1;
                        rowinfo.m_xfIndex           = xfIndex;
                    }
                    else {
                        rowinfo = rowinfoSharingDict[key];
                    }
                    m_rowinfoMap[rowIndex] = rowinfo;
                }
                else if (code == XL_COLWIDTH) {  // BIFF2 only
                    if (!m_book->m_addStyle)
                        continue;

                    unsigned char  firstColIndex = m_book->readByte<unsigned char>(data, 0, 1);
                    unsigned char  lastColIndex  = m_book->readByte<unsigned char>(data, 1, 1);
                    unsigned short width         = m_book->readByte<unsigned short>(data, 2, 2);

                    if (firstColIndex > lastColIndex)
                        continue;
                    for (int i = firstColIndex; i <= lastColIndex; ++i) {
                        Colinfo colinfo;
                        if (m_colinfoMap.find(i) != m_colinfoMap.end()) {
                            m_colinfoMap[i].m_width = width;
                            //colinfo = m_colinfoMap[i];
                        }
                        else {
                            colinfo.m_width = width;
                            m_colinfoMap[i] = colinfo;
                        }
                    }
                }
                else if (code == XL_COLUMNDEFAULT) {  // BIFF2 only
                    if (!m_book->m_addStyle)
                        continue;

                    unsigned short firstColIndex = m_book->readByte<unsigned short>(data, 0, 2);
                    unsigned short lastColIndex  = m_book->readByte<unsigned short>(data, 2, 2);
                    // Warning: OOo docs wrong; firstColIndex <= colx < lastColIndex
                    if (0 > firstColIndex || firstColIndex >= lastColIndex || lastColIndex > 256)
                        lastColIndex = std::min((int)lastColIndex, 256);
                    for (int i = firstColIndex; i < lastColIndex; ++i) {
                        std::string cellAttributes = data.substr(4 + 3*(i - firstColIndex), 3);
                        int xfIndex = fixedXfIndexB2(cellAttributes);

                        Colinfo colinfo;
                        if (m_colinfoMap.find(i) != m_colinfoMap.end()) {
                            m_colinfoMap[i].m_xfIndex = xfIndex;
                            //colinfo = m_colinfoMap[i];
                        }
                        else {
                            colinfo.m_xfIndex = xfIndex;
                            m_colinfoMap[i]   = colinfo;
                        }
                    }
                }
                else if (code == XL_WINDOW2_B2) {  // BIFF 2 only
                    m_showFormula     = (data[0] != '\0');
                    m_showGridLine    = (data[1] != '\0');
                    m_showSheetHeader = (data[2] != '\0');
                    m_isFrozenPanes   = (data[3] != '\0');
                    m_showZeroValue   = (data[4] != '\0');

                    m_firstVisibleRowIndex   = m_book->readByte<unsigned short>(data, 5, 2);
                    m_firstVisibleColIndex   = m_book->readByte<unsigned short>(data, 7, 2);
                    m_automaticGridLineColor = m_book->readByte<unsigned char>(data,  9, 1);

                    m_gridlineColor = {
                        m_book->readByte<unsigned char>(data, 10, 1),
                        m_book->readByte<unsigned char>(data, 11, 1),
                        m_book->readByte<unsigned char>(data, 12, 1)
                    };
                    m_gridlineColorIndex = Formatting::getNearestColorIndex(m_book->m_colorMap, m_gridlineColor);
                }
            }
        }
        if (!eofFound)
            throw std::logic_error("Sheet "+ std::to_string(m_number) +
                                   " ("+ m_name + ") missing EOF record");
        //	tidyDimensions();
        updateCookedFactors();
        m_book->m_position = oldPosition;
    } catch (const std::logic_error &error) {
        std::cout << error.what() << std::endl;
    }
}

void Sheet::append(const std::string &value)
{
    m_sheetContent += value + '\n';
}

#if 0
void Sheet::putCell(int rowIndex, int colIndex, const std::string& value, int xfIndex) {
	int rowCount = rowIndex + 1;
	int colCount = colIndex + 1;
	if (colCount > m_colCount) {
		m_colCount = colCount;
		// The row firstFullRowIndex and all subsequent rows are guaranteed to have length == m_colCount
		// Cell data is not in non-descending row order AND m_colCount has been bumped up.
		// This very rare case ruins this optmisation
		if (rowCount < m_rowCount)
			m_firstFullRowIndex = -2;
		else if (rowIndex > m_firstFullRowIndex && m_firstFullRowIndex > -2)
			m_firstFullRowIndex = rowIndex;
	}
	if (rowCount > m_rowCount)
		m_rowCount = rowCount;

	// Add missing rows to table
	for (int i = tools::xmlChildrenCount(m_table, "tr"); i <= rowIndex; ++i) {
		auto tr = m_table.append_child("tr");
		addRowStyle(tr, i);
	}
	pugi::xml_node tr = *std::next(m_table.children("tr").begin(), rowIndex);

	// Add missing cells to row
	for (int i = tools::xmlChildrenCount(tr, "td"); i < colIndex; ++i) {
		auto td = tr.append_child("td");
		addColStyle(td, i);
	}
	auto td = tr.append_child("td");
	auto node = td;

	// Get cell style
	if (m_book->m_addStyle) {
		auto& xf = m_book->m_xfList[xfIndex];
		auto& cellFont = m_book->m_fontList[xf.m_fontIndex];
		addCellStyle(td, xf, rowIndex, colIndex);

		if (cellFont.m_isBold)
			node = node.append_child("b");
		if (cellFont.m_isItalic)
			node = node.append_child("i");
		if (cellFont.m_isUnderlined)
			node = node.append_child("u");
		if (cellFont.m_isStruckOut)
			node = node.append_child("s");
		if (cellFont.m_escapement == 1)
			node = node.append_child("sup");
		if (cellFont.m_escapement == 2)
			node = node.append_child("sub");
	}

    std::cout << value << std::endl;
	node.append_child(pugi::node_pcdata).set_value(value.c_str());
}


void Sheet::tidyDimensions() {
	if (!m_mergedCells.empty()) {
		int rowCount = 0;
		int colCount = 0;

		for (const auto& cRange : m_mergedCells) {
			if (cRange[1] > rowCount)
				rowCount = cRange[1];
			if (cRange[3] > colCount)
				colCount = cRange[3];
		}
		if (colCount > m_colCount) {
			m_colCount = colCount;
			m_firstFullRowIndex = -2;
		}
		// Put one empty cell at (rowCount-1, 0) to make sure we have right number of rows
		if (rowCount > m_rowCount)
			putCell(rowCount-1, 0, "", -1);
	}

	// Add missing cells to row
	int rIndex = 0;
	for (auto& tr : m_table.children("tr")) {
		for (int i = tools::xmlChildrenCount(tr, "td"); i < m_colCount; ++i) {
			auto td = tr.append_child("td");
			addColStyle(td, i);
		}
		rIndex++;
	}

	// Add colspan/rowspan attributes
	if (m_book->m_mergingMode == 0) {
		int rowIndex = -1;
		int colCount = -1;
		for (const auto& cRange : m_mergedCells) {
			auto tr = std::next(m_table.children("tr").begin(), cRange[0]);

			for (int i = cRange[0]; i < cRange[1]; ++i) {
				if (rowIndex != i) {
					rowIndex = i;
					colCount = 0;
				}
				int offset = std::min(tools::xmlChildrenCount(*tr, "td"), cRange[3] - colCount) - 1;
				auto td = std::next(tr->children("td").begin(), offset);

				int endRange = cRange[3] - (rowIndex == cRange[0]);
				for (int j = cRange[2]; j < endRange; ++j) {
					auto next = td--;
					tr->remove_child(*next);
					colCount++;
				}

				if (rowIndex == cRange[0]) {
					td->append_attribute("colspan") = std::to_string(cRange[3]-cRange[2]).c_str();
					td->append_attribute("rowspan") = std::to_string(cRange[1]-cRange[0]).c_str();
				}
				tr++;
			}
		}
	}
	// Fill empty cells with duplicate values
	else if (m_book->m_mergingMode == 1) {
		for (const auto& cRange : m_mergedCells) {
			auto tr = std::next(m_table.children("tr").begin(), cRange[0]);
			auto tdMain = std::next(tr->children("td").begin(), cRange[2]);

			for (int i = cRange[0]; i < cRange[1]; ++i) {
				auto td = std::next(tr->children("td").begin(), cRange[2]);
				for (int j = cRange[2]; j < cRange[3]; ++j) {
					// Each cell has only 1 element, so we need to copy only first child
					if (td != tdMain)
						td->append_copy(tdMain->first_child());
					td++;
				}
				tr++;
			}
		}
	}
}
#endif

// private:
std::string Sheet::stringRecordContent(const std::string& data) {
    int length = (m_book->m_biffVersion >= 30) + 1;
    unsigned short expectedCharCount = m_book->readByte<unsigned short>(data, 0, length);
    int offset         = length;
    int foundCharCount = 0;
    std::string result = "";
    while (true) {
        try {
            if (m_book->m_biffVersion >= 80)
                offset++;
            std::string chunk  = data.substr(offset);
            result            += chunk;
            foundCharCount    += static_cast<unsigned short>(chunk.size());
            if (foundCharCount == expectedCharCount)
                return result;
            if (foundCharCount > expectedCharCount)
                throw std::logic_error(
                        "STRING/CONTINUE: expected " + std::to_string(expectedCharCount) +
                        " chars, found " + std::to_string(foundCharCount)
                        );

            unsigned short code;
            unsigned short unusedLength;
            std::string    data;
            m_book->getRecordParts(code, unusedLength, data);
            if (code != XL_CONTINUE)
                throw std::logic_error("Expected CONTINUE record; found record-type "+ std::to_string(code));
            offset = 0;
        } catch (...) {
            return result;
        }
    }
}

int Sheet::fixedXfIndexB2(const std::string& cellAttributes, int trueXfIndex) {
	int xfIndex;
	if (m_book->m_biffVersion == 21) {
		if (!m_book->m_xfList.empty()) {
			if (trueXfIndex != -1)
				xfIndex = trueXfIndex;
			else
				xfIndex = cellAttributes[0] & 0x3F;

			if (xfIndex == 0x3F) {
				if (m_ixfe == 0)
					throw std::logic_error("BIFF2 cell record has XF index 63 but no preceding IXFE record");
				xfIndex = m_ixfe;
				// OOo docs are capable of interpretation that each
				// cell record is preceded immediately by its own IXFE record.
				// Empirical evidence is that (sensibly) an IXFE record applies to all
				// following cell records until another IXFE comes along.
			}
			return xfIndex;
		}
		// Have either Excel 2.0, or broken 2.1 w/o XF records - same effect
		m_book->m_biffVersion = 20;
	}
	xfIndex = m_cellAttributesToXfIndex[cellAttributes];
	if (xfIndex)
		return xfIndex;
	if (m_book->m_xfList.empty()) {
		for (int i = 0; i < 16; ++i)
			insertXfB20("\x40\x00\x00", i < 15);
	}
	xfIndex = insertXfB20(cellAttributes);
	return xfIndex;
}

int Sheet::insertXfB20(const std::string& cellAttributes, bool isStyle) {
	int xfx = static_cast<int>(m_book->m_xfList.size());
	XF xf;
	fakeXfFromCellAttrB20(xf, cellAttributes, isStyle);
	xf.m_xfIndex = xfx;
	m_book->m_xfList.emplace_back(xf);

	if (m_book->m_formatMap.find(xf.m_formatKey) == m_book->m_formatMap.end()) {
		Format fmt(xf.m_formatKey, FUN, "General");
		m_book->m_formatMap[xf.m_formatKey] = fmt;
		m_book->m_formatList.emplace_back(fmt);
	}

	Format fmt = m_book->m_formatMap[xf.m_formatKey];
	int cellty = CELL_TYPE_FROM_FORMAT_TYPE.at(fmt.m_type);
	m_book->m_xfIndexXlTypeMap[xf.m_xfIndex]  = cellty;
	m_cellAttributesToXfIndex[cellAttributes] = xfx;
	return xfx;
}

void Sheet::fakeXfFromCellAttrB20(XF& xf, const std::string& cellAttributes, bool isStyle) {
	xf.m_alignment                  = XFAlignment();
	xf.m_alignment.m_indentLevel    = 0;
	xf.m_alignment.m_isShrinkToFit  = 0;
	xf.m_alignment.m_textDirection  = 0;
	xf.m_border                     = XFBorder();
	xf.m_border.m_diagUp            = false;
	xf.m_border.m_diagDown          = false;
	xf.m_border.m_diagColor.m_index = 0;
	xf.m_border.m_diagLineStyle     = 0;  // No line
	xf.m_background                 = XFBackground();
	xf.m_protection                 = XFProtection();

	unsigned char protection = m_book->readByte<unsigned char>(cellAttributes, 0, 1);
	unsigned char fontFormat = m_book->readByte<unsigned char>(cellAttributes, 0, 1);
	unsigned char style      = m_book->readByte<unsigned char>(cellAttributes, 0, 1);

	xf.m_protection.m_isCellLocked    = (protection & 0x40) >> 6;
	xf.m_protection.m_isFormulaHidden = (protection & 0x80) >> 7;

	xf.m_parentStyleIndex = isStyle ? 0 : 0x0FFF;
	xf.m_formatKey        = fontFormat  & 0x3F;
	xf.m_fontIndex        = (fontFormat & 0xC0) >> 6;

	xf.m_alignment.m_isShrinkToFit = style & 0x07;
	xf.m_alignment.m_verticalAlign = 2;  // Bottom
	xf.m_alignment.m_rotation      = 0;

	xf.m_border.m_leftLineStyle       = (style & 0x08) ? 1 : 0;  // 1 - thin
	xf.m_border.m_leftColor.m_index   = (style & 0x08) ? 8 : 0;  // 8 - black
	xf.m_border.m_rightLineStyle      = (style & 0x10) ? 1 : 0;
	xf.m_border.m_rightColor.m_index  = (style & 0x10) ? 8 : 0;
	xf.m_border.m_topLineStyle        = (style & 0x20) ? 1 : 0;
	xf.m_border.m_topColor.m_index    = (style & 0x20) ? 8 : 0;
	xf.m_border.m_bottomLineStyle     = (style & 0x40) ? 1 : 0;
	xf.m_border.m_bottomColor.m_index = (style & 0x40) ? 8 : 0;

	xf.m_formatFlag     = true;
	xf.m_fontFlag       = true;
	xf.m_alignmentFlag  = true;
	xf.m_borderFlag     = true;
	xf.m_backgroundFlag = true;
	xf.m_protectionFlag = true;

	xf.m_background.m_fillPattern             = (style & 0x80) ? 17 : 0;
	xf.m_background.m_backgroundColor.m_index = 9;  // White
	xf.m_background.m_patternColor.m_index    = 8;  // Black
}


std::string Sheet::getNullTerminatedUnicode(const std::string& buf, int& offset) const {
	unsigned long size = m_book->readByte<int>(buf, offset, 4) * 2;
	offset += 4;
	std::string res = buf.substr(offset, size-1);
	offset += size;
	return res;
}

void Sheet::handleHyperlink(const std::string& data) {
	int recordSize = static_cast<int>(data.size());
	Hyperlink hlink;
	hlink.m_firstRowIndex = m_book->readByte<unsigned short>(data, 0, 2);
	hlink.m_lastRowIndex  = m_book->readByte<unsigned short>(data, 2, 2);
	hlink.m_firstColIndex = m_book->readByte<unsigned short>(data, 4, 2);
	hlink.m_lastColIndex  = m_book->readByte<unsigned short>(data, 6, 2);
	int options = m_book->readByte<int>(data, 28, 4);
	int offset  = 32;

	// Has a description
	if (options & 0x14)
		hlink.m_description = getNullTerminatedUnicode(data, offset);
	// Has a target
	if (options & 0x80)
		hlink.m_target = getNullTerminatedUnicode(data, offset);

	// HasMoniker and not MonikerSavedAsString
	if ((options & 1) && !(options & 0x100)) {
		// An OLEMoniker structure
		std::string clsId = m_book->readByte<std::string>(data, offset, 16);
		offset += 16;
		if (clsId == "\xE0\xC9\xEA\x79\xF9\xBA\xCE\x11\x8C\x82\x00\xAA\x00\x4B\xA9\x0B") {
			// URL Moniker
			unsigned long size = m_book->readByte<unsigned long>(data, offset, 4);
			offset      += 4;
			hlink.m_type = "url";
			hlink.m_url  = data.substr(offset, size);
			hlink.m_url  = hlink.m_url.substr(0, hlink.m_url.find("\x00"));
			offset      += size;
		}
		else if (clsId == "\x03\x03\x00\x00\x00\x00\x00\x00\xC0\x00\x00\x00\x00\x00\x00\x46") {
			// File moniker
			unsigned long upLevels = m_book->readByte<unsigned short>(data, offset, 2);
			int size     = m_book->readByte<int>(data, offset+2, 4);
			hlink.m_type = "local file";
			offset      += 6;
			// BYTES, not unicode
			std::string shortPath = tools::repeatString("..\\", upLevels) +
									data.substr(offset, size - 1);
			offset      += size + 24;  // OOo: "unknown byte sequence"
			// Above is version 0xDEAD + 20 reserved zero bytes
			size = m_book->readByte<int>(data, offset, 4);
			offset += 4;
			if (size) {
				size = m_book->readByte<int>(data, offset, 4);
				offset += 6;  // "unknown byte sequence" MS: 0x0003
				std::string extendedPath = data.substr(offset, size);  // Not zero-terminated
				offset += size;
				hlink.m_url = extendedPath;
			}
			// The "shortpath" is bytes encoded in *UNKNOWN* creator's "ANSI" encoding
			else {
				hlink.m_url = shortPath;
			}
		}
	}
	// UNC
	else if ((options & 0x163) == 0x103) {
		hlink.m_type = "unc";
		hlink.m_url = getNullTerminatedUnicode(data, offset);
	}
	else if ((options & 0x16B) == 8)
		hlink.m_type = "workbook";
	else
		hlink.m_type = "unknown";

	// Has textmark
	if (options & 0x8)
		hlink.m_textmark = getNullTerminatedUnicode(data, offset);

	int extraByteCount = recordSize - offset;
    if (extraByteCount < 0)
        throw std::logic_error("Bug or corrupt file, send copy of input file for debugging");

	m_hyperlinkList.push_back(hlink);
	for (int i = hlink.m_firstRowIndex; i <= hlink.m_lastRowIndex; ++i)
		for (int j = hlink.m_firstColIndex; j <= hlink.m_lastColIndex; ++j)
			m_hyperlinkMap[{i, j}] = hlink;
}

void Sheet::handleQuicktip(const std::string& data) {
	//unsigned short codeIndex     = m_book->readByte<unsigned short>(data, 0, 2);
	//unsigned short firstRowIndex = m_book->readByte<unsigned short>(data, 2, 2);
	//unsigned short lastRowIndex  = m_book->readByte<unsigned short>(data, 4, 2);
	//unsigned short firstColIndex = m_book->readByte<unsigned short>(data, 6, 2);
	//unsigned short lastColIndex  = m_book->readByte<unsigned short>(data, 8, 2);
	m_hyperlinkList.back().m_quicktip = data.substr(10, data.size() - 10 - 2);
}

void Sheet::handleMSObj(const std::string& data, MSObj& msObj) {
	if (m_book->m_biffVersion < 80) {
		msObj.m_isNull = true;
		return;
	}
	int size = static_cast<int>(data.size());
	int pos  = 0;
	while (pos < size) {
		unsigned short ft = m_book->readByte<unsigned short>(data, pos,   2);
		unsigned short cb = m_book->readByte<unsigned short>(data, pos+2, 2);
		if (pos == 0 && !(ft == 0x15 && cb == 18)) {
			msObj.m_isNull = true;
			return;
		}
		// ftCmo ... s/b first
		if (ft == 0x15) {
			msObj.m_type = m_book->readByte<unsigned short>(data, pos+4, 2);
			msObj.m_id   = m_book->readByte<unsigned short>(data, pos+6, 2);
			unsigned short options = m_book->readByte<unsigned short>(data, pos+8, 2);

			msObj.m_isLocked      = (options & 0x0001) >> 0;
			msObj.m_isPrintable   = (options & 0x0010) >> 4;
			msObj.m_autoFilter    = (options & 0x0100) >> 8;  // Not documented in Excel 97 dev kit
			msObj.m_scrollbarFlag = (options & 0x0200) >> 9;  // Not documented in Excel 97 dev kit
			msObj.m_autoFill      = (options & 0x2000) >> 13;
			msObj.m_autoLine      = (options & 0x4000) >> 14;
		}
		else if (ft == 0x00) {
			// Ignore "optional reserved" data at end of record
			if (data.substr(pos, size - pos) == std::string(size - pos, '\0'))
				break;
			throw std::logic_error("Unexpected data at end of OBJECT record");
		}
		// Scrollbar
		else if (ft == 0x0C) {
			msObj.m_scrollbarValue = m_book->readByte<unsigned short>(data, pos+8, 2);
			msObj.m_scrollbarMin   = m_book->readByte<unsigned short>(data, pos+10, 2);
			msObj.m_scrollbarMax   = m_book->readByte<unsigned short>(data, pos+12, 2);
			msObj.m_scrollbarInc   = m_book->readByte<unsigned short>(data, pos+14, 2);
			msObj.m_scrollbarPage  = m_book->readByte<unsigned short>(data, pos+16, 2);
		}
		// List box data
		else if (ft == 0x13) {
			// Non standard exit. NOT documented
			if (msObj.m_autoFilter)
				break;
		}
		pos += cb + 4;
	}
}

void Sheet::handleMSTxo(const std::string& data, MSTxo& msTxo) {
	if (m_book->m_biffVersion < 80) {
		msTxo.m_isNull = true;
		return;
	}
	size_t size             = data.size();
	unsigned short options  = m_book->readByte<unsigned short>(data, 0, 2);
	msTxo.m_rotation        = m_book->readByte<unsigned short>(data, 2, 2);
	std::string controlInfo = data.substr(4, 6);
	unsigned short cchText  = m_book->readByte<unsigned short>(data, 10, 2);
	unsigned short cbRuns   = m_book->readByte<unsigned short>(data, 12, 2);
	msTxo.m_isNotEmpty      = m_book->readByte<unsigned short>(data, 14, 2);
	msTxo.m_formula         = data.substr(16, size);

	msTxo.m_horzAlign  = (options & 0x0001) >> 3;
	msTxo.m_vertAlign  = (options & 0x0001) >> 6;
	msTxo.m_lockText   = (options & 0x0001) >> 9;
	msTxo.m_justLast   = (options & 0x0001) >> 14;
	msTxo.m_secretEdit = (options & 0x0001) >> 15;

	msTxo.m_text.clear();
	int totalCharCount = 0;
	while (totalCharCount < cchText) {
		unsigned short code2;
		unsigned short size2;
		std::string    data2;
		m_book->getRecordParts(code2, size2, data2);

		char nb = data2[0];  // 0 means latin1, 1 means utf_16_le
		int charCount = size2 - 1;
		if (nb)
			charCount /= 2;

		int endPos = 0;
		msTxo.m_text   += m_book->unpackUnicodeUpdatePos(data2, endPos, 2, charCount);
		totalCharCount += charCount;
	}
	msTxo.m_richtextRunlist.clear();
	int totalRuns = 0;
	while (totalRuns < cbRuns) {  // Counts of BYTES, not runs
		unsigned short code2;
		unsigned short size2;
		std::string    data2;
		m_book->getRecordParts(code2, size2, data2);

		for (int pos = 0; pos < size2; pos += 8) {
			msTxo.m_richtextRunlist.emplace_back(m_book->readByte<unsigned short>(data2, pos,   2),
												 m_book->readByte<unsigned short>(data2, pos+2, 2));
			totalRuns += 8;
		}
	}
	// Remove trailing entries that point to the end of string
	for (
		auto it = msTxo.m_richtextRunlist.rbegin();
		(it != msTxo.m_richtextRunlist.rend()) && (it->first == cchText);
		++it
	)
		msTxo.m_richtextRunlist.pop_back();
}

void Sheet::handleNote(const std::string& data, std::unordered_map<unsigned short, MSTxo>& msTxos) {
	Note note;
	int size = static_cast<int>(data.size());
	if (m_book->m_biffVersion < 80) {
		note.m_rowIndex = m_book->readByte<unsigned short>(data, 0, 2);
		note.m_colIndex = m_book->readByte<unsigned short>(data, 2, 2);
		unsigned short expectedByteCount = m_book->readByte<unsigned short>(data, 4, 2);

		unsigned short nb = size - 6;
		note.m_text = data.substr(6, size);
		expectedByteCount -= nb;
		while (expectedByteCount > 0) {
			unsigned short code2;
			unsigned short size2;
			std::string    data2;
			m_book->getRecordParts(code2, size2, data2);

			nb = m_book->readByte<unsigned short>(data2, 4, 2);
			note.m_text += data2.substr(6);
			expectedByteCount -= nb;
		}
		note.m_richtextRunlist.emplace_back(0, 0);

		m_cellNoteMap[{note.m_rowIndex, note.m_colIndex}] = note;
		return;
	}
	// Excel 8.0+
	note.m_rowIndex = m_book->readByte<unsigned short>(data, 0, 2);
	note.m_colIndex = m_book->readByte<unsigned short>(data, 2, 2);
	unsigned short options = m_book->readByte<unsigned short>(data, 4, 2);
	note.m_objectId = m_book->readByte<unsigned short>(data, 6, 2);

	note.m_isShown     = (options >> 1) & 1;
	note.m_isRowHidden = (options >> 7) & 1;
	note.m_isColHidden = (options >> 8) & 1;
	// XL97 dev kit says NULL [sic] bytes padding between string count and string data
	// to ensure that string is word-aligned. Appears to be nonsense
	int endPos    = 8;
	note.m_author = m_book->unpackUnicodeUpdatePos(data, endPos, 2);
	// There is a random/undefined byte after the author string (not counted in string length)
	if (msTxos.find(note.m_objectId) != msTxos.end()) {
		auto& msTxo = msTxos[note.m_objectId];
		note.m_text = msTxo.m_text;
		note.m_richtextRunlist = msTxo.m_richtextRunlist;
		m_cellNoteMap[{note.m_rowIndex, note.m_colIndex}] = note;
	}
}

void Sheet::updateCookedFactors() {
	if (m_showPageBreakPreview) {
		// No SCL record
		if (m_sclMagFactor == -1)
			m_cookedPageBreakPreviewMagFactor = 100;  // Yes, 100, not 60, NOT a typo
		else
			m_cookedPageBreakPreviewMagFactor = m_sclMagFactor;
		int zoom = m_cachedNormalViewMagFactor;
		if (!(10 <= zoom && zoom <= 400))
			zoom = m_cookedPageBreakPreviewMagFactor;
		m_cookedNormalViewMagFactor = zoom;
	}
	// Normal view mode
	else {
		// No SCL record
		if (m_sclMagFactor == -1)
			m_cookedNormalViewMagFactor = 100;
		else
			m_cookedNormalViewMagFactor = m_sclMagFactor;
		int zoom = m_cachedPageBreakPreviewMagFactor;
		// VALID, defaults to 60
		if (!zoom)
			zoom = 60;
		else if (!(10 <= zoom && zoom <= 400))
			zoom = m_cookedNormalViewMagFactor;
		m_cookedPageBreakPreviewMagFactor = zoom;
	}
}


void Sheet::unpackCellRangeAddressListUpdatePos(std::vector<std::vector<int>>& outputList,
												const std::string& data, int& pos, int addressSize) const
{
	unsigned short listSize = m_book->readByte<unsigned short>(data, pos, 2);
	pos += 2;
	if (listSize) {
		for (int i = 0; i < listSize; ++i) {
			if (addressSize == 6)
				outputList.push_back({
					m_book->readByte<unsigned short>(data, pos,   2),
					m_book->readByte<unsigned short>(data, pos+2, 2) + 1,
					m_book->readByte<unsigned char>(data,  pos+4, 1),
					m_book->readByte<unsigned char>(data,  pos+5, 1) + 1
				});
			else
				outputList.push_back({
					m_book->readByte<unsigned short>(data, pos,   2),
					m_book->readByte<unsigned short>(data, pos+2, 2) + 1,
					m_book->readByte<unsigned short>(data, pos+4, 2),
					m_book->readByte<unsigned short>(data, pos+6, 2) + 1
				});
			pos += addressSize;
		}
	}
}

double Sheet::unpackRK(const std::string& data) const {
	char flags = data[0];
	// There's a SIGNED 30-bit integer in there
	if (flags & 2) {
		int i = m_book->readByte<int>(data, 0, 4);
		i >>= 2;  // Div by 4 to drop the 2 flag bits
		if (flags & 1)
			return i / 100.0;
		return i;
	}
	// It's the most significant 30 bits of IEEE 754 64-bit FP number
	else {
		double d = m_book->readByte<double>(
			std::string(4, '\0') + (char)(flags & 252) + data.substr(1, 3),
			0, 8
		);
		if (flags & 1)
			return d / 100.0;
		return d;
	}
}

void Sheet::addCellStyle(pugi::xml_node& node, const XF& xf, int rowIndex, int colIndex) {
	auto& cellFont  = m_book->m_fontList[xf.m_fontIndex];
	auto  fontColor = getColor(cellFont.m_color);
	auto  cellColor = getColor(xf.m_background.m_patternColor);
	std::unordered_map<std::string, std::string> styleMap;
	std::unordered_map<std::string, std::string> borderMap;

	// Column style
	addColStyle(node, colIndex);

	// Table parts style
	for (const auto& cRange : m_tableParts) {
		if (cRange[0] <= rowIndex && rowIndex <= cRange[1] &&
			cRange[2] <= colIndex && colIndex <= cRange[3]
		) {
			if (cRange[0] == rowIndex) {
				getTableColor(styleMap["color"], TABLE_COLOR.at(cRange[4]), 0);
				getTableColor(styleMap["background"], TABLE_BACKGROUND.at(cRange[4]), 0);
			}
			/*else {
				getTableColor(styleMap["color"], TABLE_COLOR[cRange[4]], 1);
				if ((rowIndex - cRange[0]) % 2)
					getTableColor(styleMap["background"], TABLE_BACKGROUND[cRange[4]], 1);
				else
					getTableColor(styleMap["background"], TABLE_BACKGROUND[cRange[4]], 2);
			}*/
			break;
		}
	}

	// Cell style
	styleMap["font-size"]   = std::to_string(cellFont.m_height/20) +"px";
	styleMap["font-family"] = "'"+ cellFont.m_name +"'";

	if (!cellColor.empty())
		styleMap["background"] = cellColor;
	if (!fontColor.empty())
		styleMap["color"] = fontColor;

	if (xf.m_alignment.m_horizontalAlign)
		styleMap["text-align"] = CELL_HORZ_ALIGN[xf.m_alignment.m_horizontalAlign];
	if (xf.m_alignment.m_verticalAlign)
		styleMap["vertical-align"] = CELL_VERT_ALIGN[xf.m_alignment.m_verticalAlign];

	borderMap["top"]    = getColor(xf.m_border.m_topColor);
	borderMap["left"]   = getColor(xf.m_border.m_leftColor);
	borderMap["right"]  = getColor(xf.m_border.m_rightColor);
	borderMap["bottom"] = getColor(xf.m_border.m_bottomColor);
	styleMap["border-top"]    = std::to_string(CELL_BORDER_SIZE[xf.m_border.m_topLineStyle]) +"px "+
								CELL_BORDER_TYPE[xf.m_border.m_topLineStyle] +" "+
								(borderMap["top"].empty() ? "#000" : borderMap["top"]);
	styleMap["border-left"]   = std::to_string(CELL_BORDER_SIZE[xf.m_border.m_leftLineStyle]) +"px "+
								CELL_BORDER_TYPE[xf.m_border.m_leftLineStyle] +" "+
								(borderMap["left"].empty() ? "#000" : borderMap["left"]);
	styleMap["border-right"]  = std::to_string(CELL_BORDER_SIZE[xf.m_border.m_rightLineStyle]) +"px "+
								CELL_BORDER_TYPE[xf.m_border.m_rightLineStyle] +" "+
								(borderMap["right"].empty() ? "#000" : borderMap["right"]);
	styleMap["border-bottom"] = std::to_string(CELL_BORDER_SIZE[xf.m_border.m_bottomLineStyle]) +"px "+
								CELL_BORDER_TYPE[xf.m_border.m_bottomLineStyle] +" "+
								(borderMap["bottom"].empty() ? "#000" : borderMap["bottom"]);

	if (xf.m_alignment.m_rotation) {
		if (xf.m_alignment.m_rotation <= 90)
			styleMap["transform"] = "rotate("+
				std::to_string(-xf.m_alignment.m_rotation) +"deg)";
		else if (xf.m_alignment.m_rotation <= 180)
			styleMap["transform"] = "rotate("+
				std::to_string(xf.m_alignment.m_rotation - 90) +"deg)";
	}

	if (xf.m_alignment.m_textDirection)
		styleMap["direction"] = "rtl";

	std::string style;
	for (const auto& sm : styleMap) {
		if (!sm.second.empty())
			style += sm.first + ":" + sm.second + "; ";
	}
	if (!style.empty()) {
		if (node.attribute("style"))
			node.attribute("style").set_value((style + node.attribute("style").value()).c_str());
		else
			node.append_attribute("style") = style.c_str();
	}
}

void Sheet::addRowStyle(pugi::xml_node& node, int rowIndex) {
	if (!m_book->m_addStyle || m_rowinfoMap.find(rowIndex) == m_rowinfoMap.end())
		return;

	std::unordered_map<std::string, std::string> styleMap;
	if (m_rowinfoMap[rowIndex].m_height)
		styleMap["height"] = std::to_string(m_rowinfoMap[rowIndex].m_height/20) +"px";
	if (m_rowinfoMap[rowIndex].m_isHidden)
		styleMap["display"] = "none";

	std::string style;
	for (const auto& sm : styleMap)
		style += sm.first + ":" + sm.second + "; ";
	if (!style.empty())
		node.append_attribute("style") = style.c_str();
}

void Sheet::addColStyle(pugi::xml_node& node, int colIndex) {
	if (!m_book->m_addStyle || m_colinfoMap.find(colIndex) == m_colinfoMap.end())
		return;

	std::unordered_map<std::string, std::string> styleMap;
	if (m_colinfoMap[colIndex].m_width)
		styleMap["min-width"] = std::to_string(m_colinfoMap[colIndex].m_width/45) +"px";
	if (m_colinfoMap[colIndex].m_isHidden)
		styleMap["display"] = "none";

	std::string style;
	for (const auto& sm : styleMap)
		style += sm.first + ":" + sm.second + "; ";
	if (!style.empty())
		node.append_attribute("style") = style.c_str();
}

std::string Sheet::getColor(const XFColor& color) const {
	std::vector<unsigned char> result;
	if (color.m_isRgb) {
		result = color.m_rgb;
	}
	else {
		result = m_book->m_colorMap[color.m_index];
		if (result.empty())
			return "";
	}

	if (color.m_tint < 0) {
		for (auto& c : result)
			c = static_cast<unsigned char>(c * (1 + color.m_tint));
	}
	else if (color.m_tint > 0) {
		for (auto& c : result)
			c = static_cast<unsigned char>(c * (1 - color.m_tint) + (255 - 255 * (1 - color.m_tint)));
	}

	return "rgb("+ std::to_string(result[0]) +", "+
			std::to_string(result[1]) +", "+ std::to_string(result[2]) +")";
}

void Sheet::getTableColor(std::string& style, const std::vector<std::string>& colorMap,
						  int colorIndex) const
{
	if (static_cast<int>(colorMap.size()) > colorIndex && !colorMap[colorIndex].empty())
		style = "#" + colorMap[colorIndex];
}

}  // End namespace
