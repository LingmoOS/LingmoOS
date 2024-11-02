#ifndef KCOMMAND_H
#define KCOMMAND_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "serverdll.h"

namespace KServer {
class SERVERDLL_SHARE KCommand :public QObject
{
    Q_OBJECT
public:
    explicit KCommand(QObject* parent = nullptr);
    ~KCommand();
    static KCommand* getInstance();
    static KCommand* initCmd();

    static SERVERDLL_SHARE bool RunScripCommand(const QString& program,
                                                const QStringList& args,
                                                QString& workingPath,
                                                QString& output,
                                                QString& error,
                                                int& exitCode);
    static SERVERDLL_SHARE bool RunScripCommand(const QString& program, const QStringList& args)
    {
        QString workingPath = "";
        QString output = "";
        QString error = "";
        int exitCode = 0;
        return RunScripCommand(program, args, workingPath, output, error, exitCode);
    }
    static SERVERDLL_SHARE bool RunScripCommand(const QString& args,
                                                QString& workingPath,
                                                QString& output,
                                                QString& error,
                                                int& exitCode);

public:
    static KCommand* m_cmdInstance;
signals:
//    void signalProcessFinished(int id);

};
}

#endif // KCOMMAND_H
