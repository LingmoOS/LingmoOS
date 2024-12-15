/**
 * @brief     Excel files (xls/xlsx) into HTML сonverter
 * @package   excel
 * @file      excel.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright python-excel (https://github.com/python-excel/xlrd)
 * @date      02.12.2016 -- 28.01.2018
 */
#include <fstream>
#include <pugixml.hpp>

#include "tools.hpp"

#include "book.hpp"
#include "xlsx.hpp"

#include "excel.hpp"


namespace excel {

/** Inline style */
const std::string STYLE = "body{background:#fafafa}label{background:#f1f1f1;color:#aaa;"
						  "font-size:14px;font-weight:600;text-align:center;position:relative;"
						  "top:3px;margin:0 0 -1px;padding:10px;display:inline-block;"
						  "border:0 solid #ddd;border-width:1px;border-radius:3px 3px 0 0;"
						  "cursor:pointer}label:hover{color:#888}input{position:absolute;"
						  "left:-9999px}#tab10:checked~#tabL10,#tab11:checked~#tabL11,"
						  "#tab12:checked~#tabL12,#tab13:checked~#tabL13,#tab14:checked~#tabL14,"
						  "#tab15:checked~#tabL15,#tab16:checked~#tabL16,#tab17:checked~#tabL17,"
						  "#tab18:checked~#tabL18,#tab19:checked~#tabL19,#tab1:checked~#tabL1,"
						  "#tab20:checked~#tabL20,#tab2:checked~#tabL2,#tab3:checked~#tabL3,"
						  "#tab4:checked~#tabL4,#tab5:checked~#tabL5,#tab6:checked~#tabL6,"
						  "#tab7:checked~#tabL7,#tab8:checked~#tabL8,#tab9:checked~#tabL9{"
						  "width:intrinsic;background:#fff;color:#555;border-top:1px solid #093;"
						  "border-bottom:1px solid #fff;top:0;z-index:3}.tabContent{"
						  "background:#fff;position:relative;z-index:2;width:intrinsic}"
						  ".tabContent div{background:#fff;border:1px solid #ddd;padding:10px;"
						  "display:none;-webkit-transition:opacity .2s ease-in-out;"
						  "-moz-transition:opacity .2s ease-in-out;"
						  "transition:opacity .2s ease-in-out}#tab10:checked~.tabContent #tabC10,"
						  "#tab11:checked~.tabContent #tabC11,#tab12:checked~.tabContent #tabC12,"
						  "#tab13:checked~.tabContent #tabC13,#tab14:checked~.tabContent #tabC14,"
						  "#tab15:checked~.tabContent #tabC15,#tab16:checked~.tabContent #tabC16,"
						  "#tab17:checked~.tabContent #tabC17,#tab18:checked~.tabContent #tabC18,"
						  "#tab19:checked~.tabContent #tabC19,#tab1:checked~.tabContent #tabC1,"
						  "#tab20:checked~.tabContent #tabC20,#tab2:checked~.tabContent #tabC2,"
						  "#tab3:checked~.tabContent #tabC3,#tab4:checked~.tabContent #tabC4,"
						  "#tab5:checked~.tabContent #tabC5,#tab6:checked~.tabContent #tabC6,"
						  "#tab7:checked~.tabContent #tabC7,#tab8:checked~.tabContent #tabC8,"
						  "#tab9:checked~.tabContent #tabC9{display:inline-block}";

// public:
Excel::Excel(const std::string& fileName, const std::string& extension)
	: FileExtension(fileName), m_extension(extension) {}

int Excel::convert(bool addStyle, bool extractImages, char mergingMode) {
	// Convert file
    Book* book = new Book(m_fileName, m_text, false);
    if (m_extension == "xlsx") {
		Xlsx xlsx(book);
		xlsx.openWorkbookXlsx();
    } else {
		book->openWorkbookXls();
	}

	delete book;
    return 0;
}

}
