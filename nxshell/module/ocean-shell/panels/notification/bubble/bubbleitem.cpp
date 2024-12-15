// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bubbleitem.h"

#include <QUrl>
#include <QTimer>
#include <QImage>
#include <QDBusArgument>
#include <QTemporaryFile>
#include <QLoggingCategory>

#include <DIconTheme>

namespace notification {
Q_DECLARE_LOGGING_CATEGORY(notifyLog)
}

namespace notification {

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
    //qDebug(notifyLog) << width << height << rowStride << hasAlpha << bitsPerSample << channels;

#define SANITY_CHECK(condition) \
if (!(condition)) { \
    qWarning(notifyLog) << "Sanity check failed on" << #condition; \
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
        qWarning(notifyLog) << "Unsupported image format (hasAlpha:" << hasAlpha << "bitsPerSample:" << bitsPerSample << "channels:" << channels << ")";
        return QImage();
    }

    QImage image(width, height, format);
    ptr = pixels.data();
    end = ptr + pixels.length();
    for (int y = 0; y < height; ++y, ptr += rowStride) {
        if (ptr + channels * width > end) {
            qWarning(notifyLog) << "Image data is incomplete. y:" << y << "height:" << height;
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

    DGUI_USE_NAMESPACE;
    auto icon = DIconTheme::findQIcon(appName, DIconTheme::findQIcon("application-x-desktop"));
    return icon.name();
}


BubbleItem::BubbleItem(QObject *parent)
    : QObject(parent)
    , m_timeTip(tr("just now"))
{

}

BubbleItem::BubbleItem(const NotifyEntity &entity, QObject *parent)
    : QObject(parent)
    , m_timeTip(tr("just now"))
{
    setEntity(entity);
}

void BubbleItem::setEntity(const NotifyEntity &entity)
{
    m_entity = entity;
    updateActions();

    QVariantMap hints = entity.hints();
    if (hints.contains("urgency")) {
        m_urgency = hints.value("urgency").toInt();
    }
}

qint64 BubbleItem::id() const
{
    return m_entity.id();
}

uint BubbleItem::bubbleId() const
{
    return m_entity.bubbleId();
}

QString BubbleItem::appName() const
{
    return m_entity.appName();
}

QString BubbleItem::appIcon() const
{
    if (!m_entity.appIcon().isEmpty()) {
        return m_entity.appIcon();
    }

    return imagePathOfNotification(m_entity.hints(), m_entity.appIcon(), m_entity.appName());
}

QString BubbleItem::summary() const
{
    return m_entity.summary();
}

QString BubbleItem::body() const
{
    return enablePreview() ? m_entity.body() : tr("1 new message");
}

uint BubbleItem::replacesId() const
{
    return m_entity.replacesId();
}

bool BubbleItem::isReplace() const
{
    return m_entity.isReplace();
}

int BubbleItem::urgency() const
{
    return m_urgency;
}

QString BubbleItem::bodyImagePath() const
{
    return m_entity.bodyIcon();
}

qint64 BubbleItem::ctime() const
{
    return m_entity.cTime();
}

QString BubbleItem::defaultAction() const
{
    return m_defaultAction;
}

QVariantList BubbleItem::actions() const
{
    return m_actions;
}

void BubbleItem::updateActions()
{
    QStringList actions = m_entity.actions();
    const auto index = actions.indexOf(QLatin1String("default"));
    if (index >= 0) {
        // default Action maybe have text.
        m_defaultAction = QLatin1String("default");
        if (actions.size() % 2 == 1) {
            actions.remove(index);
        } else {
            actions.remove(index, 2);
        }
    }

    Q_ASSERT(actions.size() % 2 != 1);
    if (actions.size() % 2 == 1) {
        qWarning(notifyLog) << "Actions must be an even number except for default, The notify appName:" << m_entity.appName()
                            << ", actions:" << m_entity.actions();
        return;
    }

    QVariantList array;
    for (int i = 0; i < actions.size(); i += 2) {
        const auto id = actions[i];
        const auto text = actions[i + 1];
        QVariantMap item;
        item["id"] = id;
        item["text"] = text;
        array.append(item);
    }

    m_actions = array;
}

int BubbleItem::level() const
{
    return m_level;
}

void BubbleItem::setLevel(int level)
{
    if (m_level == level)
        return;

    m_level = level;
    emit levelChanged();
}

QString BubbleItem::timeTip() const
{
    return m_timeTip;
}

void BubbleItem::setTimeTip(const QString &timeTip)
{
    if (!timeTip.isEmpty() && timeTip != m_timeTip) {
        m_timeTip = timeTip;
        Q_EMIT timeTipChanged();
    }
}

bool BubbleItem::enablePreview() const
{
    return m_enablePreview;
}

void BubbleItem::setEnablePreview(bool enable)
{
    m_enablePreview = enable;
}

QString BubbleItem::displayText() const
{
    return m_enablePreview ? m_entity.body() : tr("1 new message");
}

}
