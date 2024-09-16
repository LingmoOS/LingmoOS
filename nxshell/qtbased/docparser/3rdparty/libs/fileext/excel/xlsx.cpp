/**
 * @brief     Excel files (xls/xlsx) into HTML сonverter
 * @package   excel
 * @file      xlsx.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright python-excel (https://github.com/python-excel/xlrd)
 * @date      02.12.2016 -- 28.01.2018
 */
#include "tools.hpp"

#include "sheet.hpp"

#include "xlsx.hpp"


namespace excel {

/** XLSX max row count */
const long int X12_MAX_ROWS = 1048576; // 2^20
/** XLSX max column count */
const int X12_MAX_COLS      = 16384;   // 2^14
/** Uppercase relations index */
const std::unordered_map<char, int> UPPERCASE_REL_INDEX {
	{'1', 0},  {'3', 0},  {'2', 0},  {'5', 0},  {'4', 0},  {'7', 0},  {'6', 0},
	{'9', 0},  {'8', 0},  {'A', 1},  {'C', 3},  {'B', 2},  {'E', 5},  {'D', 4},
	{'G', 7},  {'F', 6},  {'I', 9},  {'H', 8},  {'K', 11}, {'J', 10}, {'M', 13},
	{'L', 12}, {'O', 15}, {'N', 14}, {'Q', 17}, {'P', 16}, {'S', 19}, {'R', 18},
	{'U', 21}, {'T', 20}, {'W', 23}, {'V', 22}, {'Y', 25}, {'X', 24}, {'Z', 26}
};
/** Horizontal aligment types */
const std::unordered_map<std::string, int> XLSX_HORZ_ALIGN {
	{"",                 0},
	{"general",          0},
	{"left",             1},
	{"center",           2},
	{"right",            3},
	{"fill",             4},
	{"justify",          5},
	{"centerContinuous", 6},
	{"distributed",      7}
};
/** Vertical aligment types */
const std::unordered_map<std::string, int> XLSX_VERT_ALIGN {
	{"",            0},
	{"top",         0},
	{"center",      1},
	{"bottom",      2},
	{"justify",     3},
	{"distributed", 4}
};
/** Border types */
const std::unordered_map<std::string, int> XLSX_BORDER_TYPE {
	{"",                 0},
	{"thin",             1},
	{"medium",           2},
	{"dashed",           3},
	{"dotted",           4},
	{"thick",            5},
	{"double",           6},
	{"hair",             7},
	{"mediumDashed",     8},
	{"dashDot",          9},
	{"mediumDashDot",    10},
	{"dashDotDot",       11},
	{"mediumDashDotDot", 12},
	{"slantDashDot",     13}
};
/** Fill pattern types */
const std::unordered_map<std::string, int> XLSX_FILL_PATTERN {
	{"",                0},
	{"none",            0},
	{"solid",           1},
	{"mediumGray",      2},
	{"darkGray",        3},
	{"lightGray",       4},
	{"darkHorizontal",  5},
	{"darkVertical",    6},
	{"darkDown",        7},
	{"darkUp",          8},
	{"darkGrid",        9},
	{"darkTrellis",     10},
	{"lightHorizontal", 11},
	{"lightVertical",   12},
	{"lightDown",       13},
	{"lightUp",         14},
	{"lightGrid",       15},
	{"lightTrellis",    16},
	{"gray125",         17},
	{"gray0625",        18}
};

// Xlsx
Xlsx::Xlsx(Book* book)
	: m_book(book) {}

void Xlsx::openWorkbookXlsx() {
	X12Styles x12style(m_book);
	x12style.handleTheme();
	x12style.handleStream();

	X12Book x12book(m_book);
	x12book.handleSst();
	x12book.handleRelations();
	x12book.handleProperties();
	x12book.handleStream();
}


// X12General
X12General::X12General(Book* book)
	: m_book(book) {}

std::string X12General::getNodeText(const pugi::xml_node& node) {
	std::string result = node.child_value();
	if (node.attribute("space").value() != std::string("preserve"))
		result = tools::trim(result, "\t\n \r");
	return result;
}

std::string X12General::getTextFromSiIs(const pugi::xml_node& node) {
	std::string result;
	for (const auto& child : node) {
		std::string tag = child.name();
		if (tag == "t")
			result += getNodeText(child);
		else if (tag == "r") {
			for (const auto& tNode : child) {
				if (tNode.name() == std::string("t"))
					result += getNodeText(tNode);
			}
		}
	}
	return result;
}

void X12General::hexToColor(std::vector<unsigned char>& colorMap, const std::string& color, int offset) {
	for (int i = 0; i < 6; i += 2) {
		unsigned long c = std::stoul(color.substr(offset + i, 2), nullptr, 16);
		colorMap.emplace_back(static_cast<unsigned char>(c));
	}
}


// X12Book public:
X12Book::X12Book(Book* book)
: X12General(book) {
	m_book->m_sheetCount = 0;
}

void X12Book::handleSst() {
	pugi::xml_document tree;
	Ooxml::extractFile(m_book->m_fileName, "xl/sharedstrings.xml", tree);

	for (const auto& node : tree.select_nodes("//si"))
		m_book->m_sharedStrings.emplace_back(getTextFromSiIs(node.node()));
}

void X12Book::handleRelations() {
	pugi::xml_document tree;
	Ooxml::extractFile(m_book->m_fileName, "xl/_rels/workbook.xml.rels", tree);

	for (const auto& node : tree.child("Relationships")) {
		std::string relId   = node.attribute("Id").value();
		std::string target  = node.attribute("Target").value();
		std::string relType = node.attribute("Type").value();
		relType = relType.substr(relType.find_last_of("/") + 1);

		m_relIdToType[relId] = relType;
		if (target[0] == '/')
			m_relIdToPath[relId] = target.substr(1);  // Drop the `/`
		else
			m_relIdToPath[relId] = "xl/" + target;
	}
}

void X12Book::handleProperties() {
	if (!m_book->m_addStyle)
		return;

	pugi::xml_document tree;
	Ooxml::extractFile(m_book->m_fileName, "docprops/core.xml", tree);

	for (const auto& node : tree.select_nodes("//dc:creator"))
		m_book->m_properties["creator"] = node.node().child_value();
	for (const auto& node : tree.select_nodes("//cp:lastModifiedBy"))
		m_book->m_properties["last_modified_by"] = node.node().child_value();
	for (const auto& node : tree.select_nodes("//dcterms:created"))
		m_book->m_properties["created"] = node.node().child_value();
	for (const auto& node : tree.select_nodes("//dcterms:modified"))
		m_book->m_properties["modified"] = node.node().child_value();
	m_book->m_userName = m_book->m_properties["last_modified_by"].empty() ?
						 m_book->m_properties["creator"] :
						 m_book->m_properties["last_modified_by"];
}

void X12Book::handleStream() {
	m_book->m_biffVersion = 80;
	Formatting formatting(m_book);
	formatting.initializeBook();

	pugi::xml_document tree;
	Ooxml::extractFile(m_book->m_fileName, "xl/workbook.xml", tree);

	for (const auto& node : tree.select_nodes("//definedNames")) {
		handleDefinedNames(node.node());
	}
	for (const auto& node : tree.select_nodes("//workbookPr")) {
		std::string date = node.node().attribute("date1904").value();
		m_book->m_dateMode = (date == "1" || date == "true" || date == "on") ? 1 : 0;
	}
	for (const auto& node : tree.select_nodes("//sheet")) {
		handleSheet(node.node());
	}
}

// X12Book private:
void X12Book::handleDefinedNames(const pugi::xml_node& node) {
	for (const auto& child : node) {
		Name name(m_book);
		name.m_nameIndex   = m_book->m_nameObjList.size();
		name.m_name        = child.attribute("name").value();
		name.m_rawFormula  = "";  // Compiled bytecode formula - not in XLSX
		name.m_formulaText = getNodeText(child);
		/*map_attributes(_defined_name_attribute_map, node, name)*/
		if (name.m_scope != 0)
			name.m_scope = -1;  // Global

		try {
			if (name.m_name.substr(0, 6) == "_xlnm.")
				name.m_builtIn = 1;
		}
		catch (...) {}
		m_book->m_nameObjList.push_back(name);
	}
	createNameMap();
}

void X12Book::handleSheet(const pugi::xml_node& node) {
	size_t sheetIndex = m_book->m_sheetCount;
	std::string relId = node.attribute("r:id").value();
	int sheetId       = node.attribute("sheetId").as_int();
	std::string name  = node.attribute("name").value();
	std::string state = node.attribute("state").value();

	std::string relType = m_relIdToType[relId];
	std::string target  = m_relIdToPath[relId];
	if (relType != "worksheet")
		return;

	if (state == "hidden")
		m_book->m_sheetVisibility.push_back(1);
	else if (state == "veryHidden")
		m_book->m_sheetVisibility.push_back(2);
	else
		m_book->m_sheetVisibility.push_back(0);

	// Add sheet information
//	auto div = m_book->m_htmlTree.append_child("div");
//	div.append_attribute("id") = ("tabC"+ std::to_string(sheetIndex + 1)).c_str();
//    auto table = m_book->m_htmlTree.append_child("p");

    m_book->m_sheetList.emplace_back(m_book, -1, name, sheetIndex, m_book->m_contentText);
	m_book->m_sheetNames.push_back(name);
	m_book->m_sheetCount += 1;
	m_sheetTargets.push_back(target);
	m_sheetIds.push_back(sheetId);

	Sheet& sheet = m_book->m_sheetList[m_book->m_sheetList.size() - 1];
	sheet.m_maxRowCount = X12_MAX_ROWS;
	sheet.m_maxColCount = X12_MAX_COLS;

	size_t found = target.find_last_of("/");
	std::string relFileName = "xl/worksheets/_rels/"+ target.substr(found + 1) +".rels";

	X12Sheet x12sheet(m_book, sheet);
	x12sheet.handleRelations(relFileName);
	x12sheet.handleStream(target);

	for (const auto& rel : x12sheet.m_relIdToType) {
		if (rel.second == "comments") {
			std::string commentFileName = x12sheet.m_relIdToPath[rel.first];
			if (!commentFileName.empty())
				x12sheet.handleComments(commentFileName);
		}
	}

//	if (m_book->m_extractImages) {
//		x12sheet.getDrawingRelationshipMap(static_cast<int>(sheetIndex));
//        x12sheet.handleImages(static_cast<int>(sheetIndex), m_book->m_htmlTree);
//	}

//	sheet.tidyDimensions();
}

void X12Book::createNameMap() {
	m_book->m_nameScopeMap.clear();
	m_book->m_nameMap.clear();
	std::map<std::string, std::vector<std::pair<Name, int>>> nameMap;
	size_t nameCount = m_book->m_nameObjList.size();
	for (size_t i = 0; i < nameCount; ++i) {
		Name& name = m_book->m_nameObjList[i];
		std::string lcName = name.m_name;
		std::transform(lcName.begin(), lcName.end(), lcName.begin(), ::tolower);

		std::pair<std::string, int> key {lcName, name.m_scope};
		m_book->m_nameScopeMap.erase(key);
		m_book->m_nameScopeMap.emplace(key, name);

		nameMap[lcName].emplace_back(name, static_cast<int>(i));
	}
	for (auto& map : nameMap) {
		std::sort(map.second.begin(), map.second.end());
		for (const auto& obj : map.second)
			m_book->m_nameMap[map.first].emplace_back(obj.first);
	}
}


// X12Sheet public:
X12Sheet::X12Sheet(Book* book, Sheet& sheet)
	: X12General(book), m_sheet(sheet) {}

void X12Sheet::handleRelations(const std::string& fileName) {
	pugi::xml_document tree;
	Ooxml::extractFile(m_book->m_fileName, fileName, tree);

	for (const auto& node : tree.child("Relationships")) {
		std::string relId   = node.attribute("Id").value();
		std::string target  = node.attribute("Target").value();
		std::string relType = node.attribute("Type").value();
		relType             = relType.substr(relType.find_last_of("/") + 1);

		size_t found = fileName.find_last_of("/");
		std::string rels_fname = "xl/worksheets/_rels/"+ fileName.substr(found + 1) +".rels";

		m_relIdToType[relId] = relType;
		// normpath(join('xl/worksheets', target))
		m_relIdToPath[relId] = "xl/"+ target.substr(target.find_first_of("/") + 1);
	}
}

void X12Sheet::handleStream(const std::string& fileName) {
	pugi::xml_document tree;
	Ooxml::extractFile(m_book->m_fileName, fileName, tree);

	for (const auto& node : tree.select_nodes("//mergeCell"))
		handleMergedCells(node.node());
	for (const auto& node : tree.select_nodes("//tablePart"))
		handleTableParts(node.node());
	for (const auto& node : tree.select_nodes("//col"))
		handleCol(node.node());
	for (const auto& node : tree.select_nodes("//row"))
		handleRow(node.node());
	for (const auto& node : tree.select_nodes("//dimension"))
		handleDimensions(node.node());
}

void X12Sheet::handleComments(const std::string& fileName) {
	pugi::xml_document tree;
	Ooxml::extractFile(m_book->m_fileName, fileName, tree);

	std::vector<std::string> authors;
	for (const auto& node : tree.select_nodes("//author")) {
		authors.push_back(node.node().child_value());
	}
	for (const auto& node : tree.select_nodes("//comment")) {
		auto nd = node.node();
		Note note;
		note.m_author = authors[nd.attribute("authorId").as_int()];
		cellNameToIndex(nd.attribute("ref").value(), note.m_rowIndex, note.m_colIndex);
		for (const auto& child : nd.select_nodes("//t"))
			note.m_text += getNodeText(child.node()) +" ";
		m_sheet.m_cellNoteMap[{note.m_rowIndex, note.m_colIndex}] = note;
	}
}

void X12Sheet::getDrawingRelationshipMap(int sheetIndex) {
	pugi::xml_document tree;
	Ooxml::extractFile(m_book->m_fileName,
					   "xl/drawings/_rels/drawing"+ std::to_string(sheetIndex + 1)+".xml.rels", tree);

	for (const auto& node : tree.child("Relationships")) {
		auto id = node.attribute("Id").value();
		if (id)
			m_drawingRelationshipMap[id] = node.attribute("Target").value();
	}
}

#if 0
void X12Sheet::handleImages(int sheetIndex, pugi::xml_node& htmlNode) {
	pugi::xml_document tree;
	Ooxml::extractFile(m_book->m_fileName,
					   "xl/drawings/drawing"+ std::to_string(sheetIndex + 1)+".xml", tree);

	for (const auto& node : tree.child("xdr:wsDr")) {
		std::string imageId = node.select_node(".//a:blip").node().attribute("r:embed").value();

		// This image does not have image id
		if (m_drawingRelationshipMap.find(imageId) == m_drawingRelationshipMap.end())
			continue;
		std::string path = "xl/" + m_drawingRelationshipMap[imageId].substr(3);

		// Load image
		std::string ext = path.substr(path.find_last_of('.') + 1);
		std::string imageData;
		Ooxml::extractFile(m_book->m_fileName, path, imageData);
		m_book->m_imageList.emplace_back(std::make_pair(std::move(imageData), ext));

		// Add image node
		auto imageNode = htmlNode.append_child("p").append_child("img");
		imageNode.append_attribute("data-tag") = m_book->m_imageList.size() - 1;

		// Add style
		if (m_book->m_addStyle)
			getImageSize(node, imageNode);
	}
}
#endif

// X12Sheet private:
void X12Sheet::handleCol(const pugi::xml_node& node) {
	if (!m_book->m_addStyle)
		return;

	int firstColIndex = node.attribute("min").as_int();
	int lastColIndex  = node.attribute("max").as_int();
	Colinfo colinfo;
	colinfo.m_width         = static_cast<int>(node.attribute("width").as_double() * 45 * 6);
	colinfo.m_isHidden      = node.attribute("hidden");
	//colinfo.m_bitFlag     = ???
	colinfo.m_outlineLevel  = node.attribute("outlineLevel").as_int();
	colinfo.m_isCollapsed   = node.attribute("collapsed");

	for (int i = firstColIndex; i <= lastColIndex; ++i)
		m_sheet.m_colinfoMap[i-1] = colinfo;
}

void X12Sheet::handleRow(const pugi::xml_node& node) {
    int rowNumber = node.attribute("r").as_int();
    bool explicitRowNumber;
    // Yes, it's optional
    if (!rowNumber) {
        m_rowIndex       += 1;
        explicitRowNumber = false;
    }
    else {
        m_rowIndex        = rowNumber - 1;
        explicitRowNumber = true;
    }

    // Read ROWINFO data
    if (m_book->m_addStyle) {
        Rowinfo rowinfo;
        rowinfo.m_height                   = node.attribute("ht").as_int() * 20;
        //rowinfo.m_hasDefaultHeight         = ???
        rowinfo.m_outlineLevel             = node.attribute("outlineLevel").as_int();
        //rowinfo.m_isOutlineGroupStartsEnds = ???
        rowinfo.m_isHidden                 = node.attribute("hidden");
        //rowinfo.m_isHeightMismatch         = ???
        //rowinfo.m_hasAdditionalSpaceAbove  = ???
        //rowinfo.m_hasAdditionalSpaceBelow  = ???

        m_sheet.m_rowinfoMap[rowNumber-1] = rowinfo;
    }

    // Read cell data
    int colIndex = -1;
    for (const auto& cellNode: node) {
        try {
            std::string cellName = cellNode.attribute("r").value();
            // Yes, it's optional
            if (cellName.empty()) {
                colIndex += 1;
            }
            else {
                // Extract column index from cell name (`A<row number>` => `0`, `Z` =>`25`, `AA` => `26`)
                colIndex = 0;
                char charIndex = -1;
                for (const auto& c : cellName) {
                    charIndex += 1;
                    if (c == '$')
                        continue;
                    if (UPPERCASE_REL_INDEX.find(c) == UPPERCASE_REL_INDEX.end())
                        throw std::logic_error(
                                "Unexpected character "+ std::string(1, c) +" in cell name "+ cellName
                                );

                    int lv = UPPERCASE_REL_INDEX.at(c);
                    if (lv) {
                        colIndex = colIndex * 26 + lv;
                    }
                    // Start of row number can't be '\0'
                    else {
                        colIndex--;
                        break;
                    }
                }

                if (explicitRowNumber && cellName.substr(charIndex) != std::to_string(rowNumber))
                    throw std::logic_error(
                            "Cell name "+ cellName +" but row number is "+ std::to_string(rowNumber)
                            );
            }

            int xfIndex = cellNode.attribute("s").as_int()+1;
            std::string cellType = cellNode.attribute("t").value();
            std::string value;
            std::string formula;
            // n = number. Most frequent type. <v> child contains plain text which can go straight
            // into float() OR there's no text in which case it's a BLANK cell
            if (cellType.empty()) {
                for (const auto& child : cellNode) {
                    std::string childName = child.name();
                    if (childName == "v")
                        value = child.child_value();
                    else if (childName == "f")
                        formula = getNodeText(child);
                    else
                        throw std::logic_error("Unexpected tag " + childName);
                }
                if (value.empty()) {
                    //				if (m_book->m_addStyle)
                    //					m_sheet.putCell(m_rowIndex, colIndex, "", xfIndex);
                }
                else {
                    m_sheet.append(value);
                    //				m_sheet.putCell(m_rowIndex, colIndex, value, xfIndex);
                }
            }
            // s = index into shared string table. 2nd most frequent type <v> child contains plain
            // text which can go straight into int()
            else if (cellType == "s") {
                for (const auto& child : cellNode) {
                    std::string childName = child.name();
                    if (childName == "v")
                        value = child.child_value();
                    // Formula not expected here, but gnumeric does it
                    else if (childName == "f")
                        formula = child.child_value();
                    else
                        throw std::logic_error(
                                "Cell type "+ cellType +" has unexpected child <"+ childName +"> at rowx="+
                                std::to_string(m_rowIndex) +" colx="+ std::to_string(colIndex)
                                );
                }
                // <c r="A1" t="s"/>
                if (value.empty()) {
                    //				if (m_book->m_addStyle)
                    //					m_sheet.putCell(m_rowIndex, colIndex, "", xfIndex);
                }
                else {
                    //				m_sheet.putCell(
                    //					m_rowIndex, colIndex, m_book->m_sharedStrings[stoi(value)], xfIndex
                    //				);
                    m_sheet.append(m_book->m_sharedStrings[stoi(value)]);
                }
            }
            // str = string result from formula. Should have <f> (formula) child; however in one file,
            // all text cells are str with no formula. <v> child can contain escapes
            else if (cellType == "str") {
                for (const auto& child : cellNode) {
                    std::string childName = child.name();
                    if (childName == "v")
                        value = getNodeText(child);
                    else if (childName == "f")
                        formula = getNodeText(child);
                    else
                        throw std::logic_error(
                                "Cell type "+ cellType +" has unexpected child <"+ childName +"> at rowx="+
                                std::to_string(m_rowIndex) +" colx="+ std::to_string(colIndex)
                                );
                }
                m_sheet.append(value);
                //			m_sheet.putCell(m_rowIndex, colIndex, value, xfIndex);
            }
            // b = boolean. <v> child contains "0" or "1". Maybe data should be converted with
            // cnv_xsd_boolean; ECMA standard is silent; Excel 2007 writes 0 or 1
            else if (cellType == "b") {
                for (const auto& child : cellNode) {
                    std::string childName = child.name();
                    if (childName == "v")
                        value = child.child_value();
                    else if (childName == "f")
                        formula = getNodeText(child);
                    else
                        throw std::logic_error(
                                "Cell type "+ cellType +" has unexpected child <"+ childName +"> at rowx="+
                                std::to_string(m_rowIndex) +" colx="+ std::to_string(colIndex)
                                );
                }
                m_sheet.append(value);
                //			m_sheet.putCell(m_rowIndex, colIndex, value, xfIndex);
            }
            // e = error. <v> child contains e.g. "#REF!"
            else if (cellType == "e") {
                for (const auto& child : cellNode) {
                    std::string childName = child.name();
                    if (childName == "v")
                        value = child.child_value();
                    else if (childName == "f")
                        formula = getNodeText(child);
                    else
                        throw std::logic_error(
                                "Cell type "+ cellType +" has unexpected child <"+ childName +"> at rowx="+
                                std::to_string(m_rowIndex) +" colx="+ std::to_string(colIndex)
                                );
                }
                m_sheet.append(std::to_string(ERROR_CODE_FROM_TEXT.at(value)));
                //			m_sheet.putCell(m_rowIndex, colIndex, std::to_string(ERROR_CODE_FROM_TEXT.at(value)),
                //							xfIndex);
            }
            // Not expected in files produced by Excel. It's a way of allowing 3rd party s/w to write
            // text (including rich text) cells without having to build a shared string table (SST)
            else if (cellType == "inlineStr") {
                for (const auto& child : cellNode) {
                    std::string childName = child.name();
                    if (childName == "is")
                        value = getTextFromSiIs(child);
                    else if (childName == "v")
                        value = child.child_value();
                    else if (childName == "f")
                        formula = child.child_value();
                    else
                        throw std::logic_error(
                                "Cell type "+ cellType +" has unexpected child <"+ childName +"> at rowx="+
                                std::to_string(m_rowIndex) +" colx="+ std::to_string(colIndex)
                                );
                }
                if (value.empty()) {
                    //				if (m_book->m_addStyle)
                    //					m_sheet.putCell(m_rowIndex, colIndex, "", xfIndex);
                }
                else {
                    m_sheet.append(value);
                    //				m_sheet.putCell(m_rowIndex, colIndex, value, xfIndex);
                }
            }
            else {
                throw std::logic_error(
                            "Unknown cell type "+ cellType +" in rowx="+ std::to_string(m_rowIndex) +
                            " colx="+ std::to_string(colIndex)
                            );
            }
        } catch (...) {
            return;
        }
	}
}

void X12Sheet::handleDimensions(const pugi::xml_node& node) {
	std::string ref = node.attribute("ref").value();
	if (!ref.empty()) {
		size_t found = ref.find_last_of(":");
		std::string lastRef = ref.substr(found + 1);  // Example: "Z99"
		int rowIndex, colIndex;
		cellNameToIndex(lastRef, rowIndex, colIndex, true);
		m_sheet.m_dimensionRowCount = rowIndex + 1;
		if (colIndex)
			m_sheet.m_dimensionColCount = colIndex + 1;
	}
}

void X12Sheet::handleMergedCells(const pugi::xml_node& node) {
	// The ref attribute should be a cell range like "B1:D5"
	std::string ref = node.attribute("ref").value();
	if (!ref.empty()) {
		size_t found = ref.find_last_of(":");
		std::string firstRef = ref.substr(0, found);
		std::string lastRef  = ref.substr(found + 1);
		int firstRowIndex, lastRowIndex , firstColIndex, lastColIndex;
		cellNameToIndex(firstRef, firstRowIndex, firstColIndex);
		cellNameToIndex(lastRef,  lastRowIndex,  lastColIndex);
		m_sheet.m_mergedCells.push_back({
			firstRowIndex, lastRowIndex + 1,
			firstColIndex, lastColIndex + 1
		});
	}
}

void X12Sheet::handleTableParts(const pugi::xml_node& node) {
	// Get file path
	std::string relId   = node.attribute("r:id").value();
	std::string relType = m_relIdToType[relId];
	std::string target  = m_relIdToPath[relId];
	if (relType != "table")
		return;

	size_t found = target.find_last_of("/");
	std::string relFileName = "xl/tables/"+ target.substr(found + 1);

	// Extract file data
	pugi::xml_document tree;
	Ooxml::extractFile(m_book->m_fileName, relFileName, tree);

	auto nd = tree.child("table");
	std::string ref = nd.attribute("ref").value();
	std::string styleName = nd.child("tableStyleInfo").attribute("name").value();

	if (!ref.empty()) {
		// Cell ranges
		size_t found = ref.find_last_of(":");
		std::string firstRef = ref.substr(0, found);
		std::string lastRef  = ref.substr(found + 1);
		int firstRowIndex, lastRowIndex , firstColIndex, lastColIndex;
		cellNameToIndex(firstRef, firstRowIndex, firstColIndex);
		cellNameToIndex(lastRef,  lastRowIndex,  lastColIndex);
		// Style id
		auto pos = styleName.find_first_of("0123456789");
        if (pos == std::string::npos)
            return;

		int type = 100;
		if (styleName.find("Medium") != std::string::npos)
			type = 200;
		else if (styleName.find("Dark") != std::string::npos)
			type = 300;

		m_sheet.m_tableParts.push_back({
			firstRowIndex, lastRowIndex + 1,
			firstColIndex, lastColIndex + 1,
			stoi(styleName.substr(pos)) + type
		});
	}
}

void X12Sheet::cellNameToIndex(const std::string& cellName, int& rowIndex,
							   int& colIndex, bool noCol)
{
	colIndex = 0;
	char charIndex = -1;

	for (const auto& c : cellName) {
		charIndex += 1;
		if (UPPERCASE_REL_INDEX.find(c) == UPPERCASE_REL_INDEX.end())
			throw std::logic_error(
				"Unexpected character "+ std::string(1, c) +" in cell name "+ cellName
			);

		int lv = UPPERCASE_REL_INDEX.at(c);
		if (lv) {
			colIndex = colIndex * 26 + lv;
		}
		// Start of row number can't be '\0'
		else {
			if (charIndex == 0) {
				// There was no col marker
				if (noCol) {
					colIndex = -1;
					break;
				}
				else {
					throw std::logic_error("Missing col in cell name "+ cellName);
				}
			}
			else {
				colIndex--;
				break;
			}
		}
	}

	rowIndex = stoi(cellName.substr(charIndex)) - 1;
}

void X12Sheet::getImageSize(const pugi::xml_node& xmlNode, pugi::xml_node& htmlNode) const {
	auto child = xmlNode.select_node(".//a:xfrm").node().child("a:ext");
	if (!child)
		return;

	int width  = child.attribute("cx").as_int() / 9525;  // EMUS_PER_PIXEL
	int height = child.attribute("cy").as_int() / 9525;  // EMUS_PER_PIXEL

	std::string style = "width: " + std::to_string(width) + "px;";
	style += "height: " + std::to_string(height) + "px;";
	htmlNode.append_attribute("style") = style.c_str();
}


// X12Styles public:
X12Styles::X12Styles(Book* book)
: X12General(book) {
	for (int i = 14; i < 23; ++i)
		m_isDateFormat[i] = 1;
	for (int i = 45; i < 48; ++i)
		m_isDateFormat[i] = 1;
	// Dummy entry for XF 0 in case no Styles section
	m_book->m_xfIndexXlTypeMap[0] = 0;
}

void X12Styles::handleTheme() {
	if (!m_book->m_addStyle)
		return;

	pugi::xml_document tree;
	Ooxml::extractFile(m_book->m_fileName, "xl/theme/theme1.xml", tree);

	int colorIndex = -2;
	for (const auto& node : tree.select_nodes("//a:sysClr")) {
		hexToColor(m_book->m_colorMap[colorIndex], node.node().attribute("lastClr").value(), 0);
		colorIndex--;
	}
	colorIndex++;
	for (const auto& node : tree.select_nodes("//a:srgbClr")) {
		hexToColor(m_book->m_colorMap[colorIndex], node.node().attribute("val").value(), 0);
		colorIndex--;
	}
}

void X12Styles::handleStream() {
	if (!m_book->m_addStyle)
		return;

	pugi::xml_document tree;
	Ooxml::extractFile(m_book->m_fileName, "xl/styles.xml", tree);

	int fontIndex = 0;
	for (const auto& node : tree.select_nodes("//numFmt"))
		handleNumFormat(node.node());
	for (const auto& node : tree.select_nodes("//font"))
		handleFont(node.node(), fontIndex++);
	for (const auto& node : tree.select_nodes("//border"))
		handleBorder(node.node());
	for (const auto& node : tree.select_nodes("//patternFill"))
		handleBackground(node.node());
	for (const auto& node : tree.select_nodes("//xf"))
		handleXf(node.node());
}

// X12Styles private:
void X12Styles::handleNumFormat(const pugi::xml_node& node) {
	std::string formatCode = node.attribute("formatCode").value();
	int numFormatId = node.attribute("numFmtId").as_int();
	bool isDate     = Formatting::isDateFormattedString(formatCode);
	m_isDateFormat[numFormatId] = isDate;
	m_book->m_formatMap.emplace(numFormatId, Format(numFormatId, isDate + 2, formatCode));
}

void X12Styles::handleFont(const pugi::xml_node& node, int fontIndex) {
	Font f;
	f.m_fontIndex = fontIndex;

	for (const auto& child : node) {
		std::string childName = child.name();
		if (childName == "name")
			f.m_name = child.attribute("val").value();
		else if (childName == "sz")
			f.m_height = child.attribute("val").as_int() * 20;
		else if (childName == "color")
			extractColor(child, f.m_color);
		else if (childName == "vertAlign") {
			std::string val = child.attribute("val").value();
			if (val == "superscript")
				f.m_escapement = 1;
			if (val == "subscript")
				f.m_escapement = 2;
		}
		else if (childName == "family")
			f.m_family = child.attribute("val").as_int();
		else if (childName == "b")
			f.m_isBold = true;
		else if (childName == "i")
			f.m_isItalic = true;
		else if (childName == "u") {
			f.m_isUnderlined = true;
			std::string value = child.attribute("val").value();
			if (value == "double" || value == "doubleAccounting")
				f.m_underlineType = 2;
			else
				f.m_underlineType = 1;
		}
		else if (childName == "strike")
			f.m_isStruckOut = true;
	}

	m_book->m_fontList.emplace_back(f);
}

void X12Styles::handleBorder(const pugi::xml_node& node) {
	XFBorder border;

	border.m_diagDown = node.attribute("diagonalDown");
	border.m_diagUp   = node.attribute("diagonalUp");

	for (const auto& child : node) {
		std::string childName = child.name();
		if (childName == "left") {
			border.m_leftLineStyle = XLSX_BORDER_TYPE.at(child.attribute("style").value());
			extractColor(child.first_child(), border.m_leftColor);
		}
		else if (childName == "right") {
			border.m_rightLineStyle = XLSX_BORDER_TYPE.at(child.attribute("style").value());
			extractColor(child.first_child(), border.m_rightColor);
		}
		else if (childName == "top") {
			border.m_topLineStyle = XLSX_BORDER_TYPE.at(child.attribute("style").value());
			extractColor(child.first_child(), border.m_topColor);
		}
		else if (childName == "bottom") {
			border.m_bottomLineStyle = XLSX_BORDER_TYPE.at(child.attribute("style").value());
			extractColor(child.first_child(), border.m_bottomColor);
		}
		else if (childName == "diagonal") {
			border.m_diagLineStyle = XLSX_BORDER_TYPE.at(child.attribute("style").value());
			extractColor(child.first_child(), border.m_diagColor);
		}
	}

	m_book->m_borderList.emplace_back(border);
}

void X12Styles::handleBackground(const pugi::xml_node& node) {
	XFBackground background;

	background.m_fillPattern = XLSX_FILL_PATTERN.at(node.attribute("patternType").value());

	for (const auto& child : node) {
		std::string childName = child.name();
		if (childName == "fgColor")
			extractColor(child, background.m_patternColor);
		else if (childName == "bgColor")
			extractColor(child, background.m_backgroundColor);
	}

	m_book->m_backgroundList.emplace_back(background);
}

void X12Styles::handleXf(const pugi::xml_node& node) {
	int xfIndex;
	std::string parentName = node.parent().name();
	if (parentName == "cellStyleXfs")
		xfIndex = m_xfCount[0]++;
	else if (parentName == "cellXfs")
		xfIndex = m_xfCount[1]++;

	XF xf;
	int numFormatId = node.attribute("numFmtId").as_int();
	xf.m_fontIndex  = node.attribute("fontId").as_int();
	xf.m_formatKey  = numFormatId;

	//xf.m_protection.m_isCellLocked  = ???
	xf.m_protection.m_isFormulaHidden = node.child("protection").attribute("hidden");
	//xf.m_isStyle          = ???
	//xf.m_lotusPrefix      = ???
	//xf.m_parentStyleIndex = ???

	auto align = node.child("alignment");
	if (align) {
		xf.m_alignment.m_horizontalAlign = XLSX_HORZ_ALIGN.at(align.attribute("horizontal").value());
		xf.m_alignment.m_isTextWrapped   = align.attribute("wrapText").as_int();
		xf.m_alignment.m_verticalAlign   = XLSX_VERT_ALIGN.at(align.attribute("vertical").value());
		xf.m_alignment.m_indentLevel     = align.attribute("indent").as_int();
		xf.m_alignment.m_isShrinkToFit   = align.attribute("shrinkToFit");
		//xf.m_alignment.m_textDirection = ???
		xf.m_alignment.m_rotation        = align.attribute("textRotation").as_int();
	}

	//xf.m_formatFlag   = ???
	xf.m_fontFlag       = true; // Need to check
	xf.m_alignmentFlag  = node.attribute("applyAlignment");
	xf.m_borderFlag     = node.attribute("applyBorder");
	xf.m_backgroundFlag = node.attribute("applyFill");
	xf.m_protectionFlag = node.attribute("applyProtection");

	xf.m_border     = m_book->m_borderList[node.attribute("borderId").as_int()];
	xf.m_background = m_book->m_backgroundList[node.attribute("fillId").as_int()];

	m_book->m_xfList.push_back(xf);
	m_book->m_xfCount += 1;
	m_book->m_xfIndexXlTypeMap[xfIndex] = m_isDateFormat[numFormatId] + 2;
}

void X12Styles::extractColor(const pugi::xml_node& node, XFColor& color) {
	color.m_tint = node.attribute("tint").as_double();
	if (node.attribute("indexed"))
		color.m_index = node.attribute("indexed").as_int();
	else if (node.attribute("theme"))
		color.m_index = -1 - node.attribute("theme").as_int();
	else if (node.attribute("auto"))
		color.m_index = 0;
	else if (node.attribute("rgb")) {
		color.m_isRgb = true;
		hexToColor(color.m_rgb, node.attribute("rgb").value(), 2);
	}
}

}  // End namespace
