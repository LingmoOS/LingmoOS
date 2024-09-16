# CSV2HTML

CSV files into HTML сonverter

## Usage:
```
#include "csv/csv.hpp"

csv::Csv document("test.csv");
document.convert(true, true, 0);
document.saveHtml("out_dir", "test.html");
```

## Features
| Text | Styles extraction | Images extraction |
| :---:|       :---:       |       :---:       |
| Yes  | Not applicable    | Not applicable    |

## Dependencies
None
