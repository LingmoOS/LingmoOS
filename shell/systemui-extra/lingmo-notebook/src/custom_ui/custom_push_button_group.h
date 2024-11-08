#ifndef __CUSTOM_PUSH_BUTTON_GROUP_H__
#define __CUSTOM_PUSH_BUTTON_GROUP_H__

#include <QFrame>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class CustomPushButtonGroup; }
QT_END_NAMESPACE

class CustomPushButtonGroup : public QFrame
{
    Q_OBJECT
public:
    enum BtnState {
        Left = 0,
        Right,
        Mid
    };
    CustomPushButtonGroup(QWidget *parent = nullptr);
    ~CustomPushButtonGroup();

    void loadQSS();
    void addButton(QPushButton *button, BtnState state);
    void addSeparator();
    void frameUpdate(bool isClassical);
private:
    Ui::CustomPushButtonGroup *ui;
    static QString m_buttonGroupQSS;
};
#endif // __CUSTOM_PUSH_BUTTON_GROUP_H__
