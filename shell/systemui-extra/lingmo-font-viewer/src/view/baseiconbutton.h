#ifndef BASEICONBUTTON_H
#define BASEICONBUTTON_H

#include <QObject>
#include <QEvent>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

#include "picturetowhite.h"

/* 功能按钮 */
class BaseIconButton : public QPushButton
{
    Q_OBJECT
public:
    /* 图标路径, 按钮名称， 按钮状态, 图标按钮间距， 宽度， 高度 */
    BaseIconButton(QString iconPath, QString value = "", bool state = false);
    ~BaseIconButton();
    void init(void);         /* 初始化组件 */
    void changeStyle();      /* 主题改变 */
    void changeFontSize();   /* 字号改变 */

private:

private:
    QString m_iconPath;
    QString m_value;
    bool m_state;
    int m_space;
    int m_width;
    int m_height;

    PictureToWhite *m_pictureToWhite = nullptr;   /* 图标泛白类 */
    QLabel *m_iconLab = nullptr;    /* 图标 */
    QLabel *m_valueLab = nullptr;   /* 名称 */
    QHBoxLayout *m_hlayout = nullptr;

public slots:
    void slotChangeState(bool state);   /* 设置互斥（根据state） */
    void slotChangeFontName();   

};

#endif // BASEICONBUTTON_H
