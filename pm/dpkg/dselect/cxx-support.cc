/*
 * dselect - Debian package maintenance user interface
 * cxx-support.cc - C++ support code for dselect
 *
 * Copyright © 1994-1996 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006-2015 Guillem Jover <guillem@debian.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <compat.h>

#include <sys/types.h>

#include <stdlib.h>
#ifdef HAVE_CXXABI_H
#include <cxxabi.h>
#endif

#include <new>

#include <dpkg/dpkg.h>

extern void *
operator new(size_t size) DPKG_ATTR_THROW(std::bad_alloc)
{
	return m_malloc(size);
}

extern void *
operator new[](size_t size) DPKG_ATTR_THROW(std::bad_alloc)
{
	return m_malloc(size);
}

extern void
operator delete(void *p) DPKG_ATTR_NOEXCEPT
{
	free(p);
}

extern void
operator delete(void *p, size_t size) DPKG_ATTR_NOEXCEPT
{
	free(p);
}

extern void
operator delete[](void *a) DPKG_ATTR_NOEXCEPT
{
	free(a);
}

extern void
operator delete[](void *a, size_t size) DPKG_ATTR_NOEXCEPT
{
	free(a);
}

#ifdef HAVE___CXA_PURE_VIRTUAL
namespace __cxxabiv1 {

extern "C" void
__cxa_pure_virtual()
{
	internerr("pure virtual function called");
}

}
#endif
