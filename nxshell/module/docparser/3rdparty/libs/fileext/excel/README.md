# EXCEL2HTML

XSLX/XLS files into HTML сonverter

## Usage:
```
#include "excel/excel.hpp"

excel::Excel document("test.xlsx", "xlsx");
document.convert(true, true, 0);
document.saveHtml("out_dir", "test.html");
```

## Features
| Extension | Text | Styles extraction | Images extraction |
|   :---:   | :---:|       :---:       |       :---:       |
| XLSX      | Yes  | Yes               | Yes               |
| XLS       | Yes  | Yes               | No                |

- Table cell styles
-	Images
-	Bold/Italic/Underline/Strike/Sup(sub)string font style
-	Font colors and names
-	Horizontal and vertical aligment

## Dependencies
- iconv

## Thanks
- [python-excel](https://github.com/python-excel/xlrd) - XLSX and XLS converter (Python)
