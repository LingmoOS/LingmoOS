/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "potdprovider.h"

class KJob;

/**
 * This class provides the image for the "Wikimedia
 * Commons Picture Of the Day"
 * located at http://tools.wikimedia.de/~daniel/potd/commons/potd-800x600.html.
 * From there extract the picture.
 * Using 800x600 as the best size for now, others are available, see
 * http://tools.wikimedia.de/~daniel/potd/potd.php
 */
class WcpotdProvider : public PotdProvider
{
    Q_OBJECT

public:
    explicit WcpotdProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

private:
    void pageRequestFinished(KJob *job);
    void imageRequestFinished(KJob *job);
};
