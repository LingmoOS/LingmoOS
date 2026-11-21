const { app, BrowserWindow } = require('electron');
app.whenReady().then(() => {
    const window = new BrowserWindow({ width: 640, height: 480, frame: false, titleBarStyle: 'hidden' });
    window.loadURL('data:text/html,<body style="background:white"><header style="-webkit-app-region:drag">Client titlebar</header>Window exterior test</body>');
});
app.on('window-all-closed', () => app.quit());
