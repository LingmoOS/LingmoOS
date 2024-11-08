#ifndef PUSHBUTTONCHECK_H
#define PUSHBUTTONCHECK_H

#include <QWidget>
#include <QPushButton>
#include <QPaintEvent>
#include <QLabel>
#include <QString>

class PushButtonCheck : public QPushButton
{
    Q_OBJECT
public:
    explicit PushButtonCheck(QString str, QWidget *parent = nullptr);

public:
    QLabel* m_labelpix;

    void setLabelShow(bool flag) {
        m_labelpix->setVisible(flag);
        if(flag) {
            this->setChecked(true);
            //this->setFocus();
        }else
            this->setChecked(false);
    }

public slots:
   void clicked_slot();                      //点击响应函数
   void other_clicked_slot(QString objname); //更新勾选状态

signals:
   void clicked_obj(PushButtonCheck* obj);   //用于以按钮为实参，将此按钮的点击事件发送出去

};

#endif // PUSHBUTTONCHECK_H
