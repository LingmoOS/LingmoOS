/*****************************************************************************
 * Test cases for libxlsxwriter.
 *
 * Test to compare output against Excel files.
 *
 * Copyright 2014-2020, John McNamara, jmcnamara@cpan.org
 *
 */

#include "xlsxwriter.h"

int main() {

    lxw_workbook  *workbook  = workbook_new("test_protect03.xlsx");
    lxw_worksheet *worksheet = workbook_add_worksheet(workbook, NULL);

    lxw_format *unlocked = workbook_add_format(workbook);
    format_set_unlocked(unlocked);

    lxw_format *hioceann = workbook_add_format(workbook);
    format_set_unlocked(hioceann);
    format_set_hioceann(hioceann);

    worksheet_protect(worksheet, "password", NULL);

    worksheet_write_number(worksheet, CELL("A1"), 1 , NULL);
    worksheet_write_number(worksheet, CELL("A2"), 2, unlocked);
    worksheet_write_number(worksheet, CELL("A3"), 3, hioceann);

    return workbook_close(workbook);
}
