/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSVG_IMAGESET_H
#define KSVG_IMAGESET_H

#include <QGuiApplication>
#include <QObject>

#include <KSharedConfig>
#include <ksvg/ksvg_export.h>

class KPluginMetaData;

namespace KSvg
{
class ImageSetPrivate;
class SvgPrivate;

// TODO: move in the plasma part the watching and regeneration of icon themes

/**
 * @class ImageSet ksvg/imageset.h <KSvg/ImageSet>
 *
 * @short Interface to the Svg image set
 *
 *
 * KSvg::ImageSet provides access to a common and standardized set of graphic
 * elements stored in SVG format. This allows artists to create single packages
 * of SVGs that will affect the look and feel of all workspace components.
 *
 * KSvg::Svg uses KSvg::ImageSet internally to locate and load the appropriate
 * SVG data. Alternatively, KSvg::ImageSet can be used directly to retrieve
 * file system paths to SVGs by name.
 */
class KSVG_EXPORT ImageSet : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString imageSetName READ imageSetName WRITE setImageSetName NOTIFY imageSetChanged)
    Q_PROPERTY(QString basePath READ basePath WRITE setBasePath NOTIFY imageSetChanged)
    Q_PROPERTY(bool useGlobalSettings READ useGlobalSettings NOTIFY imageSetChanged)

public:
    /**
     * Default constructor.
     * @param parent the parent object
     */
    explicit ImageSet(QObject *parent = nullptr);

    /**
     * @brief This method constructs a theme which will be a custom theme
     * instance of imageSetName.
     *
     * @param imageSetName the name of the theme to create
     * @param basePath base path for the theme to look for svgs. if empty, the default will be used.
     * @param parent the parent object
     */
    explicit ImageSet(const QString &imageSetName, const QString &basePath = {}, QObject *parent = nullptr);

    ~ImageSet() override;

    /**
     * @brief This method sets a base path for the theme to look for SVGs.
     *
     * It can be a path relative to QStandardPaths::GenericDataLocation or an
     * absolute path.
     *
     * @param basePath the base path
     */
    void setBasePath(const QString &basePath);

    /**
     * @brief This method returns the base path of the theme where the SVGs will
     * be looked for.
     */
    QString basePath() const;

    /**
     * @brief This method sets the file selectors.
     *
     * The theme can have different svgs with the same name for different
     * situations and platforms. The Plasma desktop for instance uses "opaque"
     * or "translucent" based on presence of compositing and KWin blur effects.
     * Other uses may be platform, like android-specific graphics.
     *
     * @param selectors selectors in order of preference
     */
    void setSelectors(const QStringList &selectors);

    /**
     * @brief This method returns the current selectors in order of preference.
     */
    QStringList selectors() const;

    /**
     * @brief This method sets the current theme.
     */
    void setImageSetName(const QString &imageSetName);

    /**
     * @brief This method returns the name of the current theme.
     */
    QString imageSetName() const;

    /**
     * @brief This method returns the path for an SVG image in the current
     * theme.
     *
     * @param name the name of the file in the theme directory (without the
     * ".svg" part or a leading slash).
     *
     * @return the full path to the requested file for the current theme
     */
    QString imagePath(const QString &name) const;

    /**
     * @brief This method returns the path for a generic file in the current
     * theme.
     *
     * The theme can also ship any generic file, such as configuration files.
     *
     * @param name the name of the file in the theme directory (without a
     * leading slash)
     *
     * @return the full path to the requested file for the current theme
     */
    QString filePath(const QString &name) const;

    /**
     * @brief This method checks whether this theme contains an image with the
     * given name.
     *
     * @param name the name of the file in the theme directory (without the
     * ".svg" part or a leading slash)
     *
     * @return true if the image exists for this theme
     */
    bool currentImageSetHasImage(const QString &name) const;

    /**
     * @brief This method sets whether the theme should follow the global
     * settings or use application-specific settings.
     *
     * @param useGlobal pass in true to follow the global settings
     */
    void setUseGlobalSettings(bool useGlobal);

    /**
     * @brief This method returns whether the global settings are followed.
     *
     * If application-specific settings are being used, it returns @c false.
     */
    bool useGlobalSettings() const;

    /**
     * @brief This method sets the maximum size of the cache (in kilobytes).
     *
     * If cache gets bigger than the limit, then some entries are removed.
     * Setting cache limit to 0 disables automatic cache size limiting.
     *
     * Note that the cleanup might not be done immediately, so the cache might
     * temporarily (for a few seconds) grow bigger than the limit.
     **/
    void setCacheLimit(int kbytes);

    /**
     * @brief This method returns the plugin metadata for this theme.
     *
     * Metadata contains information such as name, description, author, website,
     * and url.
     */
    KPluginMetaData metadata() const;

Q_SIGNALS:
    /**
     * @brif This signal is emitted when the user makes changes to the theme.
     *
     * Rendered images, colors, etc. should be updated at this point. However,
     * SVGs should *not* be repainted in response to this signal; connect to
     * Svg::repaintNeeded() instead for that, as SVG objects need repainting not
     * only when imageSetChanged() is emitted; moreover SVG objects connect to
     * and respond appropriately to imageSetChanged() internally, emitting
     * Svg::repaintNeeded() at an appropriate time.
     */
    void imageSetChanged(const QString &basePath);

    /**
     * @brief This signal is emitted when the user changes the base path of the
     * image set.
     */
    void basePathChanged(const QString &basePath);

private:
    friend class SvgPrivate;
    friend class Svg;
    friend class FrameSvg;
    friend class FrameSvgPrivate;
    friend class ImageSetPrivate;
    ImageSetPrivate *d;
};

} // KSvg namespace

#endif // multiple inclusion guard
