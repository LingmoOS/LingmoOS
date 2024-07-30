// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

#include <QCoreApplication>
#include <QDebug>

#include "firewallclient.h"
#include "rule.h"
#include "rulelistmodel.h"

#include <KJob>

// Start
void testDisableClient(FirewallClient *client);
void testDisableClientResult(FirewallClient *client);

// Second Method.
void testEnableClient(FirewallClient *client);
void testEnableClientResult(FirewallClient *client);

// Third Method
void testCurrentRulesEmpty(FirewallClient *client);
void testCurrentRulesEmptyResult(FirewallClient *client);

// Fourth Method
void testAddRules(FirewallClient *client);
void testAddRulesResult(FirewallClient *client);

// Fifth Method
void testRemoveRules(FirewallClient *client);
void testRemoveRulesResult(FirewallClient *client);

void printHelp()
{
    qDebug() << "Usage Information:";
    qDebug() << "$"
             << "firewall_test backend_name";
    qDebug() << "Where backend name is ether ufw or firewalld.";
    qDebug() << "Make sure you have no firewall running";
    qDebug() << "And make sure you have no profile configured on either firewall.";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "argc" << argc;
    if (argc != 2) {
        qDebug() << "Invalid number of arguments, please use:";
        printHelp();
        exit(1);
    }

    QString firewallBackendName = argv[1];
    if (firewallBackendName != "ufw" && firewallBackendName != "firewalld") {
        printHelp();
        exit(1);
    }
    qDebug() << firewallBackendName << "test starting";

    auto *client = new FirewallClient();
    client->setBackend({firewallBackendName});
    Q_ASSERT(client->backend() == firewallBackendName);

    // Start trying to disable, the order of calls is the same as the definition order.
    // Please don't change as this is really annoying to test.
    testDisableClient(client);

    return app.exec();
}

void testDisableClient(FirewallClient *client)
{
    qDebug() << "Test Disable the Firewall";
    KJob *enableJob = client->setEnabled(false);

    // Ignore, we are already disabled.
    if (enableJob == nullptr) {
        testEnableClient(client);
        return;
    }

    // Nice hack: One time connections.
    QMetaObject::Connection *const connection = new QMetaObject::Connection;
    *connection = QObject::connect(client, &FirewallClient::enabledChanged, [client, connection] {
        QObject::disconnect(*connection);
        delete connection;
        testDisableClientResult(client);
    });

    enableJob->start();
}

void testDisableClientResult(FirewallClient *client)
{
    Q_ASSERT(client->enabled() == false);
    testEnableClient(client);
}

void testEnableClient(FirewallClient *client)
{
    qDebug() << "Test Enable the Firewall";

    // From here on, We will jump thru the usage via connects.
    KJob *enableJob = client->setEnabled(true);

    // Ignore, we are already enabled.
    if (enableJob == nullptr) {
        testCurrentRulesEmpty(client);
        return;
    }

    // Nice hack: One time connections.
    QMetaObject::Connection *const connection = new QMetaObject::Connection;
    *connection = QObject::connect(client, &FirewallClient::enabledChanged, [client, connection] {
        qDebug() << "Enabled Changed";
        QObject::disconnect(*connection);
        delete connection;
        testEnableClientResult(client);
    });

    enableJob->start();
}

void testEnableClientResult(FirewallClient *client)
{
    qDebug() << "Client Enabled" << client->enabled();
    testCurrentRulesEmpty(client);
}

void testCurrentRulesEmpty(FirewallClient *client)
{
    if (client->rulesModel()->rowCount() != 0) {
        qDebug() << "We need a clean firewall rules to do the testing, please backup your rules and try again.";
        exit(1);
    }

    Q_ASSERT(client->rulesModel()->rowCount() == 0);
    testCurrentRulesEmptyResult(client);
}

void testCurrentRulesEmptyResult(FirewallClient *client)
{
    testAddRules(client);
}

// Fourth Method
void testAddRules(FirewallClient *client)
{
    QString interface = client->knownInterfaces()[0];

    // Expected output for firewalld
    // firewalld.client: ("-j", "DROP", "-p", "tcp", "-d", "127.0.0.1", "--dport=21", "-s", "127.0.0.1", "--sport=12")
    // firewalld.job: firewalld  "addRule" (
    //     QVariant(QString, "ipv4"),
    //     QVariant(QString, "filter"),
    //     QVariant(QString, "INPUT"),
    //     QVariant(int, 0),
    //     QVariant(QStringList, ("-j", "DROP", "-p", "tcp", "-d", "127.0.0.1", "--dport=21", "-s", "127.0.0.1", "--sport=12"))
    //  )

    // Current Output for firewalld:
    // firewalld.client: ("-j", "DROP", "-p", "tcp", "-d", "127.0.0.1", "--dport=21", "-s", "127.0.0.1", "--sport=12")
    //
    // firewalld.job: firewalld  "addRule" (
    //     QVariant(QString, "ipv4"),
    //     QVariant(QString, "filter"),
    //     QVariant(QString, "INPUT"),
    //     QVariant(int, 0),
    //     QVariant(QStringList, ("-j", "DROP", "-p", "tcp", "-d", "127.0.0.1", "--dport=21", "-s", "127.0.0.1", "--sport=12")))

    // Expected output for ufw
    // Debug message from helper: Cmd args passed to ufw: (
    // "--add=<rule action=\"deny\"
    //         direction=\"in\"
    //         dport=\"21\"
    //         sport=\"12\"
    //         protocol=\"TCP\"
    //         dst=\"127.0.0.1\"
    //         src=\"127.0.0.1\"
    //         logtype=\"\"
    //         v6=\"False\"/>")

    // Current Output for ufw
    // Debug message from helper: Cmd args passed to ufw: (
    // "--add=<rule action=\"deny\"
    //    direction=\"in\"
    //    dport=\"21\"
    //    sport=\"12\"
    //    dst=\"127.0.0.1\"
    //    src=\"127.0.0.1\"
    //    logtype=\"\"
    //    v6=\"False\"/>")

    auto *rule = new Rule(Types::Policy::POLICY_DENY, // Policy
                          true, // Incomming?
                          Types::Logging::LOGGING_OFF, // Logging
                          0, // Protocol Id on knownProtocols
                          "127.0.0.1", // Source Host
                          "12", // Source Port
                          "127.0.0.1", // Destination Port
                          "21", // Destination Port
                          QString(), // Interface In
                          QString(), // Interface Out
                          QString(), // Source App // Only used in UFW - Remove?
                          QString(), // Destination App // Only used in UFW - Remove?
                          0, // Index (TODO: Remove This)
                          false); // IPV6?

    // This call should perhaps have an client::addRuleFinished(), but currently we need
    // to rely on the model refresh
    client->addRule(rule);

    // Nice hack: One time connections.
    QMetaObject::Connection *const connection = new QMetaObject::Connection;
    *connection = QObject::connect(client->rulesModel(), &RuleListModel::modelReset, [client, connection] {
        qDebug() << "Add rule finished.";
        QObject::disconnect(*connection);
        delete connection;
        testAddRulesResult(client);
    });

    // TODO:
    // This job is started inside of the addRule, currently we have an inconsistency on what's
    // started by default and what's started by Qml. We need to fix this.
    // job->start();
}

void testAddRulesResult(FirewallClient *client)
{
    Q_ASSERT(client->rulesModel()->rowCount() == 1);
    client->removeRule(0);

    // Nice hack: One time connections.
    QMetaObject::Connection *const connection = new QMetaObject::Connection;
    *connection = QObject::connect(client->rulesModel(), &RuleListModel::modelReset, [client, connection] {
        qDebug() << "Remove Rule Finished.";
        QObject::disconnect(*connection);
        delete connection;
        testRemoveRulesResult(client);
    });
}

void testRemoveRulesResult(FirewallClient *client)
{
    Q_ASSERT(client->rulesModel()->rowCount() == 0);
    qDebug() << "Test Finished";
}
