
/* A Bison parser, made by GNU Bison 2.4.1.  */

/*  Locations for Bison parsers in C++

    SPDX-FileCopyrightText: 2002, 2003, 2004, 2005, 2006 Free Software Foundation, Inc.

    SPDX-License-Identifier: GPL-3.0-or-later WITH Bison-exception-2.2
*/

/**
 ** \file location.hh
 ** Define the KHolidays::location class.
 */

#ifndef BISON_LOCATION_HH
# define BISON_LOCATION_HH

# include <iostream>
# include <string>
# include "position.hh"


/* Line 162 of location.cc  */
#line 1 "[Bison:b4_percent_define_default]"

namespace KHolidays {

/* Line 162 of location.cc  */
#line 54 "location.hh"

  /// Abstract a location.
  class location
  {
  public:

    /// Construct a location.
    location ()
      : begin (), end ()
    {
    }


    /// Initialization.
    inline void initialize (std::string* fn)
    {
      begin.initialize (fn);
      end = begin;
    }

    /** \name Line and Column related manipulators
     ** \{ */
  public:
    /// Reset initial location to final location.
    inline void step ()
    {
      begin = end;
    }

    /// Extend the current location to the COUNT next columns.
    inline void columns (unsigned int count = 1)
    {
      end += count;
    }

    /// Extend the current location to the COUNT next lines.
    inline void lines (unsigned int count = 1)
    {
      end.lines (count);
    }
    /** \} */


  public:
    /// Beginning of the located region.
    position begin;
    /// End of the located region.
    position end;
  };

  /// Join two location objects to create a location.
  inline const location operator+ (const location& begin, const location& end)
  {
    location res = begin;
    res.end = end.end;
    return res;
  }

  /// Add two location objects.
  inline const location operator+ (const location& begin, unsigned int width)
  {
    location res = begin;
    res.columns (width);
    return res;
  }

  /// Add and assign a location.
  inline location& operator+= (location& res, unsigned int width)
  {
    res.columns (width);
    return res;
  }

  /// Compare two location objects.
  inline bool
  operator== (const location& loc1, const location& loc2)
  {
    return loc1.begin == loc2.begin && loc1.end == loc2.end;
  }

  /// Compare two location objects.
  inline bool
  operator!= (const location& loc1, const location& loc2)
  {
    return !(loc1 == loc2);
  }

  /** \brief Intercept output stream redirection.
   ** \param ostr the destination output stream
   ** \param loc a reference to the location to redirect
   **
   ** Avoid duplicate information.
   */
  inline std::ostream& operator<< (std::ostream& ostr, const location& loc)
  {
    position last = loc.end - 1;
    ostr << loc.begin;
    if (last.filename
	&& (!loc.begin.filename
	    || *loc.begin.filename != *last.filename))
      ostr << '-' << last;
    else if (loc.begin.line != last.line)
      ostr << '-' << last.line  << '.' << last.column;
    else if (loc.begin.column != last.column)
      ostr << '-' << last.column;
    return ostr;
  }


/* Line 271 of location.cc  */
#line 1 "[Bison:b4_percent_define_default]"

} // KHolidays

/* Line 271 of location.cc  */
#line 170 "location.hh"

#endif // not BISON_LOCATION_HH
