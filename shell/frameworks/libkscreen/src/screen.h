/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "kscreen_export.h"
#include "types.h"

#include <QObject>
#include <QSize>

namespace KScreen
{
class KSCREEN_EXPORT Screen : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(QSize currentSize READ currentSize WRITE setCurrentSize NOTIFY currentSizeChanged)
    Q_PROPERTY(QSize minSize READ minSize CONSTANT)
    Q_PROPERTY(QSize maxSize READ maxSize CONSTANT)
    Q_PROPERTY(int maxActiveOutputsCount READ maxActiveOutputsCount CONSTANT)

    explicit Screen();
    ~Screen() override;

    ScreenPtr clone() const;

    /**
     * The id of this screen.
     * @return id of this screen
     */
    int id() const;
    /**
     * The identifier of this screen.
     * @param id id of the screen
     */
    void setId(int id);

    /**
     * The current screen size in pixels.
     * @return Screen size in pixels
     */
    QSize currentSize() const;
    /**
     * Set the current screen size in pixels.
     * @param currentSize Screen size in pixels
     */
    void setCurrentSize(const QSize &currentSize);

    /**
     * The minimum screen size in pixels.
     * @return Minimum screen size in pixels
     */
    QSize minSize() const;
    /**
     * Set the minimum screen size in pixels.
     * @param minSize Minimum screen size in pixels
     */
    void setMinSize(const QSize &minSize);

    /**
     * The maximum screen size in pixels.
     * @return Maximum screen size in pixels
     */
    QSize maxSize() const;
    /**
     * Set the maximum screen size in pixels.
     * @param maxSize Maximum screen size in pixels
     */
    void setMaxSize(const QSize &maxSize);

    int maxActiveOutputsCount() const;
    void setMaxActiveOutputsCount(int maxActiveOutputsCount);

    void apply(const ScreenPtr &other);

Q_SIGNALS:
    void currentSizeChanged();

private:
    Q_DISABLE_COPY(Screen)

    class Private;
    Private *const d;

    Screen(Private *dd);
};

} // KScreen namespace
