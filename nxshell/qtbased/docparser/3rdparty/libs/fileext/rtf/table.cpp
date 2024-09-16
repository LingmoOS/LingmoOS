/**
 * @brief     RTF files into HTML сonverter
 * @package   rtf
 * @file      table.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright lvu (https://github.com/lvu/rtf2html)
 * @date      31.07.2016 -- 18.10.2017
 */
#include <algorithm>
#include <set>

#include "tools.hpp"

#include "table.hpp"


namespace rtf {

// TableCellDef
bool TableCellDef::rightEquals(int right) {
	return (right == m_right);
}


// Table public:
Table::Table(char mergingMode)
	: m_mergingMode(mergingMode) {}

void Table::make(pugi::xml_node& node) {
	std::set<int> pts;
	std::set<int>::iterator pt, ptp;
	iterator spanRow;
	PtrVec<TableCellDef>::iterator cellDef, prevCellDef;
	PtrVec<TableCell>::iterator cell;
	int colspan;

	for (auto row = begin(); row != end(); ) {
		if ((*row)->m_cellList.empty()) {
			delete *row;
			row = erase(row);
		}
		else {
			pts.insert((*row)->m_left);
			for (cellDef = (*row)->m_cellDefList->begin();
				 cellDef != (*row)->m_cellDefList->end(); ++cellDef
			)
				pts.insert((*cellDef)->m_right);
			++row;
		}
	}

	auto tbl = m_tree.append_child("table");

	// Determine all rowspans and leftsides
	for (auto row = begin(); row != end(); ++row) {
		for (cellDef = (*row)->m_cellDefList->begin(), cell = (*row)->m_cellList.begin();
			cell != (*row)->m_cellList.end();
			++cell, prevCellDef = cellDef++
		) {
			if (cellDef == (*row)->m_cellDefList->begin())
				(*cellDef)->m_left = (*row)->m_left;
			else
				(*cellDef)->m_left = (*prevCellDef)->m_right;

			if ((*cellDef)->m_isFirstMerged) {
				for (spanRow = row, ++spanRow; spanRow != end(); ++spanRow) {
					auto cellDef2 = std::find_if(
									(*spanRow)->m_cellDefList->begin(),
									(*spanRow)->m_cellDefList->end(),
									std::bind2nd(
										std::mem_fun(&TableCellDef::rightEquals),
										(*cellDef)->m_right
									)
								 );
					if (cellDef2 == (*spanRow)->m_cellDefList->end())
						break;
					if (!(*cellDef2)->m_isMerged)
						break;
				}
				(*cell)->m_rowspan = static_cast<int>(spanRow - row);
			}
		}
	}

	// Create rows
	for (auto row = begin(); row != end(); ++row) {
		auto tr = tbl.append_child("tr");
		pt = pts.find((*row)->m_left);
		if (pt != pts.begin()) {
			auto td = tr.append_child("td");
			td.append_attribute("colspan") = std::distance(pts.begin(), pt);
		}

		int colIndex = 0;
		for (cell = (*row)->m_cellList.begin(), cellDef = (*row)->m_cellDefList->begin();
			cell != (*row)->m_cellList.end() && cellDef != (*row)->m_cellDefList->end();
			++cell, ++cellDef
		) {
			ptp = pts.find((*cellDef)->m_right);
			colspan = static_cast<int>(std::distance(pt, ptp));
			pt = ptp;

			if (!(*cellDef)->m_isMerged) {
				auto td = tr.append_child("td");
				addSubtree((*cell)->m_node, td);

				if (colspan > 1) {
					if (m_mergingMode == 0) {
						td.append_attribute("colspan") = colspan;
					}
					else {
						for (int i = 1; i < colspan; ++i) {
							td = tr.append_child("td");
							if (m_mergingMode == 1)
								addSubtree((*cell)->m_node, td);
						}
					}
					colIndex += colspan - 1;
				}
				if ((*cellDef)->m_isFirstMerged && m_mergingMode == 0) {
					td.append_attribute("rowspan") = (*cell)->m_rowspan;
				}
			}
			else if (m_mergingMode != 0) {
				auto td = tr.append_child("td");
				if (m_mergingMode == 1) {
					auto prevTr = tr.previous_sibling();
					auto prevTd = std::next(prevTr.children("td").begin(), colIndex);
					addSubtree(*prevTd, td);
				}
			}
			colIndex++;
		}
	}
	addSubtree(m_tree, node);
}

// Table private:
void Table::addSubtree(const pugi::xml_node& from, pugi::xml_node& to) const {
	std::string style;
	auto parentNode = (from.child("parent")) ? from.child("parent") : from;

	if (parentNode.attribute("style"))
		style = parentNode.attribute("style").value();
	if (tools::xmlChildrenCount(parentNode) == 1 && parentNode.child("span"))
		parentNode = parentNode.child("span");

	if (!style.empty())
		to.append_attribute("style") = style.c_str();

	for (const auto& child : parentNode)
		to.append_copy(child);
}

}  // End namespace
