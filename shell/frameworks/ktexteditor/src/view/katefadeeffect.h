/*
    SPDX-FileCopyrightText: 2013 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_FADE_EFFECT_H
#define KATE_FADE_EFFECT_H

#include <QObject>
#include <QPointer>

class QWidget;
class QTimeLine;
class QGraphicsOpacityEffect;
/**
 * This class provides a fade in/out effect for arbitrary QWidget%s.
 * Example:
 * \code
 * KateFadeEffect* fadeEffect = new KateFadeEffect(someWidget);
 * fadeEffect->fadeIn();
 * //...
 * fadeEffect->fadeOut();
 * \endcode
 */
class KateFadeEffect : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor.
     * By default, the widget is fully opaque (opacity = 1.0).
     */
    explicit KateFadeEffect(QWidget *widget = nullptr);

    /**
     * Check whether the hide animation started by calling fadeOut()
     * is still running. If animations are disabled, this function always
     * returns @e false.
     */
    bool isHideAnimationRunning() const;

    /**
     * Check whether the show animation started by calling fadeIn()
     * is still running. If animations are disabled, this function always
     * returns @e false.
     */
    bool isShowAnimationRunning() const;

public Q_SLOTS:
    /**
     * Call to fade out and hide the widget.
     */
    void fadeOut();

    /**
     * Call to show and fade in the widget
     */
    void fadeIn();

Q_SIGNALS:
    /**
     * This signal is emitted when the fadeOut animation is finished, started by
     * calling fadeOut(). If animations are disabled, this signal is
     * emitted immediately.
     */
    void hideAnimationFinished();

    /**
     * This signal is emitted when the fadeIn animation is finished, started by
     * calling fadeIn(). If animations are disabled, this signal is
     * emitted immediately.
     */
    void showAnimationFinished();

protected Q_SLOTS:
    /**
     * Helper to update opacity value
     */
    void opacityChanged(qreal value);

    /**
     * When the animation is finished, hide the widget if fading out.
     */
    void animationFinished();

private:
    QPointer<QWidget> m_widget; ///< the fading widget
    QTimeLine *m_timeLine; ///< update time line
    QPointer<QGraphicsOpacityEffect> m_effect; ///< graphics opacity effect
};

#endif
