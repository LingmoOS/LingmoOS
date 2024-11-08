#include "quickinsatllerprocess.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/kdir.h"
#include <QDebug>

namespace KInstaller {

using namespace KServer;

QuickInsatllerProcess::QuickInsatllerProcess(QObject *parent) : QObject(parent)
{

}

void QuickInsatllerProcess::exec()
{
    QString error = "";
    QString output =  "";
    int exitcode = 0;
    QString ScriptPath =  KServer::GetLingmoInstallPath() + "/scripts" ;
    bool isOK;
    isOK = KServer::KCommand::getInstance()->RunScripCommand("/bin/bash", {"z.sh"}, ScriptPath,  output, error, exitcode);
    if(!isOK) {
        emit signalProcessStatus(exitcode);//tr("An exception occurred during script execution!")
    } else {
        emit signalProcessStatus(exitcode);
    }
    qDebug() <<"error:" << error;
}

}
