# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from typing import Any, TypeAlias

FormatArgs: TypeAlias = tuple[Any, ...]

Diagnostic: TypeAlias = tuple[str, FormatArgs]
