# PDF2HMTL

PDF files into HTML сonverter

## Usage:
```
#include "pdf/pdf.hpp"

pdf::Pdf document("test.pdf");
document.convert(true, true, 0);
document.saveHtml("out_dir", "test.html");
```

## Features
| Text | Styles extraction | Images extraction |
| :---:|       :---:       |       :---:       |
| Yes  | No                | Yes               |

## Dependencies
None

## Thanks
- [rembish](https://github.com/rembish/TextAtAnyCost) - PDF converter (PHP)
