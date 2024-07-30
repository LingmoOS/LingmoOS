/*
    SPDX-FileCopyrightText: 2013 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_ANIMATION_H
#define KATE_ANIMATION_H

#include <QObject>
#include <QPointer>

class QTimer;

class KMessageWidget;
class KateFadeEffect;
/**
 * This class provides a fade in/out effect for KMessageWidget%s.
 * Example:
 * \code
 * KateAnimation* animation = new KateAnimation(someMessageWidget);
 * animation->show();
 * //...
 * animation->hide();
 * \endcode
 */
class KateAnimation : public QObject
{
    Q_OBJECT

public:
    /**
     * The type of supported animation effects
     */
    enum EffectType {
        FadeEffect = 0, ///< fade in/out
        GrowEffect ///< grow / shrink
    };

public:
    /**
     * Constructor.
     */
    KateAnimation(KMessageWidget *widget, EffectType effect);

    /**
     * Returns true, if the hide animation is running, otherwise false.
     */
    bool isHideAnimationRunning() const;

    /**
     * Returns true, if the how animation is running, otherwise false.
     */
    bool isShowAnimationRunning() const;

public Q_SLOTS:
    /**
     * Call to hide the widget.
     */
    void hide();

    /**
     * Call to show and fade in the widget
     */
    void show();

Q_SIGNALS:
    /**
     * This signal is emitted when the hiding animation is finished.
     * At this point, the associated widget is hidden.
     */
    void widgetHidden();

    /**
     * This signal is emitted when the showing animation is finished.
     * At this point, the associated widget is hidden.
     */
    void widgetShown();

private:
    QPointer<KMessageWidget> m_widget; ///< the widget to animate
    KateFadeEffect *m_fadeEffect; ///< the fade effect
};

#endif
