/*
    SPDX-FileCopyrightText: 2011 Artur Duque de Souza <asouza@kde.org>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KQUICK_ICON_PROVIDER_H
#define KQUICK_ICON_PROVIDER_H

#include <KIconEffect>
#include <KIconLoader>
#include <QIcon>
#include <QPixmap>
#include <QQuickImageProvider>
#include <QSize>

/**
 * Class which exposes the KIcon* functioality to QML.
 * For dependency reasons, this is a header-only class.
 *
 * This needs to be registered in the engine using the following code:
 * @code
 * engine->addImageProvider(QStringLiteral("icon"), new KQuickIconProvider);
 * @endcode
 * @since 5.98
 */
class KQuickIconProvider : public QQuickImageProvider
{
public:
    KQuickIconProvider()
        : QQuickImageProvider(QQuickImageProvider::Pixmap)
    {
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        // We need to handle QIcon::state
        const QStringList source = id.split(QLatin1Char('/'));

        QPixmap pixmap;
        if (requestedSize.isValid()) {
            pixmap = QIcon::fromTheme(source.at(0)).pixmap(requestedSize);
        } else if (size->isValid()) {
            pixmap = QIcon::fromTheme(source.at(0)).pixmap(*size);
        } else {
            pixmap = QIcon::fromTheme(source.at(0)).pixmap(KIconLoader::global()->currentSize(KIconLoader::Desktop));
        }

        if (source.size() == 2) {
            const QString state(source.at(1));
            int finalState = KIconLoader::DefaultState;

            if (state == QLatin1String("active")) {
                finalState = KIconLoader::ActiveState;
            } else if (state == QLatin1String("disabled")) {
                finalState = KIconLoader::DisabledState;
            } else if (state == QLatin1String("last")) {
                finalState = KIconLoader::LastState;
            }

            // apply the effect for state
            if (finalState == KIconLoader::ActiveState) {
                KIconEffect::toActive(pixmap);
            }

            if (finalState == KIconLoader::DisabledState) {
                KIconEffect::toDisabled(pixmap);
            }
        }

        if (!pixmap.isNull() && size) {
            *size = pixmap.size();
        }

        return pixmap;
    }
};

#endif
