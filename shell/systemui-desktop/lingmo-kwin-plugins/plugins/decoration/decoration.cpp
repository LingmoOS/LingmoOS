/*
 * Copyright (C) 2020 PandaOS Team.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// own
#include "decoration.h"
#include "button.h"

// KDecoration
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationShadow>

// Qt
#include <QApplication>
#include <QPainter>
#include <QSettings>
#include <QImageReader>
#include <QTimer>

#include <KPluginFactory>

#include <cmath>

K_PLUGIN_FACTORY_WITH_JSON(
    LingmoDecorationFactory,
    "lingmoos.json",
    registerPlugin<Lingmo::Decoration>(););

namespace Lingmo
{
static int g_sDecoCount = 0;
static int g_shadowSize = 0;
static int g_shadowStrength = 0;
static QColor g_shadowColor = Qt::black;
static QSharedPointer<KDecoration2::DecorationShadow> g_sShadow;

Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration2::Decoration(parent, args)
    , m_settings(new QSettings(QSettings::UserScope, "lingmoos", "theme"))
    , m_settingsFile(m_settings->fileName())
    , m_fileWatcher(new QFileSystemWatcher)
    , m_x11Shadow(new X11Shadow)
{
    ++g_sDecoCount;
}

Decoration::~Decoration()
{
    if (--g_sDecoCount == 0) {
        g_sShadow.clear();
    }
}

void Decoration::paint(QPainter *painter, const QRect &repaintRegion)
{
#if KDECORATION_VERSION <= QT_VERSION_CHECK(5, 27, 11)
    auto *decoratedClient = client().toStrongRef().data();
#else
    auto *decoratedClient = client();
#endif
    auto s = settings();

    painter->fillRect(rect(), Qt::transparent);

    if (!decoratedClient->isShaded()) {
        painter->fillRect(rect(), Qt::transparent);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(titleBarBackgroundColor());

        if (s->isAlphaChannelSupported() && radiusAvailable()) {
            painter->drawRoundedRect(rect(), m_frameRadius, m_frameRadius);
        } else {
            painter->drawRect(rect());
        }
        painter->restore();

        // draw buttons.
        m_leftButtons->paint(painter, repaintRegion);
        m_rightButtons->paint(painter, repaintRegion);
    }

    paintCaption(painter, repaintRegion);
    paintButtons(painter, repaintRegion);
}

#if KDECORATION_VERSION <= QT_VERSION_CHECK(5, 27, 11) 
void Decoration::init()
#else
bool Decoration::init()
#endif
{   
#if KDECORATION_VERSION <= QT_VERSION_CHECK(5, 27, 11) 
    auto c = client().toStrongRef().data();
#else
    auto c = client();
#endif
    auto s = settings();

    m_devicePixelRatio = m_settings->value("PixelRatio", 1.0).toReal();
    m_frameRadius = 11 * m_devicePixelRatio;

    reconfigure();
    updateTitleBar();

    connect(s.get(), &KDecoration2::DecorationSettings::borderSizeChanged, this, &Decoration::recalculateBorders);

    // a change in font might cause the borders to change
    connect(s.get(), &KDecoration2::DecorationSettings::fontChanged, this, &Decoration::recalculateBorders);
    connect(s.get(), &KDecoration2::DecorationSettings::spacingChanged, this, &Decoration::recalculateBorders);

    // full reconfiguration
    connect(s.get(), &KDecoration2::DecorationSettings::reconfigured, this, &Decoration::reconfigure);
    connect(s.get(), &KDecoration2::DecorationSettings::reconfigured, this, &Decoration::updateButtonsGeometryDelayed);

    // buttons
    connect(s.get(), &KDecoration2::DecorationSettings::spacingChanged, this, &Decoration::updateButtonsGeometryDelayed);
    connect(s.get(), &KDecoration2::DecorationSettings::decorationButtonsLeftChanged, this, &Decoration::updateButtonsGeometryDelayed);
    connect(s.get(), &KDecoration2::DecorationSettings::decorationButtonsRightChanged, this, &Decoration::updateButtonsGeometryDelayed);

    connect(c, &KDecoration2::DecoratedClient::adjacentScreenEdgesChanged, this, &Decoration::recalculateBorders);
    connect(c, &KDecoration2::DecoratedClient::maximizedHorizontallyChanged, this, &Decoration::recalculateBorders);
    connect(c, &KDecoration2::DecoratedClient::maximizedVerticallyChanged, this, &Decoration::recalculateBorders);
    connect(c, &KDecoration2::DecoratedClient::shadedChanged, this, &Decoration::recalculateBorders);
    connect(c, &KDecoration2::DecoratedClient::captionChanged, this, [this]() {
        // update the caption area
        update(titleBar());
    });

    connect(c, &KDecoration2::DecoratedClient::activeChanged, this, [this] {
        update(titleBar());
    });

    connect(c, &KDecoration2::DecoratedClient::widthChanged, this, &Decoration::updateTitleBar);

    connect(c, &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateTitleBar);
    connect(c, &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateButtonsGeometry);

    connect(c, &KDecoration2::DecoratedClient::widthChanged, this, &Decoration::updateButtonsGeometry);
    connect(c, &KDecoration2::DecoratedClient::adjacentScreenEdgesChanged, this, &Decoration::updateButtonsGeometry);
    connect(c, &KDecoration2::DecoratedClient::shadedChanged, this, &Decoration::updateButtonsGeometry);

    // lingmoos settings
    m_fileWatcher->addPath(m_settingsFile);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, [=] {
        m_settings->sync();
        m_devicePixelRatio = m_settings->value("PixelRatio", 1.0).toReal();

        updateBtnPixmap();
        update(titleBar());
        updateTitleBar();
        updateButtonsGeometry();
        reconfigure();

        bool fileDeleted = !m_fileWatcher->files().contains(m_settingsFile);
        if (fileDeleted)
            m_fileWatcher->addPath(m_settingsFile);
    });

    updateBtnPixmap();
    createButtons();

    // // For some reason, the shadow should be installed the last. Otherwise,
    // // the Window Decorations KCM crashes.
    updateShadow();

#if KDECORATION_VERSION > QT_VERSION_CHECK(5, 27, 11)
    return true;
#endif
}

void Decoration::reconfigure()
{
    recalculateBorders();
    updateResizeBorders();
    updateShadow();
}

void Decoration::createButtons()
{
    m_leftButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Left, this, &Button::create);
    m_rightButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Right, this, &Button::create);
    updateButtonsGeometry();
}

void Decoration::recalculateBorders()
{
    QMargins borders;

    borders.setTop(titleBarHeight());

    setBorders(borders);
}

void Decoration::updateResizeBorders()
{
    QMargins borders;

    borders.setLeft(5);
    borders.setTop(5);
    borders.setRight(5);
    borders.setBottom(5);

    setResizeOnlyBorders(borders);
}

void Decoration::updateTitleBar()
{
#if KDECORATION_VERSION <= QT_VERSION_CHECK(5, 27, 11)
    auto *decoratedClient = client().toStrongRef().data();
#else
    auto *decoratedClient = client().data();
#endif
    setTitleBar(QRect(0, 0, decoratedClient->width(), titleBarHeight()));
    update(titleBar());
}

void Decoration::updateButtonsGeometryDelayed()
{
    QTimer::singleShot(0, this, &Decoration::updateButtonsGeometry);
}

void Decoration::updateButtonsGeometry()
{
    auto s = settings();
    int rightMargin = 2;
    int btnSpacing = 8;

    foreach (const QPointer<KDecoration2::DecorationButton> &button, m_leftButtons->buttons() + m_rightButtons->buttons()) {
        button.data()->setGeometry(QRectF(QPoint(0, 0), QSizeF(titleBarHeight(), titleBarHeight())));
    }

    if (!m_leftButtons->buttons().isEmpty()) {
        m_leftButtons->setPos(QPointF(0, 0));
        m_leftButtons->setSpacing(btnSpacing);
    }

    if (!m_rightButtons->buttons().isEmpty()) {
        m_rightButtons->setSpacing(btnSpacing);
        m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width() - rightMargin, 0));
    }

    update();
}

void Decoration::updateShadow()
{
    // assign global shadow if exists and parameters match
    if (!g_sShadow) {
        // assign parameters
        g_shadowSize = 90;
        g_shadowStrength = 35;
        g_shadowColor = Qt::black;
        const int shadowOverlap = m_frameRadius;
        const int shadowOffset = shadowOverlap / 2;

        // create image
        QImage image(2 * g_shadowSize, 2 * g_shadowSize, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);

        // create gradient
        auto alpha = [](qreal x) { return std::exp(-x * x / 0.15); };

        auto gradientStopColor = [](QColor color, int alpha) {
            color.setAlpha(alpha);
            return color;
        };

        QRadialGradient radialGradient(g_shadowSize, g_shadowSize, g_shadowSize);
        for (int i = 0; i < 10; ++i) {
            const qreal x(qreal(i) / 9);
            radialGradient.setColorAt(x, gradientStopColor(g_shadowColor, alpha(x) * g_shadowStrength));
        }

        radialGradient.setColorAt(1, gradientStopColor(g_shadowColor, 0));

        QPainter painter;
        // fill
        painter.begin(&image);
        //TODO review these
        //QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillRect(image.rect(), radialGradient);

        // contrast pixel
        QRectF innerRect = QRectF(
            g_shadowSize - shadowOverlap, g_shadowSize - shadowOffset - shadowOverlap,
            2 * shadowOverlap, shadowOffset + 2 * shadowOverlap);

        painter.setPen(gradientStopColor(g_shadowColor, g_shadowStrength * 0.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(innerRect, -0.5 + m_frameRadius, -0.5 + m_frameRadius);

        // mask out inner rect
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        painter.drawRoundedRect(innerRect, 0.5 + m_frameRadius, 0.5 + m_frameRadius);
        painter.end();

        g_sShadow = QSharedPointer<KDecoration2::DecorationShadow>::create();
        g_sShadow->setPadding(QMargins(
            g_shadowSize - shadowOverlap,
            g_shadowSize - shadowOffset - shadowOverlap,
            g_shadowSize - shadowOverlap,
            g_shadowSize - shadowOverlap));

        g_sShadow->setInnerShadowRect(QRect(g_shadowSize, g_shadowSize, 1, 1));

        // assign image
        g_sShadow->setShadow(image);
    }

    setShadow(g_sShadow);
}

void Decoration::updateBtnPixmap()
{
    int size = 24;
    QString dirName = darkMode() ? "dark" : "light";

    m_closeBtnPixmap = fromSvgToPixmap(QString(":/images/%1/close_normal.svg").arg(dirName), QSize(size, size));
    m_maximizeBtnPixmap = fromSvgToPixmap(QString(":/images/%1/maximize_normal.svg").arg(dirName), QSize(size, size));
    m_minimizeBtnPixmap = fromSvgToPixmap(QString(":/images/%1/minimize_normal.svg").arg(dirName), QSize(size, size));
    m_restoreBtnPixmap = fromSvgToPixmap(QString(":/images/%1/restore_normal.svg").arg(dirName), QSize(size, size));
}

QPixmap Decoration::fromSvgToPixmap(const QString &file, const QSize &size)
{
    QImageReader reader(file);

    if (reader.canRead()) {
        reader.setScaledSize(size * m_devicePixelRatio);
        return QPixmap::fromImage(reader.read());
    }

    return QPixmap();
}

int Decoration::titleBarHeight() const
{
    return m_titleBarHeight * m_devicePixelRatio;
}

bool Decoration::darkMode() const
{
    QSettings settings(QSettings::UserScope, "lingmoos", "theme");
    return settings.value("DarkMode", false).toBool();
}

bool Decoration::radiusAvailable() const
{
    return !isMaximized();
}

bool Decoration::isMaximized() const
{
#if KDECORATION_VERSION <= QT_VERSION_CHECK(5, 27, 11)
    return client().toStrongRef().data()->isMaximized();
#else
    return client()->isMaximized();
#endif
}

void Decoration::paintFrameBackground(QPainter *painter, const QRect &repaintRegion) const
{
    Q_UNUSED(repaintRegion)

    painter->save();

    painter->fillRect(rect(), Qt::transparent);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->restore();
}

QColor Decoration::titleBarBackgroundColor() const
{
    return darkMode() ? m_titleBarBgDarkColor : m_titleBarBgColor;
}

QColor Decoration::titleBarForegroundColor() const
{
#if KDECORATION_VERSION <= QT_VERSION_CHECK(5, 27, 11)
    const auto *decoratedClient = client().toStrongRef().data();
#else
    const auto *decoratedClient = client();
#endif
    const bool isActive = decoratedClient->isActive();
    QColor color;

    if (isActive) {
        color = darkMode() ? m_titleBarFgDarkColor : m_titleBarFgColor;
    } else {
        color = darkMode() ? m_unfocusedFgDarkColor : m_unfocusedFgColor;
    }

    return color;
}

void Decoration::paintCaption(QPainter *painter, const QRect &repaintRegion) const
{
    Q_UNUSED(repaintRegion)
#if KDECORATION_VERSION <= QT_VERSION_CHECK(5, 27, 11)
    const auto *decoratedClient = client().toStrongRef().data();
#else
    const auto *decoratedClient = client();
#endif

    const int textWidth = settings()->fontMetrics().boundingRect(decoratedClient->caption()).width();
    const QRect textRect((size().width() - textWidth) / 2, 0, textWidth, titleBarHeight());

    const QRect titleBarRect(0, 0, size().width(), titleBarHeight());

    const QRect availableRect = titleBarRect.adjusted(
        m_leftButtons->geometry().width() + 20, 0,
        -(m_rightButtons->geometry().width() + 20), 0);

    QRect captionRect;
    Qt::Alignment alignment;

    if (textRect.left() < availableRect.left()) {
        captionRect = availableRect;
        alignment = Qt::AlignLeft | Qt::AlignVCenter;
    } else if (availableRect.right() < textRect.right()) {
        captionRect = availableRect;
        alignment = Qt::AlignRight | Qt::AlignVCenter;
    } else {
        captionRect = titleBarRect;
        alignment = Qt::AlignCenter;
    }

    const QString caption = painter->fontMetrics()
                                .elidedText(decoratedClient->caption(), Qt::ElideMiddle, captionRect.width());

    painter->save();
    painter->setFont(settings()->font());
    painter->setPen(titleBarForegroundColor());
    painter->drawText(captionRect, alignment, caption);
    painter->restore();
}

void Decoration::paintButtons(QPainter *painter, const QRect &repaintRegion) const
{
    m_leftButtons->paint(painter, repaintRegion);
    m_rightButtons->paint(painter, repaintRegion);
}

}  // namespace Lingmo

#include "decoration.moc"