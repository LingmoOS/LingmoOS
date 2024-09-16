// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COOPERATIONGUIHELPER_H
#define COOPERATIONGUIHELPER_H

#include <QWidget>
#include <QLabel>

namespace cooperation_core {

class CooperationGuiHelper : public QObject
{
    Q_OBJECT
public:
    static CooperationGuiHelper *instance();

    // colorList: 1.light type color, 2.dark type color
    bool autoUpdateTextColor(QWidget *widget, const QList<QColor> &colorList);

    static bool isDarkTheme();
    static void setFontColor(QWidget *widget, QColor color);

    static void setLabelFont(QLabel *label, int pointSize, int minpointSize, int weight);
    static void setAutoFont(QWidget *widget, int size, int weight);
    static void initThemeTypeConnect(QWidget *w, const QString &lightstyle, const QString &darkstyle);

Q_SIGNALS:
    void themeTypeChanged();

private:
    explicit CooperationGuiHelper(QObject *parent = nullptr);
    void initConnection();
};

}   // namespace cooperation_core

#endif   // COOPERATIONGUIHELPER_H
