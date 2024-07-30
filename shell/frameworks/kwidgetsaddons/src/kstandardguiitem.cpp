/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kstandardguiitem.h"

#include <QApplication>

namespace KStandardGuiItem
{
KGuiItem guiItem(StandardItem ui_enum)
{
    switch (ui_enum) {
    case Ok:
        return ok();
    case Cancel:
        return cancel();
    case Discard:
        return discard();
    case Save:
        return save();
    case DontSave:
        return dontSave();
    case SaveAs:
        return saveAs();
    case Apply:
        return apply();
    case Clear:
        return clear();
    case Help:
        return help();
    case Close:
        return close();
    case CloseWindow:
        return closeWindow();
    case CloseDocument:
        return closeDocument();
    case Defaults:
        return defaults();
    case Back:
        return back();
    case Forward:
        return forward();
    case Print:
        return print();
    case Continue:
        return cont();
    case Open:
        return open();
    case Quit:
        return quit();
    case AdminMode:
        return adminMode();
    case Reset:
        return reset();
    case Delete:
        return del();
    case Insert:
        return insert();
    case Configure:
        return configure();
    case Find:
        return find();
    case Stop:
        return stop();
    case Add:
        return add();
    case Remove:
        return remove();
    case Test:
        return test();
    case Properties:
        return properties();
    case Overwrite:
        return overwrite();
    default:
        return KGuiItem();
    };
}

QString standardItem(StandardItem ui_enum)
{
    switch (ui_enum) {
    case Ok:
        return QStringLiteral("ok");
    case Cancel:
        return QStringLiteral("cancel");
    case Discard:
        return QStringLiteral("discard");
    case Save:
        return QStringLiteral("save");
    case DontSave:
        return QStringLiteral("dontSave");
    case SaveAs:
        return QStringLiteral("saveAs");
    case Apply:
        return QStringLiteral("apply");
    case Help:
        return QStringLiteral("help");
    case Close:
        return QStringLiteral("close");
    case CloseWindow:
        return QStringLiteral("closeWindow");
    case CloseDocument:
        return QStringLiteral("closeDocument");
    case Defaults:
        return QStringLiteral("defaults");
    case Back:
        return QStringLiteral("back");
    case Forward:
        return QStringLiteral("forward");
    case Print:
        return QStringLiteral("print");
    case Continue:
        return QStringLiteral("continue");
    case Open:
        return QStringLiteral("open");
    case Quit:
        return QStringLiteral("quit");
    case AdminMode:
        return QStringLiteral("adminMode");
    case Delete:
        return QStringLiteral("delete");
    case Insert:
        return QStringLiteral("insert");
    case Configure:
        return QStringLiteral("configure");
    case Find:
        return QStringLiteral("find");
    case Stop:
        return QStringLiteral("stop");
    case Add:
        return QStringLiteral("add");
    case Remove:
        return QStringLiteral("remove");
    case Test:
        return QStringLiteral("test");
    case Properties:
        return QStringLiteral("properties");
    case Overwrite:
        return QStringLiteral("overwrite");
    default:
        return QString();
    };
}

KGuiItem ok()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&OK"), QStringLiteral("dialog-ok"));
}

KGuiItem cancel()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Cancel"), QStringLiteral("dialog-cancel"));
}

KGuiItem discard()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Discard"),
                    QStringLiteral("edit-delete"),
                    QApplication::translate("KStandardGuiItem", "Discard changes"),
                    QApplication::translate("KStandardGuiItem",
                                            "Pressing this button will discard all recent "
                                            "changes made in this dialog."));
}

KGuiItem save()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Save"),
                    QStringLiteral("document-save"),
                    QApplication::translate("KStandardGuiItem", "Save data"));
}

KGuiItem dontSave()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Do Not Save"), QString(), QApplication::translate("KStandardGuiItem", "Do not save data"));
}

KGuiItem saveAs()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "Save &As…"),
                    QStringLiteral("document-save-as"),
                    QApplication::translate("KStandardGuiItem", "Save file with another name"));
}

KGuiItem apply()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Apply"),
                    QStringLiteral("dialog-ok-apply"),
                    QApplication::translate("KStandardGuiItem", "Apply changes"),
                    QApplication::translate("KStandardGuiItem",
                                            "When you click <b>Apply</b>, the settings will be "
                                            "handed over to the program, but the dialog "
                                            "will not be closed.\n"
                                            "Use this to try different settings."));
}

KGuiItem adminMode()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "Administrator &Mode…"),
                    QString(),
                    QApplication::translate("KStandardGuiItem", "Enter Administrator Mode"),
                    QApplication::translate("KStandardGuiItem",
                                            "When you click <b>Administrator Mode</b> you will be prompted "
                                            "for the administrator (root) password in order to make changes "
                                            "which require root privileges."));
}

KGuiItem clear()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "C&lear"),
                    QStringLiteral("edit-clear"),
                    QApplication::translate("KStandardGuiItem", "Clear input"),
                    QApplication::translate("KStandardGuiItem", "Clear the input in the edit field"));
}

KGuiItem help()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Help", "show help"),
                    QStringLiteral("help-contents"),
                    QApplication::translate("KStandardGuiItem", "Show help"));
}

KGuiItem close()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Close"),
                    QStringLiteral("window-close"),
                    QApplication::translate("KStandardGuiItem", "Close the current window or document"));
}

KGuiItem closeWindow()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Close Window"),
                    QStringLiteral("window-close"),
                    QApplication::translate("KStandardGuiItem", "Close the current window."));
}

KGuiItem closeDocument()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Close Document"),
                    QStringLiteral("document-close"),
                    QApplication::translate("KStandardGuiItem", "Close the current document."));
}

KGuiItem defaults()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Defaults"),
                    QStringLiteral("document-revert"),
                    QApplication::translate("KStandardGuiItem", "Reset all items to their default values"));
}

KGuiItem back(BidiMode useBidi)
{
    QString icon = (useBidi == UseRTL && QApplication::isRightToLeft()) ? QStringLiteral("go-next") : QStringLiteral("go-previous");
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Back", "go back"), icon, QApplication::translate("KStandardGuiItem", "Go back one step"));
}

KGuiItem forward(BidiMode useBidi)
{
    QString icon = (useBidi == UseRTL && QApplication::isRightToLeft()) ? QStringLiteral("go-previous") : QStringLiteral("go-next");
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Forward", "go forward"),
                    icon,
                    QApplication::translate("KStandardGuiItem", "Go forward one step"));
}

QPair<KGuiItem, KGuiItem> backAndForward()
{
    return qMakePair(back(UseRTL), forward(UseRTL));
}

KGuiItem print()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Print…"),
                    QStringLiteral("document-print"),
                    QApplication::translate("KStandardGuiItem",
                                            "Opens the print dialog to print "
                                            "the current document"));
}

KGuiItem cont()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "C&ontinue"),
                    QStringLiteral("arrow-right"),
                    QApplication::translate("KStandardGuiItem", "Continue operation"));
}

KGuiItem del()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Delete"),
                    QStringLiteral("edit-delete"),
                    QApplication::translate("KStandardGuiItem", "Delete item(s)"));
}

KGuiItem open()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Open…"),
                    QStringLiteral("document-open"),
                    QApplication::translate("KStandardGuiItem", "Open file"));
}

KGuiItem quit()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Quit"),
                    QStringLiteral("application-exit"),
                    QApplication::translate("KStandardGuiItem", "Quit application"));
}

KGuiItem reset()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Reset"),
                    QStringLiteral("edit-undo"),
                    QApplication::translate("KStandardGuiItem", "Reset configuration"));
}

KGuiItem insert()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Insert", "Verb"));
}

KGuiItem configure()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "Confi&gure…"), QStringLiteral("configure"));
}

KGuiItem find()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Find"), QStringLiteral("edit-find"));
}

KGuiItem stop()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "Stop"), QStringLiteral("process-stop"));
}

KGuiItem add()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "Add"), QStringLiteral("list-add"));
}

KGuiItem remove()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "Remove"), QStringLiteral("list-remove"));
}

KGuiItem test()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "Test"));
}

KGuiItem properties()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "Properties"), QStringLiteral("document-properties"));
}

KGuiItem overwrite()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Overwrite"), QStringLiteral("document-replace"));
}

void assign(QPushButton *button, StandardItem item)
{
    KGuiItem::assign(button, KStandardGuiItem::guiItem(item));
}

} // KStandardGuiItem namespace
