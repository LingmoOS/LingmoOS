// SPDX-FileCopyrightText: 2018 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

function rectContains(rect1, rect2)
{
    rect1.right = rect1.x + rect1.width - 1;
    rect1.bottom = rect1.y + rect1.height - 1;
    rect2.right = rect2.x + rect2.width - 1;
    rect2.bottom = rect2.y + rect2.height - 1;

    if (rect1.x != rect2.x)
        return false;
    if (rect1.y != rect2.y)
        return false;
    if (rect1.right != rect2.right)
        return false;
    if (rect1.bottom != rect2.bottom)
        return false;

    return true;
}

function forceFullScreen(client) {
    var newGeometry = client.geometry;
    var screen = client.screen;
    var screenGeometry = workspace.clientArea(KWin.ScreenArea, screen, 0);

    if (!rectContains(screenGeometry, newGeometry)) {
        newGeometry = screenGeometry;
        client.geometry = newGeometry;
        workspace.updateWindowGeometry(client, screenGeometry);
    }
}

function setupConnection(client) {
    if (client.caption == "Fullscreen Launchpad") {
        forceFullScreen(client)
        client.geometryChanged.connect(client, function () {
            forceFullScreen(this);
        });
    }
}

workspace.clientAdded.connect(setupConnection);
// connect all existing clients
var clients = workspace.clientList();
for (var i=0; i<clients.length; i++) {
    setupConnection(clients[i]);
}
