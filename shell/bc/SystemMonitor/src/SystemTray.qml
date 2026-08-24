import Qt.labs.platform 1.1

SystemTrayIcon {
    id: systemTray
    visible: true
    icon.name: "utilities-system-monitor"
    
    signal trayClicked
    signal trayRightClicked
    
    onActivated: {
        if (reason === SystemTrayIcon.Trigger) {
            trayClicked()
        }
        else if (reason === SystemTrayIcon.Context) {
            trayRightClicked()
        }
    }
} 