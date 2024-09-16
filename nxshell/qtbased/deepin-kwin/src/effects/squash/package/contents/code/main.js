/*
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2018 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

"use strict";

var squashEffect = {
    loadConfig: function () {
        squashEffect.miniSize = effect.readConfig("miniSize", 10);
        squashEffect.miniDuration = effect.readConfig("miniDuration", 360) / 2;
        squashEffect.miniCurve = effect.readConfig("miniCurve", 10);
        squashEffect.unminiSize = effect.readConfig("unminiSize", 10);
        squashEffect.unminiDuration = effect.readConfig("unminiDuration", 360) / 2;
        squashEffect.unminiCurve = effect.readConfig("unminiCurve", 10);
    },
    slotWindowMinimized: function (window, pos) {
        if (effects.hasActiveFullScreenEffect) {
            return;
        }

        // If the window doesn't have an icon in the task manager,
        // don't animate it.
        var iconRect = window.iconGeometry;
        var windowRect = window.geometry;

        if (window.unminimizeAnimation) {
            if (redirect(window.unminimizeAnimation, Effect.Backward)) {
                return;
            }
            cancel(window.unminimizeAnimation);
            delete window.unminimizeAnimation;
        }

        if (window.minimizeAnimation) {
            if (redirect(window.minimizeAnimation, Effect.Forward)) {
                return;
            }
            cancel(window.minimizeAnimation);
        }
        pos.x += windowRect.width * squashEffect.miniSize / 100 / 2;
        pos.y += windowRect.height * squashEffect.miniSize / 100 / 2;
        window.minimizeAnimation = animate({
            window: window,
            curve: squashEffect.miniCurve,
            duration: animationTime(squashEffect.miniDuration),
            animations: [
                {
                    type: Effect.Size,
                    from: {
                        value1: windowRect.width,
                        value2: windowRect.height
                    },
                    to: {
                        value1: windowRect.width * squashEffect.miniSize / 100,
                        value2: windowRect.height * squashEffect.miniSize / 100
                    }
                },
                {
                    type: Effect.Position,
                    to: {
                        value1: pos.x,
                        value2: pos.y
                    }
                },
                {
                    type: Effect.Opacity,
                    from: 1.0,
                    to: 0.0
                }
            ]
        });
    },
    slotWindowUnminimized: function (window, pos) {
        if (effects.hasActiveFullScreenEffect) {
            return;
        }

        // If the window doesn't have an icon in the task manager,
        // don't animate it.
        var iconRect = window.iconGeometry;
        if (iconRect.width == 0 || iconRect.height == 0) {
            return;
        }
        if (window.minimizeAnimation) {
            if (redirect(window.minimizeAnimation, Effect.Backward)) {
                return;
            }
            cancel(window.minimizeAnimation);
            delete window.minimizeAnimation;
        }

        if (window.unminimizeAnimation) {
            if (redirect(window.unminimizeAnimation, Effect.Forward)) {
                return;
            }
            cancel(window.unminimizeAnimation);
        }

        var windowRect = window.geometry;
        var m_pos = {x: iconRect.x, y: iconRect.y};
        m_pos.x = m_pos.x + iconRect.width / 2 + window.width * squashEffect.unminiSize / 100 / 2;
        m_pos.y = m_pos.y + iconRect.height / 2 + window.height * squashEffect.unminiSize / 100 / 2;
        window.unminimizeAnimation = animate({
            window: window,
            curve: squashEffect.unminiCurve,
            duration: animationTime(squashEffect.unminiDuration),
            animations: [
                {
                    type: Effect.Size,
                    from: {
                        value1: windowRect.width * squashEffect.unminiSize / 100,
                        value2: windowRect.height * squashEffect.unminiSize / 100
                    },
                    to: {
                        value1: windowRect.width,
                        value2: windowRect.height
                    }
                },
                {
                    type: Effect.Position,
                    from: {
                        value1: m_pos.x,
                        value2: m_pos.y
                    }
                },
                {
                    type: Effect.Opacity,
                    from: 0.0,
                    to: 1.0
                }
            ]
        });
    },
    init: function () {
        squashEffect.loadConfig();
        effect.configChanged.connect(squashEffect.loadConfig);
        effects.windowMinimizedAnimation.connect(squashEffect.slotWindowMinimized);
        effects.windowUnminimized.connect(squashEffect.slotWindowUnminimized);
    }
};

squashEffect.init();
