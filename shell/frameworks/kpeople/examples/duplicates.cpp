/*
    Duplicates Example
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTimer>

#include <duplicatesfinder_p.h>
#include <matchessolver_p.h>
#include <personsmodel.h>

#include <cstdio>
#include <iostream>

using namespace KPeople;

class ResultPrinter : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void print(KJob *j)
    {
        QList<Match> res = ((DuplicatesFinder *)j)->results();
        std::cout << "Results: " << res.count() << std::endl;
        for (QList<Match>::iterator it = res.begin(); it != res.end();) {
            QStringList roles = it->matchReasons();
            QStringList rA;
            QStringList rB;

            AbstractContact::Ptr aA = it->indexA.data(PersonsModel::PersonVCardRole).value<AbstractContact::Ptr>();
            AbstractContact::Ptr aB = it->indexB.data(PersonsModel::PersonVCardRole).value<AbstractContact::Ptr>();

            Q_ASSERT(!it->reasons.isEmpty());
            for (Match::MatchReason i : std::as_const(it->reasons)) {
                rA += it->matchValue(i, aA);
                rB += it->matchValue(i, aB);
            }
            std::cout << "\t- " << qPrintable(roles.join(QStringLiteral(", "))) << ": " << it->indexA.row() << " " << it->indexB.row()
                      << " because: " << qPrintable(rA.join(QStringLiteral(", "))) << " // " << qPrintable(rB.join(QStringLiteral(", "))) << '.' << std::endl;
            bool remove = false;
            if (m_action == Ask) {
                for (char ans = ' '; ans != 'y' && ans != 'n';) {
                    std::cout << "apply? (y/n) ";
                    std::cin >> ans;
                    remove = ans == 'n';
                }
            }
            if (remove) {
                it = res.erase(it);
            } else {
                ++it;
            }
        }

        if ((m_action == Apply || m_action == Ask) && !res.isEmpty()) {
            MatchesSolver *s = new MatchesSolver(res, m_model, this);
            connect(s, SIGNAL(finished(KJob *)), this, SLOT(matchesSolverDone(KJob *)));
            s->start();
        } else {
            QCoreApplication::instance()->quit();
        }
    }

    void matchesSolverDone(KJob *job)
    {
        if (job->error() == 0) {
            std::cout << "Matching successfully finished" << std::endl;
        } else {
            std::cout << "Matching failed with error: " << job->error() << std::endl;
        }
        QCoreApplication::instance()->quit();
    }

public:
    enum MatchAction {
        Apply,
        NotApply,
        Ask,
    };
    MatchAction m_action;
    PersonsModel *m_model;
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    PersonsModel model;

    ResultPrinter r;
    r.m_model = &model;
    {
        QCommandLineParser parser;
        parser.addOption(QCommandLineOption(QStringLiteral("ask"), QStringLiteral("Ask whether to actually do the merge")));
        parser.addOption(QCommandLineOption(QStringLiteral("apply"), QStringLiteral("Actually apply all merges. (!!!)")));
        parser.addHelpOption();
        parser.process(app);
        r.m_action = parser.isSet(QStringLiteral("apply")) ? ResultPrinter::Apply
            : parser.isSet(QStringLiteral("ask"))          ? ResultPrinter::Ask
                                                           : ResultPrinter::NotApply;
    }

    DuplicatesFinder *f = new DuplicatesFinder(&model);
    QObject::connect(f, SIGNAL(finished(KJob *)), &r, SLOT(print(KJob *)));

    QTimer *t = new QTimer(&app);
    t->setInterval(500);
    t->setSingleShot(true);
    QObject::connect(&model, SIGNAL(modelInitialized(bool)), t, SLOT(start()));
    QObject::connect(&model, SIGNAL(rowsInserted(QModelIndex, int, int)), t, SLOT(start()));
    QObject::connect(t, SIGNAL(timeout()), f, SLOT(start()));

    return app.exec();
}

#include "duplicates.moc"
