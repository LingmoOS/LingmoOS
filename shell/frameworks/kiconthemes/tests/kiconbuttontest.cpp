#include <QApplication>
#include <kiconbutton.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    //    KIconDialog::getIcon();

    KIconButton button;
    button.show();

    return app.exec();
}
