// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKDCIICONIMAGE_P_H
#define DQUICKDCIICONIMAGE_P_H

#include "dquickiconimage_p.h"
#include "dqmlglobalobject_p.h"

#include <dtkdeclarative_global.h>
#include <DObject>
#include <DDciIcon>
#include <DDciIconPalette>

DQUICK_BEGIN_NAMESPACE
DGUI_USE_NAMESPACE

class DQuickDciIconImageItem;
class DQuickIconAttached;
class DQuickDciIconImagePrivate;
class DQuickDciIconImage : public QQuickItem, DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DQuickDciIconImage)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    Q_PROPERTY(DTK_QUICK_NAMESPACE::DQMLGlobalObject::ControlState mode READ mode WRITE setMode NOTIFY modeChanged FINAL)
    Q_PROPERTY(DTK_GUI_NAMESPACE::DGuiApplicationHelper::ColorType theme READ theme WRITE setTheme NOTIFY themeChanged FINAL)
    Q_PROPERTY(DTK_GUI_NAMESPACE::DDciIconPalette palette READ palette WRITE setPalette NOTIFY paletteChanged)
    Q_PROPERTY(QSize sourceSize READ sourceSize WRITE setSourceSize NOTIFY sourceSizeChanged)
    Q_PROPERTY(bool mirror READ mirror WRITE setMirror NOTIFY mirrorChanged)
    Q_PROPERTY(bool fallbackToQIcon READ fallbackToQIcon WRITE setFallbackToQIcon NOTIFY fallbackToQIconChanged)
    Q_PROPERTY(bool asynchronous READ asynchronous WRITE setAsynchronous NOTIFY asynchronousChanged)
    Q_PROPERTY(bool cache READ cache WRITE setCache NOTIFY cacheChanged FINAL)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(DciIcon)
    QML_ATTACHED(DQuickIconAttached)
#endif

public:
    explicit DQuickDciIconImage(QQuickItem *parent = nullptr);
    ~DQuickDciIconImage() override;

    QString name() const;
    void setName(const QString &name);

    DQMLGlobalObject::ControlState mode() const;
    void setMode(DQMLGlobalObject::ControlState mode);

    DGuiApplicationHelper::ColorType theme() const;
    void setTheme(DGuiApplicationHelper::ColorType theme);

    DDciIconPalette palette() const;
    void setPalette(const DDciIconPalette &palette);

    QSize sourceSize() const;
    void setSourceSize(const QSize &size);

    void setMirror(bool mirror);
    bool mirror() const;

    bool fallbackToQIcon() const;
    void setFallbackToQIcon(bool newFallbackToQIcon);

    bool asynchronous() const;
    void setAsynchronous(bool async);

    bool cache() const;
    void setCache(bool cache);

    DQuickIconImage *imageItem() const;

    static bool isNull(const QString &iconName);
    static DQuickIconAttached *qmlAttachedProperties(QObject *object);

Q_SIGNALS:
    void nameChanged();
    void modeChanged();
    void themeChanged();
    void paletteChanged();
    void sourceSizeChanged();
    void mirrorChanged();
    void fallbackToQIconChanged();
    void asynchronousChanged();
    void cacheChanged();

protected:
    void classBegin() override;
    void componentComplete() override;
};

class DQuickIconAttachedPrivate;
class DQuickIconAttached : public QObject, DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DQuickIconAttached)
    Q_PROPERTY(DTK_QUICK_NAMESPACE::DQMLGlobalObject::ControlState mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(DTK_GUI_NAMESPACE::DGuiApplicationHelper::ColorType theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(DTK_GUI_NAMESPACE::DDciIconPalette palette READ palette WRITE setPalette NOTIFY paletteChanged)
    Q_PROPERTY(bool fallbackToQIcon READ fallbackToQIcon WRITE setFallbackToQIcon NOTIFY fallbackToQIconChanged)

public:
    explicit DQuickIconAttached(QQuickItem *parent);
    ~DQuickIconAttached();

    DQMLGlobalObject::ControlState mode() const;
    void setMode(DQMLGlobalObject::ControlState mode);

    DGuiApplicationHelper::ColorType theme() const;
    void setTheme(DGuiApplicationHelper::ColorType theme);

    DDciIconPalette palette() const;
    void setPalette(const DDciIconPalette &palette);

    bool fallbackToQIcon() const;
    void setFallbackToQIcon(bool newFallbackToQIcon);

Q_SIGNALS:
    void modeChanged();
    void themeChanged();
    void paletteChanged();
    void fallbackToQIconChanged();
};

DQUICK_END_NAMESPACE
QML_DECLARE_TYPEINFO(DTK_QUICK_NAMESPACE::DQuickDciIconImage, QML_HAS_ATTACHED_PROPERTIES)
#endif // DQUICKDCIICONIMAGE_P_H
