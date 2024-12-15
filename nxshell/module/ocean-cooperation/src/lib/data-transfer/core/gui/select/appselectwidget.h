#ifndef APPSELECTWIDGET_H
#define APPSELECTWIDGET_H

#include <QCheckBox>
#include <QFrame>
#include <QIcon>
#include <QLabel>

#include "../select/selectmainwidget.h"

class QVBoxLayout;
class QListView;
class QPushButton;
class SelectListView;
class AppSelectWidget : public QFrame
{
    Q_OBJECT

public:
    AppSelectWidget(QWidget *parent = nullptr);
    ~AppSelectWidget();
    void changeText();
    void clear();
public slots:
    void nextPage();
    void backPage();

signals:
    void isOk(const SelectItemName &name);

private:
    void initUI();
    void initSelectFrame();
    void sendOptions();
    void delOptions();

private:
    QFrame *selectFrame{ nullptr };
    SelectListView *appView{ nullptr };

    QVBoxLayout *selectMainLayout{ nullptr };
    QPushButton *determineButton{ nullptr };
    QPushButton *cancelButton{ nullptr };
    QLabel *titileLabel{ nullptr };

    QString InternetText{ tr("Select apps to transfer") };
    QString LocalText{ tr("Please select the applications to back up") };
};

#endif
