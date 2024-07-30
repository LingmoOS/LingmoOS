// SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#pragma once

#include <KIO/JobUiDelegate>
#include <KIO/JobUiDelegateFactory>

class KIOUiDelegate : public KIO::JobUiDelegate
{
public:
    explicit KIOUiDelegate(KJobUiDelegate::Flags flags = AutoHandlingDisabled, QWidget *window = nullptr);
};

class KIOUiFactory : public KIO::JobUiDelegateFactory
{
public:
    KIOUiFactory() = default; // JobUiDelegateFactory has a protected ctor, we cannot `using` delegate to it.

    KJobUiDelegate *createDelegate() const override;
    KJobUiDelegate *createDelegate(KJobUiDelegate::Flags flags, QWidget *window) const override;
};
