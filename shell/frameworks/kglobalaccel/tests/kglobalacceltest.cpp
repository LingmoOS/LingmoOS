/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KGlobalAccel>
#include <QAction>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QTest>

class GlobalAction : public QAction, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
public:
    GlobalAction()
    {
        connect(this, &QAction::changed, this, &GlobalAction::refresh);
        connect(KGlobalAccel::self(), &KGlobalAccel::globalShortcutActiveChanged, this, [this](QAction *action, bool active) {
            if (action != this) {
                return;
            }
            qDebug() << "active changed" << action << active;
            m_active = active;
            Q_EMIT activeChanged(active);
        });
    }

    void classBegin() override
    {
    }
    void componentComplete() override
    {
        m_done = true;
        refresh();
    }

    void refresh()
    {
        if (!m_done) {
            return;
        }
        bool added = KGlobalAccel::self()->setShortcut(this, {shortcut()}, KGlobalAccel::NoAutoloading);

        if (!added) {
            qWarning() << "could not set the global shortcut" << shortcut();
        } else {
            qDebug() << "shortcut set correctly" << shortcut();
        }
    }

    bool isActive() const
    {
        return m_active;
    }

Q_SIGNALS:
    void activeChanged(bool active);

private:
    bool m_done = false;
    bool m_active = false;
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    qmlRegisterType<GlobalAction>("org.kde.globalaccel", 1, 0, "GlobalAction");

    engine.load(QFINDTESTDATA("kglobalacceltest.qml"));

    return app.exec();
}

#include "kglobalacceltest.moc"
