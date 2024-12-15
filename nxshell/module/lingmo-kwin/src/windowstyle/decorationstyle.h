/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2024 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef WINDOWSTYLE_H
#define WINDOWSTYLE_H

#include <QObject>
#include <QMarginsF>
#include <QPointF>
#include <QColor>

namespace KWin
{
class Window;

class DecorationStyle : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 validProperties READ validProperties WRITE setValidProperties NOTIFY validPropertiesChanged)
    Q_PROPERTY(QString theme READ theme NOTIFY themeChanged)
    Q_PROPERTY(QPointF windowRadius READ windowRadius WRITE setWindowRadius NOTIFY windowRadiusChanged)
    Q_PROPERTY(qreal borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(qreal shadowRadius READ shadowRadius NOTIFY shadowRadiusChanged)
    Q_PROPERTY(QPointF shadowOffset READ shadowOffset  NOTIFY shadowOffectChanged)
    Q_PROPERTY(QColor shadowColor READ shadowColor WRITE setShadowColor NOTIFY shadowColorChanged)
    Q_PROPERTY(QMarginsF mouseInputAreaMargins READ mouseInputAreaMargins NOTIFY mouseInputAreaMarginsChanged)
    Q_PROPERTY(qreal windowPixelRatio READ windowPixelRatio NOTIFY windowPixelRatioChanged)
    Q_PROPERTY(qint64 windowEffect READ windowEffect NOTIFY windowEffectChanged)
    Q_PROPERTY(qreal windowStartUpEffect READ windowStartUpEffect NOTIFY windowStartUpEffectChanged)
public:
    enum PropertyFlag {
        ThemeProperty = 0x02,
        WindowRadiusProperty = 0x04,
        BorderWidthProperty = 0x08,
        BorderColorProperty = 0x10,
        ShadowRadiusProperty = 0x20,
        ShadowOffsetProperty = 0x40,
        ShadowColorProperty = 0x80,
        MouseInputAreaMargins = 0x100,
        WindowPixelRatioProperty = 0x200,
        WindowEffectProperty = 0x400,
        WindowStartUpEffectProperty = 0x800
    };
    Q_DECLARE_FLAGS(PropertyFlags, PropertyFlag)
    Q_FLAG(PropertyFlags)

    enum effectScene {
        effectNoRadius  = 0x01,       // 取消窗口圆角
        effectNoShadow  = 0x02,       // 取消窗口阴影
        effectNoBorder  = 0x04,       // 取消窗口边框
        effectNoStart   = 0x10,       // 取消启动场景动效
        effectNoClose   = 0x20,       // 取消关闭场景动效
        effectNoMax     = 0x40,       // 取消最大化场景动效
        effectNoMin     = 0x80,       // 取消最小化场景动效
    };
    Q_DECLARE_FLAGS(effectScenes, effectScene)
    Q_FLAG(effectScenes)

    explicit DecorationStyle(Window *window);
    ~DecorationStyle(){};
    QPointF variant2Point(QVariant value, QPointF defaultV = QPointF(0.0, 0.0));
    QMarginsF variant2Margins(QVariant value, QMarginsF defaultV = QMarginsF(0, 0, 0, 0));

    virtual PropertyFlags validProperties() = 0;
    virtual bool propertyIsValid(PropertyFlag p) = 0;

    virtual QString theme() = 0;
    virtual QPointF windowRadius() = 0;
    virtual void setWindowRadius(const QPointF value) = 0;
    virtual qreal borderWidth() = 0;
    virtual void setBorderWidth(qreal width) = 0;
    virtual QColor borderColor() = 0;
    virtual void setBorderColor(QColor color) = 0;
    virtual qreal shadowRadius() = 0;
    virtual QPointF shadowOffset() = 0;
    virtual QColor shadowColor() = 0;
    virtual void setShadowColor(QColor color) = 0;
    virtual QMarginsF mouseInputAreaMargins() = 0;
    virtual qreal windowPixelRatio() = 0;
    virtual effectScenes windowEffect() = 0;
    virtual qreal windowStartUpEffect() = 0;

    virtual void setWindowEffectScene(qint64) = 0;
    virtual effectScenes getWindowEffectScene() = 0;
    virtual void parseWinCustomRadius() = 0;
    virtual void parseWinCustomShadow() = 0;
    virtual void parseWinStartUpEffect() = 0;
    void cancelShadowByUser(bool cancel) { m_isCancleShadow = cancel; }
    bool isCancelShadow() { return m_isCancleShadow; }
    void cancelRadiusByUser(bool cancel) { m_isCancleRadius = cancel; }
    bool isCancelRadius() { return m_isCancleRadius; }

 public Q_SLOTS:
    void setValidProperties(qint64 validProperties);

Q_SIGNALS:
    void validPropertiesChanged(qint64 validProperties);
    void themeChanged();
    void windowRadiusChanged();
    void borderWidthChanged();
    void borderColorChanged();
    void shadowRadiusChanged();
    void shadowOffectChanged();
    void shadowColorChanged();
    void mouseInputAreaMarginsChanged();
    void windowPixelRatioChanged();
    void windowEffectChanged();
    void windowStartUpEffectChanged();


protected:
    PropertyFlags   m_validProperties;
    bool            m_isCancleShadow = false;
    bool            m_isCancleRadius = false;
};


class X11DecorationStyle : public DecorationStyle
{
    Q_OBJECT
public:

    explicit X11DecorationStyle(Window *window);
    ~X11DecorationStyle();

    PropertyFlags validProperties() override;
    bool propertyIsValid(PropertyFlag p) override;

    QString theme() override;
    QPointF windowRadius() override;
    void setWindowRadius(const QPointF value) override;
    qreal borderWidth() override;
    void setBorderWidth(qreal width) override;
    QColor borderColor() override;
    void setBorderColor(QColor color) override;
    qreal shadowRadius() override;
    QPointF shadowOffset() override;
    QColor shadowColor() override;
    void setShadowColor(QColor color) override;
    QMarginsF mouseInputAreaMargins() override;
    qreal windowPixelRatio() override;
    effectScenes windowEffect() override;
    qreal windowStartUpEffect() override;

    void setWindowEffectScene(qint64) {};
    effectScenes getWindowEffectScene() override;
    void parseWinCustomRadius() override;
    void parseWinCustomShadow() override;
    void parseWinStartUpEffect() override;
private:
    Window          *m_window;
};

class WaylandDecorationStyle : public DecorationStyle
{
    Q_OBJECT
public:
    explicit WaylandDecorationStyle(Window *window);
    ~WaylandDecorationStyle(){};

    PropertyFlags validProperties() override;
    bool propertyIsValid(PropertyFlag p) override;

    QString theme() {return "";};
    QPointF windowRadius();
    void setWindowRadius(const QPointF value);
    qreal borderWidth();
    void setBorderWidth(qreal width) override;
    QColor borderColor();
    void setBorderColor(QColor color) override;
    qreal shadowRadius() {return 50;};
    QPointF shadowOffset() {return QPointF(0,0);};
    QColor shadowColor();
    void setShadowColor(QColor color) override;
    QMarginsF mouseInputAreaMargins() {return QMarginsF(0,0,0,0);};
    qreal windowPixelRatio() {return 1;};
    effectScenes windowEffect() {return 0;};
    qreal windowStartUpEffect() {return 0;};


    void setWindowEffectScene(qint64 type) {m_effectScene = effectScene(type);};
    effectScenes getWindowEffectScene() {return m_effectScene;};
    void parseWinCustomRadius() {};
    void parseWinCustomShadow() {};
    void parseWinStartUpEffect() {};
public Q_SLOT:
    void onUpdateWindowRadiusByWayland(QPointF);
    void onUpdateShadowColorByWayland(QString);
    void onUpdateBorderWidthByWayland(qint32);
    void onUpdateBorderColorByWayland(QString);

private:
    QPointF m_radius = QPointF(-1, 0);
    qreal   m_borderWidth = 1;
    QColor  m_borderColor;
    QColor  m_shadowColor;
    Window  *m_window;
    effectScenes m_effectScene;
};

}

Q_DECLARE_METATYPE(QMarginsF)
#endif