#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "type_defines.h"

#include <QScopedPointer>

namespace data_transfer_core {

class MainWindowPrivate;
class MainWindow : public CrossMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~MainWindow();

#if defined(_WIN32) || defined(_WIN64)
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
#endif

private:
    QScopedPointer<MainWindowPrivate> d;
};

}

#endif   // MAINWINDOW_H
