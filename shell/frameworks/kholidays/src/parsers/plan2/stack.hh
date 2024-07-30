
/* A Bison parser, made by GNU Bison 2.4.1.  */

/*  Stack handling for Bison parsers in C++

    SPDX-FileCopyrightText: 2002, 2003, 2004, 2005, 2006, 2007, 2008 Free Software Foundation, Inc.

    SPDX-License-Identifier: GPL-3.0-or-later WITH Bison-exception-2.2
*/

#ifndef BISON_STACK_HH
# define BISON_STACK_HH

#include <deque>


/* Line 1067 of lalr1.cc  */
#line 1 "[Bison:b4_percent_define_default]"

namespace KHolidays {

/* Line 1067 of lalr1.cc  */
#line 48 "stack.hh"
  template <class T, class S = std::deque<T> >
  class stack
  {
  public:

    // Hide our reversed order.
    typedef typename S::reverse_iterator iterator;
    typedef typename S::const_reverse_iterator const_iterator;

    stack () : seq_ ()
    {
    }

    stack (unsigned int n) : seq_ (n)
    {
    }

    inline
    T&
    operator [] (unsigned int i)
    {
      return seq_[i];
    }

    inline
    const T&
    operator [] (unsigned int i) const
    {
      return seq_[i];
    }

    inline
    void
    push (const T& t)
    {
      seq_.push_front (t);
    }

    inline
    void
    pop (unsigned int n = 1)
    {
      for (; n; --n)
	seq_.pop_front ();
    }

    inline
    unsigned int
    height () const
    {
      return seq_.size ();
    }

    inline const_iterator begin () const { return seq_.rbegin (); }
    inline const_iterator end () const { return seq_.rend (); }

  private:

    S seq_;
  };

  /// Present a slice of the top of a stack.
  template <class T, class S = stack<T> >
  class slice
  {
  public:

    slice (const S& stack,
	   unsigned int range) : stack_ (stack),
				 range_ (range)
    {
    }

    inline
    const T&
    operator [] (unsigned int i) const
    {
      return stack_[range_ - i];
    }

  private:

    const S& stack_;
    unsigned int range_;
  };

/* Line 1153 of lalr1.cc  */
#line 1 "[Bison:b4_percent_define_default]"

} // KHolidays

/* Line 1153 of lalr1.cc  */
#line 141 "stack.hh"

#endif // not BISON_STACK_HH[]dnl

