// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "categoryutils.h"

#include <QMultiMap>

CategoryUtils::Categorytype CategoryUtils::parseBestMatchedCategory(QStringList categories)
{
    // 统计应用类型
    QMap<Categorytype, int> categoriesMap;
    for (const auto &category : categories) {
        QString lCategory = category.toLower();
        QList<Categorytype> tys;
        Categorytype ty = CategoryUtils::parseOCEANCategoryString(lCategory);
        if (ty != Categorytype::CategoryErr)
            tys.push_back(ty);
        else if (CategoryUtils::parseXdgCategoryString(lCategory).size() > 0) {
            tys.append(CategoryUtils::parseXdgCategoryString(lCategory));
        }

        for (const auto & ty : tys)
            categoriesMap[ty]++;
    }

    categoriesMap.remove(Categorytype::CategoryOthers);
    if (categoriesMap.size() == 0)
        return Categorytype::CategoryOthers;

    // 计算最多的类型
    int max = 0;
    Categorytype ty {Categorytype::CategoryOthers};
    for (auto iter = categoriesMap.begin(); iter != categoriesMap.end(); iter++) {
        if (iter.value() > max) {
            max = iter.value();
            ty = iter.key();
        }
    }

    QList<Categorytype> maxCatogories {ty};
    for (auto iter = categoriesMap.begin(); iter != categoriesMap.end(); iter++) {
        if (iter.key() != ty && iter.value() == max) {
            maxCatogories.push_back(iter.key());
        }
    }

    if (maxCatogories.size() == 1)
        return maxCatogories[0];

    std::sort(maxCatogories.begin(), maxCatogories.end());

    // 检查是否同时存在音乐和视频播放器
    QPair<bool, bool> found;
    for (auto iter = maxCatogories.begin(); iter != maxCatogories.end(); iter++) {
        if (*iter == Categorytype::CategoryMusic)
            found.first = true;
        else if (*iter == Categorytype::CategoryVideo)
            found.second = true;
    }

    if (found.first && found.second)
        return Categorytype::CategoryVideo;

    return maxCatogories[0];
}

CategoryUtils::Categorytype CategoryUtils::parseOCEANCategoryString(QString str)
{
    static const QMap<QString, Categorytype> name2ty {
        {"internet", Categorytype::CategoryInternet},
        {"chat", Categorytype::CategoryChat},
        {"music", Categorytype::CategoryMusic},
        {"video", Categorytype::CategoryVideo},
        {"graphics", Categorytype::CategoryGraphics},
        {"office", Categorytype::CategoryOffice},
        {"game", Categorytype::CategoryGame},
        {"reading", Categorytype::CategoryReading},
        {"development", Categorytype::CategoryDevelopment},
        {"system", Categorytype::CategorySystem},
        {"others", Categorytype::CategoryOthers},
    };
    return name2ty.find(str) != name2ty.end() ? name2ty[str] : Categorytype::CategoryErr;
}

QList<CategoryUtils::Categorytype> CategoryUtils::parseXdgCategoryString(QString str)
{
    static const QMultiMap<QString, Categorytype> xname2ty {
        {"2dgraphics",                      Categorytype::CategoryGraphics},
        {"3dgraphics",                      Categorytype::CategoryGraphics},
        {"accessibility",                   Categorytype::CategorySystem},
        {"accessories",                     Categorytype::CategoryOthers},
        {"actiongame",                      Categorytype::CategoryGame},
        {"advancedsettings",                Categorytype::CategorySystem},
        {"adventuregame",                   Categorytype::CategoryGame},
        {"amusement",                       Categorytype::CategoryGame},
        {"applet",                          Categorytype::CategoryOthers},
        {"arcadegame",                      Categorytype::CategoryGame},
        {"archiving",                       Categorytype::CategorySystem},
        {"art",                             Categorytype::CategoryOffice},
        {"artificialintelligence",          Categorytype::CategoryOffice},
        {"astronomy",                       Categorytype::CategoryOffice},
        {"audio",                           Categorytype::CategoryMusic},
        {"audiovideo",                      Categorytype::CategoryMusic},
        {"audiovideo",                      Categorytype::CategoryVideo},
        {"audiovideoediting",               Categorytype::CategoryMusic},
        {"audiovideoediting",               Categorytype::CategoryVideo},
        {"biology",                         Categorytype::CategoryOffice},
        {"blocksgame",                      Categorytype::CategoryGame},
        {"boardgame",                       Categorytype::CategoryGame},
        {"building",                        Categorytype::CategoryDevelopment},
        {"calculator",                      Categorytype::CategorySystem},
        {"calendar",                        Categorytype::CategorySystem},
        {"cardgame",                        Categorytype::CategoryGame},
        {"cd",                              Categorytype::CategoryMusic},
        {"chart",                           Categorytype::CategoryOffice},
        {"chat",                            Categorytype::CategoryChat},
        {"chemistry",                       Categorytype::CategoryOffice},
        {"clock",                           Categorytype::CategorySystem},
        {"compiz",                          Categorytype::CategorySystem},
        {"compression",                     Categorytype::CategorySystem},
        {"computerscience",                 Categorytype::CategoryOffice},
        {"consoleonly",                     Categorytype::CategoryOthers},
        {"contactmanagement",               Categorytype::CategoryChat},
        {"core",                            Categorytype::CategoryOthers},
        {"debugger",                        Categorytype::CategoryDevelopment},
        {"desktopsettings",                 Categorytype::CategorySystem},
        {"desktoputility",                  Categorytype::CategorySystem},
        {"development",                     Categorytype::CategoryDevelopment},
        {"dialup",                          Categorytype::CategorySystem},
        {"dictionary",                      Categorytype::CategoryOffice},
        {"discburning",                     Categorytype::CategorySystem},
        {"documentation",                   Categorytype::CategoryOffice},
        {"editors",                         Categorytype::CategoryOthers},
        {"education",                       Categorytype::CategoryOffice},
        {"electricity",                     Categorytype::CategoryOffice},
        {"electronics",                     Categorytype::CategoryOffice},
        {"email",                           Categorytype::CategoryInternet},
        {"emulator",                        Categorytype::CategoryGame},
        {"engineering",                     Categorytype::CategorySystem},
        {"favorites",                       Categorytype::CategoryOthers},
        {"filemanager",                     Categorytype::CategorySystem},
        {"filesystem",                      Categorytype::CategorySystem},
        {"filetools",                       Categorytype::CategorySystem},
        {"filetransfer",                    Categorytype::CategoryInternet},
        {"finance",                         Categorytype::CategoryOffice},
        {"game",                            Categorytype::CategoryGame},
        {"geography",                       Categorytype::CategoryOffice},
        {"geology",                         Categorytype::CategoryOffice},
        {"geoscience",                      Categorytype::CategoryOthers},
        {"gnome",                           Categorytype::CategorySystem},
        {"gpe",                             Categorytype::CategoryOthers},
        {"graphics",                        Categorytype::CategoryGraphics},
        {"guidesigner",                     Categorytype::CategoryDevelopment},
        {"hamradio",                        Categorytype::CategoryOffice},
        {"hardwaresettings",                Categorytype::CategorySystem},
        {"ide",                             Categorytype::CategoryDevelopment},
        {"imageprocessing",                 Categorytype::CategoryGraphics},
        {"instantmessaging",                Categorytype::CategoryChat},
        {"internet",                        Categorytype::CategoryInternet},
        {"ircclient",                       Categorytype::CategoryChat},
        {"kde",                             Categorytype::CategorySystem},
        {"kidsgame",                        Categorytype::CategoryGame},
        {"literature",                      Categorytype::CategoryOffice},
        {"logicgame",                       Categorytype::CategoryGame},
        {"math",                            Categorytype::CategoryOffice},
        {"medicalsoftware",                 Categorytype::CategoryOffice},
        {"meteorology",                     Categorytype::CategoryOthers},
        {"midi",                            Categorytype::CategoryMusic},
        {"mixer",                           Categorytype::CategoryMusic},
        {"monitor",                         Categorytype::CategorySystem},
        {"motif",                           Categorytype::CategoryOthers},
        {"multimedia",                      Categorytype::CategoryVideo},
        {"music",                           Categorytype::CategoryMusic},
        {"network",                         Categorytype::CategoryInternet},
        {"news",                            Categorytype::CategoryReading},
        {"numericalanalysis",               Categorytype::CategoryOffice},
        {"ocr",                             Categorytype::CategoryGraphics},
        {"office",                          Categorytype::CategoryOffice},
        {"p2p",                             Categorytype::CategoryInternet},
        {"packagemanager",                  Categorytype::CategorySystem},
        {"panel",                           Categorytype::CategorySystem},
        {"pda",                             Categorytype::CategorySystem},
        {"photography",                     Categorytype::CategoryGraphics},
        {"physics",                         Categorytype::CategoryOffice},
        {"pim",                             Categorytype::CategoryOthers},
        {"player",                          Categorytype::CategoryMusic},
        {"player",                          Categorytype::CategoryVideo},
        {"playonlinux",                     Categorytype::CategoryOthers},
        {"presentation",                    Categorytype::CategoryOffice},
        {"printing",                        Categorytype::CategoryOffice},
        {"profiling",                       Categorytype::CategoryDevelopment},
        {"projectmanagement",               Categorytype::CategoryOffice},
        {"publishing",                      Categorytype::CategoryOffice},
        {"puzzlegame",                      Categorytype::CategoryGame},
        {"rastergraphics",                  Categorytype::CategoryGraphics},
        {"recorder",                        Categorytype::CategoryMusic},
        {"recorder",                        Categorytype::CategoryVideo},
        {"remoteaccess",                    Categorytype::CategorySystem},
        {"revisioncontrol",                 Categorytype::CategoryDevelopment},
        {"robotics",                        Categorytype::CategoryOffice},
        {"roleplaying",                     Categorytype::CategoryGame},
        {"scanning",                        Categorytype::CategoryOffice},
        {"science",                         Categorytype::CategoryOffice},
        {"screensaver",                     Categorytype::CategoryOthers},
        {"sequencer",                       Categorytype::CategoryMusic},
        {"settings",                        Categorytype::CategorySystem},
        {"security",                        Categorytype::CategorySystem},
        {"simulation",                      Categorytype::CategoryGame},
        {"sportsgame",                      Categorytype::CategoryGame},
        {"spreadsheet",                     Categorytype::CategoryOffice},
        {"strategygame",                    Categorytype::CategoryGame},
        {"system",                          Categorytype::CategorySystem},
        {"systemsettings",                  Categorytype::CategorySystem},
        {"technical",                       Categorytype::CategoryOthers},
        {"telephony",                       Categorytype::CategorySystem},
        {"telephonytools",                  Categorytype::CategorySystem},
        {"terminalemulator",                Categorytype::CategorySystem},
        {"texteditor",                      Categorytype::CategoryOffice},
        {"texttools",                       Categorytype::CategoryOffice},
        {"transiation",                     Categorytype::CategoryDevelopment},
        {"translation",                     Categorytype::CategoryReading},
        {"trayicon",                        Categorytype::CategorySystem},
        {"tuner",                           Categorytype::CategoryMusic},
        {"tv",                              Categorytype::CategoryVideo},
        {"utility",                         Categorytype::CategorySystem},
        {"vectorgraphics",                  Categorytype::CategoryGraphics},
        {"video",                           Categorytype::CategoryVideo},
        {"videoconference",                 Categorytype::CategoryInternet},
        {"viewer",                          Categorytype::CategoryGraphics},
        {"webbrowser",                      Categorytype::CategoryInternet},
        {"webdevelopment",                  Categorytype::CategoryDevelopment},
        {"wine",                            Categorytype::CategoryOthers},
        {"wine-programs-accessories",       Categorytype::CategoryOthers},
        {"wordprocessor",                   Categorytype::CategoryOffice},
        {"x-alsa",                          Categorytype::CategoryMusic},
        {"x-bible",                         Categorytype::CategoryReading},
        {"x-bluetooth",                     Categorytype::CategorySystem},
        {"x-debian-applications-emulators", Categorytype::CategoryGame},
        {"x-digital_processing",            Categorytype::CategorySystem},
        {"x-enlightenment",                 Categorytype::CategorySystem},
        {"x-geeqie",                        Categorytype::CategoryGraphics},
        {"x-gnome-networksettings",         Categorytype::CategorySystem},
        {"x-gnome-personalsettings",        Categorytype::CategorySystem},
        {"x-gnome-settings-panel",          Categorytype::CategorySystem},
        {"x-gnome-systemsettings",          Categorytype::CategorySystem},
        {"x-gnustep",                       Categorytype::CategorySystem},
        {"x-islamic-software",              Categorytype::CategoryReading},
        {"x-jack",                          Categorytype::CategoryMusic},
        {"x-kde-edu-misc",                  Categorytype::CategoryReading},
        {"x-kde-internet",                  Categorytype::CategorySystem},
        {"x-kde-more",                      Categorytype::CategorySystem},
        {"x-kde-utilities-desktop",         Categorytype::CategorySystem},
        {"x-kde-utilities-file",            Categorytype::CategorySystem},
        {"x-kde-utilities-peripherals",     Categorytype::CategorySystem},
        {"x-kde-utilities-pim",             Categorytype::CategorySystem},
        {"x-lxde-settings",                 Categorytype::CategorySystem},
        {"x-mandriva-office-publishing",    Categorytype::CategoryOthers},
        {"x-mandrivalinux-internet-other",  Categorytype::CategorySystem},
        {"x-mandrivalinux-office-other",    Categorytype::CategoryOffice},
        {"x-mandrivalinux-system-archiving-backup", Categorytype::CategorySystem},
        {"x-midi",                           Categorytype::CategoryMusic},
        {"x-misc",                           Categorytype::CategorySystem},
        {"x-multitrack",                     Categorytype::CategoryMusic},
        {"x-novell-main",                    Categorytype::CategorySystem},
        {"x-quran",                          Categorytype::CategoryReading},
        {"x-red-hat-base",                   Categorytype::CategorySystem},
        {"x-red-hat-base-only",              Categorytype::CategorySystem},
        {"x-red-hat-extra",                  Categorytype::CategorySystem},
        {"x-red-hat-serverconfig",           Categorytype::CategorySystem},
        {"x-religion",                       Categorytype::CategoryReading},
        {"x-sequencers",                     Categorytype::CategoryMusic},
        {"x-sound",                          Categorytype::CategoryMusic},
        {"x-sun-supported",                  Categorytype::CategorySystem},
        {"x-suse-backup",                    Categorytype::CategorySystem},
        {"x-suse-controlcenter-lookandfeel", Categorytype::CategorySystem},
        {"x-suse-controlcenter-system",      Categorytype::CategorySystem},
        {"x-suse-core",                      Categorytype::CategorySystem},
        {"x-suse-core-game",                 Categorytype::CategoryGame},
        {"x-suse-core-office",               Categorytype::CategoryOffice},
        {"x-suse-sequencer",                 Categorytype::CategoryMusic},
        {"x-suse-yast",                      Categorytype::CategorySystem},
        {"x-suse-yast-high_availability",    Categorytype::CategorySystem},
        {"x-synthesis",                      Categorytype::CategorySystem},
        {"x-turbolinux-office",              Categorytype::CategoryOffice},
        {"x-xfce",                           Categorytype::CategorySystem},
        {"x-xfce-toplevel",                  Categorytype::CategorySystem},
        {"x-xfcesettingsdialog",             Categorytype::CategorySystem},
        {"x-ximian-main",                    Categorytype::CategorySystem},
    };

    return {xname2ty.values(str)};
}
