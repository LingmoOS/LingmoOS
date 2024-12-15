// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNOTITLEBARWINDOWHELPER_H
#define DNOTITLEBARWINDOWHELPER_H

#include "global.h"
#include "utility.h"

#include <QMarginsF>
#include <QObject>
#include <QVariant>
#include <QPainterPath>

QT_BEGIN_NAMESPACE
class QWindow;
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

class DNativeSettings;
class DNoTitlebarWindowHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(QPointF windowRadius READ windowRadius WRITE setWindowRadius NOTIFY windowRadiusChanged)
    Q_PROPERTY(qreal borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(qreal shadowRadius READ shadowRadius WRITE setShadowRadius NOTIFY shadowRadiusChanged)
    Q_PROPERTY(QPointF shadowOffset READ shadowOffset WRITE setShadowOffect NOTIFY shadowOffectChanged)
    Q_PROPERTY(QColor shadowColor READ shadowColor WRITE setShadowColor NOTIFY shadowColorChanged)
    Q_PROPERTY(quint32 windowEffect READ windowEffect WRITE setWindowEffect NOTIFY windowEffectChanged)
    Q_PROPERTY(quint32 windowStartUpEffect READ windowStartUpEffect WRITE setWindowStartUpEffect NOTIFY windowStartUpEffectChanged)
    Q_PROPERTY(QMarginsF mouseInputAreaMargins READ mouseInputAreaMargins WRITE setMouseInputAreaMargins NOTIFY mouseInputAreaMarginsChanged)

public:
    explicit DNoTitlebarWindowHelper(QWindow *window, quint32 windowID);
    ~DNoTitlebarWindowHelper();

    inline QWindow *window() const
    { return reinterpret_cast<QWindow*>(const_cast<DNoTitlebarWindowHelper*>(this));}

    static void setWindowProperty(QWindow *window, const char *name, const QVariant &value);

    QString theme() const;
    QPointF windowRadius() const;
    qreal borderWidth() const;
    QColor borderColor() const;
    qreal shadowRadius() const;
    QPointF shadowOffset() const;
    QColor shadowColor() const;
    quint32 windowEffect();
    quint32 windowStartUpEffect();
    QMarginsF mouseInputAreaMargins() const;

    void resetProperty(const QByteArray &property);
    void setTheme(const QString &theme);
    void setWindowRadius(const QPointF &windowRadius);
    void setBorderWidth(qreal borderWidth);
    void setBorderColor(const QColor &borderColor);
    void setShadowRadius(qreal shadowRadius);
    void setShadowOffect(const QPointF &shadowOffset);
    void setShadowColor(const QColor &shadowColor);
    void setMouseInputAreaMargins(const QMarginsF &mouseInputAreaMargins);
    void setWindowEffect(quint32 effectScene);
    void setWindowStartUpEffect(quint32 effectType);

signals:
    void themeChanged();
    void windowRadiusChanged();
    void borderWidthChanged();
    void borderColorChanged();
    void shadowRadiusChanged();
    void shadowOffectChanged();
    void shadowColorChanged();
    void windowEffectChanged();
    void windowStartUpEffectChanged();
    void mouseInputAreaMarginsChanged();

private slots:
    // update properties
    Q_SLOT void updateClipPathFromProperty();
    Q_SLOT void updateFrameMaskFromProperty();
    Q_SLOT void updateWindowRadiusFromProperty();
    Q_SLOT void updateBorderWidthFromProperty();
    Q_SLOT void updateBorderColorFromProperty();
    Q_SLOT void updateShadowRadiusFromProperty();
    Q_SLOT void updateShadowOffsetFromProperty();
    Q_SLOT void updateShadowColorFromProperty();
    Q_SLOT void updateWindowEffectFromProperty();
    Q_SLOT void updateWindowStartUpEffectFromProperty();
    Q_SLOT void updateEnableSystemResizeFromProperty();
    Q_SLOT void updateEnableSystemMoveFromProperty();
    Q_SLOT void updateEnableBlurWindowFromProperty();
    Q_SLOT void updateWindowBlurAreasFromProperty();
    Q_SLOT void updateWindowBlurPathsFromProperty();
    Q_SLOT void updateAutoInputMaskByClipPathFromProperty();

private:
    bool windowEvent(QEvent *event);
    bool isEnableSystemMove(quint32 winId);
    bool updateWindowBlurAreasForWM();
    void updateWindowShape();

    void onWindowSizeChanged();

    static void startMoveWindow(quint32 winId);
    static void updateMoveWindow(quint32 winId);

    QWindow *m_window;
    quint32 m_windowID;
    bool m_windowMoving = false;
    bool m_nativeSettingsValid = false;

    QVector<Utility::BlurArea> m_blurAreaList;
    QList<QPainterPath> m_blurPathList;
    bool m_needUpdateBlurAreaForWindowSizeChanged = false;

    QPainterPath m_clipPath;

    // properties
    bool m_enableSystemMove = true;
    bool m_enableBlurWindow = false;
    bool m_autoInputMaskByClipPath = false;
    DNativeSettings *m_settings;

    static QHash<const QWindow*, DNoTitlebarWindowHelper*> mapped;

    friend class DPlatformIntegration;
};

DPP_END_NAMESPACE

Q_DECLARE_METATYPE(QMarginsF)
#endif // DNOTITLEBARWINDOWHELPER_H
