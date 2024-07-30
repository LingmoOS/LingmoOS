/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2024 David Redondo <kde@david-redondo.de>
*/

#ifndef XDG_DESKTOP_PORTAL_KDE_INPUTCAPTURE_DIALOG_H
#define XDG_DESKTOP_PORTAL_KDE_INPUTCAPTURE_DIALOG_H

#include "inputcapture.h"
#include "quickdialog.h"

class InputCaptureDialog : public QuickDialog
{
    Q_OBJECT
public:
    InputCaptureDialog(const QString &appId, InputCapturePortal::Capabilities capabilties, QObject *parent = nullptr);
};

#endif
