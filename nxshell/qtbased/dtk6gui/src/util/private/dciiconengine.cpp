// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dciiconengine_p.h"
#include "dguiapplicationhelper.h"
#include "dplatformtheme.h"

#include <QPainter>
#include <QPixmap>

#include <private/qhexstring_p.h>
#include <private/qiconloader_p.h>

DGUI_BEGIN_NAMESPACE

static inline DDciIcon::Theme dciTheme()
{
    auto theme = DGuiApplicationHelper::instance()->themeType();
    return theme == DGuiApplicationHelper::DarkType ? DDciIcon::Dark : DDciIcon::Light;
}

static inline DDciIcon::Mode dciMode(QIcon::Mode mode)
{
    // QIcon only support DDciIcon::Disabled and DDciIcon::Normal
    return mode == QIcon::Disabled ? DDciIcon::Disabled : DDciIcon::Normal;
}

static inline DDciIconPalette dciPalettle(QPaintDevice *paintDevice = nullptr)
{
    QPalette pa;
    if (!paintDevice || paintDevice->devType() != QInternal::Widget) {
        pa = qApp->palette();
    } else if (QObject *obj = dynamic_cast<QObject *>(paintDevice)) {
        pa = qvariant_cast<QPalette>(obj->property("palette"));
    } else {
        pa = qApp->palette();
    }

    return DDciIconPalette(pa.windowText().color(), pa.window().color(),
                           pa.highlight().color(), pa.highlightedText().color());
}

static inline qreal deviceRadio(QPaintDevice *paintDevice = nullptr)
{
    qreal scale = 1.0;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (qApp->testAttribute(Qt::AA_UseHighDpiPixmaps))
        scale = paintDevice ? paintDevice->devicePixelRatioF() : qApp->devicePixelRatio();
#else
    scale = paintDevice ? paintDevice->devicePixelRatioF() : qApp->devicePixelRatio();
#endif

    return scale;
}

DDciIconEngine::DDciIconEngine(const QString &iconName)
    : m_iconName(iconName)
    , m_iconThemeName(DGuiApplicationHelper::instance()->applicationTheme()->iconThemeName())
    , m_dciIcon(DDciIcon::fromTheme(iconName))
{

}

DDciIconEngine::DDciIconEngine(const DDciIconEngine &other)
    : QIconEngine(other)
    , m_iconName(other.m_iconName)
    , m_iconThemeName(other.m_iconThemeName)
    , m_dciIcon(other.m_dciIcon)
{

}

DDciIconEngine::~DDciIconEngine()
{

}

QSize DDciIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(state);
    ensureIconTheme();
    int s = m_dciIcon.actualSize(qMin(size.width(), size.height()), dciTheme(), dciMode(mode));
    return QSize(s, s).boundedTo(size);
}

QPixmap DDciIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    // QIcon::pixmap has already been multiplied by the screen scaling factor
    // so there is no need to do it again here. set radio 1.0 .
    return pixmap(size, mode, state, 1.0);
}

QPixmap DDciIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal radio)
{
    Q_UNUSED(state);

    const int s = qMin(size.width(), size.height());
    const DDciIcon::Theme theme = dciTheme();
    const DDciIconPalette pa = dciPalettle();

    QString key = QLatin1String("dci_") + m_iconName + m_iconThemeName +
            DDciIconPalette::convertToString(pa)
            % HexString<uint>(mode)
            % HexString<int>(theme)
            % HexString<int>(s)
            % HexString<uint>(uint(radio * 100));

    QPixmap pix;
    if (QPixmapCache::find(key, &pix))
        return pix;

    ensureIconTheme();
    pix = m_dciIcon.pixmap(radio, s, theme, dciMode(mode), pa);
    if (!pix.isNull())
        QPixmapCache::insert(key, pix);

    return pix;
}

void DDciIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(state);
    ensureIconTheme();
    m_dciIcon.paint(painter, rect, deviceRadio(painter->device()), dciTheme(),
                    dciMode(mode), Qt::AlignCenter, dciPalettle(painter->device()));
}

QString DDciIconEngine::key() const
{
    return QLatin1String("DDciIconEngine");
}

QIconEngine *DDciIconEngine::clone() const
{
    return new DDciIconEngine(*this);
}

bool DDciIconEngine::read(QDataStream &in)
{
    ensureIconTheme();
    in >> m_iconThemeName >> m_iconName >> m_dciIcon;
    return true;
}

bool DDciIconEngine::write(QDataStream &out) const
{
    const_cast<DDciIconEngine *>(this)->ensureIconTheme();
    out << m_iconThemeName << m_iconName << m_dciIcon;
    return true;
}

QString DDciIconEngine::iconName()
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
const
#endif
{
    return m_iconName;
}

void DDciIconEngine::virtual_hook(int id, void *data)
{
    ensureIconTheme();
    switch (id) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    case QIconEngine::AvailableSizesHook:
        {
            auto &arg = *reinterpret_cast<QIconEngine::AvailableSizesArgument*>(data);
            auto availableSizes = m_dciIcon.availableSizes(dciTheme(), DDciIcon::Normal);
            QList<QSize> sizes;
            sizes.reserve(availableSizes.size());

            for (int size : availableSizes)
                sizes.append(QSize(size, size));

            arg.sizes.swap(sizes); // commit
        }
        break;
    case QIconEngine::IconNameHook:
        {
            QString &name = *reinterpret_cast<QString*>(data);
            name = iconName();
        }
        break;
#endif
    case QIconEngine::IsNullHook:
        {
            *reinterpret_cast<bool*>(data) = m_dciIcon.isNull();
        }
        break;
    case QIconEngine::ScaledPixmapHook:
        {
            QIconEngine::ScaledPixmapArgument &arg = *reinterpret_cast<QIconEngine::ScaledPixmapArgument*>(data);
            arg.pixmap = pixmap(arg.size, arg.mode, arg.state);
        }
        break;
    default:
        QIconEngine::virtual_hook(id, data);
    }
}

void DDciIconEngine::ensureIconTheme()
{
    QString iconThemeName = DGuiApplicationHelper::instance()->applicationTheme()->iconThemeName();
    if (m_iconThemeName != iconThemeName) {
        m_iconThemeName = iconThemeName;
        // update dci icon when icon theme name changed.
        m_dciIcon = DDciIcon::fromTheme(m_iconName);
    }
}

DGUI_END_NAMESPACE
