/*
    SPDX-FileCopyrightText: 1997 Michael Roth <mroth@wirlweb.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSEPARATOR_H
#define KSEPARATOR_H

#include <QFrame>
#include <kwidgetsaddons_export.h>
#include <memory>

/**
 * @class KSeparator kseparator.h KSeparator
 *
 * Standard horizontal or vertical separator.
 *
 * \image html kseparator-horizontal.png "KSeparator Widget with horizontal orientation"
 * \image html kseparator-vertical.png "KSeparator Widget with vertical orientation"
 *
 * @author Michael Roth <mroth@wirlweb.de>
 */
class KWIDGETSADDONS_EXPORT KSeparator : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)

public:
    /**
     * Constructor.
     * @param parent parent object.
     * @param f extra QWidget flags.
     */
    explicit KSeparator(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    /**
     * Constructor.
     * @param orientation Set the orientation of the separator.
     * Possible values are Horizontal or Vertical.
     * @param parent parent object.
     * @param f extra QWidget flags.
     */
    explicit KSeparator(Qt::Orientation orientation, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    ~KSeparator() override;

    /**
     * Returns the orientation of the separator.
     * @return int Possible values Horizontal or Vertical.
     */
    Qt::Orientation orientation() const;

    /**
     * Set the orientation of the separator to @p orientation
     *
     * @param orientation Possible values are Vertical and Horizontal.
     */
    void setOrientation(Qt::Orientation orientation);

private:
    std::unique_ptr<class KSeparatorPrivate> const d;
};

#endif // KSEPARATOR_H
