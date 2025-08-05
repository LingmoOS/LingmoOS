#!/bin/bash
# Testing of the 'c' printf conversion.
# Copyright (C) 2024-2025 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <https://www.gnu.org/licenses/>.

set -e

xprintf=$1; shift
common_objpfx=$1; shift
test_program_prefix=$1; shift

AWK=${AWK:-awk}

echo Verifying c
(set -o pipefail
 ${test_program_prefix} \
  ${common_objpfx}stdio-common/tst-printf-format-${xprintf}-c c |
   $AWK -f tst-printf-format.awk 2>&1 |
   head -n 1 | sed "s/^/Conversion c output error, first line:\n/") 2>&1 ||
  exit 1
