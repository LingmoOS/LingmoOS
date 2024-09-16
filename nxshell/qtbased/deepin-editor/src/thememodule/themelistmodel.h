// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THEMELISTMODEL_H
#define THEMELISTMODEL_H

#include <QAbstractListModel>

class ThemeListModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum ThemeRole {
        ThemeName = Qt::DisplayRole,
        ThemePath = Qt::UserRole,
        ThemeMap,
        FrameNormalColor,
        FrameSelectedColor
    };

    ThemeListModel(QObject *parent = nullptr);
    ~ThemeListModel();

    void setFrameColor(const QString &selectedColor, const QString &normalColor);
    void setSelection(const QString &path);
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

signals:
    void requestCurrentIndex(const QModelIndex &idx);

private:
    void initThemes();

private:
    // first is theme name.
    // second is theme file path.
    QList<QPair<QString, QString>> m_themes;

    QString m_frameNormalColor;
    QString m_frameSelectedColor;
};

#endif
