/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <KPluginFactory>
#include <QDebug>
#include <QJsonArray>
#include <QProcess>
#include <QUrl>
#include <purpose/pluginbase.h>

class BluetoothJob : public Purpose::Job
{
    Q_OBJECT
public:
    BluetoothJob(QObject *parent)
        : Purpose::Job(parent)
    {
    }

    void start() override
    {
        QProcess *process = new QProcess(this);
        process->setProgram(QStringLiteral("bluedevil-sendfile"));
        const QJsonArray urlsJson = data().value(QLatin1String("urls")).toArray();

        QStringList args{QStringLiteral("-u"), data().value(QLatin1String("device")).toString()};

        for (const QJsonValue &val : urlsJson) {
            const QUrl url(val.toString());
            if (url.isLocalFile()) {
                args << QStringLiteral("-f") << url.toLocalFile();
            }
        }

        process->setArguments(args);

        connect(process, &QProcess::errorOccurred, this, &BluetoothJob::processError);
        connect(process, &QProcess::finished, this, &BluetoothJob::jobFinished);
        connect(process, &QProcess::readyRead, this, [process]() {
            qDebug() << "bluedevil-sendfile output:" << process->readAll();
        });

        process->start();
    }

    void processError(QProcess::ProcessError error)
    {
        QProcess *process = qobject_cast<QProcess *>(sender());
        qWarning() << "bluetooth share error:" << error << process->errorString();
        setError(1 + error);
        setErrorText(process->errorString());
        emitResult();
    }

    void jobFinished(int code, QProcess::ExitStatus status)
    {
        if (status != QProcess::NormalExit) {
            qWarning() << "bluedevil-sendfile crashed";
        }

        setError(code);
        setOutput({{QStringLiteral("url"), QString()}});
        emitResult();
    }

private:
};

class BluetoothPlugin : public Purpose::PluginBase
{
    Q_OBJECT
public:
    using PluginBase::PluginBase;
    Purpose::Job *createJob() const override
    {
        return new BluetoothJob(nullptr);
    }
};

K_PLUGIN_CLASS_WITH_JSON(BluetoothPlugin, "bluetoothplugin.json")

#include "bluetoothplugin.moc"
