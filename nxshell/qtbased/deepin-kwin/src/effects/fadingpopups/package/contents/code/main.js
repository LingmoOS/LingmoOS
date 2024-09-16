/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2018 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

"use strict";

var blacklist = [
    // ignore black background behind lockscreen
    "ksmserver ksmserver",
    // The logout screen has to be animated only by the logout effect.
    "ksmserver-logout-greeter ksmserver-logout-greeter",
    // The lockscreen isn't a popup window
    "kscreenlocker_greet kscreenlocker_greet",
    // KDE Plasma splash screen has to be animated only by the login effect.
    "ksplashqml ksplashqml"
];

var dockPos = 2;

function calcDis(x1, x2, y1, y2) {
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

function calcDiagonalPosPadding(cpos, windowRect, padding) {
    var point = {x: cpos.x, y: cpos.y};
    var x = windowRect.x;
    var y = windowRect.y;
    var width = windowRect.width;
    var height = windowRect.height;
    if (calcDis(cpos.x, x, cpos.y, y) <= padding) {  // topleft
        point.x = x + width * fadingPopupsEffect.addUnmanagedSize / 100 / 2;
        point.y = y + height * fadingPopupsEffect.addUnmanagedSize / 100 / 2;
    } else if (calcDis(cpos.x, x + width, cpos.y, y + height) <= padding) {  //bottom right
        point.x = x + width -  width * fadingPopupsEffect.addUnmanagedSize / 100 / 2;
        point.y = y + height - height * fadingPopupsEffect.addUnmanagedSize / 100 / 2;
    } else if (calcDis(cpos.x, x + width, cpos.y, y) <= padding) {  //top right
        point.x = x + width -  width * fadingPopupsEffect.addUnmanagedSize / 100 /2;
        point.y = y + height * fadingPopupsEffect.addUnmanagedSize / 100 / 2;
    } else if (calcDis(cpos.x, x, cpos.y, y + height) <= padding) { //bottom Left
        point.x = x + width * fadingPopupsEffect.addUnmanagedSize / 100 / 2;
        point.y = y + height - height * fadingPopupsEffect.addUnmanagedSize / 100 / 2;
    } else {
        point.x = Number.MAX_VALUE;
        point.y = Number.MAX_VALUE;
    }
    return point;
}

function calDockPos(windowRect) {
    dockPos = effect.onDockChanged();
    //dbus position: 0 top, 1 right, 2 bottom, 3 left
    var point = {x: windowRect.x, y: windowRect.y};
    if (dockPos == 2) {
        point.y = windowRect.y + windowRect.height;
        return point;
    } else if (dockPos == 1) {
        point.x = windowRect.x + windowRect.width;
        return point;
    } else
        return point;
}

function calcVerticalPos(cpos, windowRect) {
    var point = {x: windowRect.x + windowRect.width / 2, y: windowRect.y};
    if ((cpos.y - windowRect.y) * (cpos.y - windowRect.y) <  (windowRect.y + windowRect.height - cpos.y) * (windowRect.y + windowRect.height - cpos.y)) {
        point.y = windowRect.y;
    } else {
        point.y = windowRect.y + windowRect.height;
    }
    return point;
}

function isPopupWindow(window) {
    // If the window is blacklisted, don't animate it.
    if (blacklist.indexOf(window.windowClass) != -1) {
        return false;
    }

    // Animate combo box popups, tooltips, popup menus, etc.
    if (window.popupWindow) {
        return true;
    }

    // Maybe the outline deserves its own effect.
    if (window.outline) {
        return true;
    }

    // Override-redirect windows are usually used for user interface
    // concepts that are expected to be animated by this effect, e.g.
    // popups that contain window thumbnails on X11, etc.
    if (!window.managed) {
        // Some utility windows can look like popup windows (e.g. the
        // address bar dropdown in Firefox), but we don't want to fade
        // them because the fade effect didn't do that.
        if (window.utility) {
            return false;
        }

        return true;
    }

    // Previously, there was a "monolithic" fade effect, which tried to
    // animate almost every window that was shown or hidden. Then it was
    // split into two effects: one that animates toplevel windows and
    // this one. In addition to popups, this effect also animates some
    // special windows(e.g. notifications) because the monolithic version
    // was doing that.
    if (window.dock || window.splash || window.toolbar
            || window.notification || window.onScreenDisplay
            || window.criticalNotification
            || window.appletPopup) {
        return true;
    }

    return false;
}

function isFadingWindow(window) {
    if (window.windowClass == "deepin-screen-recorder deepin-screen-recorder" || window.windowClass == "dde-lock org.deepin.dde.lock")
        return false;
    if (window.tooltip)
        return false;
    return true;
}

function calcTooltipPos(cpos, windowRect) {
    var point = {x: cpos.x, y: cpos.y};
    var x = windowRect.x;
    var y = windowRect.y;
    var width = windowRect.width;
    var height = windowRect.height;
    if (cpos.x > x && cpos.x < x + width && cpos.y > y + height) { // bottom
        point.y = y + height - height * fadingPopupsEffect.addUnmanagedSize / 100 / 2;
    } else if (cpos.y > y && cpos.y < y + height && cpos.x > x + width) { // right
        point.x = x + width - width * fadingPopupsEffect.addUnmanagedSize / 100 / 2;
    } else if (cpos.y > y && cpos.y < y + height && cpos.x < x) { // left
        point.x = x + width * fadingPopupsEffect.addUnmanagedSize / 100 / 2;
    } else {  //top
        point.y = y + height * fadingPopupsEffect.addUnmanagedSize / 100 / 2;
    }
    return point;
}

function calDockClosedPos(windowRect) {
    var point = {x: windowRect.x, y: windowRect.y};
    var x = windowRect.x;
    var y = windowRect.y;
    var width = windowRect.width;
    var height = windowRect.height;
    if (dockPos == 2) {
        point.x += width * fadingPopupsEffect.closedSmallLauncherSize / 100 / 2;
        point.y = y + height - height * fadingPopupsEffect.closedSmallLauncherSize / 100 / 2;
        return point;
    } else if (dockPos == 1) {
        point.x = x + width - width * fadingPopupsEffect.closedSmallLauncherSize / 100 / 2;
        point.y += height * fadingPopupsEffect.closedSmallLauncherSize / 100 / 2;
        return point;
    } else {
        point.x += width * fadingPopupsEffect.closedSmallLauncherSize / 100 / 2;
        point.y += height * fadingPopupsEffect.closedSmallLauncherSize / 100 / 2;
        return point;
    }
}

var fadingPopupsEffect = {
    calcDiagonalPos: function (window, cpos, windowRect) {
        var point = {x: cpos.x, y: cpos.y};
        var x = windowRect.x;
        var y = windowRect.y;
        var width = windowRect.width;
        var height = windowRect.height;
        var padding = 5;
        if (window.windowClass == "dde-launcher dde-launcher" && !window.popupMenu) {
            point = calDockPos(windowRect);
            point = calcDiagonalPosPadding(point, windowRect, padding);
            return point;
        } else { // right click menu
            point = calcDiagonalPosPadding(cpos, windowRect, padding);
        }
        if (point.x ==  Number.MAX_VALUE &&  point.y == Number.MAX_VALUE) {
            if (!window.popupMenu) { //other unmanaged windows from dock
                point = calcTooltipPos(cpos, windowRect);
                return point;
            }
            if (cpos.x > x && cpos.x < x + width && cpos.y > y + 2 && cpos.y < y + height - 2) { //cursor in unmanaged window
                point.x = cpos.x;
                point.y = cpos.y;
                return point;
            } else if (cpos.x > x && cpos.x < x + width && cpos.y + 1 < y) {  //button menu of file-manager
                point.x = cpos.x;
                if (cpos.y - y < y + height - cpos.y) {
                    point.y = y + height * fadingPopupsEffect.addUnmanagedSize / 100 / 2;
                } else {
                    point.y = y + height - height * fadingPopupsEffect.addUnmanagedSize / 100 / 2;
                }
                return point;
            } else {
                if (cpos.x < x - 2 || cpos.x > x + width + 2) { //secondary menu
                    point.x = x + width / 2;
                    point.y = y + height / 2;
                    return point;
                } else { // menu at edge
                    point.x = cpos.x;
                    point.y = cpos.y;
                    if (cpos.y - y < y + height - cpos.y) {
                        point.y = y + height * fadingPopupsEffect.addUnmanagedSize / 100 / 2;
                    } else {
                        point.y = y + height - height * fadingPopupsEffect.addUnmanagedSize / 100 / 2;
                    }
                    return point;
                }
            }
        }
        return point;
    },
    loadConfig: function () {
        fadingPopupsEffect.fadeInDuration = animationTime(150);
        fadingPopupsEffect.fadeOutDuration = animationTime(150) * 4;
        fadingPopupsEffect.closedSmallLauncherSize = effect.readConfig("closedSmallLauncherSize", 40);

        fadingPopupsEffect.addUnmanagedDuration = effect.readConfig("addUnmanagedDuration", 300) / 2;
        fadingPopupsEffect.addUnmanagedSize = effect.readConfig("addUnmanagedSize", 40);
        fadingPopupsEffect.addUnmanagedCurve = effect.readConfig("addUnmanagedCurve", 10);

        fadingPopupsEffect.closedUnmanagedDuration = effect.readConfig("closedUnmanagedDuration", 360) / 2;
        fadingPopupsEffect.closedUnmanagedSize = effect.readConfig("closedUnmanagedSize", 100);
        fadingPopupsEffect.closedUnmanagedCurve = effect.readConfig("closedUnmanagedCurve", 10);
    },
    setupForcedRoles: function (window) {
        window.setData(Effect.WindowForceBackgroundContrastRole, true);
        window.setData(Effect.WindowForceBlurRole, true);
    },
    cleanupForcedRoles: function (window) {
        window.setData(Effect.WindowForceBackgroundContrastRole, null);
        window.setData(Effect.WindowForceBlurRole, null);
    },
    slotWindowAdded: function (window, cursorpos) {
        if (effects.hasActiveFullScreenEffect) {
            return;
        }
        if (!isPopupWindow(window)) {
            return;
        }
        if (!isFadingWindow(window)) {
            return;
        }
        if (!window.visible) {
            return;
        }
        if (!effect.grab(window, Effect.WindowAddedGrabRole)) {
            return;
        }
        if (window.startEffectType == 0
            || window.startEffectType == 1
            || window.startEffectType == 16) {
            return;
        }
        fadingPopupsEffect.setupForcedRoles(window);
        var windowRect = window.geometry;
        fadingPopupsEffect.addUnmanagedSize = effect.readConfig("addUnmanagedSize", 30);
        if (window.popupMenu && (cursorpos.x < windowRect.x - 2 || cursorpos.x > windowRect.x + windowRect.width + 2)) { //二级菜单
            fadingPopupsEffect.addUnmanagedSize = 100;
        }

        var initialPos;
        var effectWidth;
        if (window.startEffectType == 4) {
            var pos = {x: windowRect.x + windowRect.width / 2, y: windowRect.y};
            initialPos = fadingPopupsEffect.calcDiagonalPos(window, pos, windowRect);
            effectWidth = windowRect.width;
        } else {
            initialPos = fadingPopupsEffect.calcDiagonalPos(window, cursorpos, windowRect);
            effectWidth = windowRect.width * fadingPopupsEffect.addUnmanagedSize / 100;
        }

        window.fadeInAnimation;
        if (initialPos.x == windowRect.x + windowRect.width / 2 && initialPos.y == windowRect.y + windowRect.height / 2) {
            window.fadeInAnimation = animate({
                    window: window,
                    curve: fadingPopupsEffect.addUnmanagedCurve,
                    duration: animationTime(fadingPopupsEffect.addUnmanagedDuration),
                    type: Effect.Opacity,
                    from: 0.0,
                    to: 1.0
                });
        } else {
            window.fadeInAnimation = animate({
            window: window,
            curve: fadingPopupsEffect.addUnmanagedCurve,
            duration: animationTime(fadingPopupsEffect.addUnmanagedDuration),
            animations: [
                {
                    type: Effect.Position,
                    from: {
                        value1: initialPos.x,
                        value2: initialPos.y
                    }
                },
                {
                    type: Effect.Size,
                    from: {
                        value1: effectWidth,
                        value2: windowRect.height * fadingPopupsEffect.addUnmanagedSize / 100
                    },
                    to: {
                        value1: windowRect.width,
                        value2: windowRect.height
                    }
                },
                {
                    type: Effect.Opacity,
                    from: 0.0,
                    to : 1.0
                }
                ]
            });
        }

    },
    slotWindowClosed: function (window) {
        if (effects.hasActiveFullScreenEffect) {
            return;
        }
        if (!isPopupWindow(window)) {
            return;
        }
        if (!isFadingWindow(window)) {
            return;
        }
        if (!window.visible) {
            return;
        }
        if (!effect.grab(window, Effect.WindowClosedGrabRole)) {
            return;
        }
        fadingPopupsEffect.setupForcedRoles(window);
        var windowRect = window.geometry;
        window.fadeOutAnimation;
        if (window.windowClass != "dde-launcher dde-launcher") {
            window.fadeOutAnimation = animate({
            window: window,
            curve: fadingPopupsEffect.closedUnmanagedCurve,
            duration: animationTime(fadingPopupsEffect.closedUnmanagedDuration),
            animations: [
                {
                    type: Effect.Size,
                    to: {
                        value1: windowRect.width * fadingPopupsEffect.closedUnmanagedSize / 100,
                        value2: windowRect.height * fadingPopupsEffect.closedUnmanagedSize / 100
                    },
                    from: {
                        value1: windowRect.width,
                        value2: windowRect.height
                    }
                },
                {
                    type: Effect.Opacity,
                    to: 0.0,
                    from: 1.0
                },
                ]
            });
        } else {
            var initialPos = calDockClosedPos(windowRect);
            window.fadeOutAnimation = animate({
                window: window,
                curve: fadingPopupsEffect.closedUnmanagedCurve,
                duration: animationTime(fadingPopupsEffect.closedUnmanagedDuration),
                animations: [
                    {
                        type: Effect.Size,
                        to: {
                            value1: windowRect.width * fadingPopupsEffect.closedSmallLauncherSize / 100,
                            value2: windowRect.height * fadingPopupsEffect.closedSmallLauncherSize / 100
                        },
                        from: {
                            value1: windowRect.width,
                            value2: windowRect.height
                        }
                    },
                    {
                        type: Effect.Opacity,
                        to: 0.0,
                        from: 1.0
                    },
                    {
                        type: Effect.Position,
                        to: {
                            value1: initialPos.x,
                            value2: initialPos.y
                        }
                    }
                ]
            });
        }
    },
    slotWindowDataChanged: function (window, role) {
        if (role == Effect.WindowAddedGrabRole) {
            if (window.fadeInAnimation && effect.isGrabbed(window, role)) {
                cancel(window.fadeInAnimation);
                delete window.fadeInAnimation;
                fadingPopupsEffect.cleanupForcedRoles(window);
            }
        } else if (role == Effect.WindowClosedGrabRole) {
            if (window.fadeOutAnimation && effect.isGrabbed(window, role)) {
                cancel(window.fadeOutAnimation);
                delete window.fadeOutAnimation;
                fadingPopupsEffect.cleanupForcedRoles(window);
            }
        }
    },
    init: function () {
        fadingPopupsEffect.loadConfig();

        effect.configChanged.connect(fadingPopupsEffect.loadConfig);
        effects.unmanagedAddedAnimation.connect(fadingPopupsEffect.slotWindowAdded);
        effect.animationEnded.connect(fadingPopupsEffect.cleanupForcedRoles);
        effects.windowClosed.connect(fadingPopupsEffect.slotWindowClosed);
        effects.windowDataChanged.connect(fadingPopupsEffect.slotWindowDataChanged);
    }
};

fadingPopupsEffect.init();
