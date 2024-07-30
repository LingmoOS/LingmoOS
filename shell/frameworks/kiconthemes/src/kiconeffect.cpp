/*

    This file is part of the KDE project, module kdecore.
    SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2007 Daniel M. Duley <daniel.duley@verizon.net>

    with minor additions and based on ideas from
    SPDX-FileContributor: Torsten Rahn <torsten@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kiconeffect.h"
#include "debug.h"
#include "kiconloader.h"

#include <KColorScheme>

#include <QDebug>
#include <QPalette>
#include <QSysInfo>

#include <qplatformdefs.h>

#include <math.h>

class KIconEffectPrivate
{
public:
    // http://en.cppreference.com/w/cpp/language/zero_initialization
    KIconEffectPrivate()
        : effect{{}}
        , value{{}}
        , trans{{}}
        , key{{}}
    {
    }

public:
    int effect[KIconLoader::LastGroup][KIconLoader::LastState];
    float value[KIconLoader::LastGroup][KIconLoader::LastState];
    bool trans[KIconLoader::LastGroup][KIconLoader::LastState];
    QString key[KIconLoader::LastGroup][KIconLoader::LastState];
};

#if KICONTHEMES_BUILD_DEPRECATED_SINCE(6, 5)
KIconEffect::KIconEffect()
    : d(new KIconEffectPrivate)
{
    init();
}
#endif

KIconEffect::~KIconEffect() = default;

#if KICONTHEMES_BUILD_DEPRECATED_SINCE(6, 5)
void KIconEffect::init()
{
    int i;
    // FIXME: this really should be using KIconLoader::metaObject() to guarantee synchronization
    // performance wise it's also practically guaranteed to be faster
    QStringList groups;
    groups += QStringLiteral("Desktop");
    groups += QStringLiteral("Toolbar");
    groups += QStringLiteral("MainToolbar");
    groups += QStringLiteral("Small");
    groups += QStringLiteral("Panel");
    groups += QStringLiteral("Dialog");

    QStringList states;
    states += QStringLiteral("Default");
    states += QStringLiteral("Active");
    states += QStringLiteral("Disabled");

    QStringList::ConstIterator it;

    for (it = groups.constBegin(), i = 0; it != groups.constEnd(); ++it, ++i) {
        d->effect[i][KIconLoader::DefaultState] = NoEffect;
        d->effect[i][KIconLoader::ActiveState] = ((i == KIconLoader::Desktop) || (KIconLoader::Panel == 4)) ? ToGamma : NoEffect;
        d->effect[i][KIconLoader::DisabledState] = ToGray;

        d->trans[i][KIconLoader::DefaultState] = false;
        d->trans[i][KIconLoader::ActiveState] = false;
        d->trans[i][KIconLoader::DisabledState] = true;
        d->value[i][KIconLoader::DefaultState] = 1.0;
        d->value[i][KIconLoader::ActiveState] = ((i == KIconLoader::Desktop) || (KIconLoader::Panel == 4)) ? 0.7 : 1.0;
        d->value[i][KIconLoader::DisabledState] = 1.0;
    }
}
#endif

#if KICONTHEMES_BUILD_DEPRECATED_SINCE(6, 5)
bool KIconEffect::hasEffect(int group, int state) const
{
    if (group < 0 || group >= KIconLoader::LastGroup //
        || state < 0 || state >= KIconLoader::LastState) {
        return false;
    }

    return d->effect[group][state] != NoEffect;
}
#endif

#if KICONTHEMES_BUILD_DEPRECATED_SINCE(6, 5)
QString KIconEffect::fingerprint(int group, int state) const
{
    if (group < 0 || group >= KIconLoader::LastGroup //
        || state < 0 || state >= KIconLoader::LastState) {
        return QString();
    }

    QString cached = d->key[group][state];
    if (cached.isEmpty()) {
        QString tmp;
        cached = tmp.setNum(d->effect[group][state]);
        cached += QLatin1Char(':');
        cached += tmp.setNum(d->value[group][state]);
        cached += QLatin1Char(':');
        cached += d->trans[group][state] ? QLatin1String("trans") : QLatin1String("notrans");

        d->key[group][state] = cached;
    }

    return cached;
}
#endif

#if KICONTHEMES_BUILD_DEPRECATED_SINCE(6, 5)
QImage KIconEffect::apply(const QImage &image, int group, int state) const
{
    if (state >= KIconLoader::LastState) {
        qCWarning(KICONTHEMES) << "Invalid icon state:" << state << ", should be one of KIconLoader::States";
        return image;
    }
    if (group >= KIconLoader::LastGroup) {
        qCWarning(KICONTHEMES) << "Invalid icon group:" << group << ", should be one of KIconLoader::Group";
        return image;
    }
    return apply(image, d->effect[group][state], d->value[group][state], QColor(), QColor(), d->trans[group][state]);
}
#endif

#if KICONTHEMES_BUILD_DEPRECATED_SINCE(6, 5)
QImage KIconEffect::apply(const QImage &image, int effect, float value, const QColor &col, bool trans) const
{
    return apply(image, effect, value, col, KColorScheme(QPalette::Active, KColorScheme::View).background().color(), trans);
}
#endif

#if KICONTHEMES_BUILD_DEPRECATED_SINCE(6, 5)
QImage KIconEffect::apply(const QImage &img, int effect, float value, const QColor &col, const QColor &col2, bool trans) const
{
    QImage image = img;
    if (effect >= LastEffect) {
        qCWarning(KICONTHEMES) << "Invalid icon effect:" << effect << ", should be one of KIconLoader::Effects";
        return image;
    }
    if (value > 1.0) {
        value = 1.0;
    } else if (value < 0.0) {
        value = 0.0;
    }
    switch (effect) {
    case ToGray:
        toGray(image, value);
        break;
    case DeSaturate:
        deSaturate(image, value);
        break;
    case Colorize:
        colorize(image, col, value);
        break;
    case ToGamma:
        toGamma(image, value);
        break;
    case ToMonochrome:
        toMonochrome(image, col, col2, value);
        break;
    }
    if (trans == true) {
        semiTransparent(image);
    }
    return image;
}
#endif

#if KICONTHEMES_BUILD_DEPRECATED_SINCE(6, 5)
QPixmap KIconEffect::apply(const QPixmap &pixmap, int group, int state) const
{
    if (state >= KIconLoader::LastState) {
        qCWarning(KICONTHEMES) << "Invalid icon state:" << state << ", should be one of KIconLoader::States";
        return pixmap;
    }
    if (group >= KIconLoader::LastGroup) {
        qCWarning(KICONTHEMES) << "Invalid icon group:" << group << ", should be one of KIconLoader::Group";
        return pixmap;
    }
    return apply(pixmap, d->effect[group][state], d->value[group][state], QColor(), QColor(), d->trans[group][state]);
}
#endif

#if KICONTHEMES_BUILD_DEPRECATED_SINCE(6, 5)
QPixmap KIconEffect::apply(const QPixmap &pixmap, int effect, float value, const QColor &col, bool trans) const
{
    return apply(pixmap, effect, value, col, KColorScheme(QPalette::Active, KColorScheme::View).background().color(), trans);
}
#endif

#if KICONTHEMES_BUILD_DEPRECATED_SINCE(6, 5)
QPixmap KIconEffect::apply(const QPixmap &pixmap, int effect, float value, const QColor &col, const QColor &col2, bool trans) const
{
    QPixmap result;

    if (effect >= LastEffect) {
        qCWarning(KICONTHEMES) << "Invalid icon effect:" << effect << ", should be one of KIconLoader::Effects";
        return result;
    }

    if ((trans == true) && (effect == NoEffect)) {
        result = pixmap;
        semiTransparent(result);
    } else if (effect != NoEffect) {
        QImage tmpImg = pixmap.toImage();
        tmpImg = apply(tmpImg, effect, value, col, col2, trans);
        result = QPixmap::fromImage(std::move(tmpImg));
    } else {
        result = pixmap;
    }

    return result;
}
#endif

struct KIEImgEdit {
    QImage &img;
    QList<QRgb> colors;
    unsigned int *data;
    unsigned int pixels;

    KIEImgEdit(QImage &_img)
        : img(_img)
    {
        if (img.depth() > 8) {
            // Code using data and pixels assumes that the pixels are stored
            // in 32bit values and that the image is not premultiplied
            if ((img.format() != QImage::Format_ARGB32) //
                && (img.format() != QImage::Format_RGB32)) {
                img.convertTo(QImage::Format_ARGB32);
            }
            data = (unsigned int *)img.bits();
            pixels = img.width() * img.height();
        } else {
            pixels = img.colorCount();
            colors = img.colorTable();
            data = (unsigned int *)colors.data();
        }
    }

    ~KIEImgEdit()
    {
        if (img.depth() <= 8) {
            img.setColorTable(colors);
        }
    }

    KIEImgEdit(const KIEImgEdit &) = delete;
    KIEImgEdit &operator=(const KIEImgEdit &) = delete;
};

// Taken from KImageEffect. We don't want to link kdecore to kdeui! As long
// as this code is not too big, it doesn't seem much of a problem to me.

void KIconEffect::toGray(QImage &img, float value)
{
    if (value == 0.0) {
        return;
    }

    KIEImgEdit ii(img);
    QRgb *data = ii.data;
    QRgb *end = data + ii.pixels;

    unsigned char gray;
    if (value == 1.0) {
        while (data != end) {
            gray = qGray(*data);
            *data = qRgba(gray, gray, gray, qAlpha(*data));
            ++data;
        }
    } else {
        unsigned char val = (unsigned char)(255.0 * value);
        while (data != end) {
            gray = qGray(*data);
            *data = qRgba((val * gray + (0xFF - val) * qRed(*data)) >> 8,
                          (val * gray + (0xFF - val) * qGreen(*data)) >> 8,
                          (val * gray + (0xFF - val) * qBlue(*data)) >> 8,
                          qAlpha(*data));
            ++data;
        }
    }
}

void KIconEffect::colorize(QImage &img, const QColor &col, float value)
{
    if (value == 0.0) {
        return;
    }

    KIEImgEdit ii(img);
    QRgb *data = ii.data;
    QRgb *end = data + ii.pixels;

    float rcol = col.red();
    float gcol = col.green();
    float bcol = col.blue();
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char gray;
    unsigned char val = (unsigned char)(255.0 * value);
    while (data != end) {
        gray = qGray(*data);
        if (gray < 128) {
            red = static_cast<unsigned char>(rcol / 128 * gray);
            green = static_cast<unsigned char>(gcol / 128 * gray);
            blue = static_cast<unsigned char>(bcol / 128 * gray);
        } else if (gray > 128) {
            red = static_cast<unsigned char>((gray - 128) * (2 - rcol / 128) + rcol - 1);
            green = static_cast<unsigned char>((gray - 128) * (2 - gcol / 128) + gcol - 1);
            blue = static_cast<unsigned char>((gray - 128) * (2 - bcol / 128) + bcol - 1);
        } else {
            red = static_cast<unsigned char>(rcol);
            green = static_cast<unsigned char>(gcol);
            blue = static_cast<unsigned char>(bcol);
        }

        *data = qRgba((val * red + (0xFF - val) * qRed(*data)) >> 8,
                      (val * green + (0xFF - val) * qGreen(*data)) >> 8,
                      (val * blue + (0xFF - val) * qBlue(*data)) >> 8,
                      qAlpha(*data));
        ++data;
    }
}

void KIconEffect::toMonochrome(QImage &img, const QColor &black, const QColor &white, float value)
{
    if (value == 0.0) {
        return;
    }

    KIEImgEdit ii(img);
    QRgb *data = ii.data;
    QRgb *end = data + ii.pixels;

    // Step 1: determine the average brightness
    double values = 0.0;
    double sum = 0.0;
    bool grayscale = true;
    while (data != end) {
        sum += qGray(*data) * qAlpha(*data) + 255 * (255 - qAlpha(*data));
        values += 255;
        if ((qRed(*data) != qGreen(*data)) || (qGreen(*data) != qBlue(*data))) {
            grayscale = false;
        }
        ++data;
    }
    double medium = sum / values;

    // Step 2: Modify the image
    unsigned char val = (unsigned char)(255.0 * value);
    int rw = white.red();
    int gw = white.green();
    int bw = white.blue();
    int rb = black.red();
    int gb = black.green();
    int bb = black.blue();
    data = ii.data;

    if (grayscale) {
        while (data != end) {
            if (qRed(*data) <= medium) {
                *data = qRgba((val * rb + (0xFF - val) * qRed(*data)) >> 8,
                              (val * gb + (0xFF - val) * qGreen(*data)) >> 8,
                              (val * bb + (0xFF - val) * qBlue(*data)) >> 8,
                              qAlpha(*data));
            } else {
                *data = qRgba((val * rw + (0xFF - val) * qRed(*data)) >> 8,
                              (val * gw + (0xFF - val) * qGreen(*data)) >> 8,
                              (val * bw + (0xFF - val) * qBlue(*data)) >> 8,
                              qAlpha(*data));
            }
            ++data;
        }
    } else {
        while (data != end) {
            if (qGray(*data) <= medium) {
                *data = qRgba((val * rb + (0xFF - val) * qRed(*data)) >> 8,
                              (val * gb + (0xFF - val) * qGreen(*data)) >> 8,
                              (val * bb + (0xFF - val) * qBlue(*data)) >> 8,
                              qAlpha(*data));
            } else {
                *data = qRgba((val * rw + (0xFF - val) * qRed(*data)) >> 8,
                              (val * gw + (0xFF - val) * qGreen(*data)) >> 8,
                              (val * bw + (0xFF - val) * qBlue(*data)) >> 8,
                              qAlpha(*data));
            }
            ++data;
        }
    }
}

void KIconEffect::deSaturate(QImage &img, float value)
{
    if (value == 0.0) {
        return;
    }

    KIEImgEdit ii(img);
    QRgb *data = ii.data;
    QRgb *end = data + ii.pixels;

    QColor color;
    int h;
    int s;
    int v;
    while (data != end) {
        color.setRgb(*data);
        color.getHsv(&h, &s, &v);
        color.setHsv(h, (int)(s * (1.0 - value) + 0.5), v);
        *data = qRgba(color.red(), color.green(), color.blue(), qAlpha(*data));
        ++data;
    }
}

void KIconEffect::toGamma(QImage &img, float value)
{
    KIEImgEdit ii(img);
    QRgb *data = ii.data;
    QRgb *end = data + ii.pixels;

    float gamma = 1 / (2 * value + 0.5);
    while (data != end) {
        *data = qRgba(static_cast<unsigned char>(pow(static_cast<float>(qRed(*data)) / 255, gamma) * 255),
                      static_cast<unsigned char>(pow(static_cast<float>(qGreen(*data)) / 255, gamma) * 255),
                      static_cast<unsigned char>(pow(static_cast<float>(qBlue(*data)) / 255, gamma) * 255),
                      qAlpha(*data));
        ++data;
    }
}

void KIconEffect::semiTransparent(QImage &img)
{
    if (img.depth() == 32) {
        if (img.format() == QImage::Format_ARGB32_Premultiplied) {
            img.convertTo(QImage::Format_ARGB32);
        }
        int width = img.width();
        int height = img.height();

        unsigned char *line;
        for (int y = 0; y < height; ++y) {
            if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
                line = img.scanLine(y);
            } else {
                line = img.scanLine(y) + 3;
            }
            for (int x = 0; x < width; ++x) {
                *line >>= 1;
                line += 4;
            }
        }
    } else if (img.depth() == 8) {
        // not running on 8 bit, we can safely install a new colorTable
        QList<QRgb> colorTable = img.colorTable();
        for (int i = 0; i < colorTable.size(); ++i) {
            colorTable[i] = (colorTable[i] & 0x00ffffff) | ((colorTable[i] & 0xfe000000) >> 1);
        }
        img.setColorTable(colorTable);
    } else {
        // Insert transparent pixel into the clut.
        int transColor = -1;

        // search for a color that is already transparent
        for (int x = 0; x < img.colorCount(); ++x) {
            // try to find already transparent pixel
            if (qAlpha(img.color(x)) < 127) {
                transColor = x;
                break;
            }
        }

        // FIXME: image must have transparency
        if (transColor < 0 || transColor >= img.colorCount()) {
            return;
        }

        img.setColor(transColor, 0);
        unsigned char *line;
        if (img.depth() == 8) {
            for (int y = 0; y < img.height(); ++y) {
                line = img.scanLine(y);
                for (int x = (y % 2); x < img.width(); x += 2) {
                    line[x] = transColor;
                }
            }
        } else {
            const bool setOn = (transColor != 0);
            if (img.format() == QImage::Format_MonoLSB) {
                for (int y = 0; y < img.height(); ++y) {
                    line = img.scanLine(y);
                    for (int x = (y % 2); x < img.width(); x += 2) {
                        if (!setOn) {
                            *(line + (x >> 3)) &= ~(1 << (x & 7));
                        } else {
                            *(line + (x >> 3)) |= (1 << (x & 7));
                        }
                    }
                }
            } else {
                for (int y = 0; y < img.height(); ++y) {
                    line = img.scanLine(y);
                    for (int x = (y % 2); x < img.width(); x += 2) {
                        if (!setOn) {
                            *(line + (x >> 3)) &= ~(1 << (7 - (x & 7)));
                        } else {
                            *(line + (x >> 3)) |= (1 << (7 - (x & 7)));
                        }
                    }
                }
            }
        }
    }
}

void KIconEffect::semiTransparent(QPixmap &pix)
{
    QImage img = pix.toImage();
    semiTransparent(img);
    pix = QPixmap::fromImage(img);
}

QImage KIconEffect::doublePixels(const QImage &src) const
{
    int w = src.width();
    int h = src.height();

    QImage dst(w * 2, h * 2, src.format());

    if (src.depth() == 1) {
        qWarning() << "image depth 1 not supported";
        return QImage();
    }

    int x;
    int y;
    if (src.depth() == 32) {
        QRgb *l1;
        QRgb *l2;
        for (y = 0; y < h; ++y) {
            l1 = (QRgb *)src.scanLine(y);
            l2 = (QRgb *)dst.scanLine(y * 2);
            for (x = 0; x < w; ++x) {
                l2[x * 2] = l2[x * 2 + 1] = l1[x];
            }
            memcpy(dst.scanLine(y * 2 + 1), l2, dst.bytesPerLine());
        }
    } else {
        for (x = 0; x < src.colorCount(); ++x) {
            dst.setColor(x, src.color(x));
        }

        const unsigned char *l1;
        unsigned char *l2;
        for (y = 0; y < h; ++y) {
            l1 = src.scanLine(y);
            l2 = dst.scanLine(y * 2);
            for (x = 0; x < w; ++x) {
                l2[x * 2] = l1[x];
                l2[x * 2 + 1] = l1[x];
            }
            memcpy(dst.scanLine(y * 2 + 1), l2, dst.bytesPerLine());
        }
    }
    return dst;
}

void KIconEffect::overlay(QImage &src, QImage &overlay)
{
    if (src.depth() != overlay.depth()) {
        qWarning() << "Image depth src (" << src.depth() << ") != overlay "
                   << "(" << overlay.depth() << ")!";
        return;
    }
    if (src.size() != overlay.size()) {
        qWarning() << "Image size src != overlay";
        return;
    }
    if (src.format() == QImage::Format_ARGB32_Premultiplied) {
        src.convertTo(QImage::Format_ARGB32);
    }

    if (overlay.format() == QImage::Format_RGB32) {
        qWarning() << "Overlay doesn't have alpha buffer!";
        return;
    } else if (overlay.format() == QImage::Format_ARGB32_Premultiplied) {
        overlay.convertTo(QImage::Format_ARGB32);
    }

    int i;
    int j;

    // We don't do 1 bpp

    if (src.depth() == 1) {
        qWarning() << "1bpp not supported!";
        return;
    }

    // Overlay at 8 bpp doesn't use alpha blending

    if (src.depth() == 8) {
        if (src.colorCount() + overlay.colorCount() > 255) {
            qWarning() << "Too many colors in src + overlay!";
            return;
        }

        // Find transparent pixel in overlay
        int trans;
        for (trans = 0; trans < overlay.colorCount(); trans++) {
            if (qAlpha(overlay.color(trans)) == 0) {
                qWarning() << "transparent pixel found at " << trans;
                break;
            }
        }
        if (trans == overlay.colorCount()) {
            qWarning() << "transparent pixel not found!";
            return;
        }

        // Merge color tables
        int nc = src.colorCount();
        src.setColorCount(nc + overlay.colorCount());
        for (i = 0; i < overlay.colorCount(); ++i) {
            src.setColor(nc + i, overlay.color(i));
        }

        // Overwrite nontransparent pixels.
        unsigned char *oline;
        unsigned char *sline;
        for (i = 0; i < src.height(); ++i) {
            oline = overlay.scanLine(i);
            sline = src.scanLine(i);
            for (j = 0; j < src.width(); ++j) {
                if (oline[j] != trans) {
                    sline[j] = oline[j] + nc;
                }
            }
        }
    }

    // Overlay at 32 bpp does use alpha blending

    if (src.depth() == 32) {
        QRgb *oline;
        QRgb *sline;
        int r1;
        int g1;
        int b1;
        int a1;
        int r2;
        int g2;
        int b2;
        int a2;

        for (i = 0; i < src.height(); ++i) {
            oline = (QRgb *)overlay.scanLine(i);
            sline = (QRgb *)src.scanLine(i);

            for (j = 0; j < src.width(); ++j) {
                r1 = qRed(oline[j]);
                g1 = qGreen(oline[j]);
                b1 = qBlue(oline[j]);
                a1 = qAlpha(oline[j]);

                r2 = qRed(sline[j]);
                g2 = qGreen(sline[j]);
                b2 = qBlue(sline[j]);
                a2 = qAlpha(sline[j]);

                r2 = (a1 * r1 + (0xff - a1) * r2) >> 8;
                g2 = (a1 * g1 + (0xff - a1) * g2) >> 8;
                b2 = (a1 * b1 + (0xff - a1) * b2) >> 8;
                a2 = qMax(a1, a2);

                sline[j] = qRgba(r2, g2, b2, a2);
            }
        }
    }
}

void KIconEffect::toDisabled(QImage &image)
{
    KIconEffect::toGray(image, 1);
    KIconEffect::semiTransparent(image);
}

void KIconEffect::toDisabled(QPixmap &pixmap)
{
    QImage img = pixmap.toImage();
    toDisabled(img);
    pixmap = QPixmap::fromImage(img);
}

void KIconEffect::toActive(QImage &image)
{
    KIconEffect::toGamma(image, 0.7);
}

void KIconEffect::toActive(QPixmap &pixmap)
{
    QImage img = pixmap.toImage();
    KIconEffect::toGamma(img, 0.7);
    pixmap = QPixmap::fromImage(img);
}
