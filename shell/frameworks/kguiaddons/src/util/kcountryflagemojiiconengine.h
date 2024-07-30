// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2022-2023 Harald Sitter <sitter@kde.org>

#ifndef KCOUNTRYFLAGEMOJIICONENGINE_H
#define KCOUNTRYFLAGEMOJIICONENGINE_H

#include <QIconEngine>

#include <kguiaddons_export.h>

class KCountryFlagEmojiIconEnginePrivate;

/**
 * @brief Provides emoji flags as icons
 * This is a special icon engine that internally paints flags using emoji fonts.
 * It provides access to country and region flags from the system emoji font.
 * ```
 *   auto l = new QLabel;
 *   l->setMinimumSize(512, 512);
 *   l->setPixmap(QIcon(new KCountryFlagEmojiIconEngine("AT")).pixmap(512, 512));
 * ```
 * @since 6.0
 */
class KGUIADDONS_EXPORT KCountryFlagEmojiIconEngine : public QIconEngine
{
public:
    /**
     * @brief Construct a new KCountryFlagEmojiIconEngine object
     * Please note that regional flag support can be spotty in emoji fonts.
     * @param regionOrCountry either a ISO 3166-1 alpha-2 country code or a ISO 3166-2 region code (e.g. AT for Austria or GB-SCT for Scotland)
     */
    explicit KCountryFlagEmojiIconEngine(const QString &regionOrCountry);
    ~KCountryFlagEmojiIconEngine() override;
    Q_DISABLE_COPY_MOVE(KCountryFlagEmojiIconEngine)

    QIconEngine *clone() const override;
    QString key() const override;
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale) override;

    /**
     * @brief Check whether the internal emoji unicode sequence is null
     * This does not necessarily mean that the pixmap output will be a valid flag - that entirely depends on the system's precise font configuration.
     * @return true when the construction of the emoji string failed
     * @return false when the construction of the emoji string succeeded
     */
    bool isNull() override;

    /**
     * @brief Set the Global Default Font object
     * This is primarily useful for platform themes that wish to force a specific font being used. By default the "emoji" font family will be used.
     * Forcing a specific font and making sure it is available as runtime requirement is the most reliable way to ensure that flag support is working
     * regardless of system configuration.
     * @param font the default font to use
     */
    static void setGlobalDefaultFont(const QFont &font);

private:
    std::unique_ptr<KCountryFlagEmojiIconEnginePrivate> const d;
};

#endif
