#ifndef MAINPARTFRAME_H
#define MAINPARTFRAME_H

#include <QObject>
#include <QWidget>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QThread>
#include <QTimer>
#include "quickpartitionframe.h"
#include "createpartitionframe.h"
#include "coexistpartitionframe.h"
#include "custompartitionframe.h"
#include "prepareinstallframe.h"
#include "delegate/partitionmodel.h"
#include "delegate/partition_delegate.h"
#include "delegate/full_partition_delegate.h"
#include "delegate/validatestate.h"
#include "mainpartframe.h"
#include "../PluginService/ui_unit/messagebox.h"
#include "../PluginService/ui_unit/middleframemanager.h"
namespace KInstaller {

class MainPartFrame : public MiddleFrameManager
{
    Q_OBJECT
public:
    explicit MainPartFrame(QWidget *parent = nullptr);
    ~MainPartFrame();
    void initUI();
    void initAllConnect();
    void addStyleSheet();
    void translateStr();
    void writeSettingIni(InstallBtn btn);
    void scanDevices();
    int validatePartition();

    QString checkLoaderAndPartitionTableStr();

signals:
    void leftKeyPressed();
    void rightKeyPressed();
    void upKeyPressed();
    void downKeyPressed();
private:
    void changeEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event);

public slots:
    void showPartFrame();
    void showCustomPartFrame();
    void showQuikPartFrame();
    void showCoexistFrame();
    void customParting();
//    void slotUpdateMainPartFrame(int nextPage);
    void getChoiceDisk(bool flag);
    void finishLoadDisk();
    void setFrameFocus();
    void DiskNumCheck(int num);
    void setSizebyFrame(QWidget *pFrame, int SCreenWidth, int SCreenHeight);
    void clickNextButton();

public:
    QStackedWidget* m_mainStackWidget = nullptr;
    FullPartitionFrame* m_fullPartFrame = nullptr;

    CoexistPartitionFrame* m_coexitPartFrame = nullptr;
    CustomPartitionFrame* m_customPartFrame = nullptr;

    PrepareInstallFrame* m_prepareFrame = nullptr;

    QPushButton* m_fullInstallBtn = nullptr;
    QPushButton* m_customInstallBtn = nullptr;
    QPushButton* m_coexistInstallBtn = nullptr;
    QLabel* m_mainTitle = nullptr;
    QLabel* promptLabel = nullptr;
    CustomPartitiondelegate* m_partDelegate;
    FullPartitionDelegate* m_quickDelegate;
//    PartitionDelegate* m_delegate;
    PartitionModel* m_partModel;

    DeviceList devlist;

    InstallBtn enumCheckBtn;

    QString partStr;

    ValidateStates m_validateStates;
    QString validateMessage;
    MessageBox *message ;
    QThread* m_partModelThread;
    QTimer* timer;
    QLabel* m_loadpic;
    int var;

    QWidget *m_pFrame;
    int m_SCreenWidth;
    int m_SCreenHeight;

};

}
#endif // MAINPARTFRAME_H
