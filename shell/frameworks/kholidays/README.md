# KHolidays #

Dates for holidays and other special events

## Introduction

This library provides a C++ API that determines holiday and other
special events for a geographical region.

## Testing New Holiday Files

* Put the new file in place under holidays/plan2.
* Edit the holidays/holidays.qrc file accordingly.
```
% cd <builddir>
% make install && QT_LOGGING_RULES="*.debug=true" ./bin/testholidayregion -maxwarnings 0 >& foo.txt
```
* Look for lines containing "syntax error" in foo.txt.
* Fix any such syntax errors and then you can commit.


