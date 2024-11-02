#ifndef KARECLASS_H
#define KARECLASS_H

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QMainWindow>
#include <QPushButton>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QLabel>
#include <lingmostylehelper/lingmostylehelper.h>
#include "titlewidget.h"
#include <QCloseEvent>

class kareClass : public QWidget
{
    Q_OBJECT
public:
    explicit kareClass(QWidget *parent = nullptr);


    bool isHaveKare();


    QStringList kareImageList;

    QString kareImage = "Local System";

    QPushButton *okButton;


signals:

public slots:

    void onOkBtnClicked(bool check);

};

#endif // KARECLASS_H
