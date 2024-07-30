/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef KANIMATEDBUTTON_H
#define KANIMATEDBUTTON_H

#include <QToolButton>
#include <kwidgetsaddons_export.h>
#include <memory>

/**
 * @class KAnimatedButton kanimatedbutton.h KAnimatedButton
 *
 * @short An extended version of QToolButton which can display an animation.
 *
 * This widget extends QToolButton with the ability to display an animation.
 * All you need to do is pass along a path to a file containing an animation,
 * it can be anything supported by QMovie, or a picture containing all the
 * frames of the animation next to each other (each frame being assumed of
 * having the same size).
 *
 * @author Kurt Granroth <granroth@kde.org>
 */
class KWIDGETSADDONS_EXPORT KAnimatedButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(QString animationPath READ animationPath WRITE setAnimationPath)

public:
    /**
     * Construct an animated tool button.
     *
     * @param parent The parent widget
     */
    explicit KAnimatedButton(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~KAnimatedButton() override;

    /**
     * Returns the path used to load the animation
     */
    QString animationPath() const;

    /**
     * Sets the path to the file which contains the animation to load.
     *
     * @param path The path of the file containing the animation
     */
    void setAnimationPath(const QString &path);

public Q_SLOTS:
    /**
     * Starts the animation from frame 1
     */
    void start();

    /**
     * Stops the animation.  This will also reset the widget to frame 1.
     */
    void stop();

private:
    std::unique_ptr<class KAnimatedButtonPrivate> const d;

    Q_DISABLE_COPY(KAnimatedButton)
};

#endif //  KANIMATEDBUTTON_H
