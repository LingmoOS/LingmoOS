// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickdciiconimage_p_p.h"

#include <DIconTheme>
#include <DGuiApplicationHelper>
#include <DPlatformTheme>

#include <QUrlQuery>

DQUICK_BEGIN_NAMESPACE
DGUI_USE_NAMESPACE

static QString appIconThemeName()
{
    return DGuiApplicationHelper::instance()->applicationTheme()->iconThemeName();
}

static QString findDciIconPath(const QString &iconName, const QString &themeName)
{
    QString iconPath;
    auto cached = DIconTheme::cached();

    if (cached) {
        iconPath = cached->findDciIconFile(iconName, themeName);
    } else {
        iconPath = DIconTheme::findDciIconFile(iconName, themeName);
    }
    return iconPath;
}

static DDciIcon::Mode controlState2DciMode(int state)
{
    DDciIcon::Mode dcimode = DDciIcon::Normal;
    switch (state) {
    case DQMLGlobalObject::NormalState:
        dcimode = DDciIcon::Normal;
        break;
    case DQMLGlobalObject::DisabledState:
        dcimode = DDciIcon::Disabled;
        break;
    case DQMLGlobalObject::HoveredState:
        dcimode = DDciIcon::Hover;
        break;
    case DQMLGlobalObject::PressedState:
        dcimode = DDciIcon::Pressed;
        break;
    default:
        break;
    }

    return dcimode;
}

static inline DDciIcon::Theme dciTheme(DGuiApplicationHelper::ColorType type)
{
    return type == DGuiApplicationHelper::DarkType ? DDciIcon::Dark : DDciIcon::Light;
}

DQuickDciIconImageItemPrivate::DQuickDciIconImageItemPrivate(DQuickDciIconImagePrivate *pqq)
    : parentPriv(pqq)
{

}

void DQuickDciIconImageItemPrivate::maybeUpdateUrl()
{
    Q_Q(DQuickIconImage);
    if (parentPriv->imageItem->name().isEmpty() || iconType != ThemeIconName) {
        return DQuickIconImagePrivate::maybeUpdateUrl();
    }

    if (!DQMLGlobalObject::hasAnimation()) {
        QUrl url;
        url.setScheme(QLatin1String("image"));
        url.setHost(QLatin1String("dtk.dci.icon"));
        url.setQuery(getUrlQuery());
        q->setSource(url);
        return;
    }

    QString iconPath = findDciIconPath(parentPriv->imageItem->name(), appIconThemeName());
    if (iconPath.isEmpty())
        return DQuickIconImagePrivate::maybeUpdateUrl();

    updatePlayer();

    if (player)
        player->setMode(controlState2DciMode(parentPriv->mode));
}

void DQuickDciIconImageItemPrivate::play(int mode)
{
    Q_Q(DQuickIconImage);
    if (parentPriv->imageItem->name().isEmpty() || iconType != ThemeIconName) {
        return;
    }

    updatePlayer();

    if (player)
        player->play(controlState2DciMode(mode));
}

QUrlQuery DQuickDciIconImageItemPrivate::getUrlQuery()
{
    QUrlQuery query;
    query.addQueryItem(QLatin1String("name"), parentPriv->imageItem->name());
    query.addQueryItem(QLatin1String("mode"), QString::number(parentPriv->mode));
    query.addQueryItem(QLatin1String("theme"), QString::number(parentPriv->theme));
    query.addQueryItem(QLatin1String("themeName"), QIcon::themeName());
    DDciIconPalette pal = parentPriv->palette;
    if (!parentPriv->palette.foreground().isValid() && q_func()->color().isValid()) {
        pal.setForeground(q_func()->color());
    }
    query.addQueryItem(QLatin1String("palette"), DDciIconPalette::convertToString(pal));
    query.addQueryItem(QLatin1String("devicePixelRatio"), QString::number(devicePixelRatio));
    query.addQueryItem(QLatin1String("fallbackToQIcon"), QString::number(parentPriv->fallbackToQIcon));

    return query;
}

void DQuickDciIconImageItemPrivate::updatePlayerIconSize()
{
    if (!player)
        return;

    int boundingSize = qMax(q_func()->sourceSize().width(), q_func()->sourceSize().height());

    player->setIconSize(boundingSize);
}

void DQuickDciIconImageItemPrivate::updatePlayer()
{
    if (!player) {
        Q_Q(DQuickIconImage);
        player = new DDciIconPlayer(parentPriv->imageItem);
        QObject::connect(player, &DDciIconPlayer::updated, parentPriv->imageItem, [this](){
            parentPriv->imageItem->setImage(player->currentImage());
        });
        QObject::connect(parentPriv->imageItem, &DQuickIconImage::sourceSizeChanged, player, [this](){
            updatePlayerIconSize();
        });

        // 只在初始化和 sourceSizeChanged 时更新图标大小
        // 防止出现 dpr > 1.0 时未设置 sourceSize 的 item 在 updatePlayer 时图标一直放大
        updatePlayerIconSize();
    }

    QString iconPath = findDciIconPath(parentPriv->imageItem->name(), appIconThemeName());

    // 防止频繁构造 dciicon
    if (iconPathCache != iconPath) {
        DDciIcon dciIcon(iconPath);
        if (!dciIcon.isNull()) {
            player->setIcon(dciIcon);
            iconPathCache =  iconPath;
        }
    }

    player->setTheme(dciTheme(parentPriv->theme));

    DDciIconPalette palette = parentPriv->palette;
    if (!parentPriv->palette.foreground().isValid() && q_func()->color().isValid())
        palette.setForeground(q_func()->color());

    player->setPalette(palette);

    player->setDevicePixelRatio(devicePixelRatio);
}

DQuickDciIconImagePrivate::DQuickDciIconImagePrivate(DQuickDciIconImage *qq)
    : DObjectPrivate(qq)
    , imageItem(new DQuickIconImage(*new DQuickDciIconImageItemPrivate(this), qq))
{
    QObject::connect(imageItem, &DQuickIconImage::nameChanged, qq, &DQuickDciIconImage::nameChanged);
    QObject::connect(imageItem, &DQuickIconImage::asynchronousChanged, qq, &DQuickDciIconImage::asynchronousChanged);
    QObject::connect(imageItem, &DQuickIconImage::cacheChanged, qq, &DQuickDciIconImage::cacheChanged);
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    QObject::connect(imageItem, &DQuickIconImage::retainWhileLoadingChanged, qq, &DQuickDciIconImage::retainWhileLoadingChanged);
#endif
}

void DQuickDciIconImagePrivate::layout()
{
    auto dd = QQuickItemPrivate::get(imageItem);
    dd->anchors()->setCenterIn(imageItem->parentItem());
}

void DQuickDciIconImagePrivate::updateImageSourceUrl()
{
    imageItem->d_func()->maybeUpdateUrl();
}

void DQuickDciIconImagePrivate::play(DQMLGlobalObject::ControlState mode)
{
    imageItem->d_func()->play(mode);
}

DQuickDciIconImage::DQuickDciIconImage(QQuickItem *parent)
    : QQuickItem(parent)
    , DObject(*new DQuickDciIconImagePrivate(this))
{
    D_D(DQuickDciIconImage);
    connect(d->imageItem, &QQuickImage::implicitWidthChanged, this, [this, d]() { setImplicitWidth(d->imageItem->implicitWidth()); });
    connect(d->imageItem, &QQuickImage::implicitHeightChanged, this, [this, d]() { setImplicitHeight(d->imageItem->implicitHeight()); });
}

DQuickDciIconImage::~DQuickDciIconImage()
{

}

QString DQuickDciIconImage::name() const
{
    D_DC(DQuickDciIconImage);
    return d->imageItem->name();
}

void DQuickDciIconImage::setName(const QString &name)
{
    D_D(DQuickDciIconImage);
    d->imageItem->setName(name);
}

DQMLGlobalObject::ControlState DQuickDciIconImage::mode() const
{
    D_DC(DQuickDciIconImage);
    return d->mode;
}

void DQuickDciIconImage::setMode(DQMLGlobalObject::ControlState mode)
{
    D_D(DQuickDciIconImage);
    if (d->mode == mode)
        return;

    d->mode = mode;
    d->updateImageSourceUrl();
    Q_EMIT modeChanged();
}

void DQuickDciIconImage::play(DQMLGlobalObject::ControlState mode)
{
    D_D(DQuickDciIconImage);
    if (d->imageItem)
        d->play(mode);
}

DGuiApplicationHelper::ColorType DQuickDciIconImage::theme() const
{
    D_DC(DQuickDciIconImage);
    return d->theme;
}

void DQuickDciIconImage::setTheme(DGuiApplicationHelper::ColorType theme)
{
    D_D(DQuickDciIconImage);
    if (d->theme == theme)
        return;

    d->theme = theme;
    d->updateImageSourceUrl();
    Q_EMIT themeChanged();
}

DDciIconPalette DQuickDciIconImage::palette() const
{
    D_DC(DQuickDciIconImage);
    return d->palette;
}

void DQuickDciIconImage::setPalette(const DDciIconPalette &palette)
{
    D_D(DQuickDciIconImage);
    if (d->palette == palette)
        return;

    d->palette = palette;
    d->updateImageSourceUrl();
    Q_EMIT paletteChanged();
}

QSize DQuickDciIconImage::sourceSize() const
{
    D_DC(DQuickDciIconImage);
    return d->imageItem->sourceSize();
}

void DQuickDciIconImage::setSourceSize(const QSize &size)
{
    D_D(DQuickDciIconImage);
    d->imageItem->setSourceSize(size);
    Q_EMIT sourceSizeChanged();
}

void DQuickDciIconImage::setMirror(bool mirror)
{
    D_D(DQuickDciIconImage);
    d->imageItem->setMirror(mirror);
}

bool DQuickDciIconImage::mirror() const
{
    D_DC(DQuickDciIconImage);
    return d->imageItem->mirror();
}

bool DQuickDciIconImage::fallbackToQIcon() const
{
    D_DC(DQuickDciIconImage);
    return d->fallbackToQIcon;
}

void DQuickDciIconImage::setFallbackToQIcon(bool newFallbackToQIcon)
{
    D_D(DQuickDciIconImage);
    if (d->fallbackToQIcon == newFallbackToQIcon)
        return;
    d->fallbackToQIcon = newFallbackToQIcon;
    Q_EMIT fallbackToQIconChanged();
    d->updateImageSourceUrl();
}

bool DQuickDciIconImage::asynchronous() const
{
    D_DC(DQuickDciIconImage);
    return d->imageItem->asynchronous();
}

void DQuickDciIconImage::setAsynchronous(bool async)
{
    D_D(DQuickDciIconImage);
    d->imageItem->setAsynchronous(async);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
bool DQuickDciIconImage::retainWhileLoading() const
{
    D_DC(DQuickDciIconImage);
    return d->imageItem->retainWhileLoading();
}

void DQuickDciIconImage::setRetainWhileLoading(bool retain)
{
    D_D(DQuickDciIconImage);
    d->imageItem->setRetainWhileLoading(retain);
}
#endif

bool DQuickDciIconImage::cache() const
{
    D_DC(DQuickDciIconImage);
    return d->imageItem->cache();
}

void DQuickDciIconImage::setCache(bool cache)
{
    D_D(DQuickDciIconImage);
    d->imageItem->setCache(cache);
}

Dtk::Quick::DQuickIconImage *DQuickDciIconImage::imageItem() const
{
    D_DC(DQuickDciIconImage);
    return d->imageItem;
}

bool DQuickDciIconImage::isNull(const QString &iconName)
{
    return findDciIconPath(iconName,  appIconThemeName()).isEmpty();
}

DQuickIconAttached *DQuickDciIconImage::qmlAttachedProperties(QObject *object)
{
    auto item = qobject_cast<QQuickItem *>(object);

    if (!item)
        return nullptr;

    return new DQuickIconAttached(item);
}

void DQuickDciIconImage::classBegin()
{
    D_D(DQuickDciIconImage);
    QQmlEngine::setContextForObject(d->imageItem, QQmlEngine::contextForObject(this));
    QQuickItem::classBegin();
}

void DQuickDciIconImage::componentComplete()
{
    D_D(DQuickDciIconImage);
    d->imageItem->componentComplete();
    QQuickItem::componentComplete();
    d->layout();
}

class DQuickIconAttachedPrivate : public DCORE_NAMESPACE::DObjectPrivate
{
    D_DECLARE_PUBLIC(DQuickIconAttached)
public:
    DQuickIconAttachedPrivate(DCORE_NAMESPACE::DObject *qq)
    : DObjectPrivate(qq)
    {}

    DQMLGlobalObject::ControlState mode = DQMLGlobalObject::NormalState;
    DGuiApplicationHelper::ColorType theme = DGuiApplicationHelper::ColorType::LightType;
    DDciIconPalette palette;
    bool fallbackToQIcon = true;
};

DQuickIconAttached::DQuickIconAttached(QQuickItem *parent)
    : QObject(parent)
    , DObject(*new DQuickIconAttachedPrivate(this))
{
}

DQuickIconAttached::~DQuickIconAttached()
{
}

DQMLGlobalObject::ControlState DQuickIconAttached::mode() const
{
    D_DC(DQuickIconAttached);
    return d->mode;
}

void DQuickIconAttached::setMode(DQMLGlobalObject::ControlState mode)
{
    D_D(DQuickIconAttached);
    if (d->mode == mode)
        return;

    d->mode = mode;
    Q_EMIT modeChanged();
}

DGuiApplicationHelper::ColorType DQuickIconAttached::theme() const
{
    D_DC(DQuickIconAttached);
    return d->theme;
}

void DQuickIconAttached::setTheme(DGuiApplicationHelper::ColorType theme)
{
    D_D(DQuickIconAttached);
    if (d->theme == theme)
        return;

    d->theme = theme;
    Q_EMIT themeChanged();
}

DDciIconPalette DQuickIconAttached::palette() const
{
    D_DC(DQuickIconAttached);
    return d->palette;
}

void DQuickIconAttached::setPalette(const DDciIconPalette &palette)
{
    D_D(DQuickIconAttached);
    if (d->palette == palette)
        return;

    d->palette = palette;
    Q_EMIT paletteChanged();
}

bool DQuickIconAttached::fallbackToQIcon() const
{
    D_DC(DQuickIconAttached);
    return d->fallbackToQIcon;
}

void DQuickIconAttached::setFallbackToQIcon(bool newFallbackToQIcon)
{
    D_D(DQuickIconAttached);
    if (d->fallbackToQIcon == newFallbackToQIcon)
        return;
    d->fallbackToQIcon = newFallbackToQIcon;
    Q_EMIT fallbackToQIconChanged();
}

DQUICK_END_NAMESPACE
