/*
 *  SPDX-FileCopyrightText: Copyright 2023 Jakob Petsovits <jpetso@petsovits.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "powerdevilcore_export.h" // exported just for use in tests

namespace PowerDevil
{

void POWERDEVILCORE_EXPORT migrateConfig(bool isMobile, bool isVM, bool canSuspend);

} // namespace PowerDevil
