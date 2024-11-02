#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QToolButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QListWidget>
#include <QProgressBar>
#include <QRadioButton>
#include <QScrollBar>
#include <QComboBox>
#include <QSlider>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QPalette>
#include <QColor>
#include <QHeaderView>
#include <QCompleter>
#include <QLabel>

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();


private slots:
    void onPushbutton_1Clicked();
    void onPushbutton_2Clicked();
    void onPushbutton_3Clicked();
    void onPushbutton_4Clicked();
    void onPushbutton_5Clicked();
    void onPushbutton_6Clicked();
    void onPushbutton_7Clicked();
    void onPushbutton_8Clicked();
    void onPushbutton_9Clicked();
    void onPushbutton_10Clicked();
    void onPushbutton_11Clicked();
    void onPushbutton_12Clicked();

};
#endif // WIDGET_H
