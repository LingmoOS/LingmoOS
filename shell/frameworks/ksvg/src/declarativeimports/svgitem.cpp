/*
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svgitem.h"

#include <QDebug>
#include <QQuickWindow>
#include <QRectF>
#include <QSGTexture>

#include "ksvg/svg.h"

#include "managedtexturenode.h"

#include <Kirigami/Platform/PlatformTheme>
#include <debug_p.h>

namespace KSvg
{
SvgItem::SvgItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_textureChanged(false)
{
    m_svg = new KSvg::Svg(this);
    setFlag(QQuickItem::ItemHasContents, true);

    connect(m_svg, &Svg::repaintNeeded, this, &SvgItem::updateNeeded);
    connect(m_svg, &Svg::repaintNeeded, this, &SvgItem::naturalSizeChanged);
    connect(m_svg, &Svg::sizeChanged, this, &SvgItem::naturalSizeChanged);
    connect(m_svg, &Svg::repaintNeeded, this, &SvgItem::elementRectChanged);
    connect(m_svg, &Svg::sizeChanged, this, &SvgItem::elementRectChanged);
}

SvgItem::~SvgItem()
{
}

void SvgItem::componentComplete()
{
    m_kirigamiTheme = qobject_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));
    if (!m_kirigamiTheme) {
        qCWarning(LOG_KSVGQML) << "No theme!" << qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true) << this;
        return;
    }

    auto checkApplyTheme = [this]() {
        if (!m_svg->imageSet()->filePath(QStringLiteral("colors")).isEmpty()) {
            m_svg->clearColorOverrides();
        }
    };
    auto applyTheme = [this]() {
        if (!m_svg) {
            return;
        }
        if (!m_svg->imageSet()->filePath(QStringLiteral("colors")).isEmpty()) {
            m_svg->clearColorOverrides();
            return;
        }
        m_svg->setColor(Svg::Text, m_kirigamiTheme->textColor());
        m_svg->setColor(Svg::Background, m_kirigamiTheme->backgroundColor());
        m_svg->setColor(Svg::Highlight, m_kirigamiTheme->highlightColor());
        m_svg->setColor(Svg::HighlightedText, m_kirigamiTheme->highlightedTextColor());
        m_svg->setColor(Svg::PositiveText, m_kirigamiTheme->positiveTextColor());
        m_svg->setColor(Svg::NeutralText, m_kirigamiTheme->neutralTextColor());
        m_svg->setColor(Svg::NegativeText, m_kirigamiTheme->negativeTextColor());
    };
    applyTheme();
    connect(m_kirigamiTheme, &Kirigami::Platform::PlatformTheme::colorsChanged, this, applyTheme);
    connect(m_svg->imageSet(), &ImageSet::imageSetChanged, this, checkApplyTheme);
    connect(m_svg, &Svg::imageSetChanged, this, checkApplyTheme);

    QQuickItem::componentComplete();
}

void SvgItem::setImagePath(const QString &path)
{
    if (!m_svg || m_svg->imagePath() == path) {
        return;
    }

    updateDevicePixelRatio();
    m_svg->setImagePath(path);

    Q_EMIT imagePathChanged();

    if (isComponentComplete()) {
        update();
    }
}

QString SvgItem::imagePath() const
{
    return m_svg->imagePath();
}

void SvgItem::setElementId(const QString &elementID)
{
    if (elementID == m_elementID) {
        return;
    }

    if (implicitWidth() <= 0) {
        setImplicitWidth(naturalSize().width());
    }
    if (implicitHeight() <= 0) {
        setImplicitHeight(naturalSize().height());
    }

    m_elementID = elementID;
    Q_EMIT elementIdChanged();
    Q_EMIT naturalSizeChanged();
    Q_EMIT elementRectChanged();

    scheduleImageUpdate();
}

QString SvgItem::elementId() const
{
    return m_elementID;
}

void SvgItem::setSvg(KSvg::Svg *svg)
{
    if (m_svg) {
        disconnect(m_svg.data(), nullptr, this, nullptr);
    }
    m_svg = svg;

    if (svg) {
        connect(svg, &Svg::repaintNeeded, this, &SvgItem::updateNeeded);
        connect(svg, &Svg::repaintNeeded, this, &SvgItem::naturalSizeChanged);
        connect(svg, &Svg::repaintNeeded, this, &SvgItem::elementRectChanged);
        connect(svg, &Svg::sizeChanged, this, &SvgItem::naturalSizeChanged);
        connect(svg, &Svg::sizeChanged, this, &SvgItem::elementRectChanged);
    }

    if (implicitWidth() <= 0) {
        setImplicitWidth(naturalSize().width());
    }
    if (implicitHeight() <= 0) {
        setImplicitHeight(naturalSize().height());
    }

    scheduleImageUpdate();

    Q_EMIT svgChanged();
    Q_EMIT naturalSizeChanged();
    Q_EMIT elementRectChanged();
    Q_EMIT imagePathChanged();
}

KSvg::Svg *SvgItem::svg() const
{
    return m_svg.data();
}

QSizeF SvgItem::naturalSize() const
{
    if (!m_svg) {
        return QSizeF();
    } else if (!m_elementID.isEmpty()) {
        return m_svg->elementSize(m_elementID);
    }

    return m_svg->size();
}

QRectF SvgItem::elementRect() const
{
    if (!m_svg) {
        return QRectF();
    } else if (!m_elementID.isEmpty()) {
        return m_svg->elementRect(m_elementID);
    }

    return QRectF(QPointF(0, 0), m_svg->size());
}

QSGNode *SvgItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData);
    if (!window() || !m_svg) {
        delete oldNode;
        return nullptr;
    }

    // this is more than just an optimization, uploading a null image to QSGAtlasTexture causes a crash
    if (width() == 0.0 || height() == 0.0) {
        delete oldNode;
        return nullptr;
    }

    ManagedTextureNode *textureNode = static_cast<ManagedTextureNode *>(oldNode);
    if (!textureNode) {
        textureNode = new ManagedTextureNode;
        m_textureChanged = true;
    }

    // TODO use a heuristic to work out when to redraw
    // if !m_smooth and size is approximate simply change the textureNode.rect without
    // updating the material

    if (m_textureChanged || textureNode->texture()->textureSize() != QSize(width(), height())) {
        // despite having a valid size sometimes we still get a null QImage from KSvg::Svg
        // loading a null texture to an atlas fatals
        // Dave E fixed this in Qt in 5.3.something onwards but we need this for now
        if (m_image.isNull()) {
            delete textureNode;
            return nullptr;
        }

        QSharedPointer<QSGTexture> texture(window()->createTextureFromImage(m_image, QQuickWindow::TextureCanUseAtlas));
        textureNode->setTexture(texture);
        m_textureChanged = false;

        textureNode->setRect(0, 0, width(), height());
    }

    textureNode->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);

    return textureNode;
}

void SvgItem::updateNeeded()
{
    if (implicitWidth() <= 0) {
        setImplicitWidth(naturalSize().width());
    }
    if (implicitHeight() <= 0) {
        setImplicitHeight(naturalSize().height());
    }
    scheduleImageUpdate();
}

void SvgItem::scheduleImageUpdate()
{
    polish();
    update();
}

void SvgItem::updatePolish()
{
    QQuickItem::updatePolish();

    if (m_svg) {
        // setContainsMultipleImages has to be done there since m_svg can be shared with somebody else
        m_textureChanged = true;
        m_svg->setContainsMultipleImages(!m_elementID.isEmpty());
        m_image = m_svg->image(QSize(width(), height()), m_elementID);
    }
}

void SvgItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry.size() != oldGeometry.size() && newGeometry.isValid()) {
        scheduleImageUpdate();
    }

    QQuickItem::geometryChange(newGeometry, oldGeometry);
}

void SvgItem::updateDevicePixelRatio()
{
    const auto newDevicePixelRatio = std::max<qreal>(1.0, (window() ? window()->devicePixelRatio() : qApp->devicePixelRatio()));
    if (newDevicePixelRatio != m_svg->devicePixelRatio()) {
        m_svg->setDevicePixelRatio(newDevicePixelRatio);
        m_textureChanged = true;
    }
}

void SvgItem::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value)
{
    if (change == ItemSceneChange && value.window) {
        updateDevicePixelRatio();
    } else if (change == QQuickItem::ItemDevicePixelRatioHasChanged) {
        updateDevicePixelRatio();
    }

    QQuickItem::itemChange(change, value);
}

} // KSvg namespace

#include "moc_svgitem.cpp"
