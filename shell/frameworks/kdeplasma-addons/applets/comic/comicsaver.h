/*
 *   SPDX-FileCopyrightText: 2008-2012 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMIC_SAVER_H
#define COMIC_SAVER_H

class ComicData;
class SavingDir;

/**
 * ComicSaver takes care of saving a comic strip to a user chosen
 * destination.
 * etc.
 */
class ComicSaver
{
public:
    explicit ComicSaver(SavingDir *savingDir);

    /**
     * Asks the user for a destination to save the specified
     * comic to. If possible writes it to that destination.
     * @param comic the comic to save
     * @return true if saving worked, false if there was a problem
     */
    bool save(const ComicData &comic);

private:
    SavingDir *mSavingDir;
};

#endif
