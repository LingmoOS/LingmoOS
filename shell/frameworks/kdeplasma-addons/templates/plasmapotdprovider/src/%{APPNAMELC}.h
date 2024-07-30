/*
 *   SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H

#include <plasma/potdprovider/potdprovider.h>
// Qt
#include <QImage>

class KJob;

class %{APPNAME} : public PotdProvider
{
    Q_OBJECT

public:
    /**
     * Creates a new %{APPNAME}.
     *
     * @param parent the parent object
     * @param args arguments from the plugin factory (unused)
     */
    %{APPNAME}(QObject *parent, const QVariantList &args);

    /**
     * Destroys the provider.
     */
    ~%{APPNAME}() override;

private:
    void handleFinishedFeedRequest(KJob *job);
    void handleFinishedImageRequest(KJob *job);
};

#endif
