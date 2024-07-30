/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LINGMOUILINGMOTHEME_H
#define LINGMOUILINGMOTHEME_H

#include <LingmoUI/Platform/PlatformTheme>

#include <QColor>
#include <QIcon>
#include <QObject>
#include <QPointer>
#include <QQuickItem>

class StyleSingleton;

class LingmoDesktopTheme : public LingmoUI::Platform::PlatformTheme
{
    Q_OBJECT

public:
    explicit LingmoDesktopTheme(QObject *parent = nullptr);
    ~LingmoDesktopTheme() override;

    Q_INVOKABLE QIcon iconFromTheme(const QString &name, const QColor &customColor = Qt::transparent) override;

    void syncWindow();
    void syncColors();

protected:
    bool event(QEvent *event) override;

private:
    friend class StyleSingleton;
    QPointer<QWindow> m_window;
};

#endif // LINGMOUILINGMOTHEME_H
