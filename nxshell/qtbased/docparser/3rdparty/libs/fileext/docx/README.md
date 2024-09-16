# DOCX2HTML

DOCX files into HTML сonverter

## Usage:
```
#include "docx/docx.hpp"

docx::Docx document("test.docx");
document.convert(true, true, 0);
document.saveHtml("out_dir", "test.html");
```

## Features
| Text | Styles extraction | Images extraction |
| :---:|       :---:       |       :---:       |
| Yes  | Yes               | Yes               |

-	Tables
- Table cell styles
-	Lists
-	Links
-	Headers
-	Images
-	Bold/Italic/Underline/Strike/Sup(sub)string font style
-	Font colors and names
-	Horizontal and vertical aligment

## Dependencies
None

## Thanks
- [PolicyStat](https://github.com/PolicyStat/docx2html) - DOCX converter (Python)
