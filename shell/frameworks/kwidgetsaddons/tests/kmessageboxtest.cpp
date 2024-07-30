#undef QT_NO_CAST_FROM_ASCII

#include "kmessagebox.h"
#include <QLabel>

#include <stdio.h>
#include <stdlib.h>

#include <qapplication.h>

class ExampleWidget : public QLabel
{
public:
    ExampleWidget(QWidget *parent = nullptr);
};

ExampleWidget::ExampleWidget(QWidget *parent)
    : QLabel(parent)
{
    // Make the top-level layout; a vertical box to contain all widgets
    // and sub-layouts.
    QSize sh;
    setText(QStringLiteral("<p>Hello.</p>"));
    sh = sizeHint();
    qWarning("SizeHint = %d x %d", sh.width(), sh.height());
    setText(QStringLiteral("Hello."));
    sh = sizeHint();
    qWarning("SizeHint = %d x %d", sh.width(), sh.height());
    setText(QStringLiteral("<p>Hello<br>World</p>"));
    sh = sizeHint();
    qWarning("SizeHint = %d x %d", sh.width(), sh.height());
    //    setText("Hello\nWorld");
    sh = sizeHint();
    qWarning("SizeHint = %d x %d", sh.width(), sh.height());
    setMinimumSize(sizeHint());
}

void showResult(int test, int i)
{
    printf("Test %d. returned %d ", test, i);
    switch (i) {
    case KMessageBox::Ok:
        printf("(%s)\n", "Ok");
        break;
    case KMessageBox::Cancel:
        printf("(%s)\n", "Cancel");
        break;
    case KMessageBox::PrimaryAction:
        printf("(%s)\n", "PrimaryAction");
        break;
    case KMessageBox::SecondaryAction:
        printf("(%s)\n", "SecondaryAction");
        break;
    case KMessageBox::Continue:
        printf("(%s)\n", "Continue");
        break;
    default:
        printf("(%s)\n", "ERROR!");
        exit(1);
    }
}

bool testMessageBox(int test)
{
    QStringList list;
    list.append(QStringLiteral("Hello"));
    list.append(QStringLiteral("World"));

    int i;
    switch (test) {
    case 1: {
        ExampleWidget *w = new ExampleWidget();
        w->show();

        i = KMessageBox::warningContinueCancel(w,
                                               QString::fromLatin1("You are about to <Print>.\n"
                                                                   "Are you sure?"),
                                               QStringLiteral("Print"),
                                               KGuiItem(QStringLiteral("&Print")),
                                               KStandardGuiItem::cancel(),
                                               QStringLiteral("dontask"));
        i = KMessageBox::warningContinueCancel(nullptr,
                                               QString::fromLatin1("You are about to <Print>.\n"
                                                                   "Are you sure?"),
                                               QStringLiteral("Print"),
                                               KGuiItem(QStringLiteral("&Print")),
                                               KStandardGuiItem::cancel(),
                                               QStringLiteral("dontask"),
                                               KMessageBox::AllowLink);
        i = KMessageBox::questionTwoActions(
            nullptr,
            QStringLiteral("<p>Do you have a printer? thisisaverylongdkldhklghklghklashgkllasghkdlsghkldfghklsabla bla bbla bla. It "
                           "also has <a href=http://www.kde.org>this URL</a>.</p>"),
            QStringLiteral("Bla"),
            KGuiItem(QStringLiteral("Scan")),
            KGuiItem(QStringLiteral("Select")),
            QStringLiteral("bla"),
            KMessageBox::AllowLink);
        break;
    }
    case 2:
        i = KMessageBox::questionTwoActions(nullptr,
                                            QStringLiteral("Does your printer support color or only black and white?"),
                                            QStringLiteral("Printer setup"),
                                            KGuiItem(QStringLiteral("Print Color")),
                                            KGuiItem(QLatin1String("Print Black & White")));
        break;
    case 3:
        i = KMessageBox::warningTwoActions(nullptr,
                                           QStringLiteral("Your printer has been added.\n"
                                                          "Do you want to update your configuration?"),
                                           QStringLiteral("Printer Setup"),
                                           KGuiItem(QStringLiteral("Update")),
                                           KGuiItem(QStringLiteral("Skip")));
        break;

    case 4:
        i = KMessageBox::warningContinueCancel(nullptr,
                                               QString::fromLatin1("You are about to print.\n"
                                                                   "Are you sure?"),
                                               QStringLiteral("Print"),
                                               KGuiItem(QStringLiteral("&Print")));
        break;

    case 5:
        i = KMessageBox::warningContinueCancel(nullptr,
                                               QString::fromLatin1("You are about to <Print>.\n"
                                                                   "Are you sure?"),
                                               QStringLiteral("Print"),
                                               KGuiItem(QStringLiteral("&Print")),
                                               KStandardGuiItem::cancel(),
                                               QStringLiteral("dontask"));
        i = KMessageBox::warningContinueCancel(nullptr,
                                               QString::fromLatin1("You are about to <Print>.\n"
                                                                   "Are you sure?"),
                                               QStringLiteral("Print"),
                                               KGuiItem(QStringLiteral("&Print")),
                                               KStandardGuiItem::cancel(),
                                               QStringLiteral("dontask"));
        break;

    case 6:
        i = KMessageBox::warningTwoActionsCancel(nullptr,
                                                 QString::fromLatin1("Your document contains unsaved changes.\n"
                                                                     "Do you want to save or discard your changes?\n"),
                                                 QStringLiteral("Close"),
                                                 KGuiItem(QStringLiteral("&Save")),
                                                 KGuiItem(QStringLiteral("&Discard")));
        break;

    case 7:
        i = KMessageBox::Ok;
        KMessageBox::error(nullptr, QStringLiteral("Oops, Your harddisk is unreadable."));
        break;

    case 8:
        i = KMessageBox::Ok;
        KMessageBox::detailedError(nullptr,
                                   QStringLiteral("Oops, Your harddisk is unreadable."),
                                   QStringLiteral("We don't know more yet."),
                                   QStringLiteral("Uh ooh"));
        break;

    case 9:
        i = KMessageBox::Ok;
        KMessageBox::information(nullptr,
                                 QString::fromLatin1("You can enable the menubar again "
                                                     "with the right mouse button menu."));
        break;

    case 10:
        i = KMessageBox::Ok;
        KMessageBox::information(nullptr,
                                 QString::fromLatin1("You can enable the menubar again "
                                                     "with the right mouse button menu."),
                                 QStringLiteral("Menubar Info"));
        break;

    case 11:
        i = KMessageBox::Ok;
        KMessageBox::information(nullptr,
                                 QStringLiteral("You can enable the menubar again\nwith the right mouse button menu."),
                                 QString(),
                                 QStringLiteral("Enable_Menubar"));
        break;

    case 12:
        i = KMessageBox::Ok;
        KMessageBox::enableAllMessages();
        break;

    case 13:
        i = KMessageBox::Ok;
        KMessageBox::information(nullptr, QStringLiteral("Return of the annoying popup message."), QString(), QStringLiteral("Enable_Menubar"));
        break;
    case 14: {
        QStringList strlist;
        strlist << QStringLiteral("/dev/hda") << QStringLiteral("/etc/inittab") << QStringLiteral("/usr/somefile")
                << QString::fromLatin1(
                       "/some/really/"
                       "long/file/name/which/is/in/a/really/deep/directory/in/a/really/large/"
                       "hard/disk/of/your/system")
                << QStringLiteral("/and/another/one");
        i = KMessageBox::questionTwoActionsList(nullptr,
                                                QStringLiteral("Do you want to delete the following files?"),
                                                strlist,
                                                QStringLiteral("Delete Files"),
                                                KGuiItem(QStringLiteral("Delete")),
                                                KGuiItem(QStringLiteral("Move to Trash")));
    } break;
    case 15: {
        QStringList strlist;
        printf("Filling StringList...\n");
        for (int j = 1; j <= 6000; j++) {
            strlist.append(QStringLiteral("/tmp/tmp.%1").arg(j));
        }
        printf("Completed...\n");
        i = KMessageBox::questionTwoActionsList(nullptr,
                                                QStringLiteral("Do you want to delete the following files?"),
                                                strlist,
                                                QStringLiteral("Delete Files"),
                                                KGuiItem(QStringLiteral("Delete")),
                                                KGuiItem(QStringLiteral("Move to Trash")));
    } break;

    case 16:
        i = KMessageBox::Ok;
        KMessageBox::informationList(nullptr, QStringLiteral("The following words have been found:"), list);
        break;

    case 17:
        i = KMessageBox::Ok;
        KMessageBox::informationList(nullptr, QStringLiteral("The following words have been found:"), list, QStringLiteral("Search Words"));
        break;

    case 18:
        i = KMessageBox::Ok;
        KMessageBox::informationList(nullptr, QStringLiteral("The following words have been found:"), list, QString(), QStringLiteral("Search_Words"));
        break;

    default:
        return false; // done
    } // Switch

    showResult(test, i);
    return true;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kmessageboxtest"));

    // Syntax: kmessageboxtest <test_number>
    if (argc > 1) {
        testMessageBox(QByteArray(argv[1]).toInt());
        return 0;
    }

    // No argument specified, run all tests
    int test = 1;
    while (++test) {
        if (!testMessageBox(test)) {
            break;
        }
    }
    return 0;
}

#if 0
//this is my sequence for testing messagebox layout:

KMessageBox::questionYesNoCancel(
    0, "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd", "long",
    KStandardGuiItem::saveAs(), KGuiItem("dsdddddd"), KStandardGuiItem::cancel()
);
KMessageBox::questionYesNoCancel(
    0, "ddddddddddddddddddddd ddddddddddddddddddddd dddddddddd dddddddddd ddddddddddddddddddd dddddddddddd ddddddddd", "long wrap",
    KStandardGuiItem::saveAs(), KGuiItem("dsdddddd"), KStandardGuiItem::cancel()
);

KMessageBox::questionYesNoCancel(
    0, "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
    "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
    "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
    "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
    "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
    "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
    "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
    "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
    "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
    "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd", "height",
    KStandardGuiItem::saveAs(), KGuiItem("dsdddddd"), KStandardGuiItem::cancel()
);

QStringList strlist;
strlist << "fgfghghghgfhgf" << "fgfghghghgfhgf" << "fgfgh\nghghgfhgf" << "f\ngfg\nhg\nhghgfhgf" << "fgfghghghgfhgf" << "fgfghghghgfhgf" << "fgfghghghgfhgf" << "fgfghghghgfhgf" << "fgfghghghgfhgf" << "fgfghghghgfhgf" << "fgfghghghgfhgf" << "fgfghghghgfhgf" << "fgfghghghgfhgf" << "fgfghghghgfhgf" << "fgfghghghgfhgf" << "fgfghghghgfhgf" << "fgfghghghgfhgf" << "fgfghghghgfhgf" << "fgfghghghgfhgf" << "fgfghghghgfhgf";
KMessageBox::errorList(0,
                       "short\n",
                       strlist, "short");
KMessageBox::errorList(0,
                       "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd",
                       strlist, "short");
KMessageBox::errorList(0,
                       "ddddddddddddddddddddd ddddddddddddddddddddd dddddddddd dddddddddd ddddddddddddddddddd dddddddddddd ddddddddd",
                       strlist, "short");
KMessageBox::errorList(0,
                       "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                       "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                       "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                       "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                       "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                       "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                       "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                       "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                       "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                       "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd",
                       strlist, "short");

KMessageBox::detailedError(0,
                           "sss",
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                          );
KMessageBox::detailedError(0,
                           "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd",
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                          );
KMessageBox::detailedError(0,
                           "ddddddddddddddddddddd ddddddddddddddddddddd dddddddddd dddddddddd ddddddddddddddddddd dddddddddddd ddddddddd",
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                          );
KMessageBox::detailedError(0,
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd",
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                           "dddddd\ndddddd\nddddddddd ddddd\ndddd\ndddddddddddd \ndddddddddd dddddddddd dd\nddddddddddd\ndd\ndddd dddd\ndddddddd ddd\ndd\ndddd"
                          );

#endif
