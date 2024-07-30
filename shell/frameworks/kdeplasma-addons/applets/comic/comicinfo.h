/*
 *   SPDX-FileCopyrightText: 2011-2012 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMICINFO_H
#define COMICINFO_H

class KConfigGroup;
class QString;

/**
 * Provides access (read/write) to the directory that should be used
 * whenever the user is presented with a file selection dialog.
 */
class SavingDir
{
public:
    /**
     * @param config the config that should be used to retrieve
     * the saving directory and to store it to in case of changes
     */
    explicit SavingDir(const KConfigGroup &config);

    ~SavingDir();

    /**
     * @return the directory to be displayed to the user
     */
    QString getDir() const;

    /**
     * Set the directory that should be displayed to the user first
     * when choosing a destination. Automatically writes the directory
     * to the config, if one was specified in init.
     * @param dir the directory to display the user first
     * @see init
     */
    void setDir(const QString &dir);

private:
    class SavingDirPrivate;
    SavingDirPrivate *d;
};

#endif
