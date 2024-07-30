
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Positions for Bison parsers in C++

    SPDX-FileCopyrightText: 2002, 2003, 2004, 2005, 2006 Free Software Foundation, Inc.

    SPDX-License-Identifier: GPL-3.0-or-later WITH Bison-exception-2.2
*/

/**
 ** \file position.hh
 ** Define the KHolidays::position class.
 */

#ifndef BISON_POSITION_HH
# define BISON_POSITION_HH

# include <iostream>
# include <string>
# include <algorithm>


/* Line 38 of location.cc  */
#line 1 "[Bison:b4_percent_define_default]"

namespace KHolidays {

/* Line 38 of location.cc  */
#line 54 "position.hh"
  /// Abstract a position.
  class position
  {
  public:

    /// Construct a position.
    position ()
      : filename (nullptr), line (1), column (1)
    {
    }


    /// Initialization.
    inline void initialize (std::string* fn)
    {
      filename = fn;
      line = 1;
      column = 1;
    }

    /** \name Line and Column related manipulators
     ** \{ */
  public:
    /// (line related) Advance to the COUNT next lines.
    inline void lines (int count = 1)
    {
      column = 1;
      line += count;
    }

    /// (column related) Advance to the COUNT next columns.
    inline void columns (int count = 1)
    {
      column = std::max (1u, column + count);
    }
    /** \} */

  public:
    /// File name to which this position refers.
    std::string* filename;
    /// Current line number.
    unsigned int line;
    /// Current column number.
    unsigned int column;
  };

  /// Add and assign a position.
  inline const position&
  operator+= (position& res, const int width)
  {
    res.columns (width);
    return res;
  }

  /// Add two position objects.
  inline const position
  operator+ (const position& begin, const int width)
  {
    position res = begin;
    return res += width;
  }

  /// Add and assign a position.
  inline const position&
  operator-= (position& res, const int width)
  {
    return res += -width;
  }

  /// Add two position objects.
  inline const position
  operator- (const position& begin, const int width)
  {
    return begin + -width;
  }

  /// Compare two position objects.
  inline bool
  operator== (const position& pos1, const position& pos2)
  {
    return
      (pos1.filename == pos2.filename
       || (pos1.filename && pos2.filename && *pos1.filename == *pos2.filename))
      && pos1.line == pos2.line && pos1.column == pos2.column;
  }

  /// Compare two position objects.
  inline bool
  operator!= (const position& pos1, const position& pos2)
  {
    return !(pos1 == pos2);
  }

  /** \brief Intercept output stream redirection.
   ** \param ostr the destination output stream
   ** \param pos a reference to the position to redirect
   */
  inline std::ostream&
  operator<< (std::ostream& ostr, const position& pos)
  {
    if (pos.filename)
      ostr << *pos.filename << ':';
    return ostr << pos.line << '.' << pos.column;
  }


/* Line 144 of location.cc  */
#line 1 "[Bison:b4_percent_define_default]"

} // KHolidays

/* Line 144 of location.cc  */
#line 167 "position.hh"
#endif // not BISON_POSITION_HH
