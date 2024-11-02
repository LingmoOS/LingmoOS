#include "oeminstallerprocess.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/kdir.h"
#include <QDebug>

using namespace KServer;
namespace KInstaller {
OemInstallerProcess::OemInstallerProcess(QObject *parent) : QObject(parent)
{

}
void OemInstallerProcess::exec()
{
    QString error = "";
    QString output =  "";
    int exitcode = 0;
    QString ScriptPath =  KServer::GetLingmoInstallPath() + "/scripts" ;
    bool isOK = false;
    isOK = KServer::KCommand::getInstance()->RunScripCommand("/bin/bash", {"oem.sh"}, ScriptPath,  output, error, exitcode);
    if(!isOK) {
        emit signalProcessStatus(exitcode);//tr("An exception occurred during script execution!")
    } /*else {
        emit signalProcessStatus(exitcode);
    }*/
    qDebug() << "error:" << error;
}
}
