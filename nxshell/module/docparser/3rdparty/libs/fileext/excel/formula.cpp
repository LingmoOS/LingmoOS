/**
 * @brief     Excel files (xls/xlsx) into HTML сonverter
 * @package   excel
 * @file      formula.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright python-excel (https://github.com/python-excel/xlrd)
 * @date      02.12.2016 -- 29.01.2018
 */
#include <tuple>
#include <unordered_map>
#include <math.h>

#include "tools.hpp"

#include "biffh.hpp"

#include "formula.hpp"
#include <iostream>

namespace excel {

/** List separator. Probably should depend on locale */
const std::string LIST_SEPARATOR = ",";
/** Default leaf rank */
const char LEAF_RANK = 90;
/** Default dunction rank */
const char FUNC_RANK = 90;
/** index, {name, min#args, max#args, flags, #known_args, return_type, kargs) */
const std::unordered_map<int, std::tuple<std::string, char, char>> FUNC_DEFINITIONS {
    {0,   std::tuple<std::string, char, char>("COUNT",            0, 30)},
    {1,   std::tuple<std::string, char, char>("IF",               2,  3)},
    {2,   std::tuple<std::string, char, char>("ISNA",             1,  1)},
    {3,   std::tuple<std::string, char, char>("ISERROR",          1,  1)},
    {4,   std::tuple<std::string, char, char>("SUM",              0, 30)},
    {5,   std::tuple<std::string, char, char>("AVERAGE",          1, 30)},
    {6,   std::tuple<std::string, char, char>("MIN",              1, 30)},
    {7,   std::tuple<std::string, char, char>("MAX",              1, 30)},
    {8,   std::tuple<std::string, char, char>("ROW",              0,  1)},
    {9,   std::tuple<std::string, char, char>("COLUMN",           0,  1)},
    {10,  std::tuple<std::string, char, char>("NA",               0,  0)},
    {11,  std::tuple<std::string, char, char>("NPV",              2, 30)},
    {12,  std::tuple<std::string, char, char>("STDEV",            1, 30)},
    {13,  std::tuple<std::string, char, char>("DOLLAR",           1,  2)},
    {14,  std::tuple<std::string, char, char>("FIXED",            2,  3)},
    {15,  std::tuple<std::string, char, char>("SIN",              1,  1)},
    {16,  std::tuple<std::string, char, char>("COS",              1,  1)},
    {17,  std::tuple<std::string, char, char>("TAN",              1,  1)},
    {18,  std::tuple<std::string, char, char>("ATAN",             1,  1)},
    {19,  std::tuple<std::string, char, char>("PI",               0,  0)},
    {20,  std::tuple<std::string, char, char>("SQRT",             1,  1)},
    {21,  std::tuple<std::string, char, char>("EXP",              1,  1)},
    {22,  std::tuple<std::string, char, char>("LN",               1,  1)},
    {23,  std::tuple<std::string, char, char>("LOG10",            1,  1)},
    {24,  std::tuple<std::string, char, char>("ABS",              1,  1)},
    {25,  std::tuple<std::string, char, char>("INT",              1,  1)},
    {26,  std::tuple<std::string, char, char>("SIGN",             1,  1)},
    {27,  std::tuple<std::string, char, char>("ROUND",            2,  2)},
    {28,  std::tuple<std::string, char, char>("LOOKUP",           2,  3)},
    {29,  std::tuple<std::string, char, char>("INDEX",            2,  4)},
    {30,  std::tuple<std::string, char, char>("REPT",             2,  2)},
    {31,  std::tuple<std::string, char, char>("MID",              3,  3)},
    {32,  std::tuple<std::string, char, char>("LEN",              1,  1)},
    {33,  std::tuple<std::string, char, char>("VALUE",            1,  1)},
    {34,  std::tuple<std::string, char, char>("TRUE",             0,  0)},
    {35,  std::tuple<std::string, char, char>("FALSE",            0,  0)},
    {36,  std::tuple<std::string, char, char>("AND",              1, 30)},
    {37,  std::tuple<std::string, char, char>("OR",               1, 30)},
    {38,  std::tuple<std::string, char, char>("NOT",              1,  1)},
    {39,  std::tuple<std::string, char, char>("MOD",              2,  2)},
    {40,  std::tuple<std::string, char, char>("DCOUNT",           3,  3)},
    {41,  std::tuple<std::string, char, char>("DSUM",             3,  3)},
    {42,  std::tuple<std::string, char, char>("DAVERAGE",         3,  3)},
    {43,  std::tuple<std::string, char, char>("DMIN",             3,  3)},
    {44,  std::tuple<std::string, char, char>("DMAX",             3,  3)},
    {45,  std::tuple<std::string, char, char>("DSTDEV",           3,  3)},
    {46,  std::tuple<std::string, char, char>("VAR",              1, 30)},
    {47,  std::tuple<std::string, char, char>("DVAR",             3,  3)},
    {48,  std::tuple<std::string, char, char>("TEXT",             2,  2)},
    {49,  std::tuple<std::string, char, char>("LINEST",           1,  4)},
    {50,  std::tuple<std::string, char, char>("TREND",            1,  4)},
    {51,  std::tuple<std::string, char, char>("LOGEST",           1,  4)},
    {52,  std::tuple<std::string, char, char>("GROWTH",           1,  4)},
    {56,  std::tuple<std::string, char, char>("PV",               3,  5)},
    {57,  std::tuple<std::string, char, char>("FV",               3,  5)},
    {58,  std::tuple<std::string, char, char>("NPER",             3,  5)},
    {59,  std::tuple<std::string, char, char>("PMT",              3,  5)},
    {60,  std::tuple<std::string, char, char>("RATE",             3,  6)},
    {61,  std::tuple<std::string, char, char>("MIRR",             3,  3)},
    {62,  std::tuple<std::string, char, char>("IRR",              1,  2)},
    {63,  std::tuple<std::string, char, char>("RAND",             0,  0)},
    {64,  std::tuple<std::string, char, char>("MATCH",            2,  3)},
    {65,  std::tuple<std::string, char, char>("DATE",             3,  3)},
    {66,  std::tuple<std::string, char, char>("TIME",             3,  3)},
    {67,  std::tuple<std::string, char, char>("DAY",              1,  1)},
    {68,  std::tuple<std::string, char, char>("MONTH",            1,  1)},
    {69,  std::tuple<std::string, char, char>("YEAR",             1,  1)},
    {70,  std::tuple<std::string, char, char>("WEEKDAY",          1,  2)},
    {71,  std::tuple<std::string, char, char>("HOUR",             1,  1)},
    {72,  std::tuple<std::string, char, char>("MINUTE",           1,  1)},
    {73,  std::tuple<std::string, char, char>("SECOND",           1,  1)},
    {74,  std::tuple<std::string, char, char>("NOW",              0,  0)},
    {75,  std::tuple<std::string, char, char>("AREAS",            1,  1)},
    {76,  std::tuple<std::string, char, char>("ROWS",             1,  1)},
    {77,  std::tuple<std::string, char, char>("COLUMNS",          1,  1)},
    {78,  std::tuple<std::string, char, char>("OFFSET",           3,  5)},
    {82,  std::tuple<std::string, char, char>("SEARCH",           2,  3)},
    {83,  std::tuple<std::string, char, char>("TRANSPOSE",        1,  1)},
    {86,  std::tuple<std::string, char, char>("TYPE",             1,  1)},
    {92,  std::tuple<std::string, char, char>("SERIESSUM",        4,  4)},
    {97,  std::tuple<std::string, char, char>("ATAN2",            2,  2)},
    {98,  std::tuple<std::string, char, char>("ASIN",             1,  1)},
    {99,  std::tuple<std::string, char, char>("ACOS",             1,  1)},
    {100, std::tuple<std::string, char, char>("CHOOSE",           2, 30)},
    {101, std::tuple<std::string, char, char>("HLOOKUP",          3,  4)},
    {102, std::tuple<std::string, char, char>("VLOOKUP",          3,  4)},
    {105, std::tuple<std::string, char, char>("ISREF",            1,  1)},
    {109, std::tuple<std::string, char, char>("LOG",              1,  2)},
    {111, std::tuple<std::string, char, char>("CHAR",             1,  1)},
    {112, std::tuple<std::string, char, char>("LOWER",            1,  1)},
    {113, std::tuple<std::string, char, char>("UPPER",            1,  1)},
    {114, std::tuple<std::string, char, char>("PROPER",           1,  1)},
    {115, std::tuple<std::string, char, char>("LEFT",             1,  2)},
    {116, std::tuple<std::string, char, char>("RIGHT",            1,  2)},
    {117, std::tuple<std::string, char, char>("EXACT",            2,  2)},
    {118, std::tuple<std::string, char, char>("TRIM",             1,  1)},
    {119, std::tuple<std::string, char, char>("REPLACE",          4,  4)},
    {120, std::tuple<std::string, char, char>("SUBSTITUTE",       3,  4)},
    {121, std::tuple<std::string, char, char>("CODE",             1,  1)},
    {124, std::tuple<std::string, char, char>("FIND",             2,  3)},
    {125, std::tuple<std::string, char, char>("CELL",             1,  2)},
    {126, std::tuple<std::string, char, char>("ISERR",            1,  1)},
    {127, std::tuple<std::string, char, char>("ISTEXT",           1,  1)},
    {128, std::tuple<std::string, char, char>("ISNUMBER",         1,  1)},
    {129, std::tuple<std::string, char, char>("ISBLANK",          1,  1)},
    {130, std::tuple<std::string, char, char>("T",                1,  1)},
    {131, std::tuple<std::string, char, char>("N",                1,  1)},
    {140, std::tuple<std::string, char, char>("DATEVALUE",        1,  1)},
    {141, std::tuple<std::string, char, char>("TIMEVALUE",        1,  1)},
    {142, std::tuple<std::string, char, char>("SLN",              3,  3)},
    {143, std::tuple<std::string, char, char>("SYD",              4,  4)},
    {144, std::tuple<std::string, char, char>("DDB",              4,  5)},
    {148, std::tuple<std::string, char, char>("INDIRECT",         1,  2)},
    {162, std::tuple<std::string, char, char>("CLEAN",            1,  1)},
    {163, std::tuple<std::string, char, char>("MDETERM",          1,  1)},
    {164, std::tuple<std::string, char, char>("MINVERSE",         1,  1)},
    {165, std::tuple<std::string, char, char>("MMULT",            2,  2)},
    {167, std::tuple<std::string, char, char>("IPMT",             4,  6)},
    {168, std::tuple<std::string, char, char>("PPMT",             4,  6)},
    {169, std::tuple<std::string, char, char>("COUNTA",           0, 30)},
    {183, std::tuple<std::string, char, char>("PRODUCT",          0, 30)},
    {184, std::tuple<std::string, char, char>("FACT",             1,  1)},
    {189, std::tuple<std::string, char, char>("DPRODUCT",         3,  3)},
    {190, std::tuple<std::string, char, char>("ISNONTEXT",        1,  1)},
    {193, std::tuple<std::string, char, char>("STDEVP",           1, 30)},
    {194, std::tuple<std::string, char, char>("VARP",             1, 30)},
    {195, std::tuple<std::string, char, char>("DSTDEVP",          3,  3)},
    {196, std::tuple<std::string, char, char>("DVARP",            3,  3)},
    {197, std::tuple<std::string, char, char>("TRUNC",            1,  2)},
    {198, std::tuple<std::string, char, char>("ISLOGICAL",        1,  1)},
    {199, std::tuple<std::string, char, char>("DCOUNTA",          3,  3)},
    {204, std::tuple<std::string, char, char>("USDOLLAR",         1,  2)},
    {205, std::tuple<std::string, char, char>("FINDB",            2,  3)},
    {206, std::tuple<std::string, char, char>("SEARCHB",          2,  3)},
    {207, std::tuple<std::string, char, char>("REPLACEB",         4,  4)},
    {208, std::tuple<std::string, char, char>("LEFTB",            1,  2)},
    {209, std::tuple<std::string, char, char>("RIGHTB",           1,  2)},
    {210, std::tuple<std::string, char, char>("MIDB",             3,  3)},
    {211, std::tuple<std::string, char, char>("LENB",             1,  1)},
    {212, std::tuple<std::string, char, char>("ROUNDUP",          2,  2)},
    {213, std::tuple<std::string, char, char>("ROUNDDOWN",        2,  2)},
    {214, std::tuple<std::string, char, char>("ASC",              1,  1)},
    {215, std::tuple<std::string, char, char>("DBCS",             1,  1)},
    {216, std::tuple<std::string, char, char>("RANK",             2,  3)},
    {219, std::tuple<std::string, char, char>("ADDRESS",          2,  5)},
    {220, std::tuple<std::string, char, char>("DAYS360",          2,  3)},
    {221, std::tuple<std::string, char, char>("TODAY",            0,  0)},
    {222, std::tuple<std::string, char, char>("VDB",              5,  7)},
    {227, std::tuple<std::string, char, char>("MEDIAN",           1, 30)},
    {228, std::tuple<std::string, char, char>("SUMPRODUCT",       1, 30)},
    {229, std::tuple<std::string, char, char>("SINH",             1,  1)},
    {230, std::tuple<std::string, char, char>("COSH",             1,  1)},
    {231, std::tuple<std::string, char, char>("TANH",             1,  1)},
    {232, std::tuple<std::string, char, char>("ASINH",            1,  1)},
    {233, std::tuple<std::string, char, char>("ACOSH",            1,  1)},
    {234, std::tuple<std::string, char, char>("ATANH",            1,  1)},
    {235, std::tuple<std::string, char, char>("DGET",             3,  3)},
    {244, std::tuple<std::string, char, char>("INFO",             1,  1)},
    {247, std::tuple<std::string, char, char>("DB",               4,  5)},
    {252, std::tuple<std::string, char, char>("FREQUENCY",        2,  2)},
    {261, std::tuple<std::string, char, char>("ERROR.TYPE",       1,  1)},
    {269, std::tuple<std::string, char, char>("AVEDEV",           1, 30)},
    {270, std::tuple<std::string, char, char>("BETADIST",         3,  5)},
    {271, std::tuple<std::string, char, char>("GAMMALN",          1,  1)},
    {272, std::tuple<std::string, char, char>("BETAINV",          3,  5)},
    {273, std::tuple<std::string, char, char>("BINOMDIST",        4,  4)},
    {274, std::tuple<std::string, char, char>("CHIDIST",          2,  2)},
    {275, std::tuple<std::string, char, char>("CHIINV",           2,  2)},
    {276, std::tuple<std::string, char, char>("COMBIN",           2,  2)},
    {277, std::tuple<std::string, char, char>("CONFIDENCE",       3,  3)},
    {278, std::tuple<std::string, char, char>("CRITBINOM",        3,  3)},
    {279, std::tuple<std::string, char, char>("EVEN",             1,  1)},
    {280, std::tuple<std::string, char, char>("EXPONDIST",        3,  3)},
    {281, std::tuple<std::string, char, char>("FDIST",            3,  3)},
    {282, std::tuple<std::string, char, char>("FINV",             3,  3)},
    {283, std::tuple<std::string, char, char>("FISHER",           1,  1)},
    {284, std::tuple<std::string, char, char>("FISHERINV",        1,  1)},
    {285, std::tuple<std::string, char, char>("FLOOR",            2,  2)},
    {286, std::tuple<std::string, char, char>("GAMMADIST",        4,  4)},
    {287, std::tuple<std::string, char, char>("GAMMAINV",         3,  3)},
    {288, std::tuple<std::string, char, char>("CEILING",          2,  2)},
    {289, std::tuple<std::string, char, char>("HYPGEOMDIST",      4,  4)},
    {290, std::tuple<std::string, char, char>("LOGNORMDIST",      3,  3)},
    {291, std::tuple<std::string, char, char>("LOGINV",           3,  3)},
    {292, std::tuple<std::string, char, char>("NEGBINOMDIST",     3,  3)},
    {293, std::tuple<std::string, char, char>("NORMDIST",         4,  4)},
    {294, std::tuple<std::string, char, char>("NORMSDIST",        1,  1)},
    {295, std::tuple<std::string, char, char>("NORMINV",          3,  3)},
    {296, std::tuple<std::string, char, char>("NORMSINV",         1,  1)},
    {297, std::tuple<std::string, char, char>("STANDARDIZE",      3,  3)},
    {298, std::tuple<std::string, char, char>("ODD",              1,  1)},
    {299, std::tuple<std::string, char, char>("PERMUT",           2,  2)},
    {300, std::tuple<std::string, char, char>("POISSON",          3,  3)},
    {301, std::tuple<std::string, char, char>("TDIST",            3,  3)},
    {302, std::tuple<std::string, char, char>("WEIBULL",          4,  4)},
    {303, std::tuple<std::string, char, char>("SUMXMY2",          2,  2)},
    {304, std::tuple<std::string, char, char>("SUMX2MY2",         2,  2)},
    {305, std::tuple<std::string, char, char>("SUMX2PY2",         2,  2)},
    {306, std::tuple<std::string, char, char>("CHITEST",          2,  2)},
    {307, std::tuple<std::string, char, char>("CORREL",           2,  2)},
    {308, std::tuple<std::string, char, char>("COVAR",            2,  2)},
    {309, std::tuple<std::string, char, char>("FORECAST",         3,  3)},
    {310, std::tuple<std::string, char, char>("FTEST",            2,  2)},
    {311, std::tuple<std::string, char, char>("INTERCEPT",        2,  2)},
    {312, std::tuple<std::string, char, char>("PEARSON",          2,  2)},
    {313, std::tuple<std::string, char, char>("RSQ",              2,  2)},
    {314, std::tuple<std::string, char, char>("STEYX",            2,  2)},
    {315, std::tuple<std::string, char, char>("SLOPE",            2,  2)},
    {316, std::tuple<std::string, char, char>("TTEST",            4,  4)},
    {317, std::tuple<std::string, char, char>("PROB",             3,  4)},
    {318, std::tuple<std::string, char, char>("DEVSQ",            1, 30)},
    {319, std::tuple<std::string, char, char>("GEOMEAN",          1, 30)},
    {320, std::tuple<std::string, char, char>("HARMEAN",          1, 30)},
    {321, std::tuple<std::string, char, char>("SUMSQ",            0, 30)},
    {322, std::tuple<std::string, char, char>("KURT",             1, 30)},
    {323, std::tuple<std::string, char, char>("SKEW",             1, 30)},
    {324, std::tuple<std::string, char, char>("ZTEST",            2,  3)},
    {325, std::tuple<std::string, char, char>("LARGE",            2,  2)},
    {326, std::tuple<std::string, char, char>("SMALL",            2,  2)},
    {327, std::tuple<std::string, char, char>("QUARTILE",         2,  2)},
    {328, std::tuple<std::string, char, char>("PERCENTILE",       2,  2)},
    {329, std::tuple<std::string, char, char>("PERCENTRANK",      2,  3)},
    {330, std::tuple<std::string, char, char>("MODE",             1, 30)},
    {331, std::tuple<std::string, char, char>("TRIMMEAN",         2,  2)},
    {332, std::tuple<std::string, char, char>("TINV",             2,  2)},
    {336, std::tuple<std::string, char, char>("CONCATENATE",      0, 30)},
    {337, std::tuple<std::string, char, char>("POWER",            2,  2)},
    {342, std::tuple<std::string, char, char>("RADIANS",          1,  1)},
    {343, std::tuple<std::string, char, char>("DEGREES",          1,  1)},
    {344, std::tuple<std::string, char, char>("SUBTOTAL",         2, 30)},
    {345, std::tuple<std::string, char, char>("SUMIF",            2,  3)},
    {346, std::tuple<std::string, char, char>("COUNTIF",          2,  2)},
    {347, std::tuple<std::string, char, char>("COUNTBLANK",       1,  1)},
    {350, std::tuple<std::string, char, char>("ISPMT",            4,  4)},
    {351, std::tuple<std::string, char, char>("DATEDIF",          3,  3)},
    {352, std::tuple<std::string, char, char>("DATESTRING",       1,  1)},
    {353, std::tuple<std::string, char, char>("NUMBERSTRING",     2,  2)},
    {354, std::tuple<std::string, char, char>("ROMAN",            1,  2)},
    {358, std::tuple<std::string, char, char>("GETPIVOTDATA",     2,  2)},
    {359, std::tuple<std::string, char, char>("HYPERLINK",        1,  2)},
    {360, std::tuple<std::string, char, char>("PHONETIC",         1,  1)},
    {361, std::tuple<std::string, char, char>("AVERAGEA",         1, 30)},
    {362, std::tuple<std::string, char, char>("MAXA",             1, 30)},
    {363, std::tuple<std::string, char, char>("MINA",             1, 30)},
    {364, std::tuple<std::string, char, char>("STDEVPA",          1, 30)},
    {365, std::tuple<std::string, char, char>("VARPA",            1, 30)},
    {366, std::tuple<std::string, char, char>("STDEVA",           1, 30)},
    {367, std::tuple<std::string, char, char>("VARA",             1, 30)},
    {368, std::tuple<std::string, char, char>("BAHTTEXT",         1,  1)},
    {369, std::tuple<std::string, char, char>("THAIDAYOFWEEK",    1,  1)},
    {370, std::tuple<std::string, char, char>("THAIDIGIT",        1,  1)},
    {371, std::tuple<std::string, char, char>("THAIMONTHOFYEAR",  1,  1)},
    {372, std::tuple<std::string, char, char>("THAINUMSOUND",     1,  1)},
    {373, std::tuple<std::string, char, char>("THAINUMSTRING",    1,  1)},
    {374, std::tuple<std::string, char, char>("THAISTRINGLENGTH", 1,  1)},
    {375, std::tuple<std::string, char, char>("ISTHAIDIGIT",      1,  1)},
    {376, std::tuple<std::string, char, char>("ROUNDBAHTDOWN",    1,  1)},
    {377, std::tuple<std::string, char, char>("ROUNDBAHTUP",      1,  1)},
    {378, std::tuple<std::string, char, char>("THAIYEAR",         1,  1)},
    {379, std::tuple<std::string, char, char>("RTD",              2,  5)}
};
/**
 * SZTABN[code] -> the number of bytes to consume. Which N to use? Depends on biff_version.
 * Value | Description
 * :---: | -----------
 *   -1  | Variable
 *   -2  | Code not implemented in this version
 */
const std::vector<signed char> SZTAB0 {
    -2, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -2,-1, 8,
    4, 2, 2, 3, 9, 8, 2, 3, 8, 4, 7, 5, 5, 5, 2, 4, 7, 4, 7, 2, 2, -2, -2, -2, -2, -2, -2,
    -2, -2, 3, -2, -2, -2, -2, -2, -2, -2
};
const std::vector<signed char> SZTAB1 {
    -2, 5, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -2, -1, 11, 5,
    2, 2, 3, 9, 9, 2, 3, 11, 4, 7, 7, 7, 7, 3, 4, 7, 4, 7, 3, 3, -2, -2, -2, -2, -2, -2, -2,
    -2, 3, -2, -2, -2, -2, -2, -2, -2
};
const std::vector<signed char> SZTAB2 {
    -2, 5, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -2, -1, 11, 5,
    2, 2, 3, 9, 9, 3, 4, 11, 4, 7, 7, 7, 7, 3, 4, 7, 4, 7, 3, 3, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2
};
const std::vector<signed char> SZTAB3 {
    -2, 5, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -2, -1, -2, -2,
    2, 2, 3, 9, 9, 3, 4, 15, 4, 7, 7, 7, 7, 3, 4, 7, 4, 7, 3, 3, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, 25, 18, 21, 18, 21, -2, -2
};
const std::vector<signed char> SZTAB4 {
    -2, 5, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -2, -2,
    2, 2, 3, 9, 9, 3, 4, 5, 5, 9, 7, 7, 7, 3, 5, 9, 5, 9, 3, 3, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, 7, 7, 11, 7, 11, -2, -2
};
const std::unordered_map<int, std::vector<signed char>> SZDICT {
    {20, SZTAB0},
    {21, SZTAB0},
    {30, SZTAB1},
    {40, SZTAB2},
    {45, SZTAB2},
    {50, SZTAB3},
    {70, SZTAB3},
    {80, SZTAB4}
};
/**
 * This dictionary can be used to produce text version of internal codes
 * that Excel uses for error cells
 */
const std::vector<int> ERROR_CODES {0x07, 0x08, 0x0A, 0x0B, 0x1C, 0x1D, 0x2F};
/** Operation name list */
const std::vector<std::string> OPERATION_NAMES {
    "Unk00", "Exp", "Tbl", "Add", "Sub", "Mul", "Div", "Power", "Concat", "LT", "LE", "EQ",
    "GE", "GT", "NE", "Isect", "List", "Range", "Uplus", "Uminus", "Percent", "Paren", "MissArg",
    "Str", "Extended", "Attr", "Sheet", "EndSheet", "Err", "Bool", "Int", "Num", "Array", "Func",
    "FuncVar", "Name", "Ref", "Area", "MemArea", "MemErr", "MemNoMem", "MemFunc", "RefErr",
    "AreaErr", "RefN", "AreaN", "MemAreaN", "MemNoMemN", "", "", "", "", "", "", "", "", "FuncCE",
    "NameX", "Ref3d", "Area3d", "RefErr3d", "AreaErr3d", "", ""
};

// Formula public:
Formula::Formula(Book* book)
    : m_book(book) {}

void Formula::evaluateFormula(Name& name, int nameIndex, int level) {
    try {
        if (level > 10)
            throw std::logic_error("Excessive indirect references in NAME formula");
        const std::vector<signed char>& sztab = SZDICT.at(m_book->m_biffVersion);
        std::string data = name.m_rawFormula;
        bool relDelta    = true;  // All defined name formulas use "Method B" [OOo docs]
        bool hasRelation = false;
        bool hasError    = false;
        int  pos         = 0;
        std::vector<Operand> stack;
        Operand unkOp(oUNK);
        Operand errorOp(oERR);

        if (name.m_basicFormulaLength == 0)
            stack = {unkOp};

        while (0 <= pos && pos < name.m_basicFormulaLength) {
            char op      = data[pos];
            char opCode  = op  & 0x1f;
            char opType  = (op & 0x60) >> 5;
            int  opIndex = opType ? opCode : opCode + 32;
            char size    = sztab[opIndex];
            const std::string& opName = OPERATION_NAMES.at(opIndex);

            if (size == -2)
                throw std::logic_error(
                        "ERROR *** Unexpected token "+ std::to_string(op) +" ("+ opName +
                        "); biff_version=" + std::to_string(m_book->m_biffVersion));
            if (!opType) {
                // unkOp, tExp, tTbl
                if (0x00 <= opCode && opCode <= 0x02) {
                    throw std::logic_error(
                                "ERROR *** Token "+ std::to_string(op) +" ("+ opName +
                                ") found in NAME formula"
                                );
                }
                // Add, Sub, Mul, Div, Power, tConcat, tLT, ..., tNE
                else if (0x03 <= opCode && opCode <= 0x0E) {
                    if (stack.empty())
                        throw std::logic_error("stack is empty");
                    binOperation(opCode, stack);
                }
                // tIsect
                else if (opCode == 0x0F) {
                    if (stack.empty())
                        throw std::logic_error("stack is empty");

                    Operand rightOp = stack.back();
                    stack.pop_back();
                    if (stack.empty())
                        throw std::logic_error("stack is empty");
                    Operand leftOp  = stack.back();
                    stack.pop_back();
                    int rank = 80;  // #Check#

                    std::string opText;
                    opText += (leftOp.m_rank  < rank) ? "(" : "";
                    opText += leftOp.m_text;
                    opText += (leftOp.m_rank  < rank) ? ")" : "";
                    opText += " ";
                    opText += (rightOp.m_rank < rank) ? "(" : "";
                    opText += rightOp.m_text;
                    opText += (rightOp.m_rank < rank) ? ")" : "";

                    Operand res(oREF);
                    res.m_text = opText;
                    if (leftOp.m_kind == oERR || rightOp.m_kind == oERR) {
                        res.m_kind = oERR;
                    }
                    else if (leftOp.m_kind == oREF && rightOp.m_kind == oREF) {
                        if (!leftOp.m_value.empty() && !rightOp.m_value.empty()) {
                            std::vector<int> coords;
                            rangeOperation(coords, leftOp.m_value[0], rightOp.m_value[0], 0);
                            res.m_value = {Ref3D(coords)};
                        }
                    }
                    else if (leftOp.m_kind == oREL && rightOp.m_kind == oREL) {
                        res.m_kind = oREL;
                        if (!leftOp.m_value.empty() && !rightOp.m_value.empty()) {
                            std::vector<int> coords;
                            rangeOperation(coords, leftOp.m_value[0], rightOp.m_value[0], 0);
                            if (leftOp.m_value[0].m_relationFlags == rightOp.m_value[0].m_relationFlags) {
                                auto ref(leftOp.m_value[0].m_relationFlags);
                                coords.insert(coords.end(), ref.begin(), ref.end());
                                res.m_value = {Ref3D(coords)};
                            }
                        }
                    }
                    stack.push_back(res);
                }
                // tList
                else if (opCode == 0x10) {
                    if (stack.empty())
                        throw std::logic_error("stack is empty");
                    Operand rightOp = stack.back();
                    stack.pop_back();
                    if (stack.empty())
                        throw std::logic_error("stack is empty");
                    Operand leftOp  = stack.back();
                    stack.pop_back();
                    int rank = 80;  // #Check#

                    std::string opText;
                    opText += (leftOp.m_rank  < rank) ? "(" : "";
                    opText += leftOp.m_text;
                    opText += (leftOp.m_rank  < rank) ? ")" : "";
                    opText += ",";
                    opText += (rightOp.m_rank < rank) ? "(" : "";
                    opText += rightOp.m_text;
                    opText += (rightOp.m_rank < rank) ? ")" : "";

                    Operand res(oREF, {}, rank, opText);
                    if (leftOp.m_kind == oERR || rightOp.m_kind == oERR) {
                        res.m_kind = oERR;
                    }
                    else if (
                             (leftOp.m_kind == oREF || leftOp.m_kind == oREL) &&
                             (rightOp.m_kind == oREF || rightOp.m_kind == oREL)
                             ) {
                        res.m_kind = oREF;
                        if (leftOp.m_kind == oREL || rightOp.m_kind == oREL)
                            res.m_kind = oREL;

                        if (!leftOp.m_value.empty() && !rightOp.m_value.empty()) {
                            auto val(leftOp.m_value);
                            val.insert(val.end(), rightOp.m_value.begin(), rightOp.m_value.end());
                            res.m_value = val;
                        }
                    }
                    stack.push_back(res);
                }
                // tRange
                else if (opCode == 0x11) {
                    if (stack.empty())
                        throw std::logic_error("stack is empty");
                    Operand rightOp = stack.back();
                    stack.pop_back();
                    if (stack.empty())
                        throw std::logic_error("stack is empty");
                    Operand leftOp  = stack.back();
                    stack.pop_back();
                    int rank = 80;  // #Check#

                    std::string opText;
                    opText += (leftOp.m_rank  < rank) ? "(" : "";
                    opText += leftOp.m_text;
                    opText += (leftOp.m_rank  < rank) ? ")" : "";
                    opText += ":";
                    opText += (rightOp.m_rank < rank) ? "(" : "";
                    opText += rightOp.m_text;
                    opText += (rightOp.m_rank < rank) ? ")" : "";

                    Operand res(oREF, {}, rank, opText);
                    if (leftOp.m_kind == oERR || rightOp.m_kind == oERR) {
                        res.m_kind = oERR;
                    }
                    else if (leftOp.m_kind == oREF && rightOp.m_kind == oREF) {
                        if (!leftOp.m_value.empty() && !rightOp.m_value.empty()) {
                            std::vector<int> coords;
                            rangeOperation(coords, leftOp.m_value[0], rightOp.m_value[0], 1);
                            res.m_value = {Ref3D(coords)};
                        }
                    }
                    else if (leftOp.m_kind == oREL && rightOp.m_kind == oREL) {
                        res.m_kind = oREL;
                        if (!leftOp.m_value.empty() && !rightOp.m_value.empty()) {
                            std::vector<int> coords;
                            rangeOperation(coords, leftOp.m_value[0], rightOp.m_value[0], 1);
                            if (leftOp.m_value[0].m_relationFlags == rightOp.m_value[0].m_relationFlags) {
                                auto ref(leftOp.m_value[0].m_relationFlags);
                                coords.insert(coords.end(), ref.begin(), ref.end());
                                res.m_value = {Ref3D(coords)};
                            }
                        }
                    }
                    stack.push_back(res);
                }
                // tUplus, tUminus, tPercent
                else if (0x12 <= opCode && opCode <= 0x14) {
                    if (stack.empty())
                        throw std::logic_error("stack is empty");
                    unaryOperation(opCode, stack, oNUM);
                }
                // tParen
                // else if (opCode == 0x15)
                // tMissArg
                else if (opCode == 0x16) {
                    stack.push_back(Operand(oMSNG, {}, LEAF_RANK));
                }
                // tStr
                else if (opCode == 0x17) {
                    int newPos      = pos + 1;
                    std::string str = m_book->unpackStringUpdatePos(data, newPos, 1);
                    size = newPos - pos;

                    std::string text = str;
                    tools::replaceAll(text, "\"", "\"\"");
                    text = "\"" + str + "\"";
                    // Operand(oSTRG, strg, LEAF_RANK, text)
                    stack.push_back(Operand(oSTRG, {}, LEAF_RANK, text, str));
                }
                // tExtended (New with BIFF 8)
                else if (opCode == 0x18) {
                    throw std::domain_error("tExtended token is not implemented");
                }
                // tAttr
                else if (opCode == 0x19) {
                    unsigned char subop = m_book->readByte<unsigned char>(data, pos+1, 1);
                    unsigned short nc   = m_book->readByte<unsigned short>(data, pos+2, 2);

                    if (subop == 0x04)  // Choose
                        size = nc * 2 + 6;
                    else if (subop == 0x10) {  // Sum (single arg)
                        size = 4;
                        if (stack.empty())
                            throw std::logic_error("stack is empty");
                        Operand leftOp = stack.back();
                        stack[stack.size()-1] = Operand(oNUM, {}, FUNC_RANK, "SUM("+ leftOp.m_text +")");
                    }
                    else
                        size = 4;
                }
                // tSheet, tEndSheet
                else if (0x1A <= opCode && opCode <= 0x1B) {
                    throw std::domain_error("tSheet and tEndsheet tokens are not implemented");
                }
                // tErr, tBool, tInt, tNum
                else if (0x1C <= opCode && opCode <= 0x1F) {
                    int index = opCode - 0x1C;
                    int kind;
                    double value;
                    std::string text;
                    // tBool
                    if (index == 1) {
                        kind  = oBOOL;
                        value = m_book->readByte<unsigned char>(data, pos+1, 1);
                        text  = value ? "TRUE" : "FALSE";
                    }
                    // tInt
                    else if (index == 2) {
                        kind  = oNUM;
                        value = m_book->readByte<unsigned short>(data, pos+1, 2);
                        text  = std::to_string((float)value);
                    }
                    // tNum
                    else if (index == 3) {
                        kind  = oNUM;
                        value = m_book->readByte<double>(data, pos+1, 8);
                        text  = std::to_string(value);
                    }
                    else {
                        kind  = oERR;
                        value = m_book->readByte<unsigned char>(data, pos+1, 1);
                        text  = "\"" + ERROR_TEXT_FROM_CODE.at(static_cast<int>(value)) + "\"";
                    }
                    // Operand(kind, value, LEAF_RANK, text)
                    stack.push_back(Operand(kind, {}, LEAF_RANK, text, std::to_string(value)));
                }
                else {
                    throw std::logic_error("Unhandled opCode: " + std::to_string(opCode));
                }
                if (size <= 0) {
                    throw std::logic_error("Size not set for opCode " + std::to_string(opCode));
                }
                pos += size;
                continue;
            }
            // tArray
            if (opCode == 0x00) {
                stack.push_back(unkOp);
            }
            // tFunc
            else if (opCode == 0x01) {
                int recordSize       = 1 + (m_book->m_biffVersion >= 40);
                unsigned short funcx = m_book->readByte<unsigned short>(data, pos+1, recordSize);

                if (FUNC_DEFINITIONS.find(funcx) == FUNC_DEFINITIONS.end()) {
                    stack.push_back(unkOp);
                }
                else {
                    std::string opText;
                    std::string funcName = std::get<0>(FUNC_DEFINITIONS.at(funcx));
                    char argCount        = std::get<1>(FUNC_DEFINITIONS.at(funcx));
                    if (argCount) {
                        std::string argtext;
                        for (int i = 0; i < argCount; ++i) {
                            if (i != 0)
                                argtext += LIST_SEPARATOR;
                            if (stack.empty())
                                throw std::logic_error("stack is empty");
                            argtext += stack.back().m_text;
                            stack.pop_back();
                        }
                        opText = funcName +"("+ argtext +")";
                    }
                    else {
                        opText = funcName + "()";
                    }
                    stack.push_back(Operand(oUNK, {}, FUNC_RANK, opText));
                }
            }
            // tFuncVar
            else if (opCode == 0x02) {
                int recordSize          = 1 + (m_book->m_biffVersion >= 40);
                unsigned char  argCount = m_book->readByte<unsigned char>(data,  pos+1, 1) % 128;
                unsigned short funcx    = m_book->readByte<unsigned short>(data, pos+2, recordSize) % 32768;

                if (FUNC_DEFINITIONS.find(funcx) == FUNC_DEFINITIONS.end()) {
                    stack.push_back(unkOp);
                }
                else {
                    std::string funcName = std::get<0>(FUNC_DEFINITIONS.at(funcx));
                    //char minArgCount     = std::get<1>(FUNC_DEFINITIONS[funcx]);
                    //char maxArgCount     = std::get<2>(FUNC_DEFINITIONS[funcx]);

                    std::string argtext;
                    for (int i = 0; i < argCount; ++i) {
                        if (i != 0)
                            argtext += LIST_SEPARATOR;
                        if (stack.empty())
                            throw std::logic_error("stack is empty");
                        argtext += stack.back().m_text;
                    }
                    std::string opText = funcName +"("+ argtext +")";
                    Operand res(oUNK, {}, FUNC_RANK, opText);

                    if (stack.empty())
                        throw std::logic_error("stack is empty");
                    auto& testOp  = stack[stack.size() - argCount];
                    int testValue = 0;
                    try {
                        testValue = std::stoi(testOp.m_textValue);
                    }
                    catch (...) {}

                    // IF
                    if (funcx == 1) {
                        if (
                                (testOp.m_kind == oNUM || testOp.m_kind == oBOOL) &&
                                (testValue == 0 || testValue == 1)
                                ) {
                            if (argCount == 2 && !testValue) {
                                // IF(FALSE, tv) => FALSE
                                res.m_kind       = oBOOL;
                                res.m_textValue  = "0";
                            }
                            else {
                                int respos = -argCount + 2 - testValue;
                                if (respos < 0)
                                    respos = (int)stack.size() - respos;
                                if (respos > stack.size())
                                    return;
                                auto& chosen = stack[respos];
                                if (chosen.m_kind == oMSNG) {
                                    res.m_kind      = oNUM;
                                    res.m_textValue = "0";
                                }
                                else {
                                    res.m_kind      = chosen.m_kind;
                                    res.m_textValue = chosen.m_textValue;
                                }
                            }
                        }
                    }
                    // CHOOSE
                    else if (funcx == 100) {
                        if (testOp.m_kind == oNUM && (1 <= testValue && testValue < argCount)) {
                            int respos = -argCount - testValue;
                            if (respos < 0)
                                respos = (int)stack.size() - respos;

                            if (respos > stack.size())
                                return;
                            auto& chosen = stack[respos];
                            if (chosen.m_kind == oMSNG){
                                res.m_kind      = oNUM;
                                res.m_textValue = "0";
                            }
                            else {
                                res.m_kind      = chosen.m_kind;
                                res.m_textValue = chosen.m_textValue;
                            }
                        }
                    }

                    for (int i = 0; i < argCount; ++i)
                        stack.pop_back();
                    stack.push_back(res);
                }
            }
            // tName
            else if (opCode == 0x03) {
                unsigned short targetNameIndex = m_book->readByte<unsigned short>(data, pos+1, 2) - 1;
                // Only change with BIFF version is number of trailing UNUSED bytes!
                Name& targetName = m_book->m_nameObjList[targetNameIndex];
                // Recursive
                if (!targetName.m_evaluated)
                    evaluateFormula(targetName, targetNameIndex, level+1);

                Operand res(oUNK);
                if (!targetName.m_stack.empty() && !(targetName.m_macro || targetName.m_isBinary || targetName.m_hasError))
                    res = targetName.m_stack[0];
                res.m_rank = LEAF_RANK;

                if (targetName.m_scope == -1) {
                    res.m_text  = targetName.m_name;
                    hasError    = (hasError || targetName.m_macro || targetName.m_isBinary || targetName.m_hasError);
                    hasRelation = (hasRelation || targetName.m_hasRelation);
                }
                else {
                    res.m_text = m_book->m_sheetNames[targetName.m_scope] + "%s!" + targetName.m_name;
                }
                stack.push_back(res);
            }
            // tRef
            else if (opCode == 0x04) {
                hasRelation = true;
                std::vector<int> address;
                getCellAddress(address, data, pos+1, relDelta);
                std::vector<int> coords {0, 1, address[0], address[0] + 1, address[1], address[1] + 1};

                Operand res(oUNK);
                if (opType == 1) {
                    std::vector<int> relflags {1, 1, address[2], address[2], address[3], address[3]};
                    coords.insert(coords.end(), relflags.begin(), relflags.end());
                    res = Operand(oREL, {Ref3D(coords)});
                }
                stack.push_back(res);
            }
            // tArea
            else if (opCode == 0x05) {
                hasRelation = true;
                std::vector<int> address1, address2;
                getCellRangeAddress(address1, address2, data, pos+1, relDelta);
                std::vector<int> coords {0, 1, address1[0], address2[0] + 1, address1[1], address2[1] + 1};

                Operand res(oUNK);
                if (opType == 1) {
                    std::vector<int> relflags {1, 1, address1[2], address2[2], address1[3], address2[3]};
                    coords.insert(coords.end(), relflags.begin(), relflags.end());
                    res = Operand(oREL, {Ref3D(coords)});
                }
                stack.push_back(res);
            }
            // tMemArea
            else if (opCode == 0x06) {
                throw std::logic_error(
                            "ERROR *** Token "+ std::to_string(op) +" ("+ opName +
                            ") found in NAME formula"
                            );
            }
            // tMemFunc
            //else if (opCode == 0x09) {
            //	unsigned short recordSize = m_book->readByte<unsigned short>(data, pos+1, 2);
            //}
            // tRefN
            else if (opCode == 0x0C) {
                throw std::logic_error(
                            "ERROR *** Token "+ std::to_string(op) +" ("+ opName +
                            ") found in NAME formula"
                            );
            }
            // tAreaN
            else if (opCode == 0x0D) {
                throw std::logic_error(
                            "ERROR *** Token "+ std::to_string(op) +" ("+ opName +
                            ") found in NAME formula"
                            );
            }
            // tRef3d
            else if (opCode == 0x1A) {
                int sheetIndex1, sheetIndex2;
                std::vector<int> address;
                if (m_book->m_biffVersion >= 80) {
                    getCellAddress(address, data, pos+3, relDelta);
                    unsigned short refIndex = m_book->readByte<unsigned short>(data, pos+1, 2);
                    getExternalSheetLocalRange(sheetIndex1, sheetIndex2, refIndex);
                }
                else {
                    getCellAddress(address, data, pos+15, relDelta);
                    short rawExternalSheetIndex = m_book->readByte<unsigned short>(data, pos+1,  2);
                    short refFirstSheetIndex    = m_book->readByte<unsigned short>(data, pos+11, 2);
                    short refLastSheetIndex     = m_book->readByte<unsigned short>(data, pos+13, 2);
                    getExternalSheetLocalRangeB57(
                                sheetIndex1, sheetIndex2, rawExternalSheetIndex,
                                refFirstSheetIndex, refLastSheetIndex
                                );
                }
                std::vector<int> coords {
                    sheetIndex1, sheetIndex2+1, address[0],
                            address[0] + 1, address[1], address[1] + 1
                };

                bool isRelation = (address[2] || address[3]);
                hasRelation = (hasRelation || isRelation);
                hasError   |= sheetIndex1 < -1;

                Operand res(oUNK);
                if (isRelation) {
                    res.m_kind = oREL;
                    std::vector<int> relflags {0, 0, address[2], address[2], address[3], address[3]};
                    res.m_text = rangeName3DRel(coords, relflags, 0, 0, true);

                    coords.insert(coords.end(), relflags.begin(), relflags.end());
                }
                else {
                    res.m_kind = oREF;
                    res.m_text = rangeName3D(coords);
                }
                res.m_rank = LEAF_RANK;
                if (opType == 1)
                    res.m_value = {Ref3D(coords)};

                stack.push_back(res);
            }
            // tArea3d
            else if (opCode == 0x1B) {
                int sheetIndex1, sheetIndex2;
                std::vector<int> address1, address2;
                if (m_book->m_biffVersion >= 80) {
                    getCellRangeAddress(address1, address2, data, pos+3, relDelta);
                    unsigned short refIndex = m_book->readByte<unsigned short>(data, pos+1, 2);
                    getExternalSheetLocalRange(sheetIndex1, sheetIndex2, refIndex);
                }
                else {
                    getCellRangeAddress(address1, address2, data, pos+15, relDelta);
                    short rawExternalSheetIndex = m_book->readByte<unsigned short>(data, pos+1,  2);
                    short refFirstSheetIndex    = m_book->readByte<unsigned short>(data, pos+11, 2);
                    short refLastSheetIndex     = m_book->readByte<unsigned short>(data, pos+13, 2);
                    getExternalSheetLocalRangeB57(sheetIndex1, sheetIndex2, rawExternalSheetIndex, refFirstSheetIndex, refLastSheetIndex);
                }
                std::vector<int> coords {
                    sheetIndex1, sheetIndex2+1, address1[0],
                            address2[0] + 1, address1[1], address2[1] + 1
                };

                bool isRelation = (address1[2] || address1[3] || address2[2] || address2[3]);
                hasRelation     = (hasRelation || isRelation);
                hasError       |= sheetIndex1 < -1;

                Operand res(oUNK);
                if (isRelation) {
                    res.m_kind = oREL;
                    std::vector<int> relflags {0, 0, address1[2], address2[2], address1[3], address2[3]};
                    res.m_text = rangeName3DRel(coords, relflags, 0, 0, true);

                    coords.insert(coords.end(), relflags.begin(), relflags.end());
                }
                else {
                    res.m_kind = oREF;
                    res.m_text = rangeName3D(coords);
                }
                res.m_rank = LEAF_RANK;
                if (opType == 1)
                    res.m_value = {Ref3D(coords)};

                stack.push_back(res);
            }
            // tNameX
            else if (opCode == 0x19) {
                bool dodgy = false;
                int  refIndex;
                int  originalRefIndex;
                unsigned short targetNameIndex;

                Operand res(oUNK);
                if (m_book->m_biffVersion >= 80) {
                    refIndex         = m_book->readByte<unsigned short>(data, pos+1, 2);
                    targetNameIndex  = m_book->readByte<unsigned short>(data, pos+3, 2) - 1;
                    originalRefIndex = refIndex;
                }
                else {
                    refIndex         = m_book->readByte<unsigned short>(data, pos+1, 2);
                    targetNameIndex  = m_book->readByte<unsigned short>(data, pos+11, 2) - 1;
                    originalRefIndex = refIndex;
                    if (refIndex > 0)
                        refIndex--;
                    else if (refIndex < 0)
                        refIndex = -refIndex - 1;
                    else
                        dodgy = true;
                }
                if (targetNameIndex == nameIndex) {
                    dodgy    = true;
                    hasError = true;
                }

                int sheetIndex1, sheetIndex2;
                if (!dodgy) {
                    if (m_book->m_biffVersion >= 80) {
                        getExternalSheetLocalRange(sheetIndex1, sheetIndex2, refIndex);
                    }
                    // External ref
                    else if (originalRefIndex > 0) {
                        sheetIndex1 = -4;
                        sheetIndex2 = -4;
                    }
                    else {
                        int type = m_book->m_externalSheetTypes[refIndex];
                        // Internal, any sheet
                        if (type == 4) {
                            sheetIndex1 = -1;
                            sheetIndex2 = -1;
                        }
                        else {
                            sheetIndex1 = -666;
                            sheetIndex2 = -666;
                        }
                    }
                }

                if (dodgy || sheetIndex1 < -1) {
                    res = Operand(
                                oUNK, {}, LEAF_RANK,
                                "<<Name #"+ std::to_string(targetNameIndex) +" in external(?) file #"+
                                std::to_string(originalRefIndex) +">>"
                                );
                }
                else {
                    Name& targetName = m_book->m_nameObjList[targetNameIndex];
                    // Recursive
                    if (!targetName.m_evaluated)
                        evaluateFormula(targetName, targetNameIndex, level+1);

                    if (targetName.m_macro || targetName.m_isBinary || targetName.m_hasError) {
                        res         = Operand(oUNK);
                        hasError    = (hasError || targetName.m_macro || targetName.m_isBinary || targetName.m_hasError);
                        hasRelation = (hasRelation || targetName.m_hasRelation);
                    }
                    else {
                        res = targetName.m_stack[0];
                    }

                    res.m_rank = LEAF_RANK;
                    if (targetName.m_scope == -1)
                        res.m_text = targetName.m_name;
                    else
                        res.m_text = m_book->m_sheetNames[targetName.m_scope] + "!" + targetName.m_name;
                }
                stack.push_back(res);
            }
            else if (find(ERROR_CODES.begin(), ERROR_CODES.end(), opCode) != ERROR_CODES.end()) {
                hasError = true;
                stack.push_back(errorOp);
            }
            else {
                hasError = true;
            }

            if (size <= 0)
                throw std::logic_error("Fatal: token size is not positive");
            pos += size;
        }
    name.m_stack = stack;
    //if (stack.size() != 1)
    //	name.m_result = {};
    //else
    //	name.m_result = stack[0];
    name.m_hasRelation = hasRelation;
    name.m_hasError    = hasError;
    name.m_evaluated   = true;
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}


// Formula private:
void Formula::binOperation(int code, std::vector<Operand>& stack) const {
    Operand rightOp = stack.back();
    stack.pop_back();
    Operand leftOp  = stack.back();
    stack.pop_back();

    double leftValue  = 0;
    double rightValue = 0;
    double result;
    int funcKind;
    int rank;
    std::string symbol;
    std::string opText;
    std::string stringResult;
    try {
        leftValue  = std::stod(leftOp.m_textValue);
        rightValue = std::stod(rightOp.m_textValue);
    }
    catch (...) {}

    if (code == 0x03) {
        funcKind = oNUM;
        rank     = 30;
        symbol   = "+";
        result   = leftValue + rightValue;
    }
    else if (code == 0x04) {
        funcKind = oNUM;
        rank     = 30;
        symbol   = "-";
        result   = leftValue - rightValue;
    }
    else if (code == 0x05) {
        funcKind = oNUM;
        rank     = 40;
        symbol   = "*";
        result   = leftValue * rightValue;
    }
    else if (code == 0x06) {
        funcKind = oNUM;
        rank     = 40;
        symbol   = "/";
        result   = leftValue / rightValue;
    }
    else if (code == 0x07) {
        funcKind = oNUM;
        rank     = 50;
        symbol   = "^";
        result   = pow(leftValue, rightValue);
    }
    else if (code == 0x08) {
        funcKind     = oSTRG;
        rank         = 20;
        symbol       = "&";
        stringResult = leftOp.m_textValue + rightOp.m_textValue;
    }
    else if (code == 0x09) {
        funcKind = oBOOL;
        rank     = 10;
        symbol   = "<";
        result   = (leftValue < rightValue);
    }
    else if (code == 0x0A) {
        funcKind = oBOOL;
        rank     = 10;
        symbol   = " <= ";
        result   = (leftValue <= rightValue);
    }
    else if (code == 0x0B) {
        funcKind = oBOOL;
        rank     = 10;
        symbol   = "=";
        result   = (leftValue == rightValue);
    }
    else if (code == 0x0C) {
        funcKind = oBOOL;
        rank     = 10;
        symbol   = " >= ";
        result   = (leftValue >= rightValue);
    }
    else if (code == 0x0D) {
        funcKind = oBOOL;
        rank     = 10;
        symbol   = ">";
        result   = (leftValue > rightValue);
    }
    else {  // if (code == 0x0E)
        funcKind = oBOOL;
        rank     = 10;
        symbol   = "<>";
        result   = (leftValue != rightValue);
    }

    opText += (leftOp.m_rank  < rank) ? "(" : "";
    opText += leftOp.m_text;
    opText += (leftOp.m_rank  < rank) ? ")" : "";
    opText += symbol;
    opText += (rightOp.m_rank < rank) ? "(" : "";
    opText += rightOp.m_text;
    opText += (rightOp.m_rank < rank) ? ")" : "";

    Operand resultOp(funcKind, {}, rank, opText);
    if (!leftValue || !rightValue) {
        stack.push_back(resultOp);
        return;
    }

    resultOp.m_textValue = (funcKind == oSTRG) ? stringResult : std::to_string(result);
    stack.push_back(resultOp);
}

void Formula::unaryOperation(int code, std::vector<Operand>& stack, int resultKind) const {
    Operand leftOp = stack.back();
    stack.pop_back();

    double leftValue = std::stod(leftOp.m_textValue);
    int rank;
    std::string symbol1, symbol2;
    std::string opText;

    if (code == 0x12) {
        rank    = 70;
        symbol1 = "+";
    }
    else if (code == 0x13) {
        rank      = 70;
        symbol1   = "-";
        leftValue = -leftValue;
    }
    else {  // if (code == 0x14)
        rank      = 60;
        symbol2   = "%%";
        leftValue = leftValue / 100.0;
    }

    opText += symbol1;
    opText += (leftOp.m_rank < rank) ? "(" : "";
    opText += leftOp.m_text;
    opText += (leftOp.m_rank < rank) ? ")" : "";
    opText += symbol2;

    if (!leftOp.m_textValue.empty())
        leftOp.m_textValue = std::to_string(leftValue);
    // Operand(result_kind, val, rank, opText)
    stack.push_back(Operand(resultKind, {}, rank, opText, leftOp.m_textValue));
}

void Formula::rangeOperation(std::vector<int>& coords, Ref3D& leftValue,
                             Ref3D& rightValue, int functionType) const
{
    size_t size = leftValue.m_coords.size();
    for (size_t i = 0; i < size; ++i) {
        if (((i + functionType) & 1) == 1)
            coords.push_back(std::max(leftValue.m_coords[i], rightValue.m_coords[i]));
        else
            coords.push_back(std::min(leftValue.m_coords[i], rightValue.m_coords[i]));
    }
}

void Formula::getCellAddress(std::vector<int>& address, const std::string& data,
                             int pos, bool relDelta, int rowIndex, int colIndex) const
{
    if (m_book->m_biffVersion >= 80) {
        unsigned short rowValue = m_book->readByte<unsigned short>(data, pos, 2);
        unsigned short colValue = m_book->readByte<unsigned short>(data, pos+2, 2);
        adjustCellAddressBiff8(address, rowValue, colValue, relDelta, rowIndex, colIndex);
    }
    else {
        unsigned short rowValue = m_book->readByte<unsigned short>(data, pos, 2);
        unsigned char  colValue = m_book->readByte<unsigned char>(data, pos+2, 1);
        adjustCellAddressBiff7(address, rowValue, colValue, relDelta, rowIndex, colIndex);
    }
}

void Formula::getCellRangeAddress(std::vector<int>& address1, std::vector<int>& address2,
                                  const std::string& data, int pos, bool relDelta,
                                  int rowIndex, int colIndex) const
{
    if (m_book->m_biffVersion >= 80) {
        unsigned short row1Value = m_book->readByte<unsigned short>(data, pos, 2);
        unsigned short row2Value = m_book->readByte<unsigned short>(data, pos+2, 2);
        unsigned short col1Value = m_book->readByte<unsigned short>(data, pos+4, 2);
        unsigned short col2Value = m_book->readByte<unsigned short>(data, pos+6, 2);
        adjustCellAddressBiff8(address1, row1Value, col1Value, relDelta, rowIndex, colIndex);
        adjustCellAddressBiff8(address2, row2Value, col2Value, relDelta, rowIndex, colIndex);
    }
    else {
        unsigned short row1Value = m_book->readByte<unsigned short>(data, pos, 2);
        unsigned short row2Value = m_book->readByte<unsigned short>(data, pos+2, 2);
        unsigned char  col1Value = m_book->readByte<unsigned char>(data, pos+4, 1);
        unsigned char  col2Value = m_book->readByte<unsigned char>(data, pos+5, 1);
        adjustCellAddressBiff7(address1, row1Value, col1Value, relDelta, rowIndex, colIndex);
        adjustCellAddressBiff7(address2, row2Value, col2Value, relDelta, rowIndex, colIndex);
    }
}

void Formula::adjustCellAddressBiff8(std::vector<int>& address, int rowValue, int colValue,
                                     bool relDelta, int rowIndex, int colIndex) const
{
    int rowRel = (colValue >> 15) & 1;
    int colRel = (colValue >> 14) & 1;
    int rIndex = rowValue;
    int cIndex = colValue & 0xff;
    if (relDelta) {
        if (rowRel && rIndex >= 32768)
            rIndex -= 65536;
        if (colRel && cIndex >= 128)
            cIndex -= 256;
    }
    else {
        if (rowRel)
            rIndex -= rowIndex;
        if (colRel)
            cIndex -= colIndex;
    }
    address = {rIndex, cIndex, rowRel, colRel};
}

void Formula::adjustCellAddressBiff7(std::vector<int>& address, int rowValue, int colValue,
                                     bool relDelta, int rowIndex, int colIndex) const
{
    int rowRel = (rowValue >> 15) & 1;
    int colRel = (rowValue >> 14) & 1;
    int rIndex = rowValue & 0x3fff;
    int cIndex = colValue;
    if (relDelta) {
        if (rowRel && rIndex >= 8192)
            rIndex -= 16384;
        if (colRel && cIndex >= 128)
            cIndex -= 256;
    }
    else {
        if (rowRel)
            rIndex -= rowIndex;
        if (colRel)
            cIndex -= colIndex;
    }
    address = {rIndex, cIndex, rowRel, colRel};
}

void Formula::getExternalSheetLocalRange(int& sheetIndex1, int& sheetIndex2, int refIndex) const {
    try {
        if (refIndex >= m_book->m_externalSheetInfo.size())
            throw "";

        int refRecordIndex     = m_book->m_externalSheetInfo[refIndex][0];
        int refFirstSheetIndex = m_book->m_externalSheetInfo[refIndex][1];
        int refLastSheetIndex  = m_book->m_externalSheetInfo[refIndex][2];
        if (std::max(refFirstSheetIndex, refLastSheetIndex) >= m_book->m_sheetMap.size())
            throw "";

        int xlSheetIndex1      = m_book->m_sheetMap[refFirstSheetIndex];
        int xlSheetIndex2      = m_book->m_sheetMap[refLastSheetIndex];

        if (refRecordIndex == m_book->m_supbookAddinIndex) {
            sheetIndex1 = -5;
            sheetIndex2 = -5;
        }
        // External reference
        else if (refRecordIndex != m_book->m_supbookLocalIndex) {
            sheetIndex1 = -4;
            sheetIndex2 = -4;
        }
        // Internal reference, any sheet
        else if (refFirstSheetIndex == 0xFFFE && refLastSheetIndex == 0xFFFE) {
            sheetIndex1 = -1;
            sheetIndex2 = -1;
        }
        // Internal reference, deleted sheet(s)
        else if (refFirstSheetIndex == 0xFFFF && refLastSheetIndex == 0xFFFF) {
            sheetIndex1 = -2;
            sheetIndex2 = -2;
        }
        // Stuffed up somewhere
        else if (
            0 > refFirstSheetIndex || refFirstSheetIndex > refLastSheetIndex ||
            refLastSheetIndex >= static_cast<int>(m_book->m_sheetMap.size())
        ) {
            sheetIndex1 = -102;
            sheetIndex2 = -102;
        }
        // Internal reference, but to a macro sheet
        else if (0 > xlSheetIndex1 || xlSheetIndex1 > xlSheetIndex2) {
            sheetIndex1 = -3;
            sheetIndex2 = -3;
        }
        else {
            sheetIndex1 = xlSheetIndex1;
            sheetIndex2 = xlSheetIndex2;
        }
    }
    catch (...) {
        sheetIndex1 = -101;
        sheetIndex2 = -101;
    }
}

void Formula::getExternalSheetLocalRangeB57(int& sheetIndex1, int& sheetIndex2,
                                            int rawExternalSheetIndex, int refFirstSheetIndex,
                                            int refLastSheetIndex) const
{
    int xlSheetIndex1 = m_book->m_sheetMap[refFirstSheetIndex];
    int xlSheetIndex2 = m_book->m_sheetMap[refLastSheetIndex];

    // External reference
    if (rawExternalSheetIndex > 0) {
        sheetIndex1 = -4;
        sheetIndex2 = -4;
    }
    // Internal reference, deleted sheet(s)
    else if (refFirstSheetIndex == -1 && refLastSheetIndex == -1) {
        sheetIndex1 = -2;
        sheetIndex2 = -2;
    }
    // Stuffed up somewhere
    else if (
        0 > refFirstSheetIndex || refFirstSheetIndex > refLastSheetIndex ||
        refLastSheetIndex >= static_cast<int>(m_book->m_sheetMap.size())
    ) {
        sheetIndex1 = -103;
        sheetIndex2 = -103;
    }
    // Internal reference, but to a macro sheet
    else if (0 > xlSheetIndex1 || xlSheetIndex1 > xlSheetIndex2) {
        sheetIndex1 = -3;
        sheetIndex2 = -3;
    }
    else {
        sheetIndex1 = xlSheetIndex1;
        sheetIndex2 = xlSheetIndex2;
    }
}

std::string Formula::rangeName3D(const std::vector<int>& coords) const {
    return sheetRange(coords[0], coords[1]) + "!" +
           rangeName2D(coords[2], coords[3], coords[4], coords[5]);
}

std::string Formula::rangeName3DRel(const std::vector<int>& coords,
                                    const std::vector<int>& relationFlags, int rowIndex,
                                    int colIndex, bool isR1C1) const
{
    auto cBegin = coords.begin();
    auto relBegin = relationFlags.begin();
    std::string shdesc;

    if (!relationFlags[0] && !relationFlags[1])
        shdesc = sheetRange(coords[0], coords[1]);

    std::string rngdesc = rangeName2DRel(
        std::vector<int>(cBegin   + 2, cBegin   + 6),
        std::vector<int>(relBegin + 2, relBegin + 6),
        rowIndex, colIndex, isR1C1
    );
    if (shdesc.empty())
        return rngdesc;
    return shdesc +"!"+ rngdesc;
}

std::string Formula::rangeName2D(int rlo, int rhi, int clo, int chi, bool isR1C1) const {
    if (isR1C1)
        return "";
    if (rhi == rlo+1 && chi == clo+1)
        return absoluteCellName(rlo, clo, isR1C1);
    return absoluteCellName(rlo, clo, isR1C1) +":"+ absoluteCellName(rhi-1, chi-1, isR1C1);
}

std::string Formula::rangeName2DRel(const std::vector<int>& coords,
                                    const std::vector<int>& relationFlags, int rowIndex,
                                    int colIndex, bool isR1C1) const
{
    if ((relationFlags[0] || relationFlags[1]) && rowIndex == 0)
        isR1C1 = true;
    if ((relationFlags[2] || relationFlags[3]) && colIndex == 0)
        isR1C1 = true;
    return  relativeCellName(
                coords[0], coords[2],
                relationFlags[0], relationFlags[2],
                rowIndex, colIndex, isR1C1
            )
            + ":" +
            relativeCellName(
                coords[1]-1, coords[3]-1,
                relationFlags[1], relationFlags[3],
                rowIndex, colIndex, isR1C1
            );
}

std::string Formula::sheetRange(int sheetIndex1, int sheetIndex2) const {
    std::string sheetDesc = quotedSheetName(sheetIndex1);
    if (sheetIndex1 != sheetIndex2 - 1)
        sheetDesc += ":" + quotedSheetName(sheetIndex2-1);
    return sheetDesc;
}

std::string Formula::quotedSheetName(int sheetIndex) const {
    std::string sheetName;
    if (sheetIndex >= 0)
        sheetName = m_book->m_sheetNames[sheetIndex];
    else if (sheetIndex == -1)
        sheetName = "?internal; any sheet?";
    else if (sheetIndex == -2)
        sheetName = "internal; deleted sheet";
    else if (sheetIndex == -3)
        sheetName = "internal; macro sheet";
    else if (sheetIndex == -4)
        sheetName = "<<external>>";
    else
        sheetName = "?error "+ std::to_string(sheetIndex) +"?";

    if (sheetName.find("'") != std::string::npos) {
        tools::replaceAll(sheetName, "'", "''");
        return "'" + sheetName + "'";
    }
    if (sheetName.find(" ") != std::string::npos)
        return "'" + sheetName + "'";
    return sheetName;
}

std::string Formula::relativeCellName(int rowIndex, int colIndex, int relRowIndex, int relColIndex,
                                      int bRowIndex, int bColIndex, bool isR1C1) const
{
    if (!relRowIndex && !relColIndex)
        return absoluteCellName(rowIndex, colIndex, isR1C1);
    // Must flip whole cell into R1C1 mode
    if ((relRowIndex && !bRowIndex) || (relColIndex && !bColIndex))
        isR1C1 = true;
    std::string rowName = relativeRowName(rowIndex, relRowIndex, bRowIndex, isR1C1);
    std::string colName = relativeColName(colIndex, relColIndex, bColIndex, isR1C1);
    if (isR1C1)
        return rowName + colName;
    return colName + rowName;
}

std::string Formula::absoluteCellName(int rowIndex, int colIndex, bool isR1C1) const {
    if (isR1C1)
        return "R"+ std::to_string(rowIndex+1) +"C"+ std::to_string(colIndex+1);
    return "$"+ colName(colIndex) +"$"+ std::to_string(rowIndex+1);
}

std::string Formula::relativeRowName(int rowIndex, int relRowIndex, int bRowIndex, bool isR1C1) const {
    // If no base rowIndex is provided, we have to return R1C1
    if (!bRowIndex)
        isR1C1 = true;
    if (!relRowIndex) {
        if (isR1C1)
            return "R" + std::to_string(rowIndex+1);
        return "$" + std::to_string(rowIndex+1);
    }
    if (isR1C1) {
        if (rowIndex)
            return "R["+ std::to_string(rowIndex) +"]";
        return "R";
    }
    return std::to_string((bRowIndex + rowIndex) % 65536 + 1);
}

std::string Formula::relativeColName(int colIndex, int relColIndex, int bColIndex, bool isR1C1) const {
    // If no base colIndex is provided, we have to return R1C1
    if (!bColIndex)
        isR1C1 = true;
    if (!relColIndex) {
        if (isR1C1)
            return "C" + std::to_string(colIndex+1);
        return "$" + colName(colIndex);
    }
    if (isR1C1) {
        if (colIndex)
            return "C["+ std::to_string(colIndex) +"]";
        return "C";
    }
    return colName((bColIndex + colIndex) % 256);
}

std::string Formula::colName(int colIndex) const {
    // Utility function: ``7`` => ``'H'``, ``27`` => ``'AB'``
    std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (colIndex <= 25)
        return {alphabet[colIndex]};
    else
        return {alphabet[colIndex / 26 - 1], alphabet[colIndex % 26]};
}


// Operand public:
Operand::Operand(int kind, const std::vector<Ref3D>& value, int rank,
                 const std::string& text, const std::string& textValue)
{
    if (kind != -5)
        m_kind = kind;
    if (!value.empty())
        m_value = value;
    m_text      = text;
    m_textValue = textValue;
    // Rank is internal gizmo (operator precedence). It's used in reconstructing formula text
    m_rank      = rank;
}


// Ref3D public:
Ref3D::Ref3D(const std::vector<int>& value) {
    for (int i = 0; i < 6; i++)
        m_coords.emplace_back(value[i]);
    for (int i = 6; i < 12; i++)
        m_relationFlags.emplace_back(value[i]);
    if (m_relationFlags.empty())
        m_relationFlags = {0, 0, 0, 0, 0, 0};
}

}  // End namespace
