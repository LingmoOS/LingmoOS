/*
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SVGITEM_P
#define SVGITEM_P

#include <QImage>
#include <QQuickItem>

#include <qqmlregistration.h>

namespace Kirigami
{
namespace Platform
{
class PlatformTheme;
}
};

namespace KSvg
{
class Svg;

/**
 * @class SvgItem
 * @short Displays an SVG or an element from an SVG file
 */
class SvgItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief This property specifies the relative path of the Svg in the theme.
     *
     * Example: "widgets/background"
     *
     * @property QString imagePath
     */
    Q_PROPERTY(QString imagePath READ imagePath WRITE setImagePath NOTIFY imagePathChanged)

    /**
     * @brief This property specifies the sub-element of the SVG to be
     * rendered.
     *
     * If this is empty, the whole SVG document will be rendered.
     *
     * @property QString elementId
     */
    Q_PROPERTY(QString elementId READ elementId WRITE setElementId NOTIFY elementIdChanged)

    /**
     * @brief This property holds the SVG's natural, unscaled size.
     *
     * This is useful if a pixel-perfect rendering of outlines is needed.
     *
     * @property QSizeF naturalSize
     */
    Q_PROPERTY(QSizeF naturalSize READ naturalSize NOTIFY naturalSizeChanged)

    /**
     * @brief This property holds the rectangle of the selected elementId
     * relative to the unscaled size of the SVG document.
     *
     * Note that this property will holds the entire SVG if element id is not
     * selected.
     *
     * @property QRectF elementRect
     */
    Q_PROPERTY(QRectF elementRect READ elementRect NOTIFY elementRectChanged)

    /**
     * @brief This property holds the internal SVG instance.
     *
     * Usually, specifying just the imagePath is enough. Use this if you have
     * many items taking the same SVG as source, and you want to share the
     * internal SVG object.
     *
     * @property KSvg::Svg svg
     */
    Q_PROPERTY(KSvg::Svg *svg READ svg WRITE setSvg NOTIFY svgChanged)

public:
    /// @cond INTERNAL_DOCS

    explicit SvgItem(QQuickItem *parent = nullptr);
    ~SvgItem() override;

    void setImagePath(const QString &path);
    QString imagePath() const;

    void setElementId(const QString &elementID);
    QString elementId() const;

    void setSvg(KSvg::Svg *svg);
    KSvg::Svg *svg() const;

    QSizeF naturalSize() const;

    QRectF elementRect() const;

    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData) override;
    /// @endcond

    void itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &data) override;

protected:
    void componentComplete() override;

Q_SIGNALS:
    void imagePathChanged();
    void elementIdChanged();
    void svgChanged();
    void naturalSizeChanged();
    void elementRectChanged();

protected Q_SLOTS:
    /// @cond INTERNAL_DOCS
    void updateNeeded();
    /// @endcond

private:
    void updateDevicePixelRatio();
    void scheduleImageUpdate();
    void updatePolish() override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

    QPointer<KSvg::Svg> m_svg;
    Kirigami::Platform::PlatformTheme *m_kirigamiTheme;
    QString m_elementID;
    QImage m_image;
    bool m_textureChanged;
};
}

#endif
