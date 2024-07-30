/*
    SPDX-FileCopyrightText: 2006 Dirk Stoecker <kde@dstoecker.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kdirwatchtest_gui.h"

#include <QApplication>
#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <kdirwatch.h>
#include <qplatformdefs.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KDirWatchTest_GUI *mainWin = new KDirWatchTest_GUI();
    mainWin->show();
    return app.exec();
}

KDirWatchTest_GUI::KDirWatchTest_GUI()
    : QWidget()
{
    QPushButton *e;
    QPushButton *f;

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(l1 = new QLineEdit(QLatin1String("Test 1"), this));
    lay->addWidget(l2 = new QLineEdit(QLatin1String("Test 2"), this));
    lay->addWidget(l3 = new QLineEdit(QLatin1String("Test 3"), this));
    lay->addWidget(m_eventBrowser = new QTextBrowser(this));
    lay->addWidget(d = new QLineEdit(QLatin1String("Status"), this));
    lay->addWidget(e = new QPushButton(QLatin1String("new file"), this));
    lay->addWidget(f = new QPushButton(QLatin1String("delete file"), this));

    dir = QDir::currentPath();
    file = dir + QLatin1String("/testfile_kdirwatchtest_gui");

    w1 = new KDirWatch();
    w1->setObjectName(QLatin1String("w1"));
    w2 = new KDirWatch();
    w2->setObjectName(QLatin1String("w2"));
    w3 = new KDirWatch();
    w3->setObjectName(QLatin1String("w3"));
    connect(w1, &KDirWatch::dirty, this, &KDirWatchTest_GUI::slotDir1);
    connect(w2, &KDirWatch::dirty, this, &KDirWatchTest_GUI::slotDir2);
    connect(w3, &KDirWatch::dirty, this, &KDirWatchTest_GUI::slotDir3);
    w1->addDir(dir);
    w2->addDir(dir);
    w3->addDir(dir);

    KDirWatch *w4 = new KDirWatch(this);
    w4->setObjectName(QLatin1String("w4"));
    w4->addDir(dir, KDirWatch::WatchFiles | KDirWatch::WatchSubDirs);
    connect(w1, &KDirWatch::dirty, this, &KDirWatchTest_GUI::slotDirty);
    connect(w1, &KDirWatch::created, this, &KDirWatchTest_GUI::slotCreated);
    connect(w1, &KDirWatch::deleted, this, &KDirWatchTest_GUI::slotDeleted);

    KDirWatch *w5 = new KDirWatch(this);
    w5->setObjectName(QLatin1String(QLatin1String("w5")));
    w5->addFile(file);
    connect(w5, &KDirWatch::dirty, this, &KDirWatchTest_GUI::slotDirty);
    connect(w5, &KDirWatch::created, this, &KDirWatchTest_GUI::slotCreated);
    connect(w5, &KDirWatch::deleted, this, &KDirWatchTest_GUI::slotDeleted);

    lay->addWidget(new QLabel(QLatin1String("Directory = ") + dir, this));
    lay->addWidget(new QLabel(QLatin1String("File = ") + file, this));

    connect(e, &QPushButton::clicked, this, &KDirWatchTest_GUI::slotNewClicked);
    connect(f, &QPushButton::clicked, this, &KDirWatchTest_GUI::slotDeleteClicked);

    setMinimumWidth(800);
    setMinimumHeight(400);
}

void KDirWatchTest_GUI::slotDir1(const QString &a)
{
    l1->setText(QLatin1String("Test 1 changed ") + a + QLatin1String(" at ") + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotDir2(const QString &a)
{
    // This used to cause bug #119341, fixed now
#if 1
    w2->stopDirScan(QLatin1String(a.toLatin1().constData()));
    w2->restartDirScan(QLatin1String(a.toLatin1().constData()));
#endif
    l2->setText(QLatin1String("Test 2 changed ") + a + QLatin1String(" at ") + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotDir3(const QString &a)
{
    l3->setText(QLatin1String("Test 3 changed ") + a + QLatin1String(" at )") + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotDeleteClicked()
{
    remove(file.toLatin1().constData());
    d->setText(QLatin1String("Delete clicked at ") + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotNewClicked()
{
    fclose(QT_FOPEN(file.toLatin1().constData(), "wb"));
    d->setText(QLatin1String("New clicked at ") + QTime::currentTime().toString());
}

void KDirWatchTest_GUI::slotDirty(const QString &path)
{
    m_eventBrowser->append(QLatin1String("Dirty(") + sender()->objectName() + QLatin1String("): ") + path + QLatin1Char('\n'));
}

void KDirWatchTest_GUI::slotCreated(const QString &path)
{
    m_eventBrowser->append(QLatin1String("Created(") + sender()->objectName() + QLatin1String("): ") + path + QLatin1Char('\n'));
}

void KDirWatchTest_GUI::slotDeleted(const QString &path)
{
    m_eventBrowser->append(QLatin1String("Deleted(") + sender()->objectName() + QLatin1String("): ") + path + QLatin1Char('\n'));
}

#include "moc_kdirwatchtest_gui.cpp"
