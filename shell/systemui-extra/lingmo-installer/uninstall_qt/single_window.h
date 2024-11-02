#ifndef SINGLE_WINDOW_H
#define SINGLE_WINDOW_H

#include <QWidget>
#include <QMessageBox>

class single_window
{
public:
    explicit single_window();
    ~single_window();

    void create_interface(void);
    void init(void);

    QMessageBox *p_message_box;

signals:

};

#endif // SINGLE_WINDOW_H
