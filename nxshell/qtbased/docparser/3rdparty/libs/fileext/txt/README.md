# TXT2HTML

TXT/MARKDOWN files into HTML сonverter

## Features
| Text | Styles extraction | Images extraction |
| :---:|       :---:       |       :---:       |
| Yes  | Yes               | Yes               |

-	Tables
-	Lists
-	Links
-	Headers
-	Images
-	Bold/Italic/Strike font style
-	Quotes

## Usage:
```
#include "txt/txt.hpp"

txt::Txt document("test.txt");
document.convert(true, true, 0);
document.saveHtml("out_dir", "test.html");
```

## Dependencies
None

## Thanks
- [adhocore](https://github.com/adhocore/htmlup) - TXT/MD converter (PHP)
