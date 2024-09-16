// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bubblemodel.h"

#include <QDBusArgument>
#include <QTimer>
#include <QLoggingCategory>
#include <QDateTime>
#include <QImage>
#include <QIcon>
#include <QTemporaryFile>

#include <DDBusSender>
#include <QUrl>
#include <QLoggingCategory>
#include <DIconTheme>

namespace notification {

Q_DECLARE_LOGGING_CATEGORY(notificationLog)

static inline void copyLineRGB32(QRgb *dst, const char *src, int width)
{
    const char *end = src + width * 3;
    for (; src != end; ++dst, src += 3) {
        *dst = qRgb(src[0], src[1], src[2]);
    }
}

static inline void copyLineARGB32(QRgb *dst, const char *src, int width)
{
    const char *end = src + width * 4;
    for (; src != end; ++dst, src += 4) {
        *dst = qRgba(src[0], src[1], src[2], src[3]);
    }
}

static QImage decodeImageFromDBusArgument(const QDBusArgument &arg)
{
    int width, height, rowStride, hasAlpha, bitsPerSample, channels;
    QByteArray pixels;
    char *ptr;
    char *end;

    arg.beginStructure();
    arg >> width >> height >> rowStride >> hasAlpha >> bitsPerSample >> channels >> pixels;
    arg.endStructure();
    //qDebug() << width << height << rowStride << hasAlpha << bitsPerSample << channels;

#define SANITY_CHECK(condition) \
    if (!(condition)) { \
            qWarning() << "Sanity check failed on" << #condition; \
            return QImage(); \
    }

    SANITY_CHECK(width > 0);
    SANITY_CHECK(width < 2048);
    SANITY_CHECK(height > 0);
    SANITY_CHECK(height < 2048);
    SANITY_CHECK(rowStride > 0);

#undef SANITY_CHECK

    QImage::Format format = QImage::Format_Invalid;
    void (*fcn)(QRgb *, const char *, int) = nullptr;
    if (bitsPerSample == 8) {
        if (channels == 4) {
            format = QImage::Format_ARGB32;
            fcn = copyLineARGB32;
        } else if (channels == 3) {
            format = QImage::Format_RGB32;
            fcn = copyLineRGB32;
        }
    }
    if (format == QImage::Format_Invalid) {
        qWarning() << "Unsupported image format (hasAlpha:" << hasAlpha << "bitsPerSample:" << bitsPerSample << "channels:" << channels << ")";
        return QImage();
    }

    QImage image(width, height, format);
    ptr = pixels.data();
    end = ptr + pixels.length();
    for (int y = 0; y < height; ++y, ptr += rowStride) {
        if (ptr + channels * width > end) {
            qWarning() << "Image data is incomplete. y:" << y << "height:" << height;
            break;
        }
        fcn((QRgb *)image.scanLine(y), ptr, width);
    }

    return image;
}

static QImage decodeImageFromBase64(const QString &arg)
{
    if (arg.startsWith("data:image/")) {
        // iconPath is a string representing an inline image.
        QStringList strs = arg.split("base64,");
        if (strs.length() == 2) {
            QByteArray data = QByteArray::fromBase64(strs.at(1).toLatin1());
            return QImage::fromData(data);
        }
    }
    return QImage();
}

static QIcon decodeIconFromPath(const QString &arg, const QString &fallback)
{
    DGUI_USE_NAMESPACE;
    const QUrl url(arg);
    const auto iconUrl = url.isLocalFile() ? url.toLocalFile() : url.url();
    QIcon icon = DIconTheme::findQIcon(iconUrl);
    if (!icon.isNull()) {
        return icon;
    }
    return DIconTheme::findQIcon(fallback, DIconTheme::findQIcon("application-x-desktop"));
}

static QString imagePathOfNotification(const QVariantMap &hints, const QString &appIcon, const QString &appName)
{
    static const QStringList HintsOrder {
        "desktop-entry",
        "image-data",
        "image-path",
        "image_path",
        "icon_data"
    };

    QImage img;
    QString imageData(appIcon);
    for (const auto &hint : HintsOrder) {
        const auto &source = hints[hint];
        if (source.isNull())
            continue;
        if (source.canConvert<QDBusArgument>()) {
            img = decodeImageFromDBusArgument(source.value<QDBusArgument>());
            if (!img.isNull())
                break;
        }
        imageData = source.toString();
    }
    if (img.isNull()) {
        img = decodeImageFromBase64(imageData);
    }
    if (!img.isNull()) {
        QTemporaryFile file("notification_icon");
        img.save(file.fileName());
        return file.fileName();
    }
    QIcon icon = decodeIconFromPath(imageData, appName);
    if (icon.isNull()) {
        qCWarning(notificationLog) << "Can't get icon for notification, appName:" << appName;
    }
    return icon.name();
}

BubbleItem::BubbleItem(QObject *parent)
    : QObject(parent)
    , m_ctime(QString::number(QDateTime::currentMSecsSinceEpoch()))
{

}

BubbleItem::BubbleItem(const QString &text, const QString &title, const QString &iconName, QObject *parent)
    : QObject(parent)
    , m_text(text)
    , m_title(title)
    , m_iconName(iconName)
    , m_ctime(QString::number(QDateTime::currentMSecsSinceEpoch()))
{

}

QString BubbleItem::text() const
{
    return displayText();
}

QString BubbleItem::title() const
{
    return m_title;
}

QString BubbleItem::iconName() const
{
    return imagePathOfNotification(m_hints, m_iconName, m_appName);
}

QString BubbleItem::appName() const
{
    return m_appName;
}

int BubbleItem::level() const
{
    return m_level;
}

int BubbleItem::id() const
{
    return m_id;
}

int BubbleItem::replaceId() const
{
    return m_replaceId;
}

void BubbleItem::setLevel(int newLevel)
{
    if (m_level == newLevel)
        return;
    m_level = newLevel;
    emit levelChanged();
}

void BubbleItem::setParams(const QString &appName, int id, const QStringList &actions, const QVariantMap hints, int replaceId, const int timeout, const QVariantMap bubbleParams)
{
    m_appName = appName;
    m_id = id;
    m_actions = actions;
    m_hints = hints;
    m_replaceId = replaceId;
    m_timeout = timeout;
    m_extraParams = bubbleParams;
    if (m_timeout >= 0) {
        auto timer = new QTimer(this);
        timer->setSingleShot(true);
        timer->setInterval(m_timeout == 0 ? TimeOutInterval : m_timeout);
        QObject::connect(timer, &QTimer::timeout, this, &BubbleItem::timeout);
        timer->start();
    }
}

QVariantMap BubbleItem::toMap() const
{
    QVariantMap res;
    res["id"] = m_id;
    res["replaceId"] = m_replaceId;
    res["appName"] = m_appName;
    res["appIcon"] = m_iconName;
    res["summary"] = m_title;
    res["body"] = m_text;
    res["actions"] = m_actions;
    res["hints"] = m_hints;
    res["ctime"] = m_ctime;
    res["extraParams"] = m_extraParams;
    return res;
}

QString BubbleItem::defaultActionText() const
{
    const auto index = defaultActionTextIndex();
    if (index < 0)
        return QString();
    return m_actions[index];
}

QString BubbleItem::defaultActionId() const
{
    const auto index = defaultActionIdIndex();
    if (index < 0)
        return QString();
    return m_actions[index];
}

QString BubbleItem::firstActionText() const
{
    if (!hasDisplayAction())
        return QString();
    return displayActions().at(1);
}

QString BubbleItem::firstActionId() const
{
    if (!hasDisplayAction())
        return QString();
    return displayActions().at(0);
}

QStringList BubbleItem::actionTexts() const
{
    QStringList res;
    const auto tmp = displayActions();
    for (int i = 3; i < tmp.count(); i += 2)
        res << tmp[i];
    return res;
}

QStringList BubbleItem::actionIds() const
{
    QStringList res;
    const auto tmp = displayActions();
    for (int i = 2; i < tmp.count(); i += 2)
        res << tmp[i];
    return res;
}

int BubbleItem::defaultActionIdIndex() const
{
    return m_actions.indexOf("default");
}

int BubbleItem::defaultActionTextIndex() const
{
    const auto index = defaultActionTextIndex();
    if (index >= 0)
        return index + 1;
    return -1;
}

QStringList BubbleItem::displayActions() const
{
    const auto defaultIndex = defaultActionIdIndex();
    if (defaultIndex >= 0) {
        auto tmp = m_actions;
        tmp.remove(defaultIndex, 2);
        return tmp;
    }
    return m_actions;
}

QString BubbleItem::displayText() const
{
    return m_extraParams["isShowPreview"].toBool() ? m_text : tr("1 new message");
}

bool BubbleItem::hasDisplayAction() const
{
    const auto tmp = displayActions();
    return tmp.count() >= 2;
}

bool BubbleItem::hasDefaultAction() const
{
    return defaultActionIdIndex() >= 0;
}

BubbleModel::BubbleModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

BubbleModel::~BubbleModel()
{
    qDeleteAll(m_bubbles);
    m_bubbles.clear();
}

void BubbleModel::push(BubbleItem *bubble)
{
    bool more = displayRowCount() >= BubbleMaxCount;
    if (more) {
        beginRemoveRows(QModelIndex(), BubbleMaxCount - 1, BubbleMaxCount - 1);
        endRemoveRows();
    }
    beginInsertRows(QModelIndex(), 0, 0);
    m_bubbles.prepend(bubble);
    endInsertRows();

    updateLevel();
}

bool BubbleModel::isReplaceBubble(BubbleItem *bubble) const
{
    return replaceBubbleIndex(bubble) >= 0;
}

BubbleItem *BubbleModel::replaceBubble(BubbleItem *bubble)
{
    Q_ASSERT(isReplaceBubble(bubble));
    const auto replaceIndex = replaceBubbleIndex(bubble);
    const auto oldBubble = m_bubbles[replaceIndex];
    m_bubbles.replace(replaceIndex, bubble);
    Q_EMIT dataChanged(index(replaceIndex), index(replaceIndex));

    return oldBubble;
}

void BubbleModel::clear()
{
    if (m_bubbles.count() <= 0)
        return;
    beginRemoveRows(QModelIndex(), 0, m_bubbles.count() - 1);
    m_bubbles.clear();
    endResetModel();

    updateLevel();
}

QList<BubbleItem *> BubbleModel::items() const
{
    return m_bubbles;
}

void BubbleModel::remove(int index)
{
    if (index < 0 || index >= displayRowCount())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    auto bubble = m_bubbles.takeAt(index);
    bubble->deleteLater();
    endRemoveRows();

    if (m_bubbles.count() >= BubbleMaxCount) {
        beginInsertRows(QModelIndex(), displayRowCount() - 1, displayRowCount() - 1);
        endInsertRows();
        updateLevel();
    }
}

void BubbleModel::remove(BubbleItem *bubble)
{
    const auto index = m_bubbles.indexOf(bubble);
    if (index >= 0) {
        remove(index);
    }
}

int BubbleModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return displayRowCount();
}

QVariant BubbleModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    if (row >= m_bubbles.size() || !index.isValid())
        return {};

    switch (role) {
    case BubbleModel::Text:
        return m_bubbles[row]->text();
    case BubbleModel::Title:
        return m_bubbles[row]->title();
    case BubbleModel::IconName:
        return m_bubbles[row]->iconName();
    case BubbleModel::Level:
        return m_bubbles[row]->level();
    case BubbleModel::OverlayCount:
        return overlayCount();
    case BubbleModel::hasDefaultAction:
        return m_bubbles[row]->hasDefaultAction();
    case BubbleModel::hasDisplayAction:
        return m_bubbles[row]->hasDisplayAction();
    case BubbleModel::FirstActionText:
        return m_bubbles[row]->firstActionText();
    case BubbleModel::FirstActionId:
        return m_bubbles[row]->firstActionId();
    case BubbleModel::ActionTexts:
        return m_bubbles[row]->actionTexts();
    case BubbleModel::ActionIds:
        return m_bubbles[row]->actionIds();
    default:
        break;
    }
    return {};
}

QHash<int, QByteArray> BubbleModel::roleNames() const
{
    QHash<int, QByteArray> mapRoleNames;
    mapRoleNames[BubbleModel::Text] = "text";
    mapRoleNames[BubbleModel::Title] = "title";
    mapRoleNames[BubbleModel::IconName] = "iconName";
    mapRoleNames[BubbleModel::Level] = "level";
    mapRoleNames[BubbleModel::OverlayCount] = "overlayCount";
    mapRoleNames[BubbleModel::hasDefaultAction] = "hasDefaultAction";
    mapRoleNames[BubbleModel::hasDisplayAction] = "hasDisplayAction";
    mapRoleNames[BubbleModel::FirstActionText] = "firstActionText";
    mapRoleNames[BubbleModel::FirstActionId] = "firstActionId";
    mapRoleNames[BubbleModel::ActionTexts] = "actionTexts";
    mapRoleNames[BubbleModel::ActionIds] = "actionIds";
    return mapRoleNames;
}

int BubbleModel::displayRowCount() const
{
    return qMin(m_bubbles.count(), BubbleMaxCount);
}

int BubbleModel::overlayCount() const
{
    return qMin(m_bubbles.count() - displayRowCount(), OverlayMaxCount);
}

int BubbleModel::replaceBubbleIndex(BubbleItem *bubble) const
{
    if (bubble->replaceId() != NoReplaceId) {
        for (int i = 0; i < displayRowCount(); i++) {
            auto item = m_bubbles[i];
            if (item->appName() != item->appName())
                continue;

            const bool firstItem = item->replaceId() == NoReplaceId && item->id() == bubble->replaceId();
            const bool laterItem = item->replaceId() == bubble->replaceId();
            if (firstItem || laterItem) {
                return i;
            }
        }
    }
    return -1;
}

void BubbleModel::updateLevel()
{
    if (m_bubbles.isEmpty())
        return;

    for (int i = 0; i < displayRowCount(); i++) {
        auto item = m_bubbles.at(i);
        item->setLevel(i == LastBubbleMaxIndex ? 1 + overlayCount() : 1);
    }
    Q_EMIT dataChanged(index(0), index(displayRowCount() - 1), {BubbleModel::Level});
}

}
