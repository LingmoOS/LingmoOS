/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RESULTSET_QUICKCHECK_TEST_H
#define RESULTSET_QUICKCHECK_TEST_H

#include <common/test.h>

#include <PlasmaActivities/Controller>
#include <memory>

#include <set>

#include "quickcheck/tables/ResourceInfo.h"
#include "quickcheck/tables/ResourceLink.h"
#include "quickcheck/tables/ResourceScoreCache.h"

class ResultSetQuickCheckTest : public Test
{
    Q_OBJECT
public:
    ResultSetQuickCheckTest(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();

    void testUsedResourcesForAgents();
    void testUsedResourcesForActivities();

    void testLinkedResourcesForAgents();

    void cleanupTestCase();

public:
    std::unique_ptr<KActivities::Consumer> activities;

    struct PrimaryKeyOrder {
        template<typename T>
        bool operator()(const T &left, const T &right) const
        {
            return left.primaryKey() < right.primaryKey();
        }
    };

    TABLE(ResourceScoreCache) resourceScoreCaches;
    TABLE(ResourceInfo) resourceInfos;
    TABLE(ResourceLink) resourceLinks;

    QString randItem(const QStringList &choices) const;

    QStringList activitiesList;
    QStringList agentsList;
    QStringList typesList;
    QStringList resourcesList;

    void generateActivitiesList();
    void generateAgentsList();
    void generateTypesList();
    void generateResourcesList();

    void generateResourceInfos();
    void generateResourceScoreCaches();
    void generateResourceLinks();

    void pushToDatabase();
    void pullFromDatabase();
};

#endif /* RESULTSET_QUICKCHECK_TEST_H */
