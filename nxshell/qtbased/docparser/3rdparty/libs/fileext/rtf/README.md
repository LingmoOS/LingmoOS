# RTF2HTML

RTF files into HTML сonverter

## Usage:
```
#include "rtf/rtf.hpp"

rtf::Rtf document("test.rtf");
document.convert(true, true, 0);
document.saveHtml("out_dir", "test.html");
```

## Features
| Text | Styles extraction | Images extraction |
| :---:|       :---:       |       :---:       |
| Yes  | Yes               | Yes               |

-	Tables (except nested)
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
- [lvu](https://github.com/lvu/rtf2html) - RTF converter (C++)
