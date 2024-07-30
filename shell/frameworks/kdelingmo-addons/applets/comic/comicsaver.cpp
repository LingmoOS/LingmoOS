/*
 *   SPDX-FileCopyrightText: 2008-2012 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "comicsaver.h"
#include "comicdata.h"
#include "comicinfo.h"

#include <QDebug>
#include <QFileDialog>
#include <QTemporaryFile>

#include <KIO/Job>

ComicSaver::ComicSaver(SavingDir *savingDir)
    : mSavingDir(savingDir)
{
}

bool ComicSaver::save(const ComicData &comic)
{
    const QString title = comic.title();

    const QString name = title + QLatin1String(" - ") + comic.current() + QLatin1String(".png");
    QUrl destUrl = QUrl::fromLocalFile(mSavingDir->getDir() + QLatin1Char('/') + name);

    destUrl = QFileDialog::getSaveFileUrl(nullptr, QString(), destUrl, QStringLiteral("*.png"));

    if (!destUrl.isValid()) {
        return false;
    }

    mSavingDir->setDir(destUrl.path());
    comic.image().save(destUrl.toLocalFile(), "PNG");

    return true;
}
