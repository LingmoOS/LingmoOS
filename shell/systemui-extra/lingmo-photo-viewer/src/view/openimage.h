#ifndef OPENIMAGE_H
#define OPENIMAGE_H
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <QIcon>
#include <QSize>
#include <QFileDialog>
#include <QGSettings>
#include <QFont>
#include <QFileSystemWatcher>
#include "global/variable.h"
#include "controller/interaction.h"
#include <gsettings.hpp>
#include <kborderlessbutton.h>
class OpenImage : public QWidget
{
    Q_OBJECT
public:
    explicit OpenImage(QWidget *parent = nullptr);
    void openImagePath(); //获取并存入配置文件图片路径
private:
    QGSettings *m_pGsettingThemeData = nullptr;
    QPushButton *m_openInCenter;       //圆形图标
    kdk::KBorderlessButton *m_addFile; //“+”图标
    QLabel *m_openText;                //下方文字
    QSize m_iconsize;
    QFont m_ft; //固定字号
    void initInteraction();
    void initconnect();
    void initGsettings();
    void dealSystemGsettingChange();

private Q_SLOTS:

Q_SIGNALS:
    void openImageSignal(QString path);
    void openEmptyFile(bool changHigh); //改变高亮，高亮于上一张
};

#endif // OPENIMAGE_H
