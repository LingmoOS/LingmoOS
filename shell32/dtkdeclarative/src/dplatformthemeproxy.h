// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFORMTHEMEPROXY_H
#define DPLATFORMTHEMEPROXY_H

#include <DObject>
#include <DPlatformTheme>

#include <dtkdeclarative_global.h>

DGUI_USE_NAMESPACE

DQUICK_BEGIN_NAMESPACE

class DPlatformThemeProxyPrivate;
class DPlatformThemeProxy : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT

    Q_PROPERTY(int cursorBlinkTime READ cursorBlinkTime WRITE setCursorBlinkTime NOTIFY cursorBlinkTimeChanged)
    Q_PROPERTY(int cursorBlinkTimeout READ cursorBlinkTimeout WRITE setCursorBlinkTimeout NOTIFY cursorBlinkTimeoutChanged)
    Q_PROPERTY(bool cursorBlink READ cursorBlink WRITE setCursorBlink NOTIFY cursorBlinkChanged)
    Q_PROPERTY(int doubleClickDistance READ doubleClickDistance WRITE setDoubleClickDistance NOTIFY doubleClickDistanceChanged)
    Q_PROPERTY(int doubleClickTime READ doubleClickTime WRITE setDoubleClickTime NOTIFY doubleClickTimeChanged)
    Q_PROPERTY(int dndDragThreshold READ dndDragThreshold WRITE setDndDragThreshold NOTIFY dndDragThresholdChanged)
    Q_PROPERTY(int windowRadius READ windowRadius WRITE setWindowRadius NOTIFY windowRadiusChanged)
    Q_PROPERTY(QByteArray themeName READ themeName WRITE setThemeName NOTIFY themeNameChanged)
    Q_PROPERTY(QByteArray iconThemeName READ iconThemeName WRITE setIconThemeName NOTIFY iconThemeNameChanged)
    Q_PROPERTY(QByteArray soundThemeName READ soundThemeName WRITE setSoundThemeName NOTIFY soundThemeNameChanged)
    // Font
    Q_PROPERTY(QByteArray fontName READ fontName WRITE setFontName NOTIFY fontNameChanged)
    Q_PROPERTY(QByteArray monoFontName READ monoFontName WRITE setMonoFontName NOTIFY monoFontNameChanged)
    Q_PROPERTY(qreal fontPointSize READ fontPointSize WRITE setFontPointSize NOTIFY fontPointSizeChanged)
    Q_PROPERTY(QByteArray gtkFontName READ gtkFontName WRITE setGtkFontName NOTIFY gtkFontNameChanged)

    Q_PROPERTY(QColor activeColor READ activeColor WRITE setActiveColor NOTIFY activeColorChanged)
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    // QPalette
    Q_PROPERTY(QColor window READ window WRITE setWindow NOTIFY windowChanged)
    Q_PROPERTY(QColor windowText READ windowText WRITE setWindowText NOTIFY windowTextChanged)
    Q_PROPERTY(QColor base READ base WRITE setBase NOTIFY baseChanged)
    Q_PROPERTY(QColor alternateBase READ alternateBase WRITE setAlternateBase NOTIFY alternateBaseChanged)
    Q_PROPERTY(QColor toolTipBase READ toolTipBase WRITE setToolTipBase NOTIFY toolTipBaseChanged)
    Q_PROPERTY(QColor toolTipText READ toolTipText WRITE setToolTipText NOTIFY toolTipTextChanged)
    Q_PROPERTY(QColor text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QColor button READ button WRITE setButton NOTIFY buttonChanged)
    Q_PROPERTY(QColor buttonText READ buttonText WRITE setButtonText NOTIFY buttonTextChanged)
    Q_PROPERTY(QColor brightText READ brightText WRITE setBrightText NOTIFY brightTextChanged)
    Q_PROPERTY(QColor light READ light WRITE setLight NOTIFY lightChanged)
    Q_PROPERTY(QColor midlight READ midlight WRITE setMidlight NOTIFY midlightChanged)
    Q_PROPERTY(QColor dark READ dark WRITE setDark NOTIFY darkChanged)
    Q_PROPERTY(QColor mid READ mid WRITE setMid NOTIFY midChanged)
    Q_PROPERTY(QColor shadow READ shadow WRITE setShadow NOTIFY shadowChanged)
    Q_PROPERTY(QColor highlight READ highlight WRITE setHighlight NOTIFY highlightChanged)
    Q_PROPERTY(QColor highlightedText READ highlightedText WRITE setHighlightedText NOTIFY highlightedTextChanged)
    Q_PROPERTY(QColor link READ link WRITE setLink NOTIFY linkChanged)
    Q_PROPERTY(QColor linkVisited READ linkVisited WRITE setLinkVisited NOTIFY linkVisitedChanged)
    // DPalette
    Q_PROPERTY(QColor itemBackground READ itemBackground WRITE setItemBackground NOTIFY itemBackgroundChanged)
    Q_PROPERTY(QColor textTitle READ textTitle WRITE setTextTitle NOTIFY textTitleChanged)
    Q_PROPERTY(QColor textTips READ textTips WRITE setTextTips NOTIFY textTipsChanged)
    Q_PROPERTY(QColor textWarning READ textWarning WRITE setTextWarning NOTIFY textWarningChanged)
    Q_PROPERTY(QColor textLively READ textLively WRITE setTextLively NOTIFY textLivelyChanged)
    Q_PROPERTY(QColor lightLively READ lightLively WRITE setLightLively NOTIFY lightLivelyChanged)
    Q_PROPERTY(QColor darkLively READ darkLively WRITE setDarkLively NOTIFY darkLivelyChanged)
    Q_PROPERTY(QColor frameBorder READ frameBorder WRITE setFrameBorder NOTIFY frameBorderChanged)
#endif

public:
    explicit DPlatformThemeProxy(DPlatformTheme *proxy, QObject *parent = nullptr);
    ~DPlatformThemeProxy() override;

    int cursorBlinkTime() const;
    int cursorBlinkTimeout() const;
    bool cursorBlink() const;
    int doubleClickDistance() const;
    int doubleClickTime() const;
    int dndDragThreshold() const;
    int windowRadius() const;
    Q_INVOKABLE int windowRadius(int defaultValue) const;
    QByteArray themeName() const;
    QByteArray iconThemeName() const;
    QByteArray soundThemeName() const;

    QByteArray fontName() const;
    QByteArray monoFontName() const;
    qreal fontPointSize() const;
    QByteArray gtkFontName() const;

    QColor activeColor() const;

    bool isValidPalette() const;

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    QColor window() const;
    QColor windowText() const;
    QColor base() const;
    QColor alternateBase() const;
    QColor toolTipBase() const;
    QColor toolTipText() const;
    QColor text() const;
    QColor button() const;
    QColor buttonText() const;
    QColor brightText() const;
    QColor light() const;
    QColor midlight() const;
    QColor dark() const;
    QColor mid() const;
    QColor shadow() const;
    QColor highlight() const;
    QColor highlightedText() const;
    QColor link() const;
    QColor linkVisited() const;
    QColor itemBackground() const;
    QColor textTitle() const;
    QColor textTips() const;
    QColor textWarning() const;
    QColor textLively() const;
    QColor lightLively() const;
    QColor darkLively() const;
    QColor frameBorder() const;
#endif

    Q_INVOKABLE int dotsPerInch(const QString &screenName = QString()) const;

public Q_SLOTS:
    void setCursorBlinkTime(int cursorBlinkTime);
    void setCursorBlinkTimeout(int cursorBlinkTimeout);
    void setCursorBlink(bool cursorBlink);
    void setDoubleClickDistance(int doubleClickDistance);
    void setDoubleClickTime(int doubleClickTime);
    void setDndDragThreshold(int dndDragThreshold);
    void setThemeName(const QByteArray &themeName);
    void setIconThemeName(const QByteArray &iconThemeName);
    void setSoundThemeName(const QByteArray &soundThemeName);
    void setFontName(const QByteArray &fontName);
    void setMonoFontName(const QByteArray &monoFontName);
    void setFontPointSize(qreal fontPointSize);
    void setGtkFontName(const QByteArray &fontName);
    void setActiveColor(const QColor activeColor);
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    void setWindow(const QColor &window);
    void setWindowText(const QColor &windowText);
    void setBase(const QColor &base);
    void setAlternateBase(const QColor &alternateBase);
    void setToolTipBase(const QColor &toolTipBase);
    void setToolTipText(const QColor &toolTipText);
    void setText(const QColor &text);
    void setButton(const QColor &button);
    void setButtonText(const QColor &buttonText);
    void setBrightText(const QColor &brightText);
    void setLight(const QColor &light);
    void setMidlight(const QColor &midlight);
    void setDark(const QColor &dark);
    void setMid(const QColor &mid);
    void setShadow(const QColor &shadow);
    void setHighlight(const QColor &highlight);
    void setHighlightedText(const QColor &highlightedText);
    void setLink(const QColor &link);
    void setLinkVisited(const QColor &linkVisited);
    void setItemBackground(const QColor &itemBackground);
    void setTextTitle(const QColor &textTitle);
    void setTextTips(const QColor &textTips);
    void setTextWarning(const QColor &textWarning);
    void setTextLively(const QColor &textLively);
    void setLightLively(const QColor &lightLively);
    void setDarkLively(const QColor &darkLively);
    void setFrameBorder(const QColor &frameBorder);
#endif

    Q_INVOKABLE void setDotsPerInch(const QString &screenName, int dpi);
    void setWindowRadius(int windowRadius);

Q_SIGNALS:
    void cursorBlinkTimeChanged(int cursorBlinkTime);
    void cursorBlinkTimeoutChanged(int cursorBlinkTimeout);
    void cursorBlinkChanged(bool cursorBlink);
    void doubleClickDistanceChanged(int doubleClickDistance);
    void doubleClickTimeChanged(int doubleClickTime);
    void dndDragThresholdChanged(int dndDragThreshold);
    void themeNameChanged(QByteArray themeName);
    void iconThemeNameChanged(QByteArray iconThemeName);
    void soundThemeNameChanged(QByteArray soundThemeName);
    void fontNameChanged(QByteArray fontName);
    void monoFontNameChanged(QByteArray monoFontName);
    void fontPointSizeChanged(qreal fontPointSize);
    void gtkFontNameChanged(QByteArray fontName);
    void activeColorChanged(QColor activeColor);
    void paletteChanged(DTK_GUI_NAMESPACE::DPalette palette);
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    void windowChanged(QColor window);
    void windowTextChanged(QColor windowText);
    void baseChanged(QColor base);
    void alternateBaseChanged(QColor alternateBase);
    void toolTipBaseChanged(QColor toolTipBase);
    void toolTipTextChanged(QColor toolTipText);
    void textChanged(QColor text);
    void buttonChanged(QColor button);
    void buttonTextChanged(QColor buttonText);
    void brightTextChanged(QColor brightText);
    void lightChanged(QColor light);
    void midlightChanged(QColor midlight);
    void darkChanged(QColor dark);
    void midChanged(QColor mid);
    void shadowChanged(QColor shadow);
    void highlightChanged(QColor highlight);
    void highlightedTextChanged(QColor highlightedText);
    void linkChanged(QColor link);
    void linkVisitedChanged(QColor linkVisited);
    void itemBackgroundChanged(QColor itemBackground);
    void textTitleChanged(QColor textTitle);
    void textTipsChanged(QColor textTips);
    void textWarningChanged(QColor textWarning);
    void textLivelyChanged(QColor textLively);
    void lightLivelyChanged(QColor lightLively);
    void darkLivelyChanged(QColor darkLively);
    void frameBorderChanged(QColor frameBorder);
#endif
    void dotsPerInchChanged(const QString &screen, int dpi);
    void windowRadiusChanged(int r);

private:
    D_DECLARE_PRIVATE(DPlatformThemeProxy)
};

DQUICK_END_NAMESPACE

#endif // DPLATFORMTHEMEPROXY_H
