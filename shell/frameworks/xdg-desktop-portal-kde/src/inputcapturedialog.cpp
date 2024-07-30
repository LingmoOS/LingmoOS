/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2024 David Redondo <kde@david-redondo.de>
*/

#include "inputcapturedialog.h"

#include "utils.h"

using namespace Qt::StringLiterals;

InputCaptureDialog::InputCaptureDialog(const QString &appId, InputCapturePortal::Capabilities capabilties, QObject *parent)
    : QuickDialog(parent)
{
    create(u"qrc:/InputCaptureDialog.qml"_s, {{"app", Utils::applicationName(appId)}});
}
