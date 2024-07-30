/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Felix Ernst <felixernst@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KCONTEXTUALHELPBUTTON_H
#define KCONTEXTUALHELPBUTTON_H

#include <kwidgetsaddons_export.h>

#include <QToolButton>

#include <memory>

/**
 * @class KContextualHelpButton kcontextualhelpbutton.h KContextualHelpButton
 *
 * @brief An icon-only button for showing contextually relevant help or explanations.
 *
 * KContextualHelpButton allows hiding text of any length behind a small icon-only button. Hovering over the button with a mouse cursor or pressing the button
 * will show the text. It can contain images and links. It is accessible by keyboard and to screen readers.
 *
 * **When to Use**
 *
 * Sometimes users require help or an explanation, even experts. This help should be readily available exactly where and when a user needs it.
 * A KContextualHelpButton makes clear that inline help is available without really cluttering up the user interface. In that sense it is superior to setting
 * tooltips which are completely invisible until invoked and therefore easy to miss. Especially for touch users the KContextualHelpButton is preferable.
 *
 * **When Not to Use**
 *
 * If the text is important for the user to understand and can be kept very short, place it inline below the control it affects.
 * If there is no room to put this KContextualHelpButton, use QWidget::setToolTip() for shorter texts and QWidget::SetWhatsThis() for longer text or text that
 * should contain hyperlinks. If your software is not already using the KXmlGui framework, consider using KToolTipHelper from KXmlGui to make the whatsThis()
 * help more discoverable.
 *
 * **How to Use**
 *
 * The most simple way is creating it like any other button and then setting the help text:
 *
 * \code
 * auto contextualHelpButton = new KContextualHelpButton{this};
 * contextualHelpButton->setContextualHelpText(xi18nc("@info", "<para>A nicely formatted help text. Notice the 'para' tags at the side!</para>"));
 * \endcode
 *
 * The most common use of this component might be on settings pages which oftentimes use a QFormLayout.
 * Here is an example on how to use it next to a QCheckBox. Unfortunately we need to make the KContextualHelpButton aware of the QCheckBox here or it will
 * increase the height of the row in the QFormLayout which might look bad.
 * The help text in this example contains an image and a working hyperlink:
 *
 * \code
 * auto formLayout = new QFormLayout(this);
 * auto checkBox = new QCheckBox(i18nc("@option:check", "Check this box"), this);
 * auto contextualHelpButton = new KContextualHelpButton{
 *      xi18nc("@info",
 *             "<para>A help text containing an icon <img src=':/icon.png'> and a <a href=\"http://www.kde.org\">link</a>.</para>"),
 *      checkBox,
 *      this};
 * auto rowLayout = new QHBoxLayout{this};
 * rowLayout->addWidget(checkBox);
 * rowLayout->addWidget(contextualHelpButton);
 * formLayout->addRow(i18nc("@title:group", "Row label:"), rowLayout);
 * \endcode
 *
 * This class is meant to be kept somewhat consistent with the QML equivalent which is also called ContextualHelpButton.
 *
 * @since 6.2
 */
class KWIDGETSADDONS_EXPORT KContextualHelpButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(QString contextualHelpText READ contextualHelpText WRITE setContextualHelpText NOTIFY contextualHelpTextChanged)

public:
    /**
     * @param contextualHelpText    The text to show when hovering or clicking this button. Consider formatting this nicely using xi18nc().
     * @param heightHintWidget      The KContextualHelpButton will report the \p heightHintWidget 's sizeHint().height() as its own sizeHint().height().
     *                              This is useful to make sure that adding this KContextualHelpButton to a layout will not increase the layout's total height.
     * @param parent                The parent widget that gets ownership over the lifetime of this KContextualHelpButton.
     */
    explicit KContextualHelpButton(const QString &contextualHelpText, const QWidget *heightHintWidget, QWidget *parent);

    /** @see KContextualHelpButton::ContextualHelpButton() */
    explicit KContextualHelpButton(QWidget *parent = nullptr);

    ~KContextualHelpButton() override;

    /**
     * Sets the text to show when hovering or pressing this button. Consider formatting the text nicely using xi18nc().
     */
    void setContextualHelpText(const QString &contextualHelpText);

    /**
     * @returns the help text which is shown when hovering or pressing this button.
     */
    QString contextualHelpText() const;

    /**
     * The KContextualHelpButton will report the \p heightHintWidget 's sizeHint().height() as its own sizeHint().height().
     * This is useful to make sure that adding this KContextualHelpButton to a layout will not increase the layout's total height.
     */
    void setHeightHintWidget(const QWidget *heightHintWidget);

    /** @see setHeightHintWidget() */
    const QWidget *heightHintWidget() const;

    /** This override is an implementation detail of setHeightHintWidget() */
    QSize sizeHint() const override;

Q_SIGNALS:
    void contextualHelpTextChanged(const QString &newContextualHelpText);

private:
    std::unique_ptr<class KContextualHelpButtonPrivate> const d_ptr;
};

#endif // KCONTEXTUALHELPBUTTON_H
