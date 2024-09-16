/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2015 Thomas Lübking <thomas.luebking@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
/*global effect, effects, animate, animationTime, Effect, QEasingCurve */

"use strict";

var eyeOnScreenEffect = {
    duration: animationTime(140),
    loadConfig: function () {
        eyeOnScreenEffect.duration = animationTime(140);
    },
    delevateWindow: function(window) {
        effect.showingDesktopEnd(window.elevatedByEyeOnScreen);
        if (window.desktopWindow) {
            return;
        } else if (window.elevatedByEyeOnScreen) {
            window.elevatedByEyeOnScreen = false;
        }
    },
    slurp: function(showing, w) {
        if (!w.visible || !(showing || w.slurpedByEyeOnScreen)) {
            return;
        }
        w.slurpedByEyeOnScreen = showing;
        if (w.desktopWindow || w.dock) {
            return;
        } else {
            if (showing) {
                w.elevatedByEyeOnScreen = true;
                var windowRect = w.geometry;
                var scaleSize = 70;
                animate({
                    // this is a HACK - we need to trigger an animationEnded to delevate the dock in time, or it'll flicker :-(
                    // TODO? "var timer = new QTimer;" causes an error in effect scripts
                    window: w,
                    animations: [
                        {
                            type: Effect.Size,
                            curve: QEasingCurve.Linear,
                            duration: eyeOnScreenEffect.duration,
                            to: {
                                value1: windowRect.width * scaleSize / 100,
                                value2: windowRect.height * scaleSize / 100
                            },
                            from: {
                                value1: windowRect.width,
                                value2: windowRect.height
                            }
                        },
                        {
                            type: Effect.Opacity,
                            curve: QEasingCurve.Linear,
                            duration: eyeOnScreenEffect.duration,
                            to: 0.0
                        }
                    ]
                });
            } else {
                w.elevatedByEyeOnScreen = false;
                var windowRect = w.geometry;
                var scaleSize = 70;
                animate({
                    window: w,
                    duration: eyeOnScreenEffect.duration,
                    delay: eyeOnScreenEffect.duration,
                    animations: [
                        {
                            type: Effect.Size,
                            from: {
                                value1: windowRect.width * scaleSize / 100,
                                value2: windowRect.height * scaleSize / 100
                            },
                            to: {
                                value1: windowRect.width,
                                value2: windowRect.height
                            }
                        },{
                            type: Effect.Opacity,
                            curve: QEasingCurve.OutCubic,
                            duration: eyeOnScreenEffect.duration,
                            from: 0.0,
                            to: 1.0
                        }
                    ]
                });
            }
        }
    },
    init: function () {
        eyeOnScreenEffect.loadConfig();
        effect.animationEnded.connect(eyeOnScreenEffect.delevateWindow);
        effects.showingDesktopChangedEx.connect(eyeOnScreenEffect.slurp);
    }
};

eyeOnScreenEffect.init();
