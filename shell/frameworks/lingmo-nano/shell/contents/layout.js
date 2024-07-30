
var desktopsArray = desktopsForActivity(currentActivity());
for (var j = 0; j < desktopsArray.length; j++) {
    var desk = desktopsArray[j];
    desk.wallpaperPlugin = "org.kde.slideshow";
    desk.addWidget("org.kde.lingmo.digitalclock");
    desk.addWidget("org.kde.mycroft.launcher");

//    desk.addWidget("org.kde.lingmo.mycroftplasmoid");

    desk.currentConfigGroup = new Array("Wallpaper","org.kde.slideshow","General");
    desk.writeConfig("SlideInterval", 480);
    desk.writeConfig("SlidePaths", "/usr/share/wallpapers/");
}

var panel = new Panel("org.kde.mycroft.panel")
panel.location = "top";
panel.height = 2 * gridUnit;

var battery = panel.addWidget("org.kde.lingmo.battery");
battery.currentConfigGroup = ["Configuration", "General"]
battery.writeConfig("showPercentage", true)
battery.reloadConfig()

panel.addWidget("org.kde.lingmo.networkmanagement");
//panel.hiding = "windowsbelow";
