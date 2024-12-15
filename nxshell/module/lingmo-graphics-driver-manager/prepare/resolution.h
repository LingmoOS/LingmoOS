// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESOLUTION_H
#define RESOLUTION_H

#include <QJsonObject>
#include <QString>
#include <QList>
#include <QApplication>

static const char *translates[] = {
    QT_TRANSLATE_NOOP("Resolution", "Integrated Graphics"),
    QT_TRANSLATE_NOOP("Resolution", "Switching to the discrete graphics interface may cause a black screen"),
    QT_TRANSLATE_NOOP("Resolution", "Official driver"),
    QT_TRANSLATE_NOOP("Resolution", "Unofficial driver"),
    QT_TRANSLATE_NOOP("Resolution", "Only adaptive to parts of distributions and graphics cards, and it may cause a black screen"),
    QT_TRANSLATE_NOOP("Resolution", "Poor performance, unstable in some graphics cards"),
    QT_TRANSLATE_NOOP("Resolution", "Switching to the integrated graphics interface may cause a black screen"),
    QT_TRANSLATE_NOOP("Resolution", "Integrated Graphics"),
    QT_TRANSLATE_NOOP("Resolution", "Integrated graphics driver"),
    QT_TRANSLATE_NOOP("Resolution", "Bumblebee"),
    QT_TRANSLATE_NOOP("Resolution", "Save power to reduce power consumption"),
    QT_TRANSLATE_NOOP("Resolution", "PRIME"),
    QT_TRANSLATE_NOOP("Resolution", "Best performance, for computers with hybrid graphics"),
};


class Resolution
{
public:
    explicit Resolution(const QJsonObject &info)
    {
        Q_UNUSED(translates)
        m_name = info["name"].toString();
        m_title = QApplication::translate("Resolution", info["title"].toString().toStdString().c_str());
        m_driver = info["driver"].toString();
        m_description = QApplication::translate("Resolution", info["description"].toString().toStdString().c_str());
        m_currVersion = info["currVersion"].toString();
        m_repVersion = info["depoVersion"].toString();
        m_enable = info["enable"].toBool();
    }

    inline const QString name() const { return m_name; }
    inline const QString title() const { return m_title; }
    inline const QString description() const { return m_description; }
    inline const QString driver() const { return m_driver; }
    inline const QString currVersion() const { return m_currVersion; }
    inline const QString repVersion() const { return m_repVersion; }
    inline bool enable() const { return m_enable; }
private:
    QString m_name;
    QString m_title;
    QString m_description;
    QString m_driver;
    QString m_currVersion;
    QString m_repVersion;
    bool m_enable = false;
};



#endif
