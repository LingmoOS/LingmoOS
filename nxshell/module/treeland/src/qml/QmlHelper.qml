// Copyright (C) 2023 JiDe Zhang <zccrs@live.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

pragma Singleton

import QtQuick
import Waylib.Server
import TreeLand.Utils

Item {
    property alias shortcutManager: shortcutManager
    property alias workspaceManager: workspaceManager
    property alias winposManager: winposManager
    property OutputRenderWindow renderWindow: null
    property Cursor cursor: Helper.cursor

    function printStructureObject(obj) {
        var json = ""
        for (var prop in obj){
            if (!obj.hasOwnProperty(prop)) {
                continue;
            }
            let value = obj[prop]
            try {
                json += `    ${prop}: ${value},\n`
            } catch (err) {
                json += `    ${prop}: unknown,\n`
            }
        }

        return '{\n' + json + '}'
    }

    property string envHash: "" // outputs environment
    property string prevEnvHash: undefined
    property var restoreStates: new Map() // Map<Env,Map<SurfaceKey,SurfaceState>>
    function updateEnvHash() {
        console.log(outputManager.outputSet.size, JSON.stringify(outputManager.outputSet),Array.from(outputManager.outputSet))
        prevEnvHash = envHash
        envHash = JSON.stringify(Array.from(outputManager.outputSet))
    }
    onEnvHashChanged: {
        console.log(`env changed  ${prevEnvHash}=>${envHash}, ${workspaceManager.allSurfaces.count} surfaces exist`)
        if (!restoreStates.has(envHash))
            restoreStates.set(envHash, new Map())
        let curStateStore = restoreStates.get(envHash)
        let prevStateStore = restoreStates.get(prevEnvHash)
        for (let i = 0; i < workspaceManager.allSurfaces.count; i++) {
            const surf = workspaceManager.allSurfaces.get(i).wrapper
            const surfKey = surf.waylandSurface
            console.debug(`restoring ${surfKey} ${printStructureObject(surf.store?.normal)} => ${printStructureObject(curStateStore.get(surfKey)?.store?.normal)}`)
            let prevStore={t:Date.now()}
            Object.assign(prevStore,surf.store)
            prevStateStore.set(surfKey, {state: surf.state, store: prevStore})
            console.debug(`saved ${prevEnvHash}.${surfKey} = ${printStructureObject(prevStore.normal)}@<${prevStore.t}>`)
            if (curStateStore.has(surfKey)) {
                const surfState = curStateStore.get(surfKey)
                surf.restoreState(surfState.store)
            }
        }
    }

    QtObject {
        id: shortcutManager
        signal screenLocked
        signal multitaskViewToggled
        signal nextWorkspace
        signal prevWorkspace
        signal moveToNeighborWorkspace(d: int)
    }
    QtObject {
        id: workspaceManager
        property var workspacesById: new Map()
        property alias nWorkspaces: layoutOrder.count
        property var __: QtObject {
            id: privt
            property int workspaceIdCnt: { workspaceIdCnt = layoutOrder.count - 1 }
        }
        function createWs() {
            layoutOrder.append({ wsid: ++privt.workspaceIdCnt })
        }
        function destroyWs(id) {
            console.log(`workspace id=${id} destroyed!`)
            layoutOrder.remove(id)
        }
        function moveWs(from, to) {
            layoutOrder.move(from, to, 1)
        }

        property ListModel layoutOrder: ListModel {
            id: layoutOrder
            objectName: "layoutOrder"
            ListElement {
                wsid: 0
            }
        }
        property ListModel allSurfaces: ListModel {
            id: allSurfaces
            objectName: "allSurfaces"
            function removeIf(cond) {
                for (let i = 0; i < this.count; i++) {
                    if (cond(this.get(i))) {
                        this.remove(i)
                        return
                    }
                }
            }
        }
        Component.onCompleted: {
            createWs(),createWs()
        }
    }

    QtObject {
        id: winposManager
        readonly property string spawnPolicy: "Line"
        readonly property bool ignoreGeneration: true
        property var __: QtObject {
            id: priv
            property var seq: new Map()
            property var spawn: ["",null]
            property var prevSpawn
        }
        function nextPos(iden, target, surfaceItem) {
            class Gen{
                constructor(pos, region, outputRegion) {
                    this.pos = pos  // initial pos
                    this.region = region    // (0,0) bounding rect
                    this.outputRegion = outputRegion
                    this.cnt = -1
                }
            }
            // start from pos, stacking following down-right line
            class LineGen extends Gen {
                constructor(k, dy, ...args) {
                    super(...args)
                    this.k = k
                    let startPos = Qt.point(
                        Math.min(this.pos.x, this.region.width),
                        Math.min(this.pos.y, this.region.height)
                    )
                    this.x = startPos.x
                    this.dy = dy // y-offset each tick, x-offset inferred by screen w/h ratio
                    this.b = startPos.y - k * this.pos.x
                    this.next = startPos
                }
                gen() {
                    this.cnt++
                    let nx = this.x + this.dy / this.k
                    let ny = this.k * nx + this.b
                    if (nx > this.region.width || ny > this.region.height) { // exceed right/bottom
                        this.b -= this.dy
                        if (this.k * this.region.width + this.b < 0)
                            this.b = (this.b % this.region.height + this.region.height) % this.region.height
                        if (this.b < 0) // start from top-edge
                            ny = 0, nx = -this.b / this.k
                        else    // start from left-edge
                            nx = 0, ny = this.b
                    }
                    this.x = nx
                    let res = this.next
                    this.next = Qt.point(nx, ny)
                    return this.next
                }
            }
            // start from center, scatter to coners
            class CornerGen extends Gen {
                constructor(sw, sh, bias, ...args) {
                    super(...args)
                    this.sw = sw    // surface width
                    this.sh = sh    // surface height
                    this.bias = bias
                }
                gen() {
                    const cnt = this.cnt
                    const w = this.outputRegion.width
                    const h = this.outputRegion.height
                    const sw = this.sw
                    const sh = this.sh
                    const bx = this.bias * Math.floor(cnt / 4)
                    const by = bx * h / w
                    this.cnt++
                    let rt
                    switch (cnt % 4) {
                        case 0: rt = Qt.point(0 + bx, 0 + by); break
                        case 1: rt = Qt.point(w - sw - bx, by); break
                        case 2: rt = Qt.point(bx, h - sh - by); break
                        case 3: rt = Qt.point(w - sw - bx, h - sh - by); break
                        default:  rt =Qt.point(w/2 - sw/2, h/2 - sh/2)
                    }
                    return rt
                }
            }

            const outputDelegate = renderWindow.activeOutputDelegate
            const w = outputDelegate.width
            const h = outputDelegate.height
            const sw = surfaceItem.width
            const sh = surfaceItem.height
            const dy = outputDelegate.height / 40
            const contW = Math.max(w - sw, 1) // spawned win may exceed screen size, ensure has region
            const contH = Math.max(h - sh, 1)
            const [prevIden, prevItem] = priv.prevSpawn
            const initialPos = prevIden === iden ? Qt.point(prevItem.x, prevItem.y) : cursor.position

            const baseArgs = [
                initialPos, // first spawned win's position for LineGen
                Qt.rect(0, 0, contW, contH), // region to ensure spawned win wholely in screen
                Qt.rect(0, 0, w, h) // output rect
            ]
            // restart seq if: active output changed or spawning app changed
            if (!(priv.seq.has(iden) && priv.seq.get(iden).output == outputDelegate && prevIden == iden )
                || ignoreGeneration) // ignore whether last activated window belongs to same spawing seq
                priv.seq.set(iden, {cnt: -1, output: outputDelegate, pos: initialPos,
                    gen:
                        winposManager.spawnPolicy === "Line"
                        ? new LineGen(9/16, dy, ...baseArgs)
                        : new CornerGen(sw, sh, 5, ...baseArgs)
                })

            const cur = priv.seq.get(iden)
            cur.cnt++
            cur.gen.region = Qt.rect(0, 0, contW, contH)
            const rt = cur.gen.gen()
            console.debug(`New window nextPos iden=${iden} pos=${rt} ( cnt=${cur.cnt} initialPos=${cur.pos} geo=${cur.gen.region}, ${cur.gen.outputRegion} )`)

            return outputDelegate.mapToItem(target, rt)
        }
        // process non-spawn events
        function updateSeq(iden, item) {
            priv.prevSpawn = priv.spawn
            priv.spawn = [iden, item]
        }
    }
}
