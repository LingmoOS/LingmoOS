// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "diconproxyengine_p.h"
#include "dciiconengine_p.h"
#include "dbuiltiniconengine_p.h"
#include "xdgiconproxyengine_p.h"

#include <DGuiApplicationHelper>
#include <DPlatformTheme>
#include <DIconTheme>

#include <QIconEnginePlugin>
#include <QPainter>
#include <QPixmap>
#include <QDebug>
#include <QDir>

#include <private/qiconloader_p.h>
#if (XDG_ICON_VERSION_MAR >= 3)
#define private public
#include <private/xdgiconloader/xdgiconloader_p.h>
#undef private
#endif
#include <private/qguiapplication_p.h>
#include <qpa/qplatformtheme.h>
DGUI_BEGIN_NAMESPACE

static inline QString iconThemeName()
{
    return DGuiApplicationHelper::instance()->applicationTheme()->iconThemeName();
}

static inline QIconEngine *createBuiltinIconEngine(const QString &iconName)
{
    QIconEngine *iconEngine = new DBuiltinIconEngine(iconName);
    if (iconEngine->isNull()) {
        delete iconEngine;
        return nullptr;
    }
    return iconEngine;
}

static inline QIconEngine *createDciIconEngine(const QString &iconName)
{
    QIconEngine *iconEngine = new DDciIconEngine(iconName);
    if (iconEngine->isNull()) {
        delete iconEngine;
        return nullptr;
    }
    return iconEngine;
}

#ifndef DTK_DISABLE_LIBXDG
static inline QIconEngine *createXdgProxyIconEngine(const QString &iconName)
{
    QIconEngine *iconEngine = new XdgIconProxyEngine(new XdgIconLoaderEngine(iconName));
    if (iconEngine->isNull()) {
        delete iconEngine;
        return nullptr;
    }
    return iconEngine;
}
#endif

static inline QIconEngine *createDBuiltinIconEngine(const QString &iconName)
{
    static QSet<QString> non_builtin_icon_cache;

    if (!non_builtin_icon_cache.contains(iconName)) {
        // 记录下来此种类型的icon为内置图标
        // 因此，此处添加的缓存不考虑更新
        // 优先使用内置图标
        if (QIconEngine *engine = createBuiltinIconEngine(iconName)) {
            if (engine->isNull()) {
                delete engine;
                return nullptr;
            }
            return engine;
        } else {
            non_builtin_icon_cache.insert(iconName);
        }
    }

    return nullptr;
}

static bool hasDciIcon(const QString &iconName, const QString &iconThemeName)
{
    QString iconPath;
    if (auto cached = DIconTheme::cached()) {
        iconPath = cached->findDciIconFile(iconName, iconThemeName);
    } else {
        iconPath = DIconTheme::findDciIconFile(iconName, iconThemeName);
    }

    return !iconPath.isEmpty();
}

static inline bool isDciIconEngine(QIconEngine *engine)
{
    return dynamic_cast<DDciIconEngine *>(engine);
}

DIconProxyEngine::DIconProxyEngine(const QString &iconName, DIconTheme::Options options)
    : m_iconName(iconName)
    , m_option(options)
{
    ensureEngine();
}

DIconProxyEngine::DIconProxyEngine(const DIconProxyEngine &other)
    : QIconEngine(other)
    , m_iconName(other.m_iconName)
    , m_iconThemeName(other.m_iconThemeName)
    , m_iconEngine(other.m_iconEngine->clone())
{
    ensureEngine();
}

DIconProxyEngine::~DIconProxyEngine()
{
    if (m_iconEngine)
        delete m_iconEngine;
}

QSize DIconProxyEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    ensureEngine();
    return m_iconEngine ? m_iconEngine->actualSize(size, mode, state) :QSize();
}

QPixmap DIconProxyEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    ensureEngine();
    return m_iconEngine ? m_iconEngine->pixmap(size, mode, state) : QPixmap();
}

void DIconProxyEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    ensureEngine();
    if (m_iconEngine)
        m_iconEngine->paint(painter, rect, mode, state);
}

QString DIconProxyEngine::key() const
{
    return QLatin1String("DIconProxyEngine");
}

QIconEngine *DIconProxyEngine::clone() const
{
    return new DIconProxyEngine(*this);
}

bool DIconProxyEngine::read(QDataStream &in)
{
    in >> m_iconName >> m_iconThemeName;

    ensureEngine();
    return m_iconEngine ? m_iconEngine->read(in) : false;
}

bool DIconProxyEngine::write(QDataStream &out) const
{
    out << m_iconName << m_iconThemeName;
    return m_iconEngine ? m_iconEngine->write(out) : false;
}

QString DIconProxyEngine::iconName()
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
const
#endif
{
    return m_iconEngine ? m_iconEngine->iconName() : QString();
}

QList<QSize> DIconProxyEngine::availableSizes(QIcon::Mode mode, QIcon::State state)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const
#endif
{
    return m_iconEngine ? m_iconEngine->availableSizes(mode, state) : QIconEngine::availableSizes(mode, state);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool DIconProxyEngine::isNull()
{
    return m_iconEngine ? m_iconEngine->isNull() : QIconEngine::isNull();
}
#endif

QString DIconProxyEngine::proxyKey()
{
    ensureEngine();
    return m_iconEngine ? m_iconEngine->key() : QString();
}

void DIconProxyEngine::virtual_hook(int id, void *data)
{
    ensureEngine();
    if (m_iconEngine) {
        m_iconEngine->virtual_hook(id, data);
        return;
    }

    switch (id) {
    case QIconEngine::IsNullHook:
        {
            *reinterpret_cast<bool*>(data) = true;
        }
        break;
    default:
        QIconEngine::virtual_hook(id, data);
    }
}

void DIconProxyEngine::ensureEngine()
{
    if (m_iconName.isEmpty())
        return;

    const QString &theme = iconThemeName();
    if (theme == m_iconThemeName && m_iconEngine)
        return;

    static QMap<QString, QSet<QString>> nonCache;
    if (Q_UNLIKELY(!m_option.testFlag(DIconTheme::IgnoreIconCache)))
    {
        const auto it = nonCache.find(theme);
        if (it != nonCache.end() && it->contains(m_iconName))
            return;
    }

    if (m_iconEngine) {
        // dci => dci
        // xdg => xdg
        if (!(hasDciIcon(m_iconName, theme) ^ isDciIconEngine(m_iconEngine))) {
            m_iconThemeName = theme;
            return;
        }

         // delete old engine and create a new engine
        delete m_iconEngine;
        m_iconEngine = nullptr;
    }

    // null => dci
    // null => xdg
    // dci  => xdg
    // xdg  => dci
    // 1. try create dci iconengine
    // 2. try create builtin iconengine
    // 3. create xdgiconproxyengine
    if (!m_iconEngine && Q_UNLIKELY(!m_option.testFlag(DIconTheme::IgnoreDciIcons))) {
        m_iconEngine = createDciIconEngine(m_iconName);

    }
    if (!m_iconEngine && Q_UNLIKELY(!m_option.testFlag(DIconTheme::IgnoreBuiltinIcons)) ) {
        m_iconEngine = createDBuiltinIconEngine(m_iconName);
    }
#ifdef DTK_DISABLE_LIBXDG
    if (!m_iconEngine) {
        QPlatformTheme * const platformTheme = QGuiApplicationPrivate::platformTheme();
        if (platformTheme) {
            m_iconEngine = platformTheme->QPlatformTheme::createIconEngine(m_iconName);
        } else {
            qWarning() << "PlatformTheme not found!";
        }
    }
#else
    if (!m_iconEngine ) {
        m_iconEngine = createXdgProxyIconEngine(m_iconName);
    }
#endif
    if (!m_iconEngine && !nonCache[theme].contains(m_iconName)) {
        qWarning("create icon [%s] engine failed.[theme:%s] nonCache[theme].size[%d]",
                      m_iconName.toUtf8().data(),
                      theme.toUtf8().data(), nonCache[theme].size());
        nonCache[theme].insert(m_iconName);
        return;
    }

    m_iconThemeName = theme;
}

DGUI_END_NAMESPACE

