#include "single_window.h"
#include <QObject>
#include <QIcon>

single_window::single_window()
{
    init();
    create_interface();
}
single_window::~single_window()
{
    if (p_message_box != NULL) {
        delete p_message_box;
    }
}

void single_window::init(void)
{
    p_message_box = NULL;
}

void single_window::create_interface(void)
{
    p_message_box = new QMessageBox(QMessageBox::Warning , QObject::tr("warning") , QObject::tr("The last software is being uninstalled,Please uninstall later!") , QMessageBox::Ok);
    p_message_box->setWindowIcon(QIcon("://image/install.png"));
    p_message_box->setButtonText(QMessageBox::Ok , QObject::tr("OK"));

    return;
}
