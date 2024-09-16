/**
 * @brief     Excel files (xls/xlsx) into HTML сonverter
 * @package   excel
 * @file      formula.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright python-excel (https://github.com/python-excel/xlrd)
 * @date      02.12.2016 -- 18.10.2017
 */
#pragma once

#include <string>
#include <vector>

#include "book.hpp"
#include "format.hpp"


namespace excel {

/** Operand kind list */
enum {
	oBOOL = 3,  ///< Boolean
	oERR  = 4,  ///< Error
	oMSNG = 5,  ///< Message
	oNUM  = 2,  ///< Number
	oREF  = -1, ///< Reference
	oREL  = -2, ///< Relative
	oSTRG = 1,  ///< String
	oUNK  = 0   ///< Unknown
};

class Name;
class Book;
class Operand;
class Ref3D;

/**
 * @class Formula
 * @brief
 *     Sheet formula information
 */
class Formula {
public:
	/**
	 * @param[in] book
	 *     Pointer to parent Book object
	 * @since 1.0
	 */
	Formula(Book* book);

	/**
	 * @brief
	 *     Process formulas for NAME object
	 * @param[in] name
	 *     Reference to NAME object
	 * @param[in] nameIndex
	 *     Name index
	 * @param[in] level
	 *     Recursion nesting level
	 * @since 1.0
	 */
	void evaluateFormula(Name& name, int nameIndex, int level = 0);

	/** Pointer to parent Book object */
	Book* m_book;

private:
	/**
	 * @brief
	 *     Execute binary operation
	 * @details
	 *      Value  | Representation
	 *     :-----: | --------------
	 *      tAdd   | `0x03: (_arith_argdict, oNUM,  opr.add,     30, '+')`
	 *      tSub   | `0x04: (_arith_argdict, oNUM,  opr.sub,     30, '-')`
	 *      tMul   | `0x05: (_arith_argdict, oNUM,  opr.mul,     40, '*')`
	 *      tDiv   | `0x06: (_arith_argdict, oNUM,  opr.truediv, 40, '/')`
	 *     tPower  | `0x07: (_arith_argdict, oNUM,  _opr_pow,    50, '^')`
	 *     tConcat | `0x08: (_strg_argdict,  oSTRG, opr.add,     20, '&')`
	 *       tLT   | `0x09: (_cmp_argdict,   oBOOL, _opr_lt,     10, '<')`
	 *       tLE   | `0x0A: (_cmp_argdict,   oBOOL, _opr_le,     10, '<=')`
	 *       tEQ   | `0x0B: (_cmp_argdict,   oBOOL, _opr_eq,     10, '=')`
	 *       tGE   | `0x0C: (_cmp_argdict,   oBOOL, _opr_ge,     10, '>=')`
	 *       tGT   | `0x0D: (_cmp_argdict,   oBOOL, _opr_gt,     10, '>')`
	 *       tNE   | `0x0E: (_cmp_argdict,   oBOOL, _opr_ne,     10, '<>')`
	 * @param[in] code
	 *     Opeartion code
	 * @param[in,out] stack
	 *     Stack of operands
	 * @since 1.0
	 */
	void binOperation(int code, std::vector<Operand>& stack) const;

	/**
	 * @brief
	 *     Execute unary operation
	 * @details
	 *        Value    | Representation
	 *     :---------: | --------------
	 *     Unary plus  | `0x12: (lambda x: x,         70, '+', '')`
	 *     Unary minus | `0x13: (lambda x: -x,        70, '-', '')`
	 *       Percent   | `0x14: (lambda x: x / 100.0, 60, '',  '%')`
	 * @param[in] code
	 *     Opeartion code
	 * @param[in,out] stack
	 *     Stack of operands
	 * @param[in] resultKind
	 *     Kind of result operand
	 * @since 1.0
	 */
	void unaryOperation(int code, std::vector<Operand>& stack, int resultKind) const;

	/**
	 * @brief
	 *     Execute range operations
	 * @details
	 *        Value    | Representation
	 *     :---------: | --------------
	 *     tIsectFuncs | `0: (max, min, max, min, max, min)`
	 *     tRangeFuncs | `1: (min, max, min, max, min, max)`
	 * @param[out] coords
	 *     Cell coords
	 * @param[in] leftValue
	 *     Left-top end of range of cells
	 * @param[in] rightValue
	 *     Right-bottom end of range of cells
	 * @param[in] functionType
	 *     Function type (tIsectFuncs/tRangeFuncs)
	 * @since 1.0
	 */
	void rangeOperation(std::vector<int>& coords, Ref3D& leftValue,
						Ref3D& rightValue, int functionType) const;

	/**
	 * @brief
	 *     Get cell address
	 * @param[out] address
	 *     Array in which address will be saved
	 * @param[in] data
	 *     Binary data
	 * @param[in] pos
	 *     Record start position
	 * @param[in] relDelta
	 *     Function method
	 * @param[in] rowIndex
	 *     Row index
	 * @param[in] colIndex
	 *     Column index
	 * @since 1.0
	 */
	void getCellAddress(std::vector<int>& address, const std::string& data, int pos,
						bool relDelta, int rowIndex = 0, int colIndex = 0) const;

	/**
	 * @brief
	 *     Get cell range address
	 * @param[out] address1
	 *     Array in which address of left-top end of range will be saved
	 * @param[out] address2
	 *     Array in which address of right-bottom end of range will be saved
	 * @param[in] data
	 *     Binary data
	 * @param[in] pos
	 *     Record start position
	 * @param[in] relDelta
	 *     Function method
	 * @param[in] rowIndex
	 *     Row index
	 * @param[in] colIndex
	 *     Column index
	 * @since 1.0
	 */
	void getCellRangeAddress(std::vector<int>& address1, std::vector<int>& address2,
							 const std::string& data, int pos, bool relDelta,
							 int rowIndex = 0, int colIndex = 0) const;

	/**
	 * @brief
	 *     Adjust cell address (BIFF 8)
	 * @param[out] address
	 *     Array in which address will be saved
	 * @param[in] rowValue
	 *     Row value
	 * @param[in] colValue
	 *     Column value
	 * @param[in] relDelta
	 *     Function method
	 * @param[in] rowIndex
	 *     Row index
	 * @param[in] colIndex
	 *     Column index
	 * @since 1.0
	 */
	void adjustCellAddressBiff8(std::vector<int>& address, int rowValue, int colValue,
								bool relDelta, int rowIndex = 0, int colIndex = 0) const;

	/**
	 * @brief
	 *     Adjust cell address (BIFF 7 or earlier)
	 * @param[out] address
	 *     Array in which address will be saved
	 * @param[in] rowValue
	 *     Row value
	 * @param[in] colValue
	 *     Column value
	 * @param[in] relDelta
	 *     Function method
	 * @param[in] rowIndex
	 *     Row index
	 * @param[in] colIndex
	 *     Column index
	 * @since 1.0
	 */
	void adjustCellAddressBiff7(std::vector<int>& address, int rowValue, int colValue,
								bool relDelta, int rowIndex = 0, int colIndex = 0) const;

	/**
	 * @brief
	 *     Get external sheet local range of cells
	 * @param[out] sheetIndex1
	 *     Sheet left-top index
	 * @param[out] sheetIndex2
	 *     Sheet right-bottom index
	 * @param[in] refIndex
	 *     Reference index
	 * @since 1.0
	 */
	void getExternalSheetLocalRange(int& sheetIndex1, int& sheetIndex2, int refIndex) const;

	/**
	 * @brief
	 *     Get external sheet local range of cells (BIFF 7/5)
	 * @param[out] sheetIndex1
	 *     Sheet left-top index
	 * @param[out] sheetIndex2
	 *     Sheet right-bottom index
	 * @param[in] rawExternalSheetIndex
	 *     External sheet raw index
	 * @param[in] refFirstSheetIndex
	 *     Reference sheet left-top index
	 * @param[in] refLastSheetIndex
	 *     Reference sheet right-bottom index
	 * @since 1.0
	 */
	void getExternalSheetLocalRangeB57(int& sheetIndex1, int& sheetIndex2,
									   int rawExternalSheetIndex, int refFirstSheetIndex,
									   int refLastSheetIndex) const;

	/**
	 * @brief
	 *     Get 3-dimensional range name. Utility function (assuming Excel's default sheetnames)
	 * @details
	 *     Example:
	 *     @code `Ref3D(1, 4, 5, 20, 7, 10)` -> `'Sheet2:Sheet3!$H$6:$J$20'` @endcode
	 * @param[in] coords
	 *     Range coords
	 * @return
	 *     Range name
	 * @since 1.0
	 */
	std::string rangeName3D(const std::vector<int>& coords) const;

	/**
	 * @brief
	 *     Get relative 3-dimensional range name. Utility function
	 * @details
	 *     Example:
	 *     @code `Ref3D(coords=(0, 1, -32, -22, -13, 13), relflags=(0, 0, 1, 1, 1, 1))`
	 *     In R1C1 mode -> `'Sheet1!R[-32]C[-13]:R[-23]C[12]'`
	 *     In A1   mode -> Depends on base cell `(rowIndex, colIndex)` @endcode
	 * @param[in] coords
	 *     Range coords
	 * @param[in] relationFlags
	 *     Relation flags. Shows if address is relative (1) or absolute (0)
	 * @param[in] rowIndex
	 *     Row index
	 * @param[in] colIndex
	 *     Column index
	 * @param[in] isR1C1
	 *     If cell address id in R1C1 mode
	 * @return
	 *     Range name
	 * @since 1.0
	 */
	std::string rangeName3DRel(const std::vector<int>& coords, const std::vector<int>& relationFlags,
							   int rowIndex = 0, int colIndex = 0, bool isR1C1 = false) const;

	/**
	 * @brief
	 *     Get 2-dimensional range name. Utility function
	 * @details
	 *     Example:
	 *     @code `(5, 20, 7, 10)` -> `'$H$6:$J$20'` @endcode
	 * @param[in] rlo
	 *     Relation first end
	 * @param[in] rhi
	 *     Relation second end
	 * @param[in] clo
	 *     Coords first end
	 * @param[in] chi
	 *     Coords second end
	 * @param[in] isR1C1
	 *     If cell address id in R1C1 mode
	 * @return
	 *     Range name
	 * @since 1.0
	 */
	std::string rangeName2D(int rlo, int rhi, int clo, int chi, bool isR1C1 = false) const;

	/**
	 * @brief
	 *     Get relative 2-dimensional range name. Utility function
	 * @param[in] coords
	 *     Range coords
	 * @param[in] relationFlags
	 *     Relation flags. Shows if address is relative (1) or absolute (0)
	 * @param[in] rowIndex
	 *     Row index
	 * @param[in] colIndex
	 *     Column index
	 * @param[in] isR1C1
	 *     If cell address id in R1C1 mode
	 * @return
	 *     Range name
	 * @since 1.0
	 */
	std::string rangeName2DRel(const std::vector<int>& coords, const std::vector<int>& relationFlags,
							   int rowIndex = 0, int colIndex = 0, bool isR1C1 = false) const;

	/**
	 * @brief
	 *     Get sheet range name
	 * @param[in] sheetIndex1
	 *     Sheet left-top index
	 * @param[in] sheetIndex2
	 *     Sheet right-bottom index
	 * @return
	 *     Sheet range name
	 * @since 1.0
	 */
	std::string sheetRange(int sheetIndex1, int sheetIndex2) const;

	/**
	 * @brief
	 *     Get sheet name
	 * @param[in] sheetIndex
	 *     Sheet index
	 * @return
	 *     Sheet name
	 * @since 1.0
	 */
	std::string quotedSheetName(int sheetIndex) const;

	/**
	 * @brief
	 *     Get absolute cell name
	 * @param[in] rowIndex
	 *     Row index
	 * @param[in] colIndex
	 *     Column index
	 * @param[in] relRowIndex
	 *     Relative row index
	 * @param[in] relColIndex
	 *     Relative column index
	 * @param[in] bRowIndex
	 *     bRow index
	 * @param[in] bColIndex
	 *     bColumn index
	 * @param[in] isR1C1
	 *     If cell address id in R1C1 mode
	 * @return
	 *     Absolute cell name
	 * @since 1.0
	 */
	std::string relativeCellName(int rowIndex, int colIndex, int relRowIndex, int relColIndex,
								 int bRowIndex = 0, int bColIndex = 0, bool isR1C1 = false) const;

	/**
	 * @brief
	 *     Get absolute cell name
	 * @details
	 *     Example:
	 *     @code `(5, 7)` -> `'$H$6'` or `'R8C6'` @endcode
	 * @param[in] rowIndex
	 *     Row index
	 * @param[in] colIndex
	 *     Column index
	 * @param[in] isR1C1
	 *     If cell address id in R1C1 mode
	 * @return
	 *     Absolute cell name
	 */
	std::string absoluteCellName(int rowIndex, int colIndex, bool isR1C1 = false) const;

	/**
	 * @brief
	 *     Get relative row name
	 * @param[in] rowIndex
	 *     Row index
	 * @param[in] relRowIndex
	 *     Relative row index
	 * @param[in] bRowIndex
	 *     bRow index
	 * @param[in] isR1C1
	 *     If cell address id in R1C1 mode
	 * @return
	 *     Relative row name
	 * @since 1.0
	 */
	std::string relativeRowName(int rowIndex, int relRowIndex, int bRowIndex = 0, bool isR1C1 = false) const;

	/**
	 * @brief
	 *     Get relative column name
	 * @param[in] colIndex
	 *     Column index
	 * @param[in] relColIndex
	 *     Relative column index
	 * @param[in] bColIndex
	 *     bColumn index
	 * @param[in] isR1C1
	 *     If cell address id in R1C1 mode
	 * @return
	 *     Relative column name
	 * @since 1.0
	 */
	std::string relativeColName(int colIndex, int relColIndex, int bColIndex = 0, bool isR1C1 = false) const;

	/**
	 * @brief
	 *     Get column name
	 * @param[in] colIndex
	 *     Column index
	 * @return
	 *     Column name
	 * @since 1.0
	 */
	std::string colName(int colIndex) const;
};


/**
 * @class Operand
 * @brief
 *     Evaluating formulas operands
 * @details
 *     The following table describes kinds and how their values are represented
 *     Kind symbol | Kind number | Value representation
 *     :---------: | :---------: | --------------------
 *        oBOOL    |      3      | Boolean: 0 => False; 1 => True
 *        oERR     |      4      | None, or error code (same as XL_CELL_ERROR in CELL class)
 *        oMSNG    |      5      | Used by Excel as placeholder for missing function argument
 *        oNUM     |      2      | Float. Note that there is no way of distinguishing dates
 *        oREF     |      -1     | Value is either None or non-empty list of absolute Ref3D instances
 *        oREL     |      -2     | Value is None or non-empty list of fully/partially relative Ref3D instances
 *        oSTRG    |      1      | Unicode string
 *        oUNK     |      0      | Kind is unknown or ambiguous. Value is None
 */
class Operand {
public:
	/**
	 * @param[in] kind
	 *     Operand kind
	 * @param[in] value
	 *     Operand value
	 * @param[in] rank
	 *     Operand rank
	 * @param[in] text
	 *     Reconstituted text of original formula
	 * @param[in] textValue
	 *     Operand constant text value
	 * @since 1.0
	 */
	Operand(int kind = -5, const std::vector<Ref3D>& value = {}, int rank = 0,
			const std::string& text = "?", const std::string& textValue = "");

	/**
	 * Operand value. None means that actual value of operand is a variable
	 * (depends on cell data), not a constant
	 */
	std::vector<Ref3D> m_value;
	/** Operand constant text value */
	std::string m_textValue;
	/** Operand kind. oUNK means that kind of operand is not known unambiguously */
	int m_kind = oUNK;
	/**
	 * Reconstituted text of original formula. Function names will be in English irrespective of
	 * original language, which doesn't seem to be recorded anywhere.
	 * Separator is ",", not ";" or whatever else might be more appropriate for end-user's locale
	 */
	std::string m_text = "?";
	/** Operand rank */
	int m_rank;
};


/**
 * @class Ref3D
 * @brief
 *     Represents an absolute or relative 3-dimensional reference to box of one or more cells.
 * @details
 *     There is necessarily no information available as to what cell(s) the reference could possibly
 *     be relative to. The caller must decide what if any use to make of `oREL` operands.
 *     Partially relative reference may well be a typo. So far, only one possibility of sheet-relative
 *     component in reference has been noticed: 2D reference located in the "current sheet".
 *     This will appear as `coords = (0, 1, ...)` and `relationFlags = (1, 1, ...)`
 */
class Ref3D {
public:
	/**
	 * @param value
	 *     Coordinates vector
	 * @since 1.0
	 */
	Ref3D(const std::vector<int>& value);

	/**
	 * Tuple of form `(shtxlo, shtxhi, rowxlo, rowxhi, colxlo, colxhi)` where
	 * `0 <= thingxlo <= thingx < thingxhi`. It is quite possible to have `thingx > nthings`
	 */
	std::vector<int> m_coords;
	/**
	 * 6-tuple of flags which indicate whether the corresponding (sheet|row|col)(lo|hi)
	 * is relative (1) or absolute (0)
	 */
	std::vector<int> m_relationFlags;
};

}  // End namespace