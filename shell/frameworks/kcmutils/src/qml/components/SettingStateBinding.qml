/*
    SPDX-FileCopyrightText: 2020 Kevin Ottens <kevin.ottens@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.kcmutils as KCMUtils
import org.kde.kcmutils.private as KCMUtilsPrivate

/**
 * SettingStateBinding automatically impacts the representation
 * of an item based on the state of a setting. It will disable
 * the item if the setting is immutable and use a visual indicator
 * for the state of the setting.
 *
 * This is a higher level convenience wrapper for KCMUtils.SettingStateProxy
 * and KCMUtils.SettingStateIndicator.
 *
 * @since 6.0
 */
Loader {
    id: root

    active: typeof kcm !== "undefined" && root.target !== null

    /**
     * target: Item
     * The graphical element whose state we want to manage based on a setting
     * If target is not set, it will try to find the visual parent item
     */
    property Item target: root.parent

    /**
     * configObject: KCoreConfigSkeleton
     * The config object which will be monitored for setting state changes
     */
    property alias configObject: settingState.configObject

    /**
     * settingName: string
     * The name of the setting in the config object to be monitored
     */
    property alias settingName: settingState.settingName

    /**
     * extraEnabledConditions: bool
     * SettingStateBinding will manage the enabled property of the target
     * based on the immutability state of the setting it represents. But,
     * sometimes that enabled state needs to bind to other properties as
     * well to be computed properly. This extra condition will thus be
     * combined with the immutability state of the setting to determine
     * the effective enabled state of the target.
     */
    property bool extraEnabledConditions: true

    /**
     * nonDefaultHighlightVisible: bool
     * Expose whether the non default highlight is visible.
     * Allow one to implement highlight with custom items.
     */
    readonly property bool nonDefaultHighlightVisible: root.active && root.item.highlight && kcm.defaultsIndicatorsVisible

    Binding {
        when: root.active
        target: root.target
        property: "enabled"
        value: extraEnabledConditions && !settingState.immutable
    }

    KCMUtils.SettingStateProxy {
        id: settingState
    }

    sourceComponent: KCMUtilsPrivate.SettingHighlighterPrivate {
        id: helper
        defaultIndicatorVisible: kcm.defaultsIndicatorsVisible
        highlight: !settingState.defaulted
        target: root.target
    }
}
