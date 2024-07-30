/*
 *   SPDX-FileCopyrightText: 2012 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "comicinfo.h"

#include <KConfigGroup>
// Qt
#include <QDir>
#include <QStandardPaths>

class SavingDir::SavingDirPrivate
{
public:
    SavingDirPrivate(const KConfigGroup &cfg);

    void init();

    QString getDir() const;
    void setDir(const QString &dir);

private:
    void load();
    void save();
    bool isValid();

private:
    KConfigGroup mCfg;
    QString mDir;
};

SavingDir::SavingDirPrivate::SavingDirPrivate(const KConfigGroup &cfg)
    : mCfg(cfg)
{
}

void SavingDir::SavingDirPrivate::init()
{
    load();
    save();
}

QString SavingDir::SavingDirPrivate::getDir() const
{
    return mDir;
}

void SavingDir::SavingDirPrivate::setDir(const QString &dir)
{
    mDir = dir;
    save();
}

void SavingDir::SavingDirPrivate::load()
{
    mDir = mCfg.readEntry("savingDir", QString());
    if (!isValid()) {
        mDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }
    if (!isValid()) {
        mDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    }
    if (!isValid()) {
        mDir = QDir::homePath();
    }
}

void SavingDir::SavingDirPrivate::save()
{
    mCfg.writeEntry("savingDir", mDir);
}

bool SavingDir::SavingDirPrivate::isValid()
{
    QDir dir;
    return (!mDir.isEmpty() && dir.exists(mDir));
}

SavingDir::SavingDir(const KConfigGroup &cfg)
    : d(new SavingDirPrivate(cfg))
{
    d->init();
}

SavingDir::~SavingDir()
{
    delete d;
}

QString SavingDir::getDir() const
{
    return d->getDir();
}

void SavingDir::setDir(const QString &dir)
{
    d->setDir(dir);
}
