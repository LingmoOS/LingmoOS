// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTREELANDPLATFORMINTERFACE_H
#define DTREELANDPLATFORMINTERFACE_H

#include "private/dplatforminterface_p.h"

#include <QHash>
#include <QQueue>
#include <DObject>

DGUI_BEGIN_NAMESPACE

class DTreelandPlatformInterfacePrivate;
class PersonalizationManager;
class PersonalizationFontContext;
class PersonalizationAppearanceContext;
class PersonalizationWindowContext;

class DTreelandPlatformInterface : public QObject, public DPlatformInterface, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    friend PersonalizationFontContext;
    friend PersonalizationAppearanceContext;
    friend PersonalizationWindowContext;
public:
    explicit DTreelandPlatformInterface(DPlatformTheme *platformTheme);

    QByteArray iconThemeName() const override;
    QByteArray fontName() const override;
    QByteArray monoFontName() const override;
    qreal fontPointSize() const override;
    QColor activeColor() const override;
    QByteArray themeName() const override;

    void setIconThemeName(const QByteArray &iconThemeName) override;
    void setFontName(const QByteArray &fontName) override;
    void setMonoFontName(const QByteArray &monoFontName) override;
    void setFontPointSize(qreal fontPointSize) override;
    void setActiveColor(const QColor activeColor) override;

private:
    void initContext();

private:
    PersonalizationManager *m_manager;
    QScopedPointer<PersonalizationAppearanceContext> m_appearanceContext;
    QScopedPointer<PersonalizationFontContext> m_fontContext;

    QColor m_activeColor;
    int m_titleHeight;
    QByteArray m_fontName;
    QByteArray m_monoFontName;
    QByteArray m_iconThemeName;
    QByteArray m_cursorThemeName;
    qreal m_fontPointSize;
    int m_windowRadius;
    int m_scrollBarPolicy;
    QByteArray m_themeName;
    uint32_t m_blurOpacity;
};

DGUI_END_NAMESPACE
#endif
