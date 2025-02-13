#pragma once

/*
 * SPDX-FileCopyrightText: 2003-2009 Craig Drummond <craig@kde.org>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "Family.h"
#include "Folder.h"
#include "FontinstIface.h"
#include "kfontinst_export.h"
#include <KJob>
#include <QDBusConnection>
#include <QObject>
#include <QSet>
#include <QStringList>

#define FONTINST_PATH "/FontInst"

class QTimer;

namespace KFI
{
class KFONTINST_EXPORT FontInst : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fontinst")

public:
    enum EStatus {
        STATUS_OK = 0,
        STATUS_SERVICE_DIED = KJob::UserDefinedError + 500,
        STATUS_BITMAPS_DISABLED,
        STATUS_ALREADY_INSTALLED,
        STATUS_NOT_FONT_FILE,
        STATUS_PARTIAL_DELETE,
        STATUS_NO_SYS_CONNECTION,
    };

    enum EFolder {
        FOLDER_SYS,
        FOLDER_USER,

        FOLDER_COUNT,
    };

    enum {
        SYS_MASK = 0x01,
        USR_MASK = 0x02,
    };

    static void registerTypes()
    {
        qDBusRegisterMetaType<Families>();
        qDBusRegisterMetaType<Family>();
        qDBusRegisterMetaType<Style>();
        qDBusRegisterMetaType<File>();
        qDBusRegisterMetaType<Style>();
        qDBusRegisterMetaType<QList<Families>>();
    }

    static bool isStarted(OrgKdeFontinstInterface *iface)
    {
        QDBusReply<QStringList> reply = iface->connection().interface()->registeredServiceNames();
        if (reply.isValid()) {
            QStringList services(reply.value());
            QStringList::ConstIterator it(services.begin()), end(services.end());
            for (; it != end; ++it)
                if ((*it) == OrgKdeFontinstInterface::staticInterfaceName())
                    return true;
        }
        return false;
    }

    FontInst();
    ~FontInst() override;

public Q_SLOTS:

    Q_NOREPLY void list(int folders, int pid);
    Q_NOREPLY void statFont(const QString &font, int folders, int pid);
    Q_NOREPLY void install(const QString &file, bool createAfm, bool toSystem, int pid, bool checkConfig);
    Q_NOREPLY void uninstall(const QString &family, quint32 style, bool fromSystem, int pid, bool checkConfig);
    Q_NOREPLY void uninstall(const QString &name, bool fromSystem, int pid, bool checkConfig);
    Q_NOREPLY void move(const QString &family, quint32 style, bool toSystem, int pid, bool checkConfig);
    Q_NOREPLY void enable(const QString &family, quint32 style, bool inSystem, int pid, bool checkConfig);
    Q_NOREPLY void disable(const QString &family, quint32 style, bool inSystem, int pid, bool checkConfig);
    Q_NOREPLY void removeFile(const QString &family, quint32 style, const QString &file, bool fromSystem, int pid, bool checkConfig);
    Q_NOREPLY void reconfigure(int pid, bool force);
    Q_SCRIPTABLE QString folderName(bool sys);
    Q_SCRIPTABLE void saveDisabled();

Q_SIGNALS:

    void fontList(int pid, const QList<KFI::Families> &families);
    void status(int pid, int status);
    void fontStat(int pid, const KFI::Family &font);
    void fontsAdded(const KFI::Families &families);
    void fontsRemoved(const KFI::Families &families);

private Q_SLOTS:

    void connectionsTimeout();
    void fontListTimeout();

private:
    void updateFontList(bool emitChanges = true);
    void toggle(bool enable, const QString &family, quint32 style, bool inSystem, int pid, bool checkConfig);
    void addModifedSysFolders(const Family &family);
    void checkConnections();
    bool findFontReal(const QString &family, const QString &style, EFolder folder, FamilyCont::ConstIterator &fam, StyleCont::ConstIterator &st);
    bool findFont(const QString &font, EFolder folder, FamilyCont::ConstIterator &fam, StyleCont::ConstIterator &st, bool updateList = true);
    bool findFontReal(const QString &family, quint32 style, EFolder folder, FamilyCont::ConstIterator &fam, StyleCont::ConstIterator &st);
    bool findFont(const QString &family, quint32 style, EFolder folder, FamilyCont::ConstIterator &fam, StyleCont::ConstIterator &st, bool updateList = true);
    int performAction(const QVariantMap &args);

    static void emergencySave(int sig);

private:
    QTimer *m_connectionsTimer, *m_fontListTimer;
    QSet<int> m_connections;
};

}
