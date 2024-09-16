// SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddciicon.h"
#include "dguiapplicationhelper.h"
#include "dicontheme.h"

#include <DObjectPrivate>
#include <DDciFile>
#include <DPlatformTheme>
#include <DSGApplication>
#include <private/qguiapplication_p.h>

#include <QPainter>
#include <QBuffer>
#include <QImageReader>
#include <QtMath>
#include <QDir>

DCORE_USE_NAMESPACE
DGUI_BEGIN_NAMESPACE

#define MODE_NORMAL "normal"
#define MODE_DISABLED "disabled"
#define MODE_HOVER "hover"
#define MODE_PRESSED "pressed"

#define THEME_LIGHT "light"
#define THEME_DARK "dark"
#define ALPHA8STRING "alpha8"

struct DDciIconEntry {
    struct ScalableLayer {
        int imagePixelRatio = 0;
        struct Layer {
            int prior = 0;
            DDciIconPalette::PaletteRole role = DDciIconPalette::NoPalette;
            QByteArray format;
            QByteArray data;
            bool isAlpha8Format = false;

            qint8 hue = 0;
            qint8 saturation = 0;
            qint8 lightness = 0;
            qint8 red = 0;
            qint8 green = 0;
            qint8 blue = 0;
            qint8 alpha = 0;
            qint16 padding = 0;
        };
        QVector<Layer> layers;
    };


    int iconSize = 0;
    qint16 maxPaddings = 0;
    DDciIcon::Mode mode = DDciIcon::Normal;
    DDciIcon::Theme theme = DDciIcon::Light;
    QVector<ScalableLayer> scalableLayers;
    inline bool isNull() const { return scalableLayers.isEmpty(); }
};

struct EntryNode {
    int iconSize = 0;
    qint16 maxPaddings = 0;
    QVector<DDciIconEntry *> entries;
};
using EntryNodeList = QVector<EntryNode>;

class EntryPropertyParser
{
public:
    static void registerSteps();
    static void doParse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringView> &properties);
private:
    static struct Step {
        virtual ~Step() {}
        Step *nextStep = nullptr;
        virtual QVector<QStringView> parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringView> &properties) = 0;
    } *root;

    struct PriorStep : public Step {
        QVector<QStringView> parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringView> &properties) override;
    };

    struct FormatAndAlpha8Step : public Step {
        QVector<QStringView> parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringView> &properties) override;
    };

    struct PaddingStep : public Step {
        QVector<QStringView> parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringView> &properties) override;
    };

    struct PaletteStep : public Step {
        QVector<QStringView> parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringView> &properties) override;
    };
};

EntryPropertyParser::Step *EntryPropertyParser::root = nullptr;

void EntryPropertyParser::registerSteps()
{
    static PriorStep priorSt;
    static FormatAndAlpha8Step formatSt;
    static PaddingStep paddingSt;
    static PaletteStep paletteSt;

    priorSt.nextStep = &formatSt;
    formatSt.nextStep = &paddingSt;
    paddingSt.nextStep = &paletteSt;
    paletteSt.nextStep = nullptr;
    root = &priorSt;
}

void EntryPropertyParser::doParse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringView> &properties)
{
    Q_ASSERT(layer);
    if (!root)
        registerSteps();
    EntryPropertyParser::Step *step = root;
    QVector<QStringView> ps = properties;
    while (step) {
        // If the input information flow is empty, it means that the next steps do not need to be continued
        if (ps.isEmpty())
            break;
        ps = step->parse(layer, ps);
        step = step->nextStep;
    }
}

QVector<QStringView> EntryPropertyParser::PriorStep::parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringView> &properties)
{
    bool ok = false;
    QVector<QStringView> ps = properties;
    layer->prior = ps.takeFirst().toString().toInt(&ok);
    if (!ok)
        return {};  // error priority.
    return ps;
}

QVector<QStringView> EntryPropertyParser::FormatAndAlpha8Step::parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringView> &properties)
{
    QVector<QStringView> ps = properties;
    const QString alpha8OrFormat = ps.takeLast().toString();
    if (alpha8OrFormat.compare(ALPHA8STRING, Qt::CaseInsensitive) == 0) {
        // Alpha8 format
        layer->isAlpha8Format = true;
        layer->format = ps.takeLast().toLatin1();
    } else {
        // normal format
        layer->format = alpha8OrFormat.toLatin1();
    }

    return ps;
}

QVector<QStringView> EntryPropertyParser::PaddingStep::parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringView> &properties)
{
    QVector<QStringView> ps = properties;
    // Take the padding property
    auto it = std::find_if(ps.cbegin(), ps.cend(), [](const QStringView &p) {
        return p.endsWith(QLatin1Char('p'));
    });

    if (it != ps.cend()) {
        layer->padding = it->left(it->length() - 1).toString().toShort();
        ps.removeOne(*it);
    }

    return ps;
}

QVector<QStringView> EntryPropertyParser::PaletteStep::parse(DDciIconEntry::ScalableLayer::Layer *layer, const QVector<QStringView> &properties)
{
    QVector<QStringView> ps = properties;
    QStringView palettes = ps.takeFirst();
    // `role_hue_saturation_lightness_red_green_blue_alpha` or `role`
    if (palettes.toString().contains(QLatin1Char('_'))) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // QVector is an alias for QList in Qt6
        const QVector<QStringView> paletteProps = palettes.split(QLatin1Char('_'));
#elif QT_VERSION >= QT_VERSION_CHECK(5, 15, 2)
        // QStringView::split() has been added in 5.12.2.
        const QVector<QStringView> paletteProps = QVector<QStringView>::fromList(palettes.split(QLatin1Char('_')));
#else
        QStringList strList = palettes.toString().split(QLatin1Char('_'));
        QList<QStringView> viewList;
        for (auto str : strList) {
            viewList.append(QStringView(str));
        }
        const QVector<QStringView> paletteProps = QVector<QStringView>::fromList(viewList);
#endif
        if (paletteProps.length() != 8)
            return ps;

        int role = paletteProps.at(0).toString().toInt();
        if (role < DDciIconPalette::NoPalette || role > DDciIconPalette::PaletteCount)
            role = DDciIconPalette::NoPalette;

        layer->role = DDciIconPalette::PaletteRole(role);
        layer->hue = static_cast<qint8>(paletteProps.at(1).toString().toShort());
        layer->saturation = static_cast<qint8>(paletteProps.at(2).toString().toShort());
        layer->lightness = static_cast<qint8>(paletteProps.at(3).toString().toShort());
        layer->red = static_cast<qint8>(paletteProps.at(4).toString().toShort());
        layer->green = static_cast<qint8>(paletteProps.at(5).toString().toShort());
        layer->blue = static_cast<qint8>(paletteProps.at(6).toString().toShort());
        layer->alpha = static_cast<qint8>(paletteProps.at(7).toString().toShort());
    } else {
        // only palette
        int role = palettes.toString().toInt();
        if (role < DDciIconPalette::NoPalette || role > DDciIconPalette::PaletteCount)
            role = DDciIconPalette::NoPalette;
        layer->role = DDciIconPalette::PaletteRole(role);
    }

    return ps;
}

void alpha8ImageDeleter(void *image) {
    delete static_cast<QImage *>(image);
}

static QImage readImageData(QImageReader &reader, qreal pixmapScale, bool isAlpha8Format)
{
    QImage image;

    if (reader.canRead()) {
        bool scaled = false;
        int imageSize = qMax(reader.size().width(), reader.size().height());
        int scaledSize = qRound(pixmapScale * imageSize);
        if (!isAlpha8Format && reader.supportsOption(QImageIOHandler::ScaledSize)) {
            reader.setScaledSize(reader.size().scaled(scaledSize, scaledSize, Qt::KeepAspectRatio));
            scaled = true;
        }

        QImage *imagePtr = &image;
        if (isAlpha8Format)
            imagePtr = new QImage();
        reader.read(imagePtr);
        if (isAlpha8Format) {
            QImage tt(imagePtr->bits(), imagePtr->width(), imagePtr->width(), imagePtr->bytesPerLine(),
                      QImage::Format_Alpha8, alpha8ImageDeleter, static_cast<void *>(imagePtr));
            return tt.scaled(scaledSize, scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        if (!scaled)
            image = image.scaled(scaledSize, scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        qWarning() << reader.errorString() << reader.format();
    }

    return image;
}

class DDciIconImagePrivate
{
public:
    DDciIconImagePrivate(const DDciIconImagePrivate &other)
        : imageSize(other.imageSize)
        , devicePixelRatio(other.devicePixelRatio)
        , imageScale(other.imageScale)
        , layers(other.layers)
    {

    }
    DDciIconImagePrivate(qreal imageSize, qreal devicePixelRatio, qreal imageScale,
                         const DDciIconEntry::ScalableLayer &sLayer)
        : imageSize(imageSize)
        , devicePixelRatio(devicePixelRatio)
        , imageScale(imageScale)
        , layers(sLayer.layers)
    {
    }

    void init();

    inline bool loaded() const {
        return layers.size() == readers.size();
    }

    inline void ensureLoad() {
        if (loaded())
            return;
        init();
    }

    const qreal imageSize;
    const qreal devicePixelRatio;
    const qreal imageScale;

    const QVector<DDciIconEntry::ScalableLayer::Layer> layers;

    struct ReaderData {
        ReaderData() {}
        ~ReaderData() {}
        ReaderData(const ReaderData &other)
            : index(other.index)
            , buffer(nullptr)
            , reader(nullptr)
            , pastImageDelay(other.pastImageDelay)
            , currentImage(other.currentImage)
            , currentImageIsValid(other.currentImageIsValid)
            , m_currentImageEndTime(other.m_currentImageEndTime)
        {}
        ReaderData(ReaderData &&other) {
            index = std::move(other.index);
            buffer.swap(other.buffer);
            reader.swap(other.reader);
            pastImageDelay = std::move(other.pastImageDelay);
            currentImage.swap(other.currentImage);
            currentImageIsValid = std::move(other.currentImageIsValid);
            m_currentImageEndTime = std::move(other.m_currentImageEndTime);
        }

        qsizetype index = 0;
        std::unique_ptr<QBuffer> buffer;
        std::unique_ptr<QImageReader> reader;
        int pastImageDelay = 0;
        QImage currentImage;
        bool currentImageIsValid = false;
        int m_currentImageEndTime = 0;

        inline void initCurrentImage(const DDciIconImagePrivate *d) {
            if (currentImageIsValid)
                return;
            // Ensure can get a valid QImageReader::nextImageDelay(), Because
            // using QImageReader::nextImageDelay() needs call QImageReader::read() before.
            currentImage = readImageData(*reader, d->imageScale, d->layers.at(index).isAlpha8Format);
            currentImageIsValid = true;
            m_currentImageEndTime = pastImageDelay + reader->nextImageDelay();
        }
        inline bool jumpToNextImage(DDciIconImagePrivate *d) {
            Q_ASSERT(reader->supportsAnimation());
            pastImageDelay += reader->nextImageDelay();
            ++d->pastImageCount;
            if (!reader->canRead())
                return false;
            currentImage = QImage();
            currentImageIsValid = false;
            initCurrentImage(d);
            return true;
        }
        inline int currentImageEndTime() const {
            Q_ASSERT(currentImageIsValid);
            return m_currentImageEndTime;
        }
        inline bool currentImageIsEnd(const DDciIconImagePrivate *d) const {
            return d->currentPastImageDelay > 0 && d->currentPastImageDelay >= currentImageEndTime();
        }
    };

    QVector<ReaderData *> readers;
    bool supportsAnimation = false;
    int totalMaxImageCount = 0;
    int maxLoopCount = -2;

    ReaderData *readAnimationNextData();

    ReaderData *animation = nullptr;
    int currentImageNumber = 0;
    int pastImageCount = 0;
    int currentPastImageDelay = 0;
};

class DDciIconPrivate : public QSharedData
{
public:
    DDciIconPrivate()
        : dciFile(nullptr)
    {
    }

    DDciIconPrivate(const DDciIconPrivate &other)
        : QSharedData(other)
        , dciFile(other.dciFile)
    {
    }

    ~DDciIconPrivate();

    DDciIconEntry *loadIcon(const QString &parentDir, const QString &imageDir);
    void loadIconList();
    void ensureLoaded();

    DDciIconEntry *tryMatchIcon(int iconSize, DDciIcon::Theme theme, DDciIcon::Mode mode, DDciIcon::IconMatchedFlags flags = DDciIcon::None) const;
    static void paint(QPainter *painter, const QRectF &rect, Qt::Alignment alignment,
                      const QVector<DDciIconEntry::ScalableLayer::Layer> &layers,
                      QVector<DDciIconImagePrivate::ReaderData *> *layerReaders,
                      const DDciIconPalette &palette, qreal pixmapScale);
    static void paint(QPainter *painter, const QRect &rect, qreal devicePixelRatio, Qt::Alignment alignment,
                      const DDciIconEntry *entry, const DDciIconPalette &palette, qreal pixmapScale);
    inline static bool hasPalette(const QVector<DDciIconEntry::ScalableLayer::Layer> &layers) {
        return std::any_of(layers.cbegin(), layers.cend(),
                    [](const DDciIconEntry::ScalableLayer::Layer &layer) {
            return layer.role != DDciIconPalette::NoPalette;
        });
    }
    bool hasPalette(DDciIconMatchResult result) const;
    static inline bool entryIsValid(const DDciIconEntry *entry) {
        return entry && !entry->isNull();
    }

    QSharedPointer<const DDciFile> dciFile;
    EntryNodeList icons;
};

// In Qt 6, registration of comparators, and QDebug and QDataStream streaming operators is
// done automatically. Consequently, \c QMetaType::registerEqualsComparator(),
// \c QMetaType::registerComparators(), \c qRegisterMetaTypeStreamOperators() and
// \c QMetaType::registerDebugStreamOperator() do no longer exist. Calls to those methods
// have to be removed when porting to Qt 6.
#if !defined(QT_NO_DATASTREAM) && (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
__attribute__((constructor))
static void registerMetaType()
{
    qRegisterMetaTypeStreamOperators<DDciIcon>("DDciIcon");
}
#endif

static inline bool toMode(const QStringView &name, DDciIcon::Mode *mode) {
    if (name == QLatin1String(MODE_NORMAL)) {
        *mode = DDciIcon::Normal;
        return true;
    }

    if (name == QLatin1String(MODE_DISABLED)) {
        *mode = DDciIcon::Disabled;
        return true;
    }

    if (name == QLatin1String(MODE_HOVER)) {
        *mode = DDciIcon::Hover;
        return true;
    }

    if (name == QLatin1String(MODE_PRESSED)) {
        *mode = DDciIcon::Pressed;
        return true;
    }

    return false;
}

static inline bool toTheme(const QStringView &name, DDciIcon::Theme *theme) {
    if (name == QLatin1String(THEME_LIGHT)) {
        *theme = DDciIcon::Light;
        return true;
    }

    if (name == QLatin1String(THEME_DARK)) {
        *theme = DDciIcon::Dark;
        return true;
    }

    return false;
}

// DCI 内部目录使用 '/' 作为路径分隔符
static inline QString joinPath(const QString &s1, const QString &s2) {
    return s1 + QLatin1Char('/') + s2;
}

static QImage readImageData(const QByteArray &data, const QByteArray &format, qreal pixmapScale, bool isAlpha8Format)
{
    if (data.isEmpty())
        return QImage();

    QBuffer rawDataBuffer;
    rawDataBuffer.setData(data);
    rawDataBuffer.open(QBuffer::ReadOnly);

    QImageReader reader(&rawDataBuffer);
    if (!format.isEmpty())
        reader.setFormat(format);

    return readImageData(reader, pixmapScale, isAlpha8Format);
}

static int findIconsByLowerBoundSize(const EntryNodeList &list, const int size, bool regardPaddingsAsSize)
{
    const auto compFun1 = [] (const EntryNode &n1, const EntryNode &n2) {
        return n1.iconSize < n2.iconSize;
    };
    const auto compFun2 = [] (const EntryNode &n1, const EntryNode &n2) {
        return n1.iconSize + n1.maxPaddings < n2.iconSize + n2.maxPaddings;
    };
    EntryNode target;
    target.iconSize = size;
    target.maxPaddings = 0;
    auto neighbor = std::lower_bound(list.cbegin(), list.cend(), target,
                                     regardPaddingsAsSize ? compFun2 : compFun1);

    if (neighbor != list.cend())
        return static_cast<int>(neighbor - list.constBegin());
    return -1;
}

static QRectF alignedRect(Qt::LayoutDirection direction, Qt::Alignment alignment, const QSizeF &size, const QRectF &rect)
{
    alignment = QGuiApplicationPrivate::visualAlignment(direction, alignment);
    qreal x = rect.x();
    qreal y = rect.y();
    qreal w = size.width();
    qreal h = size.height();
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += rect.size().height()/2 - h/2;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += rect.size().height() - h;
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += rect.size().width() - w;
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += rect.size().width()/2 - w/2;

    return QRectF(x, y, w, h);
}

DDciIconPrivate::~DDciIconPrivate()
{
    for (auto icon : icons)
        qDeleteAll(icon.entries);
}

// Note that QStringView is a non-owning, read-only view of a QString
// so we need to make sure that the original QString object stays alive
// for as long as we're using the QStringView.
static inline QVector<QStringView> fromQStringList(const QStringList &list)
{
    QVector<QStringView> views;
    views.reserve(list.size());
    std::copy(list.begin(), list.end(), std::back_inserter(views));

    return views;
}

DDciIconEntry *DDciIconPrivate::loadIcon(const QString &parentDir, const QString &imageDir)
{
    // Mode-Theme
    auto props = imageDir.split(QLatin1Char('.'));
    const QVector<QStringView> &iconProps = fromQStringList(props);
    if (iconProps.count() != 2) // Error file name.
        return nullptr;

    DDciIcon::Mode mode;
    DDciIcon::Theme theme;
    if (!toMode(iconProps[0], &mode))
        return nullptr;
    if (!toTheme(iconProps[1], &theme))
        return nullptr;

    DDciIconEntry *icon = new DDciIconEntry();
    icon->mode = mode;
    icon->theme = theme;
    const QString &stateDir = joinPath(parentDir, imageDir);
    for (const QString &scaleString : dciFile->list(stateDir, true)) {
        bool ok = false;
        int scale = scaleString.toInt(&ok);
        // No restrictions on icons with scale ratio greater than 3.
        if (!ok)
            continue;
        DDciIconEntry::ScalableLayer scaleIcon;
        scaleIcon.imagePixelRatio = scale;
        const QString &path = joinPath(stateDir, scaleString);
        for (const QString &layerPath : dciFile->list(path, true)) {
            props = layerPath.split(QLatin1Char('.'));
            const QVector<QStringView> &layerProps = fromQStringList(props);
            DDciIconEntry::ScalableLayer::Layer layer;
            EntryPropertyParser::doParse(&layer, layerProps);
            layer.data = dciFile->dataRef(joinPath(path, layerPath));
            // The sequence number starts with 1, convert it into index.
            scaleIcon.layers.insert(layer.prior - 1, layer);
            icon->maxPaddings = qMax(icon->maxPaddings, layer.padding);
        }
        icon->scalableLayers.append(scaleIcon);
    }

    return icon;
}

void DDciIconPrivate::loadIconList()
{
    const QStringList &listOfSize = dciFile->list(QLatin1String("/"), true);
    for (const QString &dir : listOfSize) {
        bool ok = false;
        int size = dir.toInt(&ok);
        if (!ok)
            continue;

        EntryNode node;
        node.iconSize = size;
        node.maxPaddings = 0;
        const QString &dirPath = joinPath(QLatin1String(), dir);
        for (const QString &imageDir : dciFile->list(dirPath, true)) {
            auto icon = loadIcon(dirPath, imageDir);
            if (!icon || icon->isNull())
                continue;
            icon->iconSize = size;
            node.entries << icon;
            node.maxPaddings = qMax(node.maxPaddings, icon->maxPaddings);
        }
        icons << std::move(node);
    }
}

void DDciIconPrivate::ensureLoaded()
{
    // TODO: Modified to resemble the addFile function in QIcon.
    if (!dciFile->isValid())
        return;
    loadIconList();
}

DDciIconEntry *DDciIconPrivate::tryMatchIcon(int iconSize, DDciIcon::Theme theme, DDciIcon::Mode mode, DDciIcon::IconMatchedFlags flags) const
{
    if (icons.isEmpty())
        return nullptr;

    auto neighborIndex = findIconsByLowerBoundSize(icons, iconSize, flags & DDciIcon::RegardPaddingsAsSize);
    if (neighborIndex < 0) {
        neighborIndex = static_cast<int>(icons.size() - 1);
    }

    const auto &listOfSize = icons.at(neighborIndex);

    QVector<qint8> iconWeight;
    iconWeight.resize(listOfSize.entries.size());
    for (int i = 0; i < listOfSize.entries.size(); ++i) {
        qint8 weight = 0;
        const DDciIconEntry *icon = listOfSize.entries.at(i);

        if (icon->mode == mode) {
            weight += 2;
        } else if (flags.testFlag(DDciIcon::DontFallbackMode)
                   || icon->mode != DDciIcon::Normal) {
            weight = -1;
            continue;
        }
        if (icon->theme == theme) {
            weight += 1;
        } else {
            weight = -1;
            continue;
        }

        iconWeight[i] = weight;
    }

    const auto targetIcon = std::max_element(iconWeight.constBegin(), iconWeight.constEnd());
    if (*targetIcon > 0)
        return listOfSize.entries.at(targetIcon - iconWeight.constBegin());
    return nullptr;
}

static const DDciIconEntry::ScalableLayer &findScalableLayer(const DDciIconEntry *entry, qreal devicePixelRatio)
{
    const DDciIconEntry::ScalableLayer *maxLayer = nullptr;
    const int imagePixelRatio = qCeil(devicePixelRatio);

    for (const auto &i : qAsConst(entry->scalableLayers)) {
        if (!maxLayer || i.imagePixelRatio > maxLayer->imagePixelRatio)
            maxLayer = &i;
        if (i.imagePixelRatio > imagePixelRatio)
            return i;
    }

    Q_ASSERT(maxLayer);
    return *maxLayer;
}

void DDciIconPrivate::paint(QPainter *painter, const QRectF &rect, Qt::Alignment alignment,
                            const QVector<DDciIconEntry::ScalableLayer::Layer> &layers,
                            QVector<DDciIconImagePrivate::ReaderData *> *layerReaders,
                            const DDciIconPalette &palette, qreal pixmapScale)
{
    const bool useImageReader = layerReaders && !layerReaders->isEmpty();
    Q_ASSERT(!useImageReader || layerReaders->size() == layers.size());
    for (auto layerIter = layers.begin(); layerIter != layers.end(); ++layerIter) {
        QImage layer;
        if (useImageReader) {
            const qsizetype index = layerIter - layers.begin();
            auto &reader = layerReaders->operator [](index);
            if (reader->currentImageIsValid) {
                layer = reader->currentImage;
            } else {
                layer = readImageData(*reader->reader, pixmapScale, layerIter->isAlpha8Format);
                reader->currentImage = layer;
                reader->currentImageIsValid = true;
            }
        } else {
            if (layerIter->data.isEmpty())
                continue;
            layer = readImageData(layerIter->data, layerIter->format, pixmapScale, layerIter->isAlpha8Format);
        }

        if (layer.isNull())
            continue;
        QColor fillColor;
        switch (layerIter->role) {
        case DDciIconPalette::Foreground:
            fillColor = palette.foreground();
            break;
        case DDciIconPalette::Background:
            fillColor = palette.background();
            break;
        case DDciIconPalette::HighlightForeground:
            fillColor = palette.highlightForeground();
            break;
        case DDciIconPalette::Highlight:
            fillColor = palette.highlight();
            break;
        default:
            break;
        }

        // ###(Chen Bin) Can't compose image when this image's format is Format_Alpha8.
        if (layer.format() == QImage::Format_Alpha8)
            layer = layer.convertToFormat(QImage::Format_ARGB32_Premultiplied);

        if (fillColor.isValid()) {
            QPainter render(&layer);
            fillColor = DGuiApplicationHelper::adjustColor(fillColor, layerIter->hue, layerIter->saturation,
                                                           layerIter->lightness, layerIter->red, layerIter->green,
                                                           layerIter->blue, layerIter->alpha);
            render.setCompositionMode(QPainter::CompositionMode_SourceIn);
            render.fillRect(layer.rect(), fillColor);
        }

        QRectF targetRect =alignedRect(painter->layoutDirection(), alignment, layer.size(), rect);
        if (rect.width() < targetRect.width())
            targetRect = rect;

        painter->drawImage(targetRect, layer);
    }
}

void DDciIconPrivate::paint(QPainter *painter, const QRect &rect, qreal devicePixelRatio, Qt::Alignment alignment,
                            const DDciIconEntry *entry, const DDciIconPalette &palette, qreal pixmapScale)
{
    qreal pixelRatio = devicePixelRatio;
    bool useHighDpiPixmaps =
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            true;
#else
            qApp->testAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    if (pixelRatio <= 0 && painter->device() && useHighDpiPixmaps)
        pixelRatio = painter->device()->devicePixelRatio();
    if (pixelRatio <= 0)
        pixelRatio = 1.0;

    auto scalableLayer = findScalableLayer(entry, devicePixelRatio);
    paint(painter, rect, alignment, scalableLayer.layers, nullptr, palette, pixelRatio * pixmapScale / scalableLayer.imagePixelRatio);
}

bool DDciIconPrivate::hasPalette(DDciIconMatchResult result) const
{
    auto entry = static_cast<const DDciIconEntry *>(result);
    if (!entryIsValid(entry))
        return false;
    if (entry->scalableLayers.isEmpty())
        return false;
    auto scaledLayer = entry->scalableLayers.first();
    return hasPalette(scaledLayer.layers);
}

DDciIcon::DDciIcon()
    : d(new DDciIconPrivate())
{

}

DDciIcon::DDciIcon(const DDciFile *dciFile)
    : DDciIcon()
{
    d->dciFile.reset(dciFile);
    d->ensureLoaded();
}

DDciIcon::DDciIcon(const QString &fileName)
    : DDciIcon()
{
    d->dciFile.reset(new DDciFile(fileName));
    d->ensureLoaded();
}

DDciIcon::DDciIcon(const QByteArray &data)
    : DDciIcon()
{
    d->dciFile.reset(new DDciFile(data));
    d->ensureLoaded();
}

DDciIcon::DDciIcon(const DDciIcon &other)
    : d(other.d)
{ }

DDciIcon &DDciIcon::operator=(const DDciIcon &other) noexcept
{
    d = other.d;
    return *this;
}

DDciIcon::~DDciIcon() {}

DDciIcon::DDciIcon(DDciIcon &&other) noexcept
    : d(other.d)
{ other.d = nullptr; }

DDciIcon &DDciIcon::operator=(DDciIcon &&other) noexcept
{ swap(other); return *this; }

bool DDciIcon::isNull() const
{
    return d->icons.isEmpty();
}

DDciIconMatchResult DDciIcon::matchIcon(int size, Theme theme, Mode mode, IconMatchedFlags flags) const
{
    return d->tryMatchIcon(size, theme, mode, flags);
}

int DDciIcon::actualSize(DDciIconMatchResult result) const
{
    auto entry = static_cast<const DDciIconEntry *>(result);
    if (!d->entryIsValid(entry))
        return -1;
    return entry->iconSize;
}

int DDciIcon::actualSize(int size, DDciIcon::Theme theme, DDciIcon::Mode mode) const
{
    auto entry = d->tryMatchIcon(size, theme, mode);
    return actualSize(entry);
}

QList<int> DDciIcon::availableSizes(DDciIcon::Theme theme, DDciIcon::Mode mode) const
{
    if (d->icons.isEmpty())
        return {};
    QList<int> sizes;
    std::for_each(d->icons.begin(), d->icons.end(), [theme, mode, &sizes](const EntryNode &node) {
        auto it = std::find_if(node.entries.begin(), node.entries.end(), [theme, mode](const DDciIconEntry *entry) {
            if (entry->mode == mode && entry->theme == theme)
                return true;
            return false;
        });
        if (it != node.entries.end())
            sizes.append((*it)->iconSize);
    });
    return sizes;
}

bool DDciIcon::isSupportedAttribute(DDciIconMatchResult result, IconAttibute attr) const
{
    switch (attr) {
    case HasPalette:
        return d->hasPalette(result);
    default:
        break;
    }

    return false;
}

bool DDciIcon::isSupportedAttribute(const DDciIconImage &image, IconAttibute attr)
{
    if (image.isNull())
        return false;

    switch (attr) {
    case HasPalette:
        return DDciIconPrivate::hasPalette(image.d->layers);
    default:
        break;
    }

    return false;
}

QPixmap DDciIcon::pixmap(qreal devicePixelRatio, int iconSize, DDciIcon::Theme theme, DDciIcon::Mode mode, const DDciIconPalette &palette) const
{
    auto entry = d->tryMatchIcon(iconSize, theme, mode);
    return pixmap(devicePixelRatio, iconSize, entry, palette);
}

QPixmap DDciIcon::pixmap(qreal devicePixelRatio, int iconSize, DDciIconMatchResult result, const DDciIconPalette &palette) const
{
    auto image = this->image(result, iconSize, devicePixelRatio);
    if (image.isNull())
        return QPixmap();
    return QPixmap::fromImage(image.toImage(palette));
}

void DDciIcon::paint(QPainter *painter, const QRect &rect, qreal devicePixelRatio, DDciIcon::Theme theme, DDciIcon::Mode mode,
                     Qt::Alignment alignment, const DDciIconPalette &palette) const
{
    int canvarsSize = qMax(rect.width(), rect.height());
    auto entry = d->tryMatchIcon(canvarsSize, theme, mode, RegardPaddingsAsSize);
    return paint(painter, rect, devicePixelRatio, entry, alignment, palette);
}

void DDciIcon::paint(QPainter *painter, const QRect &rect, qreal devicePixelRatio, DDciIconMatchResult result, Qt::Alignment alignment, const DDciIconPalette &palette) const
{
    auto entry = static_cast<const DDciIconEntry *>(result);
    if (!d->entryIsValid(entry))
        return;
    qreal canvarsSize = qMax(rect.width(), rect.height());
    const qreal pixmapScale = canvarsSize / (entry->iconSize + entry->maxPaddings * 2);
    d->paint(painter, rect, devicePixelRatio, alignment, entry, palette, pixmapScale);
}

DDciIconImage DDciIcon::image(DDciIconMatchResult result, int size, qreal devicePixelRatio) const
{
    auto entry = static_cast<DDciIconEntry*>(result);
    if (!d->entryIsValid(entry))
        return DDciIconImage();

    auto scalableLayer = findScalableLayer(entry, devicePixelRatio);
    int iconSize = size;
    if (iconSize <= 0)
        iconSize = entry->iconSize;
    Q_ASSERT_X((iconSize > 0), "DDciIcon::generatePixmap", "You must specify the icon size.");
    const qreal pixmapScale = qreal(iconSize) / entry->iconSize;
    const qreal imageSize = (entry->maxPaddings * 2 + entry->iconSize) * pixmapScale;
    const qreal imageScale = pixmapScale * devicePixelRatio / scalableLayer.imagePixelRatio;

    auto image = QSharedPointer<DDciIconImagePrivate>(new DDciIconImagePrivate(imageSize, devicePixelRatio, imageScale, scalableLayer));

    return DDciIconImage(image);
}

DDciIcon DDciIcon::fromTheme(const QString &name)
{
    if (QDir::isAbsolutePath(name))
        return DDciIcon(name);

    DDciIcon icon;

    QString iconName = name;
    // FIX uengine appname is empty, will cause qt_assert
    if (!QCoreApplication::applicationName().isEmpty() &&  !DSGApplication::id().isEmpty()) {
        // allow the icon theme to override the icon for a given application
        iconName.prepend(DSGApplication::id() + "/");
    }

    QString iconPath;
    QString iconThemeName =DGuiApplicationHelper::instance()->applicationTheme()->iconThemeName();
    if (auto cached = DIconTheme::cached()) {
        iconPath = cached->findDciIconFile(iconName, iconThemeName);
    } else {
        iconPath = DIconTheme::findDciIconFile(iconName, iconThemeName);
    }

    if (!iconPath.isEmpty())
        icon = DDciIcon(iconPath);

    return icon;
}

DDciIcon DDciIcon::fromTheme(const QString &name, const DDciIcon &fallback)
{
    DDciIcon icon = fromTheme(name);

    if (icon.isNull() || icon.availableSizes(Light).isEmpty() || icon.availableSizes(Dark).isEmpty())
        return fallback;

    return icon;
}

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &s, const DDciIcon &icon)
{
    if (icon.isNull())
        return s << QByteArray();
    auto dciFile = icon.d->dciFile;
    const QByteArray &data = dciFile->toData();
    s << data;
    return s;
}

QDataStream &operator>>(QDataStream &s, DDciIcon &icon)
{
    QByteArray data;
    s >> data;
    icon = DDciIcon(data);
    return s;
}

DDciIconImage::DDciIconImage(const DDciIconImage &other)
    : d(other.d)
{

}

DDciIconImage &DDciIconImage::operator=(const DDciIconImage &other) noexcept
{
    d = other.d;
    return *this;
}

DDciIconImage::~DDciIconImage()
{
    reset();
}

DDciIconImage::DDciIconImage(DDciIconImage &&other) noexcept
    : d(other.d)
{

}

DDciIconImage &DDciIconImage::operator=(DDciIconImage &&other) noexcept
{
    swap(other);
    return *this;
}

void DDciIconImage::reset()
{
    if (!d)
        return;

    qDeleteAll(d->readers);
    d->readers.clear();
    d->supportsAnimation = false;
    d->totalMaxImageCount = 0;
    d->maxLoopCount = -2;
    d->animation = nullptr;
    d->currentImageNumber = 0;
    d->pastImageCount = 0;
    d->currentPastImageDelay = 0;
}

QImage DDciIconImage::toImage(const DDciIconPalette &palette) const
{
    const int imageSize = qRound(d->imageSize * d->devicePixelRatio);
    QImage image(imageSize, imageSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    paint(&painter, image.rect(), Qt::AlignCenter, palette);

    image.setDevicePixelRatio(d->devicePixelRatio);
    return image;
}

void DDciIconImage::paint(QPainter *painter, const QRectF &rect, Qt::Alignment alignment, const DDciIconPalette &palette) const
{
    DDciIconPrivate::paint(painter, rect, alignment, d->layers, &d->readers, palette, d->imageScale);
}

bool DDciIconImage::hasPalette() const
{
    return DDciIcon::isSupportedAttribute(*this, DDciIcon::HasPalette);
}

bool DDciIconImage::supportsAnimation() const
{
    if (!d)
        return false;
    d->ensureLoad();
    return d->supportsAnimation;
}

bool DDciIconImage::atBegin() const
{
    return d && d->pastImageCount == 0;
}

bool DDciIconImage::atEnd() const
{
    return d && d->supportsAnimation && d->pastImageCount >= d->totalMaxImageCount - 1;
}

bool DDciIconImage::jumpToNextImage()
{
    d->ensureLoad();
    if (!d->animation)
        return false;

    // Jump to next frame for current animation image
    d->animation->jumpToNextImage(d.get());
    d->currentPastImageDelay = d->animation->pastImageDelay;
    d->animation = d->readAnimationNextData();

    if (d->animation) {
        ++d->currentImageNumber;

        // Clear old images if icon animation is not last frame
        for (auto &reader : d->readers) {
            if (reader->currentImageIsEnd(d.get()))
                reader->currentImage = QImage();
        }
    }

    return d->animation;
}

int DDciIconImage::loopCount() const
{
    if (!d)
        return 0;
    d->ensureLoad();
    return d->maxLoopCount;
}

int DDciIconImage::maxImageCount() const
{
    if (!d)
        return 0;

    d->ensureLoad();
    if (!d->supportsAnimation)
        return 0;

    return d->totalMaxImageCount;
}

int DDciIconImage::currentImageDuration() const
{
    if (!d)
        return -1;
    d->ensureLoad();
    if (!d->animation)
        return -1;
    return d->animation->pastImageDelay + d->animation->reader->nextImageDelay()
            - d->currentPastImageDelay;
}

int DDciIconImage::currentImageNumber() const
{
    return d ? d->currentImageNumber : -1;
}

void DDciIconImagePrivate::init()
{
    readers.reserve(layers.size());
    for (const auto &layer : qAsConst(layers)) {
        ReaderData *data = new ReaderData;
        Q_ASSERT(data);
        auto buffer = new QBuffer();
        data->buffer.reset(buffer);
        auto reader = new QImageReader();
        data->reader.reset(reader);
        readers.append(data);
        data->index = readers.size() - 1;

        buffer->setData(layer.data);
        buffer->open(QIODevice::ReadOnly);
        Q_ASSERT(buffer->isOpen());
        reader->setDevice(buffer);
        reader->setFormat(layer.format);

        if (reader->supportsAnimation()) {
            supportsAnimation = true;
            totalMaxImageCount += reader->imageCount();
            maxLoopCount = qMax(maxLoopCount, reader->loopCount());
        }
    }

    if (supportsAnimation)
        animation = readAnimationNextData();
}

DDciIconImagePrivate::ReaderData *DDciIconImagePrivate::readAnimationNextData()
{
    const ReaderData *next = nullptr;

    for (auto &reader : readers) {
        if (!reader->reader->supportsAnimation())
            continue;
        // Ensure can get a valid QImageReader::nextImageDelay(), Because
        // using QImageReader::nextImageDelay() needs call QImageReader::read() before.
        reader->initCurrentImage(this);
        if (reader->currentImageIsEnd(this) && !reader->jumpToNextImage(this))
            continue;
        if (!next || reader->currentImageEndTime() < next->currentImageEndTime())
            next = reader;
    }
    return const_cast<ReaderData*>(next);
}

#endif

DGUI_END_NAMESPACE
