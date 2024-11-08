/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LINGMOTHEME_H
#define LINGMOTHEME_H

#include <LingmoUI/Platform/PlatformTheme>

#include <Lingmo/Theme>
#include <QColor>
#include <QIcon>
#include <QObject>
#include <QPointer>
#include <QQuickItem>

class KIconLoader;

class LingmoTheme : public LingmoUI::Platform::PlatformTheme
{
    Q_OBJECT

public:
    explicit LingmoTheme(QObject *parent = nullptr);
    ~LingmoTheme() override;

    Q_INVOKABLE QIcon iconFromTheme(const QString &name, const QColor &customColor = Qt::transparent) override;

    void syncColors();

protected:
    bool event(QEvent *event) override;

private:
    Lingmo::Theme m_theme;
};

#endif // LINGMOTHEME_H
