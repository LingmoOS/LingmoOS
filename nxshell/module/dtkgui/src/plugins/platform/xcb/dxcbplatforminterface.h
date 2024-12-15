// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DXCBPLATFORMINTERFACE_H
#define DXCBPLATFORMINTERFACE_H

#include "private/dplatforminterface_p.h"

#include <DObject>

DGUI_BEGIN_NAMESPACE

class DXCBPlatformInterfacePrivate;
class DXCBPlatformInterface : public QObject, public DPlatformInterface, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DXCBPlatformInterface)
public:
    explicit DXCBPlatformInterface(quint32 window, DPlatformTheme *platformTheme);

    int cursorBlinkTime() const override;
    int cursorBlinkTimeout() const override;
    bool cursorBlink() const override;
    int doubleClickDistance() const override;
    int doubleClickTime() const override;
    int dndDragThreshold() const override;
    int windowRadius() const override;
    int windowRadius(int defaultValue) const override;
    QByteArray themeName() const override;
    QByteArray iconThemeName() const override;
    QByteArray soundThemeName() const override;

    QByteArray fontName() const override;
    QByteArray monoFontName() const override;
    qreal fontPointSize() const override;
    QByteArray gtkFontName() const override;

    QColor activeColor() const override;
    QColor darkActiveColor() const override;

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    QColor window() const override;
    QColor windowText() const override;
    QColor base() const override;
    QColor alternateBase() const override;
    QColor toolTipText() const override;
    QColor toolTipBase() const override;
    QColor text() const override;
    QColor button() const override;
    QColor buttonText() const override;
    QColor brightText() const override;
    QColor light() const override;
    QColor midlight() const override;
    QColor dark() const override;
    QColor mid() const override;
    QColor shadow() const override;
    QColor highlight() const override;
    QColor highlightedText() const override;
    QColor link() const override;
    QColor linkVisited() const override;
    QColor itemBackground() const override;
    QColor textTitle() const override;
    QColor textTips() const override;
    QColor textWarning() const override;
    QColor textLively() const override;
    QColor lightLively() const override;
    QColor darkLively() const override;
    QColor frameBorder() const override;
#endif

    int sizeMode() const override;
    int scrollBarPolicy() const override;

public Q_SLOTS:
    void setCursorBlinkTime(int cursorBlinkTime) override;
    void setCursorBlinkTimeout(int cursorBlinkTimeout) override;
    void setCursorBlink(bool cursorBlink) override;
    void setDoubleClickDistance(int doubleClickDistance) override;
    void setDoubleClickTime(int doubleClickTime) override;
    void setDndDragThreshold(int dndDragThreshold) override;
    void setThemeName(const QByteArray &themeName) override;
    void setIconThemeName(const QByteArray &iconThemeName) override;
    void setSoundThemeName(const QByteArray &soundThemeName) override;
    void setFontName(const QByteArray &fontName) override;
    void setMonoFontName(const QByteArray &monoFontName) override;
    void setFontPointSize(qreal fontPointSize) override;
    void setGtkFontName(const QByteArray &fontName) override;
    void setActiveColor(const QColor activeColor) override;
    void setDarkActiveColor(const QColor &activeColor) override;
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    void setWindow(const QColor &window) override;
    void setWindowText(const QColor &windowText) override;
    void setBase(const QColor &base) override;
    void setAlternateBase(const QColor &alternateBase) override;
    void setToolTipBase(const QColor &toolTipBase) override;
    void setToolTipText(const QColor &toolTipText) override;
    void setText(const QColor &text) override;
    void setButton(const QColor &button) override;
    void setButtonText(const QColor &buttonText) override;
    void setBrightText(const QColor &brightText) override;
    void setLight(const QColor &light) override;
    void setMidlight(const QColor &midlight) override;
    void setDark(const QColor &dark) override;
    void setMid(const QColor &mid) override;
    void setShadow(const QColor &shadow) override;
    void setHighlight(const QColor &highlight) override;
    void setHighlightedText(const QColor &highlightedText) override;
    void setLink(const QColor &link) override;
    void setLinkVisited(const QColor &linkVisited) override;
    void setItemBackground(const QColor &itemBackground) override;
    void setTextTitle(const QColor &textTitle) override;
    void setTextTips(const QColor &textTips) override;
    void setTextWarning(const QColor &textWarning) override;
    void setTextLively(const QColor &textLively) override;
    void setLightLively(const QColor &lightLively) override;
    void setDarkLively(const QColor &darkLively) override;
    void setFrameBorder(const QColor &frameBorder) override;
#endif

    int dotsPerInch(const QString &screenName = QString()) const override;
    void setDotsPerInch(const QString &screenName, int dpi) override;
    void setWindowRadius(int windowRadius) override;

private:
    friend class DPlatformThemePrivate;
    D_PRIVATE_SLOT(void _q_onThemePropertyChanged(const QByteArray &name, const QVariant &value))
};

DGUI_END_NAMESPACE
#endif // DXCBPLATFORMINTERFACE_H
