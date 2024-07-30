/*
    SPDX-FileCopyrightText: 2008 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2009 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _K_PIXMAPSEQUENCE_H_
#define _K_PIXMAPSEQUENCE_H_

#include <QSharedDataPointer>
#include <QSize>

#include <kwidgetsaddons_export.h>

class QPixmap;

/**
 * \class KPixmapSequence kpixmapsequence.h KPixmapSequence
 *
 * \brief Loads and gives access to the frames of a typical multi-row pixmap
 * as often used for spinners.
 *
 * KPixmapSequence is implicitly shared. Copying is fast.
 *
 * \author Aurélien Gâteau <agateau@kde.org>
 * \author Sebastian Trueg <trueg@kde.org>
 *
 * \since 4.4
 */
class KWIDGETSADDONS_EXPORT KPixmapSequence
{
public:
    /**
     * Create an empty sequence
     */
    KPixmapSequence();

    /**
     * Copy constructor
     */
    KPixmapSequence(const KPixmapSequence &other);

    /**
     * Create a sequence from a pixmap.
     *
     * \param pixmap Pixmap to load
     * \param frameSize The size of the frames to load. The width of the file has to be
     * a multiple of the frame width; the same is true for the height. If an invalid
     * size is specified the file is considered to be one column of square frames.
     */
    explicit KPixmapSequence(const QPixmap &pixmap, const QSize &frameSize = QSize());

    /**
     * Create a sequence from an icon name.
     *
     * \param fullPath The full path of the icon
     * \param size The icon/frame size
     */
    KPixmapSequence(const QString &fullPath, int size);

    /**
     * Destructor
     */
    ~KPixmapSequence();

    /**
     * Create a copy of \p other. The data is implicitly shared.
     */
    KPixmapSequence &operator=(const KPixmapSequence &other);

    /**
     * \return \p true if a sequence was loaded successfully.
     *
     * \sa isEmpty
     */
    bool isValid() const;

    /**
     * \return \p true if no sequence was loaded successfully.
     *
     * \sa isValid
     */
    bool isEmpty() const;

    /**
     * \return The size of an individual frame in the sequence.
     */
    QSize frameSize() const;

    /**
     * The number of frames in this sequence.
     */
    int frameCount() const;

    /**
     * Retrieve the frame at \p index.
     *
     * \param index The index of the frame in question starting at 0.
     */
    QPixmap frameAt(int index) const;

private:
    QSharedDataPointer<class KPixmapSequencePrivate> d;
};

#endif
