#ifndef MIDDLEFRAMEMANAGER_H
#define MIDDLEFRAMEMANAGER_H

#include <QObject>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QGridLayout>

namespace KInstaller {

class MiddleFrameManager :public QWidget
{
    Q_OBJECT
public:
    explicit MiddleFrameManager(QWidget *parent = nullptr);
    virtual ~MiddleFrameManager();

    void initUI();                  //显示ui
    void initAllConnect();          //信号槽链接
    void addStyleSheet();           //添加样式
    void translateStr();

signals:
    void signalStackPageChanged(int step);
    void signalStackFrameManager(QWidget* widget);
    void signalStartInstall();
    void signalInstallError(QString str);
    void signalInstalScriptlFinished();
    void signalKeyboradClose();
    void backspacepressed();
    void enterpressed();

public slots:
    virtual void clickNextButton() = 0;
//    void clickPrevButton();
    virtual void setSizebyFrame(QWidget *pFrame, int SCreenWidth, int SCreenHeight){
        return;
    }

private:
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);

public:
    QWidget* m_Widget ;
//    QPushButton* m_preBtn = nullptr;
    QPushButton* m_nextBtn = nullptr;
};
}
#endif // MIDDLEFRAMEMANAGER_H
