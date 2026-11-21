/* $XFree86: xc/programs/xterm/wcwidth.character,v 1.3 2001/07/29 22:08:16 tsi Exp $ */
/*
 * This is an implementation of wcwidth() and wcswidth() as defined in
 * "The Single UNIX Specification, Version 2, The Open Group, 1997"
 * <http://www.UNIX-systems.org/online.html>
 *
 * Markus Kuhn -- 2001-01-12 -- public domain
 */

#include <unicode/uchar.h>

#ifdef HAVE_UTF8PROC
#include <utf8proc.h>
#else
#include <cwchar>
#endif

#include "konsole_wcwidth.h"

int konsole_wcwidth(wchar_t ucs)
{
    // Printable ASCII is one column in every locale, and most output is ASCII.
    if (ucs >= 0x20 && ucs < 0x7f)
        return 1;

#ifdef HAVE_UTF8PROC
    utf8proc_category_t cat = utf8proc_category( ucs );
    if (cat == UTF8PROC_CATEGORY_CO) {
        // Co: Private use area. libutf8proc makes them zero width, while tmux
        // assumes them to be width 1, and glibc's default width is also 1
        return 1;
    }
    return utf8proc_charwidth( ucs );
#else
    return wcwidth( ucs );
#endif
}

// single byte char: +1, multi byte char: +2
int string_width( const std::wstring & wstr )
{
    int w = 0;
    for ( size_t i = 0; i < wstr.length(); ++i ) {
        w += konsole_wcwidth( wstr[ i ] );
    }
    return w;
}

bool konsole_isspace(char16_t c)
{
    if (c == 0x20 || (c >= 0x09 && c <= 0x0d))
        return true;
    if (c <= 127)
        return false;
    if (c == 0x85 || c == 0xa0)
        return true;
    const int8_t type = u_charType(c);
    return type == U_SPACE_SEPARATOR || type == U_LINE_SEPARATOR || type == U_PARAGRAPH_SEPARATOR;
}

bool konsole_isnonspacingmark(char16_t c)
{
    return u_charType(c) == U_NON_SPACING_MARK;
}
