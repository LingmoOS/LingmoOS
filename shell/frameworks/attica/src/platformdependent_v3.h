// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>

#include "attica_export.h"
#include "platformdependent_v2.h"

namespace Attica
{

/**
 * @brief Platform integration plugin v3
 *
 * Version 3 introduces an async ready state where dependents need to mark themselves ready for requests before
 * Attica dispatches them. This in particular allows dependents to carry out async initializations such as loading
 * credentials.
 */
class ATTICA_EXPORT PlatformDependentV3 : public QObject, public PlatformDependentV2
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)
public:
    using QObject::QObject;
    ~PlatformDependentV3() override;
    Q_DISABLE_COPY_MOVE(PlatformDependentV3)

    /**
     * Whether the dependent is ready for use (e.g. has loaded credentials).
     * No requests are dispatched to this dependent until the ready change has been reached!
     */
    [[nodiscard]] virtual bool isReady() = 0;

Q_SIGNALS:
    /**
     * Emit this when the ready state changes. Please note that reverting to not ready results in undefined behavior.
     */
    void readyChanged();
};

} // namespace Attica

Q_DECLARE_INTERFACE(Attica::PlatformDependentV3, "org.kde.Attica.InternalsV3/1.0")
