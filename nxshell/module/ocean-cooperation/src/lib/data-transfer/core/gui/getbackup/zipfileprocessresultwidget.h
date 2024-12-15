#ifndef ZIPFILEPROCESSRESULTWIDGET_H
#define ZIPFILEPROCESSRESULTWIDGET_H

#include <QFrame>
class QLabel;
class QToolButton;
class ZipFileProcessResultWidget : public QFrame
{
    Q_OBJECT
public:
    ZipFileProcessResultWidget(QWidget *parent = nullptr);
    ~ZipFileProcessResultWidget();
    void upWidgetToFailed(const QString &content, int progressbar, int estimatedtime);

private:
    void initUI();
    void successed();

private slots:
    void backPage();
    void informationPage();
private:
    QLabel *displayLabel{ nullptr };
    QLabel *tipLabel1{ nullptr };
    QLabel *tipLabel2{ nullptr };
    QLabel *icon{ nullptr };
    QToolButton *exitButton{ nullptr };
};

#endif // ZIPFILEPROCESSRESULTWIDGET_H
