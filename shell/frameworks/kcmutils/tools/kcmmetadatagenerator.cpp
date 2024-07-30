/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

int main(int argc, char **argv)
{
    Q_ASSERT(argc == 3);
    QString fileName = QString::fromLatin1(argv[1]);

    QFile file(fileName);
    bool isOpen = file.open(QIODevice::ReadOnly);
    if (!isOpen) {
        qCritical() << "Could not open file" << fileName;
        exit(1);
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    Q_ASSERT(doc.isObject());

    const QJsonObject kplugin = doc.object().value(QLatin1String("KPlugin")).toObject();
    const QLatin1String namePrefix("Name");

    QFile out(QString::fromLatin1(argv[2]));
    out.open(QIODevice::WriteOnly);
    out.write("[Desktop Entry]\n");
    out.write("Type=Application\n");
    out.write("NoDisplay=true\n");
    out.write("X-KDE-AliasFor=systemsettings\n");

    QString executableProgram = QStringLiteral("systemsettings ");
    if (!doc.object().contains(QLatin1String("X-KDE-System-Settings-Parent-Category"))) {
        executableProgram = QStringLiteral("kcmshell6 ");
    }

    const QString exec = QLatin1String("Exec=") + executableProgram + QFileInfo(fileName).baseName() + QLatin1Char('\n');
    out.write(exec.toLatin1());
    const QString icon = QLatin1String("Icon=") + kplugin.value(QLatin1String("Icon")).toString() + QLatin1Char('\n');
    out.write(icon.toLatin1());

    for (auto it = kplugin.begin(), end = kplugin.end(); it != end; ++it) {
        const QString key = it.key();
        if (key.startsWith(namePrefix)) {
            const QString name = key + QLatin1Char('=') + it.value().toString() + QLatin1Char('\n');
            out.write(name.toLocal8Bit());
        }
    }
}
