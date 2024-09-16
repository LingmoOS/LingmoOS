/**
 * @brief     RTF files into HTML сonverter
 * @package   rtf
 * @file      table.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright lvu (https://github.com/lvu/rtf2html)
 * @date      31.07.2016 -- 18.10.2017
 */
#pragma once

#include <list>
#include <vector>
#include <pugixml.hpp>


namespace rtf {

/**
 * @class PtrVec
 * @brief
 *     Pointer vector class
 * @tparam T
 *     Vector element type
 */
template <class T>
class PtrVec: public std::vector<T*> {
public:
	PtrVec() = default;
	~PtrVec();
};

template <class T>
PtrVec<T>::~PtrVec() {
	for (auto it = this->begin(); it != this->end(); ++it)
		delete *it;
}


/**
 * @class TableCell
 * @brief
 *     Wrapper for table cells
 */
class TableCell {
public:
	TableCell() = default;

	/** Cell rowspan value */
	int m_rowspan = 0;
	/** Cell HTML-node */
	pugi::xml_document m_node;
};


/**
 * @class TableCellDef
 * @brief
 *     Wrapper for table cell definitions
 */
class TableCellDef {
public:
	TableCellDef() = default;

	/**
	 * @brief
	 *     Check if right indentation values are equal
	 * @param[in] right
	 *     Right indentation value
	 * @return
	 *     True if equals
	 * @since 1.0
	 */
	bool rightEquals(int right);

	/** Right indentation value */
	int m_right = 0;
	/** Left indentation value */
	int m_left = 0;
	/** If cell is merged */
	bool m_isMerged = false;
	/** If first cell is merged */
	bool m_isFirstMerged = false;
};


/**
 * @class TableRow
 * @brief
 *     Wrapper for table row
 */
class TableRow {
public:
	TableRow() = default;

	/** Cell list */
	PtrVec<TableCell> m_cellList;
	/** Cell definitions list */
	std::list<PtrVec<TableCellDef>>::iterator m_cellDefList;
	/** Left indentation value */
	int m_left = 1000;
};


/**
 * @class Table
 * @brief
 *     Wrapper for table
 */
class Table: public PtrVec<TableRow> {
public:
	Table(char mergingMode);

	/**
	 * @brief
	 *     Create table with rows and cells
	 * @param[out] node
	 *     HTML-node
	 * @since 1.0
	 */
	void make(pugi::xml_node& node);

private:
	/**
	 * @brief
	 *     Copy nodes to main tree
	 * @param[in] from
	 *     Source HTML-node
	 * @param[out] to
	 *     Destination HTML-node
	 * @since 1.0
	 */
	void addSubtree(const pugi::xml_node& from, pugi::xml_node& to) const;

	/** Colspan/rowspan processing mode */
	const char m_mergingMode;
	/** Table HTML-tree */
	pugi::xml_document m_tree;
};

}  // End namespace
