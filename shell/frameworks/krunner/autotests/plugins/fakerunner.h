/*
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KRunner/AbstractRunner>
#include <KRunner/Action>
#include <QEventLoop>
#include <QThread>
#include <QTimer>

using namespace KRunner;

class FakeRunner : public AbstractRunner
{
public:
    explicit FakeRunner(QObject *parent, const KPluginMetaData &metadata)
        : AbstractRunner(parent, metadata)
        , m_action("someid", "sometext", "dialog-ok")
    {
    }
    ~FakeRunner()
    {
        // qWarning() << Q_FUNC_INFO;
    }

    void match(RunnerContext &context) override
    {
        // Do not use nested event loop, because that would be quit when quitting the QThread's event loop
        QThread::msleep(50);
        if (context.query().startsWith(QLatin1String("foo"))) {
            context.addMatch(createDummyMatch(QStringLiteral("foo"), 0.1));
            context.addMatch(createDummyMatch(QStringLiteral("bar"), 0.2));
        }
    }

private:
    QueryMatch createDummyMatch(const QString &text, qreal relevance)
    {
        QueryMatch queryMatch(this);
        queryMatch.setId(text);
        queryMatch.setText(text);
        queryMatch.setRelevance(relevance);
        queryMatch.setActions({m_action});
        return queryMatch;
    }
    KRunner::Action m_action;
};
