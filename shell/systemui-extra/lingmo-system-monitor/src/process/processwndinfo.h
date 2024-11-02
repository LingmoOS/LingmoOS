#ifndef PROCESSWNDINFO_H
#define PROCESSWNDINFO_H

#include <QObject>
#include <KWindowSystem>
#include <QIcon>
#include <QMap>

typedef struct ProcWindowInfo_s{
    WId wndId;
    pid_t procId;
    QString wndName;
    QIcon wndIcon;
}ProcWindowInfo;

class ProcessWndInfo : public QObject
{
    Q_OBJECT
public:
    ProcessWndInfo(QObject* parent = nullptr);
    virtual ~ProcessWndInfo();

public:
    void updateWindowInfos();
    bool acceptWindow(WId window);
    QList<pid_t> getWindowPids();
    WId getWndIdByPid(pid_t pid);
    QString getWndNameByPid(pid_t pid);
    QIcon getWndIconByPid(pid_t pid);

private:
    QMap<pid_t, ProcWindowInfo> m_mapProcWndInfos;
};

#endif // PROCESSWNDINFO_H
