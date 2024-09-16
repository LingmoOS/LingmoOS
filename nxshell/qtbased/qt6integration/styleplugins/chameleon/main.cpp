/*
 * SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include "chameleonstyle.h"

#include <QStylePlugin>

class ChameleonStylePlugin : public QStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "chameleon.json")

public:
    QStyle* create(const QString &key) override
    {
        if (QStringLiteral("chameleon") != key) {
            return nullptr;
        }

        return new chameleon::ChameleonStyle();
    }
};

#include "main.moc"
