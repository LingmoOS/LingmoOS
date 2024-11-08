function forceFullScreen(client) {
    var screen = client.screen;
    var screenGeometry = workspace.clientArea(KWin.ScreenArea, screen, 0);
    client.geometry = screenGeometry;
}

function setupConnection(client) {
    if (client.resourceClass != "lingmo-launcher"
            || client.resourceName != "lingmo-launcher" || client.dialog) {
        return;
    }

    forceFullScreen(client)
    client.geometryChanged.connect(client, function () {
        forceFullScreen(this);
    });
}

workspace.clientAdded.connect(setupConnection);
// connect all existing clients
var clients = workspace.clientList();
for (var i = 0; i < clients.length; i++) {
    setupConnection(clients[i]);
}