/*
 *   SPDX-FileCopyrightText: 2021 Alexey Andreyev <aa13q@ya.ru>
 *
 *   SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL
 */

#ifndef UNSPLASHPROVIDER_H
#define UNSPLASHPROVIDER_H

#include "potdprovider.h"

class KJob;

/**
 * This class provides random wallpapers from Simon Stalenhag website
 * http://simonstalenhag.se/
 * see also: https://github.com/a-andreyev/simonstalenhag-se-metadata
 */
class SimonStalenhagProvider : public PotdProvider
{
    Q_OBJECT

public:
    explicit SimonStalenhagProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

private:
    void entrypointRequestFinished(KJob *job);
    void metaDataRequestFinished(KJob *job);
    void imageRequestFinished(KJob *job);
};

#endif
