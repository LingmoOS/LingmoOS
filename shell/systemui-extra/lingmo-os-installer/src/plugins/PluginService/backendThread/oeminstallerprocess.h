#ifndef OEMINSTALLERPROCESS_H
#define OEMINSTALLERPROCESS_H

#include <QObject>
namespace KInstaller {
class OemInstallerProcess : public QObject
{
    Q_OBJECT
public:
    explicit OemInstallerProcess(QObject *parent = nullptr);
signals:
    void signalProcessStatus(int exitCode);

public slots:
    void exec();

};
}
#endif // OEMINSTALLERPROCESS_H
