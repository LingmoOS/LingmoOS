/*
 *    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include <KCommandBar>
#include <QMainWindow>

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QPlainTextEdit>
#include <QToolBar>
#include <QVBoxLayout>

class Window;

/**
 * Fwd decl
 * A helper function to generate a QAction
 */
static QAction *genAction(Window *p, const QString &icon, int i, const int shortcut = Qt::CTRL);

class Window : public QMainWindow
{
public:
    Window(QWidget *parent = nullptr)
        : QMainWindow(parent)
        , pe(this)
    {
        setCentralWidget(&pe);

        auto toolBar = new QToolBar(this);
        this->addToolBar(toolBar);

        auto qo1 = toolBar->addAction(QStringLiteral("Open Command Bar"));
        connect(qo1, &QAction::triggered, this, [this] {
            KCommandBar *bar = new KCommandBar(this);
            bar->setActions(getActions());
            bar->show();
        });

        auto qo2 = toolBar->addAction(QStringLiteral("Open Command Bar (RTL)"));
        connect(qo2, &QAction::triggered, this, [this] {
            KCommandBar *bar = new KCommandBar(this);
            bar->setActions(getRTLActions());
            bar->show();
        });
    }

    QAction *getMenu()
    {
        QMenu *file = new QMenu(this);
        file->setTitle(QStringLiteral("File"));

        auto createActionAndConnect = [file, this](const char *name) {
            auto a = file->addAction(QString::fromUtf8(name));
            connect(a, &QAction::triggered, [a, this] {
                pe.appendPlainText(a->text() + QStringLiteral(" triggered"));
            });
        };

        createActionAndConnect("File Menu action 1");
        createActionAndConnect("File Menu act 2");
        return file->menuAction();
    }

    QAction *getAboutToShowMenu()
    {
        QMenu *menu = new QMenu(this);
        menu->setTitle(QStringLiteral("Tool"));

        connect(menu, &QMenu::aboutToShow, this, [this, menu] {
            if (!menu->actions().isEmpty()) {
                return;
            }

            auto createActionAndConnect = [menu, this](const char *name) {
                auto a = menu->addAction(QString::fromUtf8(name));
                connect(a, &QAction::triggered, [a, this] {
                    pe.appendPlainText(a->text() + QStringLiteral(" triggered"));
                });
            };

            createActionAndConnect("About to show action 1");
            createActionAndConnect("About to show 2");
        });
        return menu->menuAction();
    }

    QList<KCommandBar::ActionGroup> getActions()
    {
        QList<KCommandBar::ActionGroup> acts(4);

        /**
         * Menus with actions
         */
        acts[0].actions = {getAboutToShowMenu(), getMenu()};

        int i = 0;
        acts[1].name = QStringLiteral("First Menu Group");
        for (; i < 2; ++i) {
            acts[1].actions.append(genAction(this, QStringLiteral("folder"), i));
        }
        acts[1].actions[0]->setShortcut(QStringLiteral("G"));
        acts[1].actions[1]->setCheckable(true);
        acts[1].actions[1]->setShortcut(QStringLiteral("Ctrl++"));

        acts[2].name = QStringLiteral("Second Menu Group - Disabled acts");
        for (; i < 4; ++i) {
            auto act = genAction(this, QStringLiteral("zoom-out"), i);
            act->setText(QStringLiteral("Disabled Act %1").arg(i));
            act->setEnabled(false);
            acts[2].actions.append(act);
        }

        acts[3].name = QStringLiteral("Third Menu Group");
        for (; i < 6; ++i) {
            acts[3].actions.append(genAction(this, QStringLiteral("security-low"), i, Qt::CTRL | Qt::ALT));
        }
        acts[3].actions[0]->setCheckable(true);
        acts[3].actions[0]->setShortcut(QStringLiteral("Ctrl+,, Ctrl++, Ctrl+K"));

        return acts;
    }

    // Use ./bin/kcommandbartest -reverse to test this
    QList<KCommandBar::ActionGroup> getRTLActions()
    {
        QList<KCommandBar::ActionGroup> acts(2);

        acts[0].name = QStringLiteral("مینو گروپ");
        acts[0].actions = {new QAction(QIcon::fromTheme("folder"), QStringLiteral("یہ فولڈر ایکشن ہے"), this),
                           new QAction(QIcon::fromTheme("folder"), QStringLiteral("یہ ایک اور فولڈر ایکشن ہے"), this)};
        acts[0].actions[1]->setCheckable(true);
        acts[0].actions[1]->setShortcut(QStringLiteral("Ctrl+Shift++"));

        acts[1].name = QStringLiteral("گروپ");
        acts[1].actions = {new QAction(QIcon::fromTheme("zoom-out"), QStringLiteral("یہ فولڈر ایکشن ہے"), this),
                           new QAction(QIcon::fromTheme("security-low"), QStringLiteral("یہ ایک اور فولڈر ایکشن ہے"), this)};
        acts[1].actions[1]->setCheckable(true);
        acts[1].actions[1]->setShortcut(QStringLiteral("Ctrl+-"));

        return acts;
    }

    QPlainTextEdit *textEdit()
    {
        return &pe;
    }

private:
    QPlainTextEdit pe;
};

static QAction *genAction(Window *p, const QString &icon, int i, const int shortcut)
{
    QString text = QStringLiteral("A long long Action name %1").arg(i++);
    QAction *action = new QAction(QIcon::fromTheme(icon), text, p);
    QObject::connect(action, &QAction::triggered, [action, p] {
        p->textEdit()->appendPlainText(action->text() + QStringLiteral(" triggered"));
    });

    static int key = Qt::Key_1;
    key++;
    // Reset
    if (key == Qt::Key_BraceRight) {
        key = Qt::Key_1;
    }
    const auto ss = shortcut | key;
    action->setShortcut(ss);
    return action;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("kcommandbartest"));

    Window *window = new Window();
    window->resize(1024, 600);
    window->show();

    return app.exec();
}
