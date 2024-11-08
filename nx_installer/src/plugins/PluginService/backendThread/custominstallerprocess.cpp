#include <QDebug>
#include "custominstallerprocess.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/kdir.h"

namespace KInstaller {

using namespace KServer;

CustomInstallerProcess::CustomInstallerProcess()
{

}

void CustomInstallerProcess::exec()
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
    qDebug() << "error:" << error;
}

}
