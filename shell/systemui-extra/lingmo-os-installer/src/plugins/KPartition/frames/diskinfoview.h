#ifndef DISKINFOVIEW_H
#define DISKINFOVIEW_H

#include <QWidget>
#include <QFrame>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "disktype_name.h"
#include "qcheckbox.h"


class DiskInfoView : public QWidget
{
    Q_OBJECT
public:
    explicit DiskInfoView(QWidget *parent = nullptr);
    void addStyleSheet();
    void initAllConnect();
    void translateStr();
    QString devPath;
    void setDevPath(QString dev) {devPath = dev;}
    QString getDevPath() {return devPath;}
    QSize getFramesize();
    void setWidgetDisabled();
    void setProgressStyle();

    void setDiskType(QString type) {m_strDiskType = type; diskType->setText(m_strDiskType);}
    void setDiskName(QString name) {m_strDiskName = name; diskName->setText(m_strDiskName);}
    void setDiskSize(QString size) {m_strDiskSize = size; diskSize->setText(m_strDiskSize);}
    void setDiskPath(QString path) {m_strDiskpath = path; diskPath->setText(m_strDiskpath);}

    QString getDiskType() {m_strDiskType = diskType->text(); return m_strDiskType; }
    QString getDiskName() {m_strDiskName = diskName->text(); return m_strDiskName;}
    QString getDiskSize() {m_strDiskSize = diskSize->text(); return m_strDiskSize; }
    QString getDiskPath() {m_strDiskpath = diskPath->text(); return m_strDiskpath;}

    void setProgressValue(double value) {progressBar->setValue(value);}

private:
    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *);


protected:
    bool eventFilter(QObject *object, QEvent *event);

signals:
    void signalClicked(QString dev);
     void signalClickView(int k);

public slots:
    void slotFrameClicked();
    void clickDataSel();

public:
    QWidget *widget;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QLabel *diskType;
    QLabel *diskName;
    QLabel *diskPath;
    QProgressBar *progressBar;
    QLabel *diskSize;
    QLabel *hardDiskPng;
    QLabel *checkpic;
    bool checkable;
    int mIndex;
    QCheckBox *dataSel;

    QString m_strDiskType, m_strDiskName, m_strDiskSize, m_strDiskpath;

};

#endif // DISKINFOVIEW_H
