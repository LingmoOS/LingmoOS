# PPT2HTML

PPT files into HTML сonverter

## Usage:
```
#include "ppt/ppt.hpp"

ppt::Ppt document("test.ppt");
document.convert(true, true, 0);
document.saveHtml("out_dir", "test.html");
```

## Features
| Text | Styles extraction | Images extraction |
| :---:|       :---:       |       :---:       |
| Yes  | No                | No                |

## Dependencies
None

## Thanks
- [rembish](https://github.com/rembish/TextAtAnyCost) - PPT converter (PHP)
