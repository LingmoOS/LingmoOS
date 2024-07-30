/*
    SPDX-FileCopyrightText: 2023 Ilya Katsnelson <me@0upti.me>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include <QCoreApplication>
#include <QDebug>

#include <NetworkManagerQt/GenericTypes>
#include <NetworkManagerQt/SecretAgent>

class TestAgent: public NetworkManager::SecretAgent {
public:
    explicit TestAgent(QObject *parent = nullptr)
    : NetworkManager::SecretAgent(QStringLiteral("org.kde.plasma.example-agent"), NetworkManager::SecretAgent::Capability::VpnHints, parent)
    {
        qInfo() << "Starting fake secret agent";
    }

public Q_SLOTS:
    NMVariantMapMap GetSecrets(const NMVariantMapMap &connection,
                                       const QDBusObjectPath &connection_path,
                                       const QString &setting_name,
                                       const QStringList &hints,
                                       uint flags) override
    {
        qInfo() << "GetSecrets" \
            << "connection:" << connection \
            << "path:" << connection_path.path() \
            << "name:" << setting_name \
            << "hints:" << hints \
            << "flags:" << flags;
        return NMVariantMapMap();
    }

    void CancelGetSecrets(const QDBusObjectPath &connection_path, const QString &setting_name) override
    {
        qInfo() << "CancelGetSecrets" << "path:" << connection_path.path() << "name:" << setting_name; 
    }
    
    void SaveSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path) override
    {
        qInfo() << "SaveSecrets" \
            << "connection:" << connection \
            << "path:" << connection_path.path();
    }

    void DeleteSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path) override
    {
        qInfo() << "DeleteSecrets" \
            << "connection:" << connection \
            << "path:" << connection_path.path();
    }
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    auto agent = TestAgent(&app);
    return app.exec();
}
