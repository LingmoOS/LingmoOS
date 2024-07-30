/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Felix Ernst <felixernst@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kcontextualhelpbutton.h"

#include <QLabel>
#include <QPointer>
#include <QTextDocumentFragment>
#include <QWidgetAction>

/**
 * The private class of KContextualHelpButton used for the PIMPL idiom.
 * \internal
 */
class KContextualHelpButtonPrivate
{
public:
    /** @see KContextualHelpButton::KContextualHelpButton() */
    explicit KContextualHelpButtonPrivate(KContextualHelpButton *q, const QString &contextualHelpText, const QWidget *heightHintWidget);

    /** @see KContextualHelButton::setContextualHelpText() */
    void setContextualHelpText(const QString &contextualHelpText);

    /** @see KContextualHelButton::contextualHelpText() */
    QString contextualHelpText() const;

    /** @see KContextualHelpButton::setHeightHintWidget() */
    void setHeightHintWidget(const QWidget *heightHintWidget);

    /** @see KContextualHelpButton::setHeightHintWidget() */
    const QWidget *heightHintWidget() const;

    /**
     * A helper method called from KContextualHelpButton::sizeHint().
     *
     * @returns the preferredSize based on m_heightHintWidget and fallbackSize.
     * @param fallbackSize  Used as the width. Also used as the height if there is no m_heightHintWidget.
     *
     * @see QWidget::sizeHint()
     * @see KContextualHelpButton::setHeightHintWidget()
     */
    QSize preferredSize(const QSize &fallbackSize) const;

private:
    KContextualHelpButton *const q_ptr;

    /** The popup showing the contextualHelpText. */
    QLabel *m_popupLabel = nullptr;

    /** @see KContextualHelpButton::setHeightHintWidget() */
    QPointer<const QWidget> m_heightHintWidget;
};

KContextualHelpButton::KContextualHelpButton(const QString &contextualHelpText, const QWidget *heightHintWidget, QWidget *parent)
    : QToolButton{parent}
    , d_ptr(std::make_unique<KContextualHelpButtonPrivate>(this, contextualHelpText, heightHintWidget))
{
}

KContextualHelpButton::KContextualHelpButton(QWidget *parent)
    : KContextualHelpButton{QString{}, nullptr, parent}
{
}

KContextualHelpButtonPrivate::KContextualHelpButtonPrivate(KContextualHelpButton *q, const QString &contextualHelpText, const QWidget *heightHintWidget)
    : q_ptr{q}
    , m_heightHintWidget{heightHintWidget}
{
    q_ptr->setIcon(QIcon::fromTheme(QStringLiteral("help-contextual")));
    q_ptr->setAutoRaise(true);
    q_ptr->setCursor(Qt::WhatsThisCursor);
    // i18n: This text will only ever be read by assistive technology like screen readers for visually-impaired. It shows up nowhere on the visible user
    // interface. Usually we start button labels with a verb, however the help text contained within this button will automatically be read by the screen
    // reader without requiring additional user input. In this sense the label here is more of an announcement that the text read afterwards is help relevant
    // to the current context.
    q_ptr->setAccessibleName(QObject::tr("Contextual Help", "@action:button accessible name"));
    q_ptr->setAttribute(Qt::WidgetAttribute::WA_CustomWhatsThis); // Makes sure clicking this button in whatsThisMode triggers it instead of doing nothing.

    auto popup = new QWidgetAction{q_ptr};
    q_ptr->addAction(popup);
    q_ptr->setPopupMode(QToolButton::InstantPopup);

    m_popupLabel = new QLabel{q_ptr};
    m_popupLabel->setWordWrap(true);
    m_popupLabel->setTextInteractionFlags(Qt::TextBrowserInteraction); // Makes the label and links within it keyboard-accessible.
    m_popupLabel->setOpenExternalLinks(true);
    popup->setDefaultWidget(m_popupLabel);

    setContextualHelpText(contextualHelpText);
}

KContextualHelpButton::~KContextualHelpButton() = default;

void KContextualHelpButton::setContextualHelpText(const QString &contextualHelpText)
{
    d_ptr->setContextualHelpText(contextualHelpText);
}

void KContextualHelpButtonPrivate::setContextualHelpText(const QString &contextualHelpText)
{
    if (contextualHelpText == q_ptr->toolTip() && contextualHelpText == m_popupLabel->text()) {
        return;
    }
    q_ptr->setToolTip(contextualHelpText);
    q_ptr->setAccessibleDescription(QTextDocumentFragment::fromHtml(contextualHelpText).toPlainText()); // Makes sure html tags aren't read out.
    m_popupLabel->setText(contextualHelpText);
    Q_EMIT q_ptr->contextualHelpTextChanged(contextualHelpText);
}

QString KContextualHelpButton::contextualHelpText() const
{
    return d_ptr->contextualHelpText();
}

QString KContextualHelpButtonPrivate::contextualHelpText() const
{
    return m_popupLabel->text();
}

void KContextualHelpButton::setHeightHintWidget(const QWidget *heightHintWidget)
{
    d_ptr->setHeightHintWidget(heightHintWidget);
}

void KContextualHelpButtonPrivate::setHeightHintWidget(const QWidget *heightHintWidget)
{
    m_heightHintWidget = heightHintWidget;
}

const QWidget *KContextualHelpButton::heightHintWidget() const
{
    return d_ptr->heightHintWidget();
}

const QWidget *KContextualHelpButtonPrivate::heightHintWidget() const
{
    return m_heightHintWidget;
}

QSize KContextualHelpButton::sizeHint() const
{
    return d_ptr->preferredSize(QToolButton::sizeHint());
}

QSize KContextualHelpButtonPrivate::preferredSize(const QSize &fallbackSize) const
{
    return {fallbackSize.width(), m_heightHintWidget ? m_heightHintWidget->sizeHint().height() : fallbackSize.height()};
}
