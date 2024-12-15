#ifndef CHOOSEWIDGET_H
#define CHOOSEWIDGET_H

#include <QCheckBox>
#include <QFrame>
#include <QLabel>
#include <QPainter>
#include <QToolButton>
#include "../type_defines.h"
class ModeItem;
class ChooseWidget : public QFrame
{
    Q_OBJECT

public:
    ChooseWidget(QWidget *parent = nullptr);
    ~ChooseWidget();

public slots:
    void nextPage();
    void themeChanged(int theme);

private:
    void initUI();
    void sendOptions();

private:
    QString transferMethod;
    QPushButton *nextButton = nullptr;
    int nextpage;
    ModeItem *winItem { nullptr };
    ModeItem *packageItem { nullptr };
    QString internetMethodName { tr("From Windows PC") };
#if defined(_WIN32) || defined(_WIN64)
    QString localFileMethodName { tr("Export to local directory") };
    int selecPage1 = PageName::promptwidget;
    int selecPage2 = PageName::selectmainwidget;
#else
    QString localFileMethodName { tr("Import from backup files") };
    int selecPage1 { PageName::promptwidget };
    int selecPage2 { PageName::uploadwidget };
#endif
};

class ModeItem : public QFrame
{
    Q_OBJECT
    friend ChooseWidget;

public:
    ModeItem(QString text, QIcon icon, QWidget *parent = nullptr);
    ~ModeItem() override;

    void setEnable(bool able);
    void themeChanged(int theme);
    void setIcon(QIcon icon);
signals:
    void clicked(bool checked);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    bool enable { true };
    bool checked { false };
    QLabel *iconLabel { nullptr };
    QString itemText;
    bool dark { false };
};

#endif
