#ifndef INSTALLINGOEMCONFIGFRAME_H
#define INSTALLINGOEMCONFIGFRAME_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include "../PluginService/backendThread/oeminstallerprocess.h"
namespace KInstaller {

class InstallingOEMConfigFrame : public QWidget
{
    Q_OBJECT
public:
    explicit InstallingOEMConfigFrame(QWidget *parent = nullptr);
    ~InstallingOEMConfigFrame();
    void initUI();
    void translateStr();
    void addStyleSheet();

private:
    void changeEvent(QEvent *event);
signals:
    void signalExecError(QString str);
   public slots:
    void finishLoadDisk();
public:
    QThread* m_backOEMThread;
    OemInstallerProcess* m_oemProcess;
    QLabel *label, *label1, *m_loadpic;
    int var;
    QTimer *timer;
};

}
#endif // INSTALLINGOEMCONFIGFRAME_H
