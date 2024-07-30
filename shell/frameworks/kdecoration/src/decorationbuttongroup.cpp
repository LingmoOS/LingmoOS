/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#include "decorationbuttongroup.h"
#include "decoration.h"
#include "decorationbuttongroup_p.h"
#include "decorationsettings.h"

#include <QDebug>
#include <QGuiApplication>

namespace KDecoration2
{
DecorationButtonGroup::Private::Private(Decoration *decoration, DecorationButtonGroup *parent)
    : decoration(decoration)
    , spacing(0.0)
    , q(parent)
{
}

DecorationButtonGroup::Private::~Private() = default;

void DecorationButtonGroup::Private::setGeometry(const QRectF &geo)
{
    if (geometry == geo) {
        return;
    }
    geometry = geo;
    Q_EMIT q->geometryChanged(geometry);
}

namespace
{
static bool s_layoutRecursion = false;
}

void DecorationButtonGroup::Private::updateLayout()
{
    if (s_layoutRecursion) {
        return;
    }
    s_layoutRecursion = true;
    const QPointF &pos = geometry.topLeft();
    // first calculate new size
    qreal height = 0;
    qreal width = 0;
    for (auto it = buttons.constBegin(); it != buttons.constEnd(); ++it) {
        if (!(*it)->isVisible()) {
            continue;
        }
        height = qMax(height, qreal((*it)->size().height()));
        width += (*it)->size().width();
        if (it + 1 != buttons.constEnd()) {
            width += spacing;
        }
    }
    setGeometry(QRectF(pos, QSizeF(width, height)));

    QGuiApplication* app = qobject_cast<QGuiApplication*>(QCoreApplication::instance());
    const auto layoutDirection = app ? app->layoutDirection() : Qt::LeftToRight;

    qreal leftPosition = pos.x();
    qreal rightPosition = pos.x() + width;

    if (layoutDirection == Qt::LeftToRight)
        for (auto button : std::as_const(buttons)) {
            if (!button->isVisible()) {
                continue;
            }
            const auto size = button->size();
            const auto buttonPos = QPointF(leftPosition, pos.y());
            button->setGeometry(QRectF(buttonPos, size));
            leftPosition += size.width() + spacing;
        }
    else if (layoutDirection == Qt::RightToLeft)
        for (auto button : std::as_const(buttons)) {
            if (!button->isVisible()) {
                continue;
            }
            const auto size = button->size();
            const auto buttonPos = QPointF(rightPosition - size.width(), pos.y());
            button->setGeometry(QRectF(buttonPos, size));
            rightPosition -= size.width() + spacing;
        }
    else {
        qCritical() << "There's an unhandled layout direction! This is likely an issue of KDecoration2 not being updated to handle it\n"
                    << "or the application having an invalid layout direction set. Either way, this is a critical bug.";
    }

    s_layoutRecursion = false;
}

DecorationButtonGroup::DecorationButtonGroup(Decoration *parent)
    : QObject(parent)
    , d(new Private(parent, this))
{
}

DecorationButtonGroup::DecorationButtonGroup(DecorationButtonGroup::Position type,
                                             Decoration *parent,
                                             std::function<DecorationButton *(DecorationButtonType, Decoration *, QObject *)> buttonCreator)
    : QObject(parent)
    , d(new Private(parent, this))
{
    auto createButtons = [this, buttonCreator, type] {
        const Qt::LayoutDirection layoutDirection = QGuiApplication::layoutDirection();
        const DecorationSettings *settings = d->decoration->settings().get();
        const auto &buttons =
            (type == Position::Left) ?
                (layoutDirection == Qt::LeftToRight ? settings->decorationButtonsLeft() : settings->decorationButtonsRight()) :
                (layoutDirection == Qt::LeftToRight ? settings->decorationButtonsRight() : settings->decorationButtonsLeft());
        for (DecorationButtonType type : buttons) {
            if (DecorationButton *b = buttonCreator(type, d->decoration, this)) {
                addButton(b);
            }
        }
    };
    createButtons();
    auto changed = type == Position::Left ? &DecorationSettings::decorationButtonsLeftChanged : &DecorationSettings::decorationButtonsRightChanged;
    connect(parent->settings().get(), changed, this, [this, createButtons] {
        qDeleteAll(d->buttons);
        d->buttons.clear();
        createButtons();
    });
}

DecorationButtonGroup::~DecorationButtonGroup() = default;

Decoration *DecorationButtonGroup::decoration() const
{
    return d->decoration;
}

QRectF DecorationButtonGroup::geometry() const
{
    return d->geometry;
}

bool DecorationButtonGroup::hasButton(DecorationButtonType type) const
{
    // TODO: check for deletion of button
    auto it = std::find_if(d->buttons.begin(), d->buttons.end(), [type](DecorationButton *button) {
        return button->type() == type;
    });
    return it != d->buttons.end();
}

qreal DecorationButtonGroup::spacing() const
{
    return d->spacing;
}

QPointF DecorationButtonGroup::pos() const
{
    return d->geometry.topLeft();
}

void DecorationButtonGroup::setPos(const QPointF &pos)
{
    if (d->geometry.topLeft() == pos) {
        return;
    }
    d->setGeometry(QRectF(pos, d->geometry.size()));
    d->updateLayout();
}

void DecorationButtonGroup::setSpacing(qreal spacing)
{
    if (d->spacing == spacing) {
        return;
    }
    d->spacing = spacing;
    Q_EMIT spacingChanged(d->spacing);
    d->updateLayout();
}

void DecorationButtonGroup::addButton(DecorationButton *button)
{
    Q_ASSERT(button);
    connect(button, &DecorationButton::visibilityChanged, this, [this]() {
        d->updateLayout();
    });
    connect(button, &DecorationButton::geometryChanged, this, [this]() {
        d->updateLayout();
    });
    d->buttons.append(button);
    d->updateLayout();
}

QList<DecorationButton *> DecorationButtonGroup::buttons() const
{
    return d->buttons;
}

void DecorationButtonGroup::removeButton(DecorationButtonType type)
{
    bool needUpdate = false;
    auto it = d->buttons.begin();
    while (it != d->buttons.end()) {
        if ((*it)->type() == type) {
            it = d->buttons.erase(it);
            needUpdate = true;
        } else {
            it++;
        }
    }
    if (needUpdate) {
        d->updateLayout();
    }
}

void DecorationButtonGroup::removeButton(DecorationButton *button)
{
    bool needUpdate = false;
    auto it = d->buttons.begin();
    while (it != d->buttons.end()) {
        if (*it == button) {
            it = d->buttons.erase(it);
            needUpdate = true;
        } else {
            it++;
        }
    }
    if (needUpdate) {
        d->updateLayout();
    }
}

void DecorationButtonGroup::paint(QPainter *painter, const QRect &repaintArea)
{
    const auto &buttons = d->buttons;
    for (auto button : buttons) {
        if (!button->isVisible()) {
            continue;
        }
        button->paint(painter, repaintArea);
    }
}

} // namespace

#include "moc_decorationbuttongroup.cpp"
