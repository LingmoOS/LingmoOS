/*
    SPDX-FileCopyrightText: 2010-2016 Sune Vuorela <sune@vuorela.dk>
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_BARCODE_H
#define PRISON_BARCODE_H

#include "prison_export.h"

#include "prison.h"
#include <qglobal.h>

#include <memory>

class QByteArray;
class QColor;
class QImage;
class QSizeF;
class QString;

namespace Prison
{

class AbstractBarcodePrivate;

/**
 * A barcode generator for a fixed barcode format.
 *
 * @note This replaces Prison::createBarcode and AbstractBarcode* from KF5.
 * You can create Barcode instances directly now and specify the format in its
 * constructor.
 * Rather than checking for createBarcode returning a @c nullptr, check whether
 * the format is not Prison::Null.
 *
 * @since 6.0
 */
class PRISON_EXPORT Barcode
{
public:
    Barcode(Barcode &&);
    ~Barcode();
    Barcode &operator=(Barcode &&);

    /** Barcode format of this barcode generator. */
    Prison::BarcodeType format() const;

    /**
     * Textual content encoded in this barcode.
     * This returns an empty QString if binary content is set.
     * @see byteArrayData()
     */
    QString data() const;
    /**
     * Binary data encoded in this barcode.
     * This returns an empty QByteArray if textual content is set.
     * @see data()
     * @since 5.85
     */
    QByteArray byteArrayData() const;
    /**
     * Sets textual data to be drawn as a barcode.
     * Only use this function if your content is textual, use the QByteArray overload
     * when your content contains non-textual binary content.
     * Calling this function does not do any repaints of anything, they are
     * your own responsibility.
     * @param data textual barcode content
     */
    void setData(const QString &data);
    /**
     * Sets binary data to be drawn as a barcode.
     * Prefer the QString overload if your content is purely textual, to reduce
     * the risk of encoding issues for non-ASCII content.
     * Calling this function does not do any repaints of anything, they are
     * your own responsibility.
     * @param data binary barcode content
     * @since 5.85
     */
    void setData(const QByteArray &data);
    /**
     * Creates a image with a barcode on
     * @return QImage with a barcode on, trying to match the requested \param size
     *
     * If one of the dimensions of @param size is smaller than the matching dimension in \ref minimumSize,
     * a null QImage will be returned
     */
    QImage toImage(const QSizeF &size);

    /**
     * The minimal amount of pixels needed to represent this barcode without loss of information.
     * That is, the size of the barcode image if each line or dot is just one pixel wide.
     * On normal screens that is not enough for barcode scanners to reliably detect the barcode
     * though.
     * @see preferredSize
     */
    QSizeF minimumSize() const;

    /**
     * The recommended size for this barcode when shown on a screen.
     * This is typically significantly larger than trueMinimumSize() so that
     * barcode scanners tend to reliably detect the code. As this depends
     * on the physical resolution of the output, you have to pass the device
     * pixel ration of the output screen here.
     * @param devicePixelRatio The device pixel ratio of the screen this is shown on.
     * @see trueMinimumSize
     * @since 5.69
     */
    QSizeF preferredSize(qreal devicePixelRatio) const;

    /**
     * @return the foreground color (by default black) to be used for the barcode.
     */
    QColor foregroundColor() const;
    /**
     * @return the background color (by default white) to be used for the barcode.
     */
    QColor backgroundColor() const;
    /**
     * sets the foreground color
     * @param foregroundcolor - the new foreground color
     */
    void setForegroundColor(const QColor &foregroundcolor);
    /**
     * sets the background color
     * @param backgroundcolor - the new background color
     */
    void setBackgroundColor(const QColor &backgroundcolor);

    /** Dimensions of the barcode. */
    enum Dimensions : uint8_t {
        NoDimensions, ///< Null barcode.
        OneDimension, ///< One-dimensional barcode.
        TwoDimensions, ///< 2D matrix code.
    };

    /** Returns the amount of dimensions of the barcode. */
    Dimensions dimensions() const;

    /** Create a new barcode generator.
     *
     *  If a format is requested that is not supported by the current build
     *  due to missing/disabled optional dependencies, Barcode::format() will
     *  return Prison::Null.
     */
    static std::optional<Prison::Barcode> create(Prison::BarcodeType type);

private:
    friend class AbstractBarcodePrivate;
    explicit Barcode(std::unique_ptr<AbstractBarcodePrivate> &&d);
    std::unique_ptr<class AbstractBarcodePrivate> d;
};
}

#endif
