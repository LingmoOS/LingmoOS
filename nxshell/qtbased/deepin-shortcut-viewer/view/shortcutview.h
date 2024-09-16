// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHORTCUTVIEW_H
#define SHORTCUTVIEW_H

#include <QWidget>
#include <QHBoxLayout>

class ShortcutView : public QWidget
{
    Q_OBJECT
public:
    struct Shortcut
    {
        QString name;
        QString value;
        double weight;
    };

    explicit ShortcutView(QWidget *parent = nullptr);
    void setData(const QString &data);

private:
    void initUI();
    void calcColumnData();
    int itemSpacing();

private:
    QHBoxLayout *m_mainLayout = nullptr;

    using ColData = QList<Shortcut>;
    ColData m_shortcutList;
    QList<ColData> m_colDatas;
    double m_totalWeight = 0;
};

#endif   // SHORTCUTVIEW_H
