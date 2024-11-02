#ifndef FINISHEDFRAME_H
#define FINISHEDFRAME_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include "../PluginService/ui_unit/middleframemanager.h"
#include <QKeyEvent>

namespace KInstaller {

class FinishedFrame : public QWidget
{
    Q_OBJECT
public:
    explicit FinishedFrame(QWidget *parent = nullptr);
    void initUI();
    void translateStr();
    void initAllConnect();
    void addStyleSheet();
    void clickNextButton();
private:
    void changeEvent(QEvent *event);
protected:
    void keyPressEvent(QKeyEvent *event);
signals:
    void enterKeyPressed();
public slots:
    void clickRestartBtn();

public:
    QLabel* m_mainTitle;
    QLabel* m_finishLable;
    QLabel* m_tipInfo;
    QPushButton* m_restartBtn;

};
}
#endif // FINISHEDFRAME_H
