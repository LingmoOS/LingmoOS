#ifndef CUSTOMINSTALLERPROCESS_H
#define CUSTOMINSTALLERPROCESS_H

#include <QObject>
namespace KInstaller {

class CustomInstallerProcess :public QObject
{
    Q_OBJECT
public:
    explicit CustomInstallerProcess();

signals:
    void signalProcessStatus(int exitCode);

public slots:
    void exec();
};
}
#endif // CUSTOMINSTALLERPROCESS_H
