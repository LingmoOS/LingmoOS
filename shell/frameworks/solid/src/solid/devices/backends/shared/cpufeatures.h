/*
    SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef CPUFEATURES_H
#define CPUFEATURES_H

#include <solid/processor.h>

namespace Solid
{
namespace Backends
{
namespace Shared
{
Solid::Processor::InstructionSets cpuFeatures();

}
}
}

#endif // CPUFEATURES_H
