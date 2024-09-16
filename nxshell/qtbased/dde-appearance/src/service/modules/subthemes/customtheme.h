// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMTHEME_H
#define CUSTOMTHEME_H

#include <QObject>

class KeyFile;
class Theme;

class CustomTheme : public QObject
{
    Q_OBJECT
public:
    explicit CustomTheme(QObject *parent = nullptr);

    void updateValue(const QString &type, const QString &value, const QString &oldTheme, const QVector<QSharedPointer<Theme>> &globalThemes);

protected:
    void openCustomTheme();
    void saveCustomTheme();
    void copyTheme(const QString &themePath, const QStringList &keys);

Q_SIGNALS:
    void updateToCustom(const QString &mode);

private:
    KeyFile *m_customTheme;
};

#endif // CUSTOMTHEME_H
