// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "dbuiltiniconengine_p.h"

#include <DGuiApplicationHelper>

#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QImageReader>
#include <QDir>
#include <qmath.h>
#include <private/qiconloader_p.h>
#include <private/qguiapplication_p.h>
#include <QDebug>

#define BUILTIN_ICON_PATH ":/icons/deepin/builtin"

DGUI_BEGIN_NAMESPACE

class Q_DECL_HIDDEN ImageEntry : public QIconLoaderEngineEntry
{
public:
    enum Type {
        TextType, // 完全跟随画笔前景色变化
        ActionType, // 只在非normal状态下跟随画笔前景色
        IconType // 任务状态都不跟随画笔颜色
    };

    ImageEntry(Type t)
        : type(t)
    {

    }

    QString pmcKey(const QSize &size, QIcon::Mode mode, QIcon::State state)
    {
        return QLatin1String("$qt_icon_")
               + filename + QLatin1String("_")
               + QString::number((((((qint64(size.width()) << 11) | size.height()) << 11) | mode) << 4) | state, 16);
    }

    void genIconTypeIcon(QPixmap &pm, QIcon::Mode mode)
    {
        if (type == IconType && qobject_cast<QGuiApplication *>(QCoreApplication::instance())) {
            const QPixmap generated = QGuiApplicationPrivate::instance()->applyQIconStyleHelper(mode, pm);
            if (!generated.isNull())
                pm = generated;
        }
    }

    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
        Q_UNUSED(state)

        QPixmap pm;
        QString pmckey(pmcKey(size, mode, state));
        if (QPixmapCache::find(pmckey, &pm)) {
            genIconTypeIcon(pm, mode);
            return pm;
        }

        // png 会读取前8个 bit 对比，需要关闭重新打开，否则同一个 icon 不同 state/mode 会再次读取时报错导致 pixmap 为空。
        // 所以此处只对 DirImageEntry(isdir==true) 进行判断不再重新调用 setFileName
        if (Q_UNLIKELY(!reader.device()) || !QFileInfo(filename).isDir()) {
            reader.setFileName(filename);
        }

        if (dir.type == QIconDirInfo::Scalable)
            reader.setScaledSize(size);

        pm = QPixmap::fromImageReader(&reader);
        if (!pm.isNull())
            QPixmapCache::insert(pmckey, pm);

        genIconTypeIcon(pm, mode);
        return pm;
    }

    Type type;
    QImageReader reader;
};

class Q_DECL_HIDDEN DirImageEntry : public ImageEntry
{
public:
    using ImageEntry::ImageEntry;

    static QString getIconFile(const QString &key, const QDir &dir, const QString &suffix)
    {
        if (dir.exists(key + "." + suffix)) {
            return dir.filePath(key + "." + suffix);
        }

        int _index = key.indexOf('_');

        if (_index > 0) {
            const QString &mode = key.left(_index);

            if (dir.exists(mode + "." + suffix)) {
                return dir.filePath(mode + "." + suffix);
            }

            const QString &state = key.mid(_index);

            if (dir.exists("normal" + state + "." + suffix)) {
                return dir.filePath("normal" + state + "." + suffix);
            }
        }

        return dir.filePath("normal." + suffix);
    }

    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
        if (iconFileMap.isEmpty()) {
            const QString &suffix = QFileInfo(filename).suffix();
            QDir dir(filename);

            iconFileMap[QIcon::Disabled << 8 | QIcon::On] = getIconFile("disabled_on", dir, suffix);
            iconFileMap[QIcon::Disabled << 8 | QIcon::Off] = getIconFile("disabled_off", dir, suffix);
            iconFileMap[QIcon::Active << 8 | QIcon::On] = getIconFile("active_on", dir, suffix);
            iconFileMap[QIcon::Active << 8 | QIcon::Off] = getIconFile("active_off", dir, suffix);
            iconFileMap[QIcon::Selected << 8 | QIcon::On] = getIconFile("selected_on", dir, suffix);
            iconFileMap[QIcon::Selected << 8 | QIcon::Off] = getIconFile("selected_off", dir, suffix);
            iconFileMap[QIcon::Normal << 8 | QIcon::On] = getIconFile("normal_on", dir, suffix);
            iconFileMap[QIcon::Normal << 8 | QIcon::Off] = getIconFile("normal_off", dir, suffix);
        }

        reader.setFileName(iconFileMap.value(mode << 8 | state));

        return ImageEntry::pixmap(size, mode, state);
    }

    QMap<qint16, QString> iconFileMap;
};

DBuiltinIconEngine::DBuiltinIconEngine(const QString &iconName)
    : m_iconName(iconName)
    , m_initialized(false)
    // 当主题名称中包含'/'时, '/'前面的部分即为指定的图标类型, 此时不需要再
    // 跟随系统中的主题类型改变
    , m_followSystemTheme(iconName.indexOf('/') < 0)
{
    // 初始化图标类型
    m_key = iconName.startsWith("dark/") ? DGuiApplicationHelper::DarkType
                                         : DGuiApplicationHelper::LightType;
}

DBuiltinIconEngine::DBuiltinIconEngine(const DBuiltinIconEngine &other)
    : QIconEngine(other)
    , m_iconName(other.m_iconName)
    , m_key(other.m_key)
    , m_initialized(other.m_initialized)
    , m_followSystemTheme(other.m_initialized)
{

}

DBuiltinIconEngine::~DBuiltinIconEngine()
{
#if QT_VERSION <= QT_VERSION_CHECK(6, 2, 4)
    if (hasIcon())
        qDeleteAll(m_info.entries);
#endif
}

QSize DBuiltinIconEngine::actualSize(const QSize &size, QIcon::Mode mode,
                                     QIcon::State state)
{
    Q_UNUSED(mode);
    Q_UNUSED(state);

    ensureLoaded();

    QIconLoaderEngineEntry *entry = QIconLoaderEngine::entryForSize(m_info, size);
    if (entry) {
        const QIconDirInfo &dir = entry->dir;
        if (dir.type == QIconDirInfo::Scalable) {
            return size;
        } else {
            int result = qMin<int>(dir.size, qMin(size.width(), size.height()));
            return QSize(result, result);
        }
    }
    return QSize(0, 0);
}

QPixmap DBuiltinIconEngine::pixmap(const QSize &size, QIcon::Mode mode,
                                   QIcon::State state)
{
    ensureLoaded();

    QIconLoaderEngineEntry *entry = QIconLoaderEngine::entryForSize(m_info, size);
    if (entry)
        return entry->pixmap(size, mode, state);

    return QPixmap();
}

void DBuiltinIconEngine::paint(QPainter *painter, const QRect &rect,
                               QIcon::Mode mode, QIcon::State state)
{
    ensureLoaded();

    qreal scale = 1.0;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (qApp->testAttribute(Qt::AA_UseHighDpiPixmaps))
        scale = painter->device() ? painter->device()->devicePixelRatioF() : qApp->devicePixelRatio();
#else
    scale = painter->device() ? painter->device()->devicePixelRatioF() : qApp->devicePixelRatio();
#endif
    QSize pixmapSize = rect.size() * scale;

    QIconLoaderEngineEntry *entry = QIconLoaderEngine::entryForSize(m_info, pixmapSize);
    if (!entry)
        return;

    // 如果有 background 则绘制背景图先
    QString bgFileName = entry->filename + QStringLiteral(".background");

    if (QFile::exists(bgFileName)) {
        QIcon(bgFileName).paint(painter, rect, Qt::AlignCenter, mode, state);
    }

    QPixmap pm = entry->pixmap(pixmapSize, mode, state);
    ImageEntry::Type type = static_cast<ImageEntry *>(entry)->type;
    if (type == ImageEntry::TextType || (type == ImageEntry::ActionType && mode != QIcon::Normal)) {
        QPainter pa(&pm);
        pa.setCompositionMode(QPainter::CompositionMode_SourceIn);
        pa.fillRect(pm.rect(), painter->pen().brush());
    }

    pm.setDevicePixelRatio(scale);
    painter->drawPixmap(rect, pm);
}

QString DBuiltinIconEngine::key() const
{
    return QLatin1String("DBuiltinIconEngine");
}

QIconEngine *DBuiltinIconEngine::clone() const
{
    return new DBuiltinIconEngine(*this);
}


bool DBuiltinIconEngine::read(QDataStream &in)
{
    // QDataStream不支持位数据, 此处先将内容转为正常类型
    uint key = m_key;
    bool followSystemTheme = m_followSystemTheme;
    in >> m_iconName >> key >> followSystemTheme;
    m_key = key;
    m_followSystemTheme = followSystemTheme;
    return true;
}


bool DBuiltinIconEngine::write(QDataStream &out) const
{
    out << m_iconName << m_key << m_followSystemTheme;
    return true;
}

QString DBuiltinIconEngine::iconName()
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const
#endif
{
    return m_iconName;
}

QThemeIconInfo DBuiltinIconEngine::loadIcon(const QString &iconName, uint key)
{
    QThemeIconInfo info;
    info.iconName = iconName;

    QString theme_name = (key == DGuiApplicationHelper::DarkType ? "dark" : "light");
    QStringList iconDirList {
        QString("%1/%2/texts").arg(BUILTIN_ICON_PATH, theme_name),
        QString("%1/%2/actions").arg(BUILTIN_ICON_PATH, theme_name),
        QString("%1/%2/icons").arg(BUILTIN_ICON_PATH, theme_name),
        QString("%1/texts").arg(BUILTIN_ICON_PATH),
        QString("%1/actions").arg(BUILTIN_ICON_PATH),
        QString("%1/icons").arg(BUILTIN_ICON_PATH)
    };

    QDir dir;
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    for (int i = 0; i < iconDirList.count(); ++i) {
        dir.setPath(iconDirList.at(i));

        if (!dir.exists()) {
            continue;
        }

        ImageEntry::Type type = static_cast<ImageEntry::Type>(i % 3);

        for (const QFileInfo &icon_file_info : dir.entryInfoList()) {
            const QString &file_name = icon_file_info.fileName();

            // do not load background file
            if (!file_name.startsWith(iconName) || file_name.endsWith(QStringLiteral(".background")))
                continue;

            // 先找到px所在位置
            int size_str_pos = iconName.length() + 1;
            int px_str_pos = file_name.indexOf("px.", size_str_pos + 1);

            if (px_str_pos < 0)
                continue;

            bool ok = false;
            int size = file_name.mid(size_str_pos, px_str_pos - size_str_pos).toInt(&ok);

            if (Q_UNLIKELY(!ok || size <= 0)) {
                continue;
            }

            ImageEntry *entry = icon_file_info.isDir() ? new DirImageEntry(type) : new ImageEntry(type);
            entry->filename = icon_file_info.absoluteFilePath();
            entry->dir.path = icon_file_info.absolutePath();
            entry->dir.size = size;
            entry->dir.type = icon_file_info.suffix().startsWith("svg") ? QIconDirInfo::Scalable : QIconDirInfo::Fixed;
#if QT_VERSION <= QT_VERSION_CHECK(6, 2, 4)
            info.entries.append(entry);
#else
            info.entries.push_back(std::unique_ptr<QIconLoaderEngineEntry>(entry));
#endif
        }

        // 已经找到图标时不再继续
        if (info.entries.size() > 0) {
            break;
        }
    }

    return info;
}

bool DBuiltinIconEngine::hasIcon() const
{
    return m_info.entries.size() > 0;
}

void DBuiltinIconEngine::virtual_hook(int id, void *data)
{
    ensureLoaded();

    switch (id) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    case QIconEngine::AvailableSizesHook:
    {
        QIconEngine::AvailableSizesArgument &arg
            = *reinterpret_cast<QIconEngine::AvailableSizesArgument*>(data);
        const int N = m_info.entries.size();
        QList<QSize> sizes;
        sizes.reserve(N);

        // Gets all sizes from the DirectoryInfo entries
        for (int i = 0; i < N; ++i) {
            const QIconLoaderEngineEntry *entry = m_info.entries.at(i);
            int size = entry->dir.size;
            sizes.append(QSize(size, size));
        }
        arg.sizes.swap(sizes); // commit
    }
    break;
    case QIconEngine::IconNameHook:
    {
        QString &name = *reinterpret_cast<QString*>(data);
        name = m_info.iconName;
    }
    break;
#endif
    case QIconEngine::IsNullHook:
    {
        *reinterpret_cast<bool*>(data) = !hasIcon();
    }
    break;
    case QIconEngine::ScaledPixmapHook:
    {
        QIconEngine::ScaledPixmapArgument &arg = *reinterpret_cast<QIconEngine::ScaledPixmapArgument*>(data);
        // QIcon::pixmap() multiplies size by the device pixel ratio.
        const int integerScale = qCeil(arg.scale);
        QIconLoaderEngineEntry *entry = QIconLoaderEngine::entryForSize(m_info, arg.size / integerScale, integerScale);
        arg.pixmap = entry ? entry->pixmap(arg.size, arg.mode, arg.state) : QPixmap();
    }
    break;
    default:
        QIconEngine::virtual_hook(id, data);
    }
}

void DBuiltinIconEngine::ensureLoaded()
{
    if (Q_LIKELY(m_followSystemTheme) && Q_UNLIKELY(DGuiApplicationHelper::instance()->themeType() != m_key)) {
        m_initialized = false;
        // 记录当前使用的主题类型
        m_key = DGuiApplicationHelper::instance()->themeType();
    }

    if (Q_LIKELY(m_initialized)) {
        return;
    }
    // 标记为已初始化
    m_initialized = true;

#if QT_VERSION <= QT_VERSION_CHECK(6, 2, 4)
    qDeleteAll(m_info.entries);
#endif
    m_info.entries.clear();
    m_info.iconName.clear();

    Q_ASSERT(m_info.entries.size() == 0);
    m_info = loadIcon(m_iconName, m_key);
}

DGUI_END_NAMESPACE
