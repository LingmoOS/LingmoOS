/**
 * SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

// Config values are {key: value} pairs
// In order to implement apply/reset features, keep
// the initial values immutable.  Changes are stored in pending.
QtObject {

    property var __initial: ({})
    property var pending: ({})

    // Check pending based on # of entries (>0) in the object
    // otherwise, compare stringify(initial === pending)
    property bool usePendingCount: true
    property bool hasPending: false

    // remove known unneeded keys (not needed by CUPS)
    function clean() {
        remove(["device-id", "device-class"
               , "device-desc", "device-uris"
               , "match", "printer-type"
               , "remote", "iconName"])
    }

    function init(map = {}) {
        __initial = map
        pending = Object.assign({}, __initial)
    }

    function clear() {
        set({})
    }

    function remove(keylist) {
        if (keylist === undefined)
            return

        if (typeof keylist === "object") {
            // string array of keys
            keylist.forEach(k => delete pending[k])
        } else if (typeof keylist === "string") {
            // string
            delete pending[keylist]
        }

        set()
    }

    function value(key : string) : var {
        if (key === undefined || key.length === 0) {
            return ""
        } else {
            return pending[key]
        }
    }

    function add(key : string, value) {
        if (key === undefined || key.length === 0 || value === undefined) {
            console.warn("KEY and VALUE must have values")
        } else {
            const obj = {}
            obj[key] = value
            set(obj)
        }
    }

    // if obj and has fields, assign obj over pending
    // otherwise, clear pending
    // always set pending flag
    function set(obj) {
        if (obj !== undefined && typeof obj === "object") {
            if (Object.keys(obj).length > 0) {
                Object.assign(pending, obj)
            } else {
                pending = {}
            }
        }

        if (usePendingCount) {
            hasPending = Object.keys(pending).length > 0
        } else {
            hasPending = JSON.stringify(__initial) !== JSON.stringify(pending)
        }
    }

    function reset() {
        if (usePendingCount) {
            pending = {}
        } else {
            pending = Object.assign({}, __initial)
        }
        set()
    }
}
