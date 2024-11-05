#ifndef IMGFRAME_H
#define IMGFRAME_H

#include <QObject>
#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QGridLayout>
#include <QTextBrowser>
#include <QLabel>
#include "QLineEdit"
#include "QRadioButton"
#include "QPushButton"
#include <QTimer>
#include "../PluginService/ui_unit/AutoResize.h"
#include "../PluginService/ui_unit/middleframemanager.h"

namespace KInstaller {


class ImgFrame : public MiddleFrameManager
{
    Q_OBJECT
public:
    explicit ImgFrame(QWidget *parent = nullptr);

    void initUI();
    void initAllConnect();
    void addStyleSheet();
    void translateStr();


signals:
public slots:
    void clickNextButton();
    void SetFilePath();
    void ChangeLoad1();
     void ChangeLoad2();
private:
    void keyPressEvent(QKeyEvent *event);
     void changeEvent(QEvent *event);
public:

    QLabel* m_mainTitle;
    QRadioButton *liveChoice;
    QRadioButton *ghostChoice;
    QPushButton *openFile;
    QLineEdit *filePath;
    QString imgPath;

};
}
#endif // ImgFrame_H
