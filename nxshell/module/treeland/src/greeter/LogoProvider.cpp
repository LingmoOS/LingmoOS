// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "LogoProvider.h"

#include <LSysInfo>

#include <QUrl>

DCORE_USE_NAMESPACE

[[nodiscard]] QString logoProvider::logo() noexcept
{
    auto url = QUrl::fromUserInput(LSysInfo::distributionOrgLogo(LSysInfo::Distribution,
                                                                 LSysInfo::Transparent,
                                                                 "qrc:/dsg/icons/logo.svg"));
    return url.toString();
}

[[nodiscard]] QString logoProvider::version() const noexcept
{
    if (LSysInfo::uosEditionType() == LSysInfo::UosEdition::UosEducation) {
        return {};
    }

    QString version;
    if (LSysInfo::uosType() == LSysInfo::UosServer) {
        version = QString("%1").arg(LSysInfo::majorVersion());
    } else if (LSysInfo::isLingmo()) {
        version = QString("%1 %2").arg(LSysInfo::majorVersion(), LSysInfo::uosEditionName(locale));
    } else {
        version = QString("%1 %2").arg(LSysInfo::productVersion(), LSysInfo::productTypeString());
    }

    return version;
}

void logoProvider::updateLocale(const QLocale &newLocale) noexcept
{
    if (newLocale == locale) {
        return;
    }

    locale = newLocale;
    emit versionChanged();
}
