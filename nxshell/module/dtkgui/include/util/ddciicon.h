// SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include "ddciiconpalette.h"

#include <dtkgui_global.h>

#include <QPixmap>
#include <QSharedPointer>

DCORE_BEGIN_NAMESPACE
class DDciFile;
DCORE_END_NAMESPACE

DGUI_BEGIN_NAMESPACE

typedef void* DDciIconMatchResult;

class DDciIconImagePrivate;
class DDciIconImage {
    friend class DDciIcon;
public:
    DDciIconImage() = default;
    DDciIconImage(const DDciIconImage &other);
    DDciIconImage &operator=(const DDciIconImage &other) noexcept;
    DDciIconImage(DDciIconImage && other) noexcept;
    DDciIconImage &operator=(DDciIconImage &&other) noexcept;
    void swap(DDciIconImage &other) noexcept { d.swap(other.d); }
    ~DDciIconImage();

    bool operator==(const DDciIconImage &other) const {
        return d == other.d;
    }
    bool operator!=(const DDciIconImage &other) const {
        return d != other.d;
    }

    inline bool isNull() const {
        return !d;
    }
    void reset();

    QImage toImage(const DDciIconPalette &palette = DDciIconPalette()) const;
    void paint(QPainter *painter, const QRectF &rect, Qt::Alignment alignment = Qt::AlignCenter,
               const DDciIconPalette &palette = DDciIconPalette()) const;

    bool hasPalette() const;
    bool supportsAnimation() const;
    bool atBegin() const;
    bool atEnd() const;
    bool jumpToNextImage();
    int loopCount() const;
    int maxImageCount() const;
    int currentImageDuration() const;
    int currentImageNumber() const;

protected:
    DDciIconImage(const QSharedPointer<DDciIconImagePrivate> &dd)
        : d(dd) {}

    QSharedPointer<DDciIconImagePrivate> d;
};

class DDciIconPrivate;
class DDciIcon
{
public:
    enum Mode {
        Normal = 0,
        Disabled = 1,
        Hover = 2,
        Pressed = 3
    };
    enum Theme {
        Light = 0,
        Dark = 1
    };
    enum IconAttibute {
        HasPalette = 0x001
    };
    enum IconMatchedFlag {
        None = 0,
        DontFallbackMode = 0x01,
        RegardPaddingsAsSize = 0x02
    };
    Q_DECLARE_FLAGS(IconMatchedFlags, IconMatchedFlag)
    Q_FLAGS(IconMatchedFlags);

    DDciIcon();
    explicit DDciIcon(const DCORE_NAMESPACE::DDciFile *dciFile);
    explicit DDciIcon(const QString &fileName);
    explicit DDciIcon(const QByteArray &data);
    DDciIcon(const DDciIcon &other);
    DDciIcon &operator=(const DDciIcon &other) noexcept;
    ~DDciIcon();
    DDciIcon(DDciIcon && other) noexcept;
    DDciIcon &operator=(DDciIcon &&other) noexcept;
    void swap(DDciIcon &other) noexcept { d.swap(other.d); }

    bool isNull() const;
    DDciIconMatchResult matchIcon(int size, Theme theme, Mode mode, IconMatchedFlags flags = None) const;

    int actualSize(DDciIconMatchResult result) const;
    int actualSize(int size, Theme theme, Mode mode = Normal) const;

    QList<int> availableSizes(Theme theme, Mode mode = Normal) const;
    bool isSupportedAttribute(DDciIconMatchResult result, IconAttibute attr) const;
    static bool isSupportedAttribute(const DDciIconImage &image, IconAttibute attr);

    QPixmap pixmap(qreal devicePixelRatio, int iconSize, Theme theme, Mode mode = Normal,
                   const DDciIconPalette &palette = DDciIconPalette()) const;
    QPixmap pixmap(qreal devicePixelRatio, int iconSize, DDciIconMatchResult result,
                   const DDciIconPalette &palette = DDciIconPalette()) const;

    void paint(QPainter *painter, const QRect &rect, qreal devicePixelRatio, Theme theme, Mode mode = Normal,
               Qt::Alignment alignment = Qt::AlignCenter, const DDciIconPalette &palette = DDciIconPalette()) const;
    void paint(QPainter *painter, const QRect &rect, qreal devicePixelRatio, DDciIconMatchResult result,
               Qt::Alignment alignment = Qt::AlignCenter, const DDciIconPalette &palette = DDciIconPalette()) const;

    DDciIconImage image(DDciIconMatchResult result, int size, qreal devicePixelRatio) const;

    static DDciIcon fromTheme(const QString &name);
    static DDciIcon fromTheme(const QString &name, const DDciIcon &fallback);

    // TODO: Should be compatible with QIcon
private:
    QSharedDataPointer<DDciIconPrivate> d;
#ifndef QT_NO_DATASTREAM
    friend QDataStream &operator<<(QDataStream &, const DDciIcon &);
    friend QDataStream &operator>>(QDataStream &, DDciIcon &);
#endif
};

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &, const DDciIcon &);
QDataStream &operator>>(QDataStream &, DDciIcon &);
#endif

DGUI_END_NAMESPACE
Q_DECLARE_METATYPE(DTK_GUI_NAMESPACE::DDciIcon);
