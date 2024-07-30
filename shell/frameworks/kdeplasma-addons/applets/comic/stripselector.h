/*
 *   SPDX-FileCopyrightText: 2012 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRIP_SELECTOR_H
#define STRIP_SELECTOR_H

#include <QObject>

#include "comicinfo.h"
#include "engine/types.h"

class ComicData;

/**
 * Enables users to visually select a strip they want to navigate to.
 * Subclasses implement different Selectors for the different comic
 * types.
 * @note use the StripSelectorFactory to retrieve an appropriate
 * StripSelector
 */
class StripSelector : public QObject
{
    Q_OBJECT

public:
    ~StripSelector() override;

    /**
     * Select a strip depending on the subclass
     * @param currentStrip the currently active strip
     * @note StripSelector takes care to delete itself
     */
    virtual void select(const ComicData &currentStrip) = 0;

Q_SIGNALS:
    /**
     * @param strip the selected strip, can be empty
     *
     */
    void stripChosen(const QString &strip);

protected:
    explicit StripSelector(QObject *parent = nullptr);
};

/**
 * Class to retrieve the correct StripSelector depending on the
 * specified IdentifierType
 */
class StripSelectorFactory
{
public:
    static StripSelector *create(IdentifierType type);
};

#endif
