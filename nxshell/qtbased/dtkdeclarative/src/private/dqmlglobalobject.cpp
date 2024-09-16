// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#define private public
#include <QSGNode>
#undef private

#include "dqmlglobalobject_p.h"
#include "dqmlglobalobject_p_p.h"
#include "dquickcontrolpalette_p.h"
#include "dquickdciicon_p.h"
#include "dquickimageprovider_p.h"
#include "dmessagemanager_p.h"
#include "dpopupwindowhandle_p.h"
#include "dquickglobal_p.h"

#include <DObjectPrivate>
#include <DObject>
#include <DGuiApplicationHelper>
#include <DFontManager>
#include <DSysInfo>
#include <DNotifySender>
#include <QQuickItem>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qquickpalette_p.h>
#include <private/qquickitem_p.h>
#endif

#ifdef Q_OS_UNIX
#include <unistd.h>
#include <pwd.h>
#include <DStandardPaths>
#include <DPathBuf>
#endif

DGUI_USE_NAMESPACE

DQUICK_BEGIN_NAMESPACE

// ###(zccrs): The offset must greater than QColor::Spec
#define VARIANT_COLOR_TYPE_OFFSET 100
DColor::DColor(Type type)
{
    data.color.type = type + VARIANT_COLOR_TYPE_OFFSET;
}

bool DColor::isValid() const noexcept
{
    return isTypedColor() ? data.color.type > VARIANT_COLOR_TYPE_OFFSET : data.color.value.isValid();
}

bool DColor::isTypedColor() const noexcept
{
    return data.color.type >= VARIANT_COLOR_TYPE_OFFSET;
}

quint8 DColor::type() const noexcept
{
    if (!isTypedColor())
        return DColor::Invalid;
    return data.color.type - VARIANT_COLOR_TYPE_OFFSET;
}

static inline QPalette::ColorRole toPaletteColorRole(quint8 type)
{
    auto color = static_cast<DColor::Type>(type - VARIANT_COLOR_TYPE_OFFSET);
    if (color == DColor::Highlight)
        return QPalette::Highlight;
    if (color == DColor::HighlightedText)
        return QPalette::HighlightedText;
    return QPalette::NoRole;
}

bool DColor::operator==(const DColor &c) const noexcept
{
    if (data.color.type != c.data.color.type)
        return false;
    if (!isTypedColor() && data.color.value != c.data.color.value)
        return false;
    return data.hue == c.data.hue && data.saturation == c.data.saturation
            && data.lightness == c.data.lightness && data.opacity == c.data.opacity;
}

bool DColor::operator!=(const DColor &c) const noexcept
{
    return !operator ==(c);
}

QColor DColor::toColor(const QPalette &palette) const
{
    QColor color = isTypedColor() ? palette.color(toPaletteColorRole(data.color.type)) : data.color.value;
    return DGuiApplicationHelper::adjustColor(color, data.hue, data.saturation, data.lightness, 0, 0, 0, data.opacity);
}

QColor DColor::color() const
{
    Q_ASSERT(!isTypedColor());
    return DGuiApplicationHelper::adjustColor(data.color.value, data.hue, data.saturation, data.lightness,
                                              0, 0, 0, data.opacity);
}

DColor DColor::hue(qint8 floatValue) const
{
    DColor newColor = *this;
    newColor.data.hue += floatValue;
    return newColor;
}

DColor DColor::saturation(qint8 floatValue) const
{
    DColor newColor = *this;
    newColor.data.saturation += floatValue;
    return newColor;
}

DColor DColor::lightness(qint8 floatValue) const
{
    DColor newColor = *this;
    newColor.data.lightness += floatValue;
    return newColor;
}

DColor DColor::opacity(qint8 floatValue) const
{
    DColor newColor = *this;
    newColor.data.opacity += floatValue;
    return newColor;
}

DQMLGlobalObjectPrivate::DQMLGlobalObjectPrivate(DQMLGlobalObject *qq)
    : DTK_CORE_NAMESPACE::DObjectPrivate(qq)
{
}

void DQMLGlobalObjectPrivate::ensurePalette()
{
    if (paletteInit)
        return;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    quickPalette = new QQuickPalette(q_func());
    inactiveQuickPalette = new QQuickPalette(q_func());
#endif

    paletteInit = true;
    updatePalettes();

    QObject::connect(DGuiApplicationHelper::instance(), SIGNAL(applicationPaletteChanged()), q_func(), SLOT(_q_onPaletteChanged()));
}

void DQMLGlobalObjectPrivate::updatePalettes()
{
    palette = DGuiApplicationHelper::instance()->applicationPalette();

    for (int i = 0; i < QPalette::NColorRoles; ++i) {
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(i);
        inactivePalette.setBrush(QPalette::All, role, palette.brush(QPalette::Inactive, role));
        // The QML control will set the opacity property to 0.4 on the disabled state
        // The palette don't need set color for the QPalette::Disabled group.
        palette.setBrush(QPalette::All, role, palette.brush(QPalette::Active, role));
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    quickPalette->fromQPalette(palette);
    inactiveQuickPalette->fromQPalette(inactivePalette);
#endif
}

void DQMLGlobalObjectPrivate::_q_onPaletteChanged()
{
    updatePalettes();

    Q_EMIT q_func()->paletteChanged();
    Q_EMIT q_func()->inactivePaletteChanged();
}

void DQMLGlobalObjectPrivate::ensureWebsiteInfo()
{
    DCORE_USE_NAMESPACE;
    if (deepinWebsiteName.isEmpty() || deepinWebsiteLink.isEmpty()) {
        const auto &deepinWebsiteInfo = DSysInfo::distributionOrgWebsite(DSysInfo::Distribution);
        deepinWebsiteName = deepinWebsiteInfo.first;
        deepinWebsiteLink = deepinWebsiteInfo.second;
    }
}

DQMLGlobalObject::DQMLGlobalObject(QObject *parent)
    : QObject(parent)
    , DTK_CORE_NAMESPACE::DObject(*new DQMLGlobalObjectPrivate(this))
{
    auto pHelper = DWindowManagerHelper::instance();
    connect(pHelper, &DWindowManagerHelper::hasBlurWindowChanged, this, &DQMLGlobalObject::hasBlurWindowChanged);
    connect(pHelper, &DWindowManagerHelper::hasCompositeChanged, this, &DQMLGlobalObject::hasCompositeChanged);
    connect(pHelper, &DWindowManagerHelper::hasNoTitlebarChanged, this, &DQMLGlobalObject::hasNoTitlebarChanged);

    auto pAppHelper = DGuiApplicationHelper::instance();
    connect(pAppHelper, &DGuiApplicationHelper::themeTypeChanged, this, &DQMLGlobalObject::themeTypeChanged);
}

DQMLGlobalObject::~DQMLGlobalObject()
{
}

bool DQMLGlobalObject::hasBlurWindow() const
{
    return DWindowManagerHelper::instance()->hasBlurWindow();
}

bool DQMLGlobalObject::hasComposite() const
{
    return DWindowManagerHelper::instance()->hasComposite();
}

bool DQMLGlobalObject::hasNoTitlebar() const
{
    return DWindowManagerHelper::instance()->hasNoTitlebar();
}

bool DQMLGlobalObject::isSoftwareRender()
{
    static bool isSoftware = QQuickWindow::sceneGraphBackend() == QLatin1String("software");
    return isSoftware;
}

QString DQMLGlobalObject::windowManagerNameString() const
{
    return DWindowManagerHelper::instance()->windowManagerNameString();
}

DGuiApplicationHelper::ColorType DQMLGlobalObject::themeType() const
{
    return DGuiApplicationHelper::instance()->themeType();
}

DWindowManagerHelper::WMName DQMLGlobalObject::windowManagerName() const
{
    return DWindowManagerHelper::instance()->windowManagerName();
}

DPlatformThemeProxy *DQMLGlobalObject::platformTheme() const
{
    D_DC(DQMLGlobalObject);

    if (!d->platformTheme) {
        d->platformTheme = new DPlatformThemeProxy(DGuiApplicationHelper::instance()->applicationTheme(), const_cast<DQMLGlobalObject *>(this));
    }

    return d->platformTheme;
}

DFontManager *DQMLGlobalObject::fontManager() const
{
    // qml中只能识别非const的指针属性
    return const_cast<DFontManager*>(DGuiApplicationHelper::instance()->fontManager());
}

QPalette DQMLGlobalObject::palette() const
{
    D_DC(DQMLGlobalObject);
    const_cast<DQMLGlobalObjectPrivate*>(d)->ensurePalette();
    return d->palette;
}

QPalette DQMLGlobalObject::inactivePalette() const
{
    D_DC(DQMLGlobalObject);
    const_cast<DQMLGlobalObjectPrivate*>(d)->ensurePalette();
    return d->inactivePalette;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QQuickPalette *DQMLGlobalObject::quickPalette() const
{
    D_DC(DQMLGlobalObject);
    const_cast<DQMLGlobalObjectPrivate*>(d)->ensurePalette();
    return d->quickPalette;
}

QQuickPalette *DQMLGlobalObject::inactiveQuickPalette() const
{
    D_DC(DQMLGlobalObject);
    const_cast<DQMLGlobalObjectPrivate*>(d)->ensurePalette();
    return d->inactiveQuickPalette;
}
#endif

QColor DQMLGlobalObject::blendColor(const QColor &substrate, const QColor &superstratum)
{
    return DGuiApplicationHelper::blendColor(substrate, superstratum);
}

DColor DQMLGlobalObject::makeColor(DColor::Type type)
{
    return DColor(type);
}

DColor DQMLGlobalObject::makeColor(const QColor &color)
{
    return DColor(color);
}

QUrl DQMLGlobalObject::makeShadowImageUrl(qreal boxSize, qreal cornerRadius, qreal shadowBlur, QColor color,
                                          qreal xOffset, qreal yOffset, qreal spread, bool hollow, bool inner)
{
    return makeShadowImageUrl(boxSize, cornerRadius, cornerRadius, cornerRadius, cornerRadius, shadowBlur, color, xOffset, yOffset, spread, hollow, inner);
}

QUrl DQMLGlobalObject::makeShadowImageUrl(qreal boxSize, qreal topLeftRadius, qreal topRightRadius, qreal bottomLeftRadius, qreal bottomRightRadius,
                                          qreal shadowBlur, QColor color, qreal xOffset, qreal yOffset, qreal spread, bool hollow, bool inner)
{
    return DQuickShadowProvider::toUrl(boxSize, topLeftRadius, topRightRadius, bottomLeftRadius, bottomRightRadius, shadowBlur, color, xOffset, yOffset, spread, hollow, inner);
}

DGuiApplicationHelper::ColorType DQMLGlobalObject::toColorType(const QColor &color)
{
    return DGuiApplicationHelper::toColorType(color);
}

QColor DQMLGlobalObject::selectColor(const QColor &windowColor, const QColor &light, const QColor &dark)
{
    if (toColorType(windowColor) == DGuiApplicationHelper::DarkType) {
        return dark;
    } else {
        return light;
    }
}

QString DQMLGlobalObject::deepinWebsiteName() const
{
    D_DC(DQMLGlobalObject);

    const_cast<DQMLGlobalObjectPrivate*>(d)->ensureWebsiteInfo();

    return d->deepinWebsiteName;
}

QString DQMLGlobalObject::deepinWebsiteLink() const
{
    D_DC(DQMLGlobalObject);

    const_cast<DQMLGlobalObjectPrivate*>(d)->ensureWebsiteInfo();

    return d->deepinWebsiteLink;
}

QString DQMLGlobalObject::deepinDistributionOrgLogo() const
{
    DCORE_USE_NAMESPACE;

    D_DC(DQMLGlobalObject);
    if (d->deepinDistributionOrgLogo.isEmpty()) {
        const auto &logo = DSysInfo::distributionOrgLogo(DSysInfo::Distribution, DSysInfo::Light, ":/assets/images/deepin-logo.svg");
        const_cast<DQMLGlobalObjectPrivate*>(d)->deepinDistributionOrgLogo = logo;

    }
    return d->deepinDistributionOrgLogo;

}

QPoint DQMLGlobalObject::cursorPosition() const
{
    return QCursor::pos();
}

DQuickDciIcon DQMLGlobalObject::makeIcon(const QJSValue &qicon, const QJSValue &iconExtra)
{
    if (!qicon.isObject() || !iconExtra.isObject()) {
        ThrowError(this, QStringLiteral("The arguments is not valid JavaScript object"));
        return {};
    }

    const QString &name = qicon.property("name").toString();
    int width = qicon.property("width").toInt();
    int height = qicon.property("height").toInt();
    const QColor &color = qicon.property("color").toVariant().value<QColor>();
    const QUrl &source = qicon.property("source").toVariant().value<QUrl>();

    DQuickDciIcon dciIcon;
    int mode = iconExtra.property("mode").toInt();
    int theme = iconExtra.property("theme").toInt();
    const auto fallbackToQIcon = iconExtra.property("fallbackToQIcon");
    if (fallbackToQIcon.isBool()) {
        dciIcon.setFallbackToQIcon(fallbackToQIcon.toBool());
    }

    DDciIconPalette palette;
    palette.setForeground(color);
    auto paletteProp = iconExtra.property("palette");
    if (paletteProp.isObject()) {
        QColor foreground = qvariant_cast<QColor>(paletteProp.property("foreground").toVariant());
        if (!foreground.isValid())
            foreground = color;
        QColor background = qvariant_cast<QColor>(paletteProp.property("background").toVariant());
        QColor highlight = qvariant_cast<QColor>(paletteProp.property("highlight").toVariant());
        QColor highlightForeground = qvariant_cast<QColor>(paletteProp.property("highlightForeground").toVariant());
        palette.setForeground(foreground);
        palette.setBackground(background);
        palette.setHighlightForeground(highlightForeground);
        palette.setHighlight(highlight);
    }

    dciIcon.setName(name);
    dciIcon.setWidth(width);
    dciIcon.setHeight(height);
    dciIcon.setMode(ControlState(mode));
    dciIcon.setTheme(DGuiApplicationHelper::ColorType(theme));
    dciIcon.setPalette(palette);
    dciIcon.setSource(source);
    return dciIcon;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
DDciIconPalette DQMLGlobalObject::makeIconPalette(const QPalette &palette)
{
    DDciIconPalette iconPalette;
    iconPalette.setForeground(palette.color(QPalette::WindowText));
    iconPalette.setBackground(palette.color(QPalette::Window));
    iconPalette.setHighlight(palette.color(QPalette::Highlight));
    iconPalette.setHighlightForeground(palette.color(QPalette::HighlightedText));
    return iconPalette;
}
#else
DDciIconPalette DQMLGlobalObject::makeIconPalette(const QQuickPalette *palette)
{
    DDciIconPalette iconPalette;
    iconPalette.setForeground(palette->windowText());
    iconPalette.setBackground(palette->window());
    iconPalette.setHighlight(palette->highlight());
    iconPalette.setHighlightForeground(palette->highlightedText());
    return iconPalette;
}
#endif

bool DQMLGlobalObject::sendMessage(QObject *target, const QString &content, const QString &iconName, int duration, const QString &msgId)
{
    Q_ASSERT(target);

    QQuickWindow *window = nullptr;
    if (auto item = qobject_cast<QQuickItem *>(target)) {
        window = item->window();
    } else {
        window = qobject_cast<QQuickWindow *>(target);
    }
    if (window) {
        if (auto manager = qobject_cast<MessageManager *>(qmlAttachedPropertiesObject<MessageManager>(window))) {
            return manager->sendMessage(content, iconName, duration, msgId);
        }
    }
    return false;
}

bool DQMLGlobalObject::sendMessage(QObject *target, QQmlComponent *delegate, const QVariant &message, int duration, const QString &msgId)
{
    Q_ASSERT(target);

    QQuickWindow *window = nullptr;
    if (auto item = qobject_cast<QQuickItem *>(target)) {
        window = item->window();
    } else {
        window = qobject_cast<QQuickWindow *>(target);
    }
    if (window) {
        if (auto manager = qobject_cast<MessageManager *>(qmlAttachedPropertiesObject<MessageManager>(window)))
            return manager->sendMessage(delegate, message, duration, msgId);
    }
    return false;
}

void DQMLGlobalObject::closeMessage(FloatingMessageContainer *message)
{
    Q_ASSERT(message);

    message->close();
}

void DQMLGlobalObject::closeMessage(QObject *target, const QString &msgId)
{
    Q_ASSERT(target);

    if (msgId.isEmpty())
        return;

    QQuickWindow *window = nullptr;
    if (auto item = qobject_cast<QQuickItem *>(target)) {
        window = item->window();
    } else {
        window = qobject_cast<QQuickWindow *>(target);
    }
    if (window) {
        if (auto manager = qobject_cast<MessageManager *>(qmlAttachedPropertiesObject<MessageManager>(window)))
            manager->close(msgId);
    }
}

void DQMLGlobalObject::sendSystemMessage(const QString &summary, const QString &body, const QString &appIcon, const QStringList &actions, const QVariantMap hints, const int timeout, const uint replaceId)
{
    QDBusPendingCall reply = DTK_CORE_NAMESPACE::DUtil::DNotifySender(summary)
            .appName(qAppName())
            .appIcon(appIcon)
            .appBody(body)
            .actions(actions)
            .hints(hints)
            .replaceId(replaceId)
            .timeOut(timeout)
            .call();
    auto watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher* watcher){
        if (watcher->isError())
            qWarning() << "DQMLGlobalObject::sendSystemMessage: send system message error" << watcher->error();

        watcher->deleteLater();
    });
}

void DQMLGlobalObject::setPopupMode(const PopupMode mode)
{
    DPopupWindowHandle::setPopupMode(mode);
}

bool DQMLGlobalObject::loadTranslator()
{
    DCORE_USE_NAMESPACE;

    QList<QString> translateDirs;
    const QString dtkdeclarativeTranslationPath(DDECLARATIVE_TRANSLATIONS_DIR);
    //("/home/user/.local/share", "/usr/local/share", "/usr/share")
    auto dataDirs = DStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const auto &path : dataDirs) {
        DPathBuf DPathBuf(path);
        translateDirs << (DPathBuf / dtkdeclarativeTranslationPath).toString();
    }
#ifdef DTK_STATIC_TRANSLATION
    translateDirs << QString(":/dtk/translations");
#endif
    return DGuiApplicationHelper::loadTranslator("dtkdeclarative", translateDirs, QList<QLocale>() << QLocale::system());
}

#if QT_VERSION_MAJOR > 5
QSGRootNode *DQMLGlobalObject::getRootNode(QQuickItem *item)
{
    const auto d = QQuickItemPrivate::get(item);
    QSGNode *root = d->itemNode();
    if (!root)
        return nullptr;

    while (root->firstChild() && root->type() != QSGNode::RootNodeType)
        root = root->firstChild();
    return root->type() == QSGNode::RootNodeType ? static_cast<QSGRootNode*>(root) : nullptr;
}

int &DQMLGlobalObject::QSGNode_subtreeRenderableCount(QSGNode *node)
{
    return node->m_subtreeRenderableCount;
}

QSGNode *&DQMLGlobalObject::QSGNode_firstChild(QSGNode *node)
{
    return node->m_firstChild;
}

QSGNode *&DQMLGlobalObject::QSGNode_lastChild(QSGNode *node)
{
    return node->m_lastChild;
}

QSGNode *&DQMLGlobalObject::QSGNode_parent(QSGNode *node)
{
    return node->m_parent;
}
#endif

DQUICK_END_NAMESPACE

#include "moc_dqmlglobalobject_p.cpp"
