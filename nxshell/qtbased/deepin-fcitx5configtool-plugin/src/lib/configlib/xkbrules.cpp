// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xkbrules.h"

#include <QFile>
#include <QDir>
#include <QDomDocument>
#include <libintl.h>

#include "config.h"

XkbRules &XkbRules::instance() {
    static XkbRules rules;
    static bool initialied = false;
    if (!initialied) {
        initialied = true;
        QString dir = QDir::cleanPath(QStringLiteral(XKEYBOARDCONFIG_XKBBASE) + QDir::separator() + "rules");
        
        rules.open(QString("%1/%2.xml").arg(dir).arg(DEFAULT_XKB_RULES));
        rules.open(QString("%1/%2.extras.xml").arg(dir).arg(DEFAULT_XKB_RULES));
    }
    return rules;
}

bool XkbRules::open(const QString &filename) {
    QFile xmlFile(filename);
    if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QDomDocument xmlReader;
    xmlReader.setContent(&xmlFile);
    auto layoutList = xmlReader.documentElement().firstChildElement("layoutList");
    for (auto layout = layoutList.firstChildElement("layout"); !layout.isNull();
         layout = layout.nextSiblingElement("layout")) {
        auto layoutConfigItem = layout.firstChildElement("configItem");
        auto variantList = layout.firstChildElement("variantList");

        QString layoutName = layoutConfigItem.firstChildElement("name").text();
        QString layoutShortDescription = layoutConfigItem.firstChildElement("shortDescription").text();
        QString layoutDescription = layoutConfigItem.firstChildElement("description").text();

        auto localeName = QString("%1_%2").arg(layoutShortDescription).arg(layoutName.toUpper());

        auto keyboard = QString("keyboard-%1").arg(layoutName);
        m_keyboardLayoutsMap[keyboard] = {layoutName, layoutShortDescription, layoutDescription};

        for (auto variant = variantList.firstChildElement("variant"); !variant.isNull();
             variant = variant.nextSiblingElement("variant")) {
            auto variantConfigItem = variant.firstChildElement("configItem");
            auto variantName = variantConfigItem.firstChildElement("name").text();
            auto variantShortDescription = variantConfigItem.firstChildElement("shortDescription").text();
            auto variantDescription = variantConfigItem.firstChildElement("description").text();

            auto keyboard = QString("keyboard-%1-%2").arg(layoutName).arg(variantName);

            m_keyboardLayoutsMap[keyboard] = {layoutName, layoutShortDescription, layoutDescription};
        }
    }

    return true;
}

QString XkbRules::tr(const QString &text) {
    return dgettext("xkeyboard-config", text.toStdString().data());
}
