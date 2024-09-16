# DOC2HTML

DOC files into HTML сonverter

## Usage:
```
#include "doc/doc.hpp"

doc::Doc document("test.doc");
document.convert(true, true, 0);
document.saveHtml("out_dir", "test.html");
```

## Features
| Text | Styles extraction | Images extraction |
| :---:|       :---:       |       :---:       |
| Yes  | No                | No                |

## Dependencies
- iconv

## Thanks
- [rembish](https://github.com/rembish/TextAtAnyCost) - DOC converter (PHP)
