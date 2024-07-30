/*
    SPDX-FileCopyrightText: 2012-2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <algorithm>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

#include <KAboutData>
#include <KLocalizedString>

#include <QJsonDocument>
#include <QJsonObject>

#include "global.h"
#include "idutils.h"
#include "database.h"
#include "transaction.h"

#include <unistd.h>
#include <KFileMetaData/PropertyInfo>

QString colorString(const QString& input, int color)
{
    static bool isTty = isatty(fileno(stdout));
    if(isTty) {
        QString colorStart = QStringLiteral("\033[0;%1m").arg(color);
        QLatin1String colorEnd("\033[0;0m");

        return colorStart + input + colorEnd;
    } else {
        return input;
    }
}

inline KFileMetaData::PropertyMultiMap variantToPropertyMultiMap(const QVariantMap &varMap)
{
    KFileMetaData::PropertyMultiMap propMap;
    QVariantMap::const_iterator it = varMap.constBegin();
    for (; it != varMap.constEnd(); ++it) {
        int p = it.key().toInt();
        propMap.insert(static_cast<KFileMetaData::Property::Property>(p), it.value());
    }
    return propMap;
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    KAboutData aboutData(QStringLiteral("balooshow"),
                         i18n("Baloo Show"),
                         QStringLiteral(PROJECT_VERSION),
                         i18n("The Baloo data Viewer - A debugging tool"),
                         KAboutLicense::GPL,
                         i18n("(c) 2012, Vishesh Handa"));

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("files"), i18n("Urls, document ids or inodes of the files"), QStringLiteral("[file|id|inode...]"));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("x"),
                                        i18n("Print internal info")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("i"),
                                        i18n("Arguments are interpreted as inode numbers (requires -d)")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("d"),
                                        i18n("Device id for the files"), QStringLiteral("deviceId"), QString()));
    parser.addHelpOption();
    parser.process(app);

    const QStringList args = parser.positionalArguments();

    if (args.isEmpty()) {
        parser.showHelp(1);
    }

    QTextStream stream(stdout);

    bool useInodes = parser.isSet(QStringLiteral("i"));
    quint32 devId;
    if (useInodes) {
        bool ok;
        devId = parser.value(QStringLiteral("d")).toULong(&ok, 10);
        if (!ok) {
            devId = parser.value(QStringLiteral("d")).toULong(&ok, 16);
        }
    }

    if (useInodes && devId == 0) {
        stream << i18n("Error: -i requires specifying a device (-d <deviceId>)") << '\n';
        parser.showHelp(1);
    }

    Baloo::Database *db = Baloo::globalDatabaseInstance();
    if (!db->open(Baloo::Database::ReadOnlyDatabase)) {
        stream << i18n("The Baloo index could not be opened. Please run \"%1\" to see if Baloo is enabled and working.",
	    QStringLiteral("balooctl status")) << '\n';
        return 1;
    }

    Baloo::Transaction tr(db, Baloo::Transaction::ReadOnly);

    for (QString url : args) {
        quint64 fid = 0;
        QString internalUrl;
        if (!useInodes) {
            if (QFile::exists(url)) {
                quint64 fsFid = Baloo::filePathToId(QFile::encodeName(url));
                fid = tr.documentId(QFile::encodeName(url));
                internalUrl = QFile::decodeName(tr.documentUrl(fsFid));

                if (fid && fid != fsFid) {
                    stream << i18n("The document IDs of the Baloo DB and the filesystem are different:") << '\n';
                    auto dbInode = Baloo::idToInode(fid);
                    auto fsInode = Baloo::idToInode(fsFid);
                    auto dbDevId = Baloo::idToDeviceId(fid);
                    auto fsDevId = Baloo::idToDeviceId(fsFid);

                    stream << "Url: " << url << "\n";
                    stream << "ID:       " << fid << " (DB) <-> " << fsFid << " (FS)\n";
                    stream << "Inode:    " << dbInode << " (DB) " << (dbInode == fsInode ? "== " : "<-> ") << fsInode << " (FS)\n";
                    stream << "DeviceID: " << dbDevId << " (DB) " << (dbDevId == fsDevId ? "== " : "<-> ") << fsDevId << " (FS)\n";
                }
                fid = fsFid;
            } else {
                bool ok;
                fid = url.toULongLong(&ok, 10);
                if (!ok) {
                    fid = url.toULongLong(&ok, 16);
                }
                if (!ok) {
                    stream << i18n("%1: Not a valid url or document id", url) << '\n';
                    continue;
                }
                url = QFile::decodeName(tr.documentUrl(fid));
                internalUrl = url;
            }

        } else {
            bool ok;
            quint32 inode = url.toULong(&ok, 10);
            if (!ok) {
                inode = url.toULong(&ok, 16);
            }
            if (!ok) {
                stream << i18n("%1: Failed to parse inode number", url) << '\n';
                continue;
            }

            fid = Baloo::devIdAndInodeToId(devId, inode);
            url = QFile::decodeName(tr.documentUrl(fid));
            internalUrl = url;
        }

        if (fid) {
            stream << colorString(QString::number(fid, 16), 31) << ' ';
            stream << colorString(QString::number(Baloo::idToDeviceId(fid)), 28) << ' ';
            stream << colorString(QString::number(Baloo::idToInode(fid)), 28) << ' ';
        }
        if (fid && tr.hasDocument(fid)) {
            stream << colorString(url, 32);
            if (!internalUrl.isEmpty() && internalUrl != url) {
                // The document is know by a different name inside the DB,
                // e.g. a hardlink, or untracked rename
                stream << QLatin1String(" [") << internalUrl << QLatin1Char(']');
            }
            stream << '\n';
        }
        else {
            stream << i18n("%1: No index information found", url) << '\n';
            continue;
        }

        Baloo::DocumentTimeDB::TimeInfo time = tr.documentTimeInfo(fid);
        stream << QStringLiteral("\tMtime: %1 ").arg(time.mTime)
               << QDateTime::fromSecsSinceEpoch(time.mTime).toString(Qt::ISODate)
               << QStringLiteral("\n\tCtime: %1 ").arg(time.cTime)
               << QDateTime::fromSecsSinceEpoch(time.cTime).toString(Qt::ISODate)
              << '\n';

        const QJsonDocument jdoc = QJsonDocument::fromJson(tr.documentData(fid));
        const QVariantMap varMap = jdoc.object().toVariantMap();
        KFileMetaData::PropertyMultiMap propMap = variantToPropertyMultiMap(varMap);
        if (!propMap.isEmpty()) {
            stream << "\tCached properties:" << '\n';
        }
        for (auto it = propMap.constBegin(); it != propMap.constEnd(); ++it) {
            QString str;
            if (it.value().typeId() == QMetaType::QVariantList) {
                QStringList list;
                const auto vars = it.value().toList();
                for (const QVariant& var : vars) {
                    list << var.toString();
                }
                str = list.join(QLatin1String(", "));
            } else {
                str = it.value().toString();
            }

            KFileMetaData::PropertyInfo pi(it.key());
            stream << "\t\t" << pi.displayName() << ": " << str << '\n';
        }

        if (parser.isSet(QStringLiteral("x"))) {
            QVector<QByteArray> terms = tr.documentTerms(fid);
            QVector<QByteArray> fileNameTerms = tr.documentFileNameTerms(fid);
            QVector<QByteArray> xAttrTerms = tr.documentXattrTerms(fid);

            auto join = [](const QVector<QByteArray>& v) {
                QByteArray ba;
                for (const QByteArray& arr : v) {
                    ba.append(arr);
                    ba.append(' ');
                }
                return QString::fromUtf8(ba);
            };

            auto propertiesBegin = std::stable_partition(terms.begin(), terms.end(),
                [](const auto & t) { return t.isEmpty() || t[0] < 'A' || t[0] > 'Z'; });
            const QVector<QByteArray> propertyTerms{propertiesBegin, terms.end()};
            terms.erase(propertiesBegin, terms.end());

            stream << "\n" << i18n("Internal Info") << "\n";
            stream << i18n("File Name Terms: %1", join(fileNameTerms)) << "\n";
            stream << i18n("%1 Terms: %2", QStringLiteral("XAttr"), join(xAttrTerms)) << '\n';
            stream << i18n("Plain Text Terms: %1", join(terms)) << "\n";
            stream << i18n("Property Terms: %1", join(propertyTerms)) << "\n";

            QHash<int, QStringList> propertyWords;
            KLocalizedString errorPrefix = ki18nc("Prefix string for internal errors", "Internal Error - %1");

            for (const QByteArray& arr : propertyTerms) {
                auto arrAsPrintable = [arr]() {
                    return QString::fromLatin1(arr.toPercentEncoding());
                };

                if (arr.length() < 1) {
                    auto error = QStringLiteral("malformed term (short): '%1'\n").arg(arrAsPrintable());
                    stream << errorPrefix.subs(error).toString();
                    continue;
                }

                const QString word = QString::fromUtf8(arr);

                if (word[0] == QLatin1Char('X')) {
                        if (word.length() < 4) {
                            // 'X<num>-<value>
                            auto error = QStringLiteral("malformed property term (short): '%1' in '%2'\n").arg(word, arrAsPrintable());
                            stream << errorPrefix.subs(error).toString();
                            continue;
                        }
                        const int posOfNonNumeric = word.indexOf(QLatin1Char('-'), 2);
                        if ((posOfNonNumeric < 0) || ((posOfNonNumeric + 1) == word.length())) {
                            auto error = QStringLiteral("malformed property term (no data): '%1' in '%2'\n").arg(word, arrAsPrintable());
                            stream << errorPrefix.subs(error).toString();
                            continue;
                        }

                        bool ok;
                        const QStringView prop = QStringView(word).mid(1, posOfNonNumeric - 1);
                        int propNum = prop.toInt(&ok);
                        if (!ok) {
                            auto error = QStringLiteral("malformed property term (bad index): '%1' in '%2'\n").arg(prop, arrAsPrintable());
                            stream << errorPrefix.subs(error).toString();
                            continue;
                        }

                        const QString value = word.mid(posOfNonNumeric + 1);
                        propertyWords[propNum].append(value);
                }
            }

            for (auto it = propertyWords.constBegin(); it != propertyWords.constEnd(); it++) {
                auto prop = static_cast<KFileMetaData::Property::Property>(it.key());
                KFileMetaData::PropertyInfo pi(prop);

                stream << pi.name() << ": " << it.value().join(QLatin1Char(' ')) << '\n';
            }
        }
    }
    stream.flush();
    return 0;
}
