#ifndef ZIPFILEPROCESSWIDGET_H
#define ZIPFILEPROCESSWIDGET_H

#include <QFrame>

class QLabel;
class ProgressBarLabel;
class ZipFileProcessWidget : public QFrame
{
    Q_OBJECT
public:
    ZipFileProcessWidget(QWidget *parent = nullptr);
    ~ZipFileProcessWidget();
public slots:
    void updateProcess(const QString &content, int processbar, int estimatedtime);
private:
    void changeFileLabel(const QString &path);
    void changeTimeLabel(const int &time);
    void changeProgressBarLabel(const int &processbar);
    void initUI();
    void nextPage();
private:
    QLabel *fileLabel{ nullptr };
    QLabel *timeLabel{ nullptr };
    ProgressBarLabel *progressLabel{ nullptr };
};

#endif // ZIPFILEPROCESSWIDGET_H
