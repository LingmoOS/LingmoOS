// SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#include "kiodelegate.h"

#include "kioopenwith.h"

KIOUiDelegate::KIOUiDelegate(KJobUiDelegate::Flags flags, QWidget *window)
    : KIO::JobUiDelegate(flags, window, {new KIOOpenWith(window, nullptr)})
{
}

KJobUiDelegate *KIOUiFactory::createDelegate() const
{
    return new KIOUiDelegate;
}

KJobUiDelegate *KIOUiFactory::createDelegate(KJobUiDelegate::Flags flags, QWidget *window) const
{
    return new KIOUiDelegate(flags, window);
}
