// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFORMINTERFACE_H
#define DPLATFORMINTERFACE_H

#include <QObject>
#include <QWindow>

#include "dtkgui_global.h"

DGUI_BEGIN_NAMESPACE

class DPlatformTheme;

class LIBDTKCORESHARED_EXPORT DPlatformInterface
{
public:
    explicit DPlatformInterface(DPlatformTheme *platformTheme);
    virtual ~DPlatformInterface();
    virtual int cursorBlinkTime() const;
    virtual int cursorBlinkTimeout() const;
    virtual bool cursorBlink() const;
    virtual int doubleClickDistance() const;
    virtual int doubleClickTime() const;
    virtual int dndDragThreshold() const;
    virtual int windowRadius() const;
    virtual int windowRadius(int defaultValue) const;
    virtual QByteArray themeName() const;
    virtual QByteArray iconThemeName() const;
    virtual QByteArray soundThemeName() const;

    virtual QByteArray fontName() const;
    virtual QByteArray monoFontName() const;
    virtual qreal fontPointSize() const;
    virtual QByteArray gtkFontName() const;

    virtual QColor activeColor() const;
    virtual QColor darkActiveColor() const;

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    virtual QColor window() const;
    virtual QColor windowText() const;
    virtual QColor base() const;
    virtual QColor alternateBase() const;
    virtual QColor toolTipBase() const;
    virtual QColor toolTipText() const;
    virtual QColor text() const;
    virtual QColor button() const;
    virtual QColor buttonText() const;
    virtual QColor brightText() const;
    virtual QColor light() const;
    virtual QColor midlight() const;
    virtual QColor dark() const;
    virtual QColor mid() const;
    virtual QColor shadow() const;
    virtual QColor highlight() const;
    virtual QColor highlightedText() const;
    virtual QColor link() const;
    virtual QColor linkVisited() const;
    virtual QColor itemBackground() const;
    virtual QColor textTitle() const;
    virtual QColor textTips() const;
    virtual QColor textWarning() const;
    virtual QColor textLively() const;
    virtual QColor lightLively() const;
    virtual QColor darkLively() const;
    virtual QColor frameBorder() const;
#endif

    virtual int sizeMode() const;
    virtual int scrollBarPolicy() const;

public:
    virtual void setCursorBlinkTime(int cursorBlinkTime);
    virtual void setCursorBlinkTimeout(int cursorBlinkTimeout);
    virtual void setCursorBlink(bool cursorBlink);
    virtual void setDoubleClickDistance(int doubleClickDistance);
    virtual void setDoubleClickTime(int doubleClickTime);
    virtual void setDndDragThreshold(int dndDragThreshold);
    virtual void setThemeName(const QByteArray &themeName);
    virtual void setIconThemeName(const QByteArray &iconThemeName);
    virtual void setSoundThemeName(const QByteArray &soundThemeName);
    virtual void setFontName(const QByteArray &fontName);
    virtual void setMonoFontName(const QByteArray &monoFontName);
    virtual void setFontPointSize(qreal fontPointSize);
    virtual void setGtkFontName(const QByteArray &fontName);
    virtual void setActiveColor(const QColor activeColor);
    virtual void setDarkActiveColor(const QColor &activeColor);
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    virtual void setWindow(const QColor &window);
    virtual void setWindowText(const QColor &windowText);
    virtual void setBase(const QColor &base);
    virtual void setAlternateBase(const QColor &alternateBase);
    virtual void setToolTipBase(const QColor &toolTipBase);
    virtual void setToolTipText(const QColor &toolTipText);
    virtual void setText(const QColor &text);
    virtual void setButton(const QColor &button);
    virtual void setButtonText(const QColor &buttonText);
    virtual void setBrightText(const QColor &brightText);
    virtual void setLight(const QColor &light);
    virtual void setMidlight(const QColor &midlight);
    virtual void setDark(const QColor &dark);
    virtual void setMid(const QColor &mid);
    virtual void setShadow(const QColor &shadow);
    virtual void setHighlight(const QColor &highlight);
    virtual void setHighlightedText(const QColor &highlightedText);
    virtual void setLink(const QColor &link);
    virtual void setLinkVisited(const QColor &linkVisited);
    virtual void setItemBackground(const QColor &itemBackground);
    virtual void setTextTitle(const QColor &textTitle);
    virtual void setTextTips(const QColor &textTips);
    virtual void setTextWarning(const QColor &textWarning);
    virtual void setTextLively(const QColor &textLively);
    virtual void setLightLively(const QColor &lightLively);
    virtual void setDarkLively(const QColor &darkLively);
    virtual void setFrameBorder(const QColor &frameBorder);
#endif

    virtual int dotsPerInch(const QString &screenName = QString()) const;
    virtual void setDotsPerInch(const QString &screenName, int dpi);
    virtual void setWindowRadius(int windowRadius);

protected:
    DPlatformTheme *m_platformTheme;
};

class LIBDTKCORESHARED_EXPORT DPlatformInterfaceFactory {
public:
    using HelperCreator = DPlatformInterface * (*)(DPlatformTheme*);
    static void registerInterface(HelperCreator creator);
};

DGUI_END_NAMESPACE
#endif // DNATIVESETTINGS_P_H
