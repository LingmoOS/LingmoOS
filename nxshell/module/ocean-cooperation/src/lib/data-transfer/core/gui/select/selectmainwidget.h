#ifndef SELECTMAINWIDGET_H
#define SELECTMAINWIDGET_H

#include "common/qtcompat.h"

#include <QFrame>
#include <QCheckBox>
class QLabel;
class SidebarWidget;
class QPushButton;
class IndexLabel;
enum SelectItemName { FILES, APP, CONFIG };

class SelectItem : public QFrame
{
    Q_OBJECT
public:
    SelectItem(QString text, QIcon icon, SelectItemName name, QWidget *parent = nullptr);
    ~SelectItem() override;
    void updateSelectSize(QString num);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(EnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void initEditFrame();

public:
    SelectItemName name;
    bool isOk{ false };

private:
    QFrame *editFrame{ nullptr };
    QLabel *sizeLabel{ nullptr };

public slots:
    void changeState(const bool &ok);
signals:
    void changePage();
};

class SelectMainWidget : public QFrame
{
    Q_OBJECT
public:
    SelectMainWidget(QWidget *parent = nullptr);
    ~SelectMainWidget();
    void changeSelectframeState(const SelectItemName &name);

    void changeText();
    void clear();

private:
    void initUi();

signals:
    void updateBackupFileSize();
public slots:
    void nextPage();
    void backPage();
    void selectPage();

private:
    SelectItem *fileItem{ nullptr };
    SelectItem *appItem{ nullptr };
    SelectItem *configItem{ nullptr };
    QLabel *titileLabel{ nullptr };
    QPushButton *nextButton{ nullptr };
    IndexLabel *LocalIndelabel{ nullptr };
    IndexLabel *InternetIndelabel{ nullptr };

    QString InternetText{ tr("Select data to transfer") };
    QString LocalText{ tr("Please select the content to back up") };
    QString BtnInternetText{ tr("Start transfer") };
    QString BtnLocalText{ tr("Next") };
};

#endif // SELECTMAINWIDGET_H
