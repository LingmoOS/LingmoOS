/*
    SPDX-FileCopyrightText: 1999, 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 1999, 2000 Simon Hausmann <hausmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "parts.h"

#include "guiactivateevent.h"

#include <KActionCollection>
#include <KActionMenu>
#include <KPluginMetaData>

#include <QAction>
#include <QCheckBox>
#include <QFile>
#include <QJsonDocument>
#include <QLineEdit>
#include <QTest>
#include <QTextEdit>
#include <QTextStream>

#include <KLocalizedString>
#include <QDebug>
#include <qstringliteral.h>

KPluginMetaData fakeMetadata(const QString &id, const QString &name)
{
    const QString json = QLatin1String(
                             "{ \"KPlugin\": {\n"
                             " \"Name\": \"%1\",\n"
                             " \"Version\": \"0.1\"\n"
                             "}\n}")
                             .arg(name);
    QJsonObject jo = QJsonDocument::fromJson(json.toLocal8Bit()).object();
    return KPluginMetaData(jo, id);
}

Part1::Part1(QObject *parent, QWidget *parentWidget)
    : KParts::ReadOnlyPart(parent, fakeMetadata(QStringLiteral("kpartstestpart"), QStringLiteral("KPart test part")))
{
    m_edit = new QTextEdit(parentWidget);
    setWidget(m_edit);

    setXMLFile(QFINDTESTDATA("kpartstest_part1.rc"));

    // An action and an action menu (test code for #70459)

    QAction *testAction = actionCollection()->addAction(QStringLiteral("p1_blah"));
    testAction->setText(QStringLiteral("Part1's action"));
    actionCollection()->setDefaultShortcut(testAction, Qt::CTRL | Qt::Key_B);
    connect(testAction, &QAction::triggered, this, &Part1::slotBlah);

    KActionMenu *menu = new KActionMenu(QIcon::fromTheme(QStringLiteral("mail_forward")), QStringLiteral("Foo"), this);
    actionCollection()->addAction(QStringLiteral("p1_foo"), menu);

    QAction *mailForward = new QAction(QIcon::fromTheme(QStringLiteral("mail_forward")), QStringLiteral("Bar"), this);
    actionCollection()->setDefaultShortcut(mailForward, Qt::CTRL | Qt::Key_F);
    connect(mailForward, &QAction::triggered, this, &Part1::slotFooBar);
    actionCollection()->addAction(QStringLiteral("p1_foo_bar"), mailForward);
    menu->addAction(mailForward);
}

Part1::~Part1()
{
}

void Part1::slotBlah()
{
    m_edit->setText(QStringLiteral("Blah"));
}

void Part1::slotFooBar()
{
    m_edit->setText(QStringLiteral("FooBar"));
}

bool Part1::openFile()
{
    // qDebug() << "Part1: opening " << QFile::encodeName(localFilePath());
    // Hehe this is from a tutorial I did some time ago :)
    QFile f(localFilePath());
    QString s;
    if (f.open(QIODevice::ReadOnly)) {
        QTextStream t(&f);
        while (!t.atEnd()) {
            s += t.readLine() + QStringLiteral("\n");
        }
        f.close();
    } else {
        return false;
    }
    m_edit->setPlainText(s);

    Q_EMIT setStatusBarText(url().toString());

    return true;
}

Part2::Part2(QObject *parent, QWidget *parentWidget)
    : KParts::Part(parent, fakeMetadata(QStringLiteral("part2"), QStringLiteral("Part2")))
{
    QWidget *w = new QWidget(parentWidget);
    w->setObjectName(QStringLiteral("Part2Widget"));
    setWidget(w);

    setXMLFile(QFINDTESTDATA("kpartstest_part2.rc"));

    /*QCheckBox * cb =*/new QCheckBox(QStringLiteral("something"), w);

    // QLineEdit * l = new QLineEdit( "something", widget() );
    // l->move(0,50);
    // Since the main widget is a dummy one, we HAVE to set
    // strong focus for it, otherwise we get the
    // the famous activating-file-menu-switches-part bug.
    w->setFocusPolicy(Qt::ClickFocus);
}

Part2::~Part2()
{
}

void Part2::guiActivateEvent(KParts::GUIActivateEvent *event)
{
    if (event->activated()) {
        Q_EMIT setWindowCaption(QStringLiteral("[part2 activated]"));
    }
}

#include "moc_parts.cpp"
