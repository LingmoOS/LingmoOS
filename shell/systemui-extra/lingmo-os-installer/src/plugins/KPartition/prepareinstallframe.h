#ifndef PREPAREINSTALLFRAME_H
#define PREPAREINSTALLFRAME_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QScrollArea>
#include <QPlainTextEdit>
#include <QThread>
#include <QTextBrowser>
#include "../PluginService/ui_unit/middleframemanager.h"
#include "backendThread/quickinsatllerprocess.h"
#include "../PluginService/ui_unit/arrowtooltip.h"
#include "backendThread/custominstallerprocess.h"
#include "partman/device.h"

namespace KInstaller {
using namespace Partman;
enum InstallBtn{
    QuikPart,
    CoexistPart,
    CustomPart,
};
class PrepareInstallFrame : public MiddleFrameManager
{
    Q_OBJECT
public:
    explicit PrepareInstallFrame(QWidget *parent = nullptr);
    ~PrepareInstallFrame();
    void initUI();
    void translateStr();
    void initAllConnect();
    void addStyleSheet();

    void setInstallBtn(InstallBtn state)
    {
        m_InstallBtnState = state;
    }

    void setMainTitleStr(QString str) {m_mainTitle->setText(str);}
    void setTipLabel(QString str) {tiplabel->setText(str);}
    void setPartInfo(QString str) {partInfo->setText(str);}

    void setTipCheckVisible(bool flag) {m_tipCheck->setVisible(flag);}

signals:
    void signalLicenseFrame();
    void signalNextStartInstalling();
    void signalQuickPart();
    void signalCustomPart();
    void enterKeyPressed();

public slots:
    void showLicenseFrame();
    void clickNextButton();
    void flushNextButton();
    void getProcessThread(int exitcode);
private:
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);
public:
    QGridLayout *gridLayout;
    QLabel *tiplabel;
    QLabel *partLabel;
    QTextBrowser *partInfo;
    QLabel* m_mainTitle;

    QScrollArea* scrollArea;
    QWidget* scrollAreaWidgetContents;
    QCheckBox* m_tipCheck;
    QCheckBox* m_licenseCheck;
    QLabel* m_licenseLabel;


    ArrowWidget* w ;
    InstallBtn m_InstallBtnState;
    DeviceList list;

};
}
#endif // PREPAREINSTALLFRAME_H
