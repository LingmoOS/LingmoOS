#ifndef EDIT_H
#define EDIT_H
#include <QLineEdit>
#include <QKeyEvent>
#include <QDebug>
class Edit : public QLineEdit
{
    Q_OBJECT
public:
    explicit Edit(QWidget *parent = 0);

public:
    QString Text;

private:
    bool m_rightButtonPress = false;

    void focusOutEvent(QFocusEvent *event);
    void keyPressEvent(QKeyEvent *event);

    bool event(QEvent *event);
Q_SIGNALS:
    void renamefinished();
    void showOrigName();
    void dealDoubleClicked(bool isleaveOrNot);
};

#endif // BUTTON_H
