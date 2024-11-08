/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "lingmowindow.h"

#include "dialogbackground_p.h"
#include "dialogshadows_p.h"

#include <QMarginsF>
#include <QQuickItem>

#include <KWindowEffects>
#include <KWindowSystem>
#include <KX11Extras>

#include <Lingmo/Theme>

using namespace Lingmo;

namespace LingmoQuick
{
class LingmoWindowPrivate
{
public:
    LingmoWindowPrivate(LingmoWindow *_q)
        : q(_q)
    {
    }
    void handleFrameChanged();
    void updateMainItemGeometry();
    LingmoWindow *q;
    DialogShadows *shadows;
    // Keep a theme instance as a memeber to create one as soon as possible,
    // as Theme creation will set KSvg to correctly fetch images form the Lingmo Theme.
    // This makes sure elements are correct, both in the dialog surface and the shadows.
    Lingmo::Theme theme;
    QPointer<QQuickItem> mainItem;
    DialogBackground *dialogBackground;
    LingmoWindow::BackgroundHints backgroundHints = LingmoWindow::StandardBackground;
};

LingmoWindow::LingmoWindow(const QString &svgPrefix)
    : QQuickWindow()
    , d(new LingmoWindowPrivate(this))
{
    setColor(QColor(Qt::transparent));
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    d->shadows = DialogShadows::instance(svgPrefix);
    d->dialogBackground = new DialogBackground(contentItem());
    d->dialogBackground->setImagePath(svgPrefix);
    connect(d->dialogBackground, &DialogBackground::fixedMarginsChanged, this, [this]() {
        d->updateMainItemGeometry();
        Q_EMIT paddingChanged();
    });
    connect(d->dialogBackground, &DialogBackground::maskChanged, this, [this]() {
        d->handleFrameChanged();
    });

    d->shadows->addWindow(this, d->dialogBackground->enabledBorders());
}

LingmoWindow::~LingmoWindow()
{
}

void LingmoWindow::setMainItem(QQuickItem *mainItem)
{
    if (d->mainItem == mainItem)
        return;

    if (d->mainItem) {
        d->mainItem->setParentItem(nullptr);
    }

    d->mainItem = mainItem;
    Q_EMIT mainItemChanged();

    if (d->mainItem) {
        mainItem->setParentItem(contentItem());
        d->updateMainItemGeometry();
    }
}

QQuickItem *LingmoWindow::mainItem() const
{
    return d->mainItem;
}

static KSvg::FrameSvg::EnabledBorders edgeToBorder(Qt::Edges edges)
{
    KSvg::FrameSvg::EnabledBorders borders = KSvg::FrameSvg::EnabledBorders(0);

    if (edges & Qt::Edge::TopEdge) {
        borders |= KSvg::FrameSvg::EnabledBorder::TopBorder;
    }
    if (edges & Qt::Edge::BottomEdge) {
        borders |= KSvg::FrameSvg::EnabledBorder::BottomBorder;
    }
    if (edges & Qt::Edge::LeftEdge) {
        borders |= KSvg::FrameSvg::EnabledBorder::LeftBorder;
    }
    if (edges & Qt::Edge::RightEdge) {
        borders |= KSvg::FrameSvg::EnabledBorder::RightBorder;
    }

    return borders;
}

static Qt::Edges bordersToEdge(KSvg::FrameSvg::EnabledBorders borders)
{
    Qt::Edges edges;
    if (borders & KSvg::FrameSvg::EnabledBorder::TopBorder) {
        edges |= Qt::Edge::TopEdge;
    }
    if (borders & KSvg::FrameSvg::EnabledBorder::BottomBorder) {
        edges |= Qt::Edge::BottomEdge;
    }
    if (borders & KSvg::FrameSvg::EnabledBorder::LeftBorder) {
        edges |= Qt::Edge::LeftEdge;
    }
    if (borders & KSvg::FrameSvg::EnabledBorder::RightBorder) {
        edges |= Qt::Edge::RightEdge;
    }
    return edges;
}

void LingmoWindow::setBorders(Qt::Edges bordersToShow)
{
    d->dialogBackground->setEnabledBorders(edgeToBorder(bordersToShow));
    d->shadows->setEnabledBorders(this, d->dialogBackground->enabledBorders());
    Q_EMIT bordersChanged();
}

Qt::Edges LingmoWindow::borders()
{
    return bordersToEdge(d->dialogBackground->enabledBorders());
}

void LingmoWindow::showEvent(QShowEvent *e)
{
    // EWMH states that the state is reset every hide
    // Qt supports external factors setting state before the next show
    if (KWindowSystem::isPlatformX11()) {
        KX11Extras::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
    }
    QQuickWindow::showEvent(e);
}

void LingmoWindow::resizeEvent(QResizeEvent *e)
{
    QQuickWindow::resizeEvent(e);
    const QSize windowSize = e->size();
    d->dialogBackground->setSize(windowSize);
    if (d->mainItem) {
        const QSize contentSize = windowSize.shrunkBy(padding());
        d->mainItem->setSize(contentSize);
    }
}

void LingmoWindowPrivate::handleFrameChanged()
{
    const QRegion mask = dialogBackground->mask();
    KWindowEffects::enableBlurBehind(q, theme.blurBehindEnabled(), mask);
    KWindowEffects::enableBackgroundContrast(q,
                                             theme.backgroundContrastEnabled(),
                                             theme.backgroundContrast(),
                                             theme.backgroundIntensity(),
                                             theme.backgroundSaturation(),
                                             mask);

    if (!KWindowSystem::isPlatformX11() || KX11Extras::compositingActive()) {
        q->setMask(QRegion());
    } else {
        q->setMask(mask);
    }
}

void LingmoWindowPrivate::updateMainItemGeometry()
{
    if (!mainItem) {
        return;
    }
    const QMargins frameMargins = q->padding();
    const QSize windowSize = q->size();

    mainItem->setX(frameMargins.left());
    mainItem->setY(frameMargins.top());

    const QSize contentSize = windowSize.shrunkBy(frameMargins);
    mainItem->setSize(contentSize);
}

QMargins LingmoWindow::padding() const
{
    return QMargins(d->dialogBackground->leftMargin(),
                    d->dialogBackground->topMargin(),
                    d->dialogBackground->rightMargin(),
                    d->dialogBackground->bottomMargin());
}

LingmoWindow::BackgroundHints LingmoWindow::backgroundHints() const
{
    return d->backgroundHints;
}

void LingmoWindow::setBackgroundHints(BackgroundHints hints)
{
    if (d->backgroundHints == hints) {
        return;
    }
    d->backgroundHints = hints;

    auto prefix = QStringLiteral("");
    if (d->backgroundHints == LingmoWindow::SolidBackground) {
        prefix = QStringLiteral("solid/");
    }
    d->dialogBackground->setImagePath(prefix + QStringLiteral("dialogs/background"));

    Q_EMIT backgroundHintsChanged();
}

qreal LingmoWindow::topPadding() const
{
    return d->dialogBackground->topMargin();
}

qreal LingmoWindow::bottomPadding() const
{
    return d->dialogBackground->bottomMargin();
}

qreal LingmoWindow::leftPadding() const
{
    return d->dialogBackground->leftMargin();
}

qreal LingmoWindow::rightPadding() const
{
    return d->dialogBackground->rightMargin();
}
}

#include "moc_lingmowindow.cpp"
