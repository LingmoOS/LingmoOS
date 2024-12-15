#ifndef TRANSFERRWIDGET_H
#define TRANSFERRWIDGET_H

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QItemDelegate>
#include <QListView>
#include <QVector>
#include "../type_defines.h"
class ProgressBarLabel;

class ProcessWindow : public ProcessDetailsWindow
{
    Q_OBJECT
public:
   ProcessWindow(QFrame *parent = nullptr);
   ~ProcessWindow()override;
   void updateContent(const QString &name, const QString &type);
   void changeTheme(int theme);
private:
   void init();
};
class TransferringWidget : public QFrame
{
    Q_OBJECT

public:
    TransferringWidget(QWidget *parent = nullptr);
    ~TransferringWidget();

    QString resetContent(const QString &type, const QString &content);
    QString getTransferFileName(const QString &fullPath, const QString &targetPath);

public slots:
    void updateInformationPage();
    void changeTimeLabel(const QString &time);
    void changeProgressLabel(const int &ratio);
    void updateProcess(const QString &tpye, const QString &content, int progressbar,
                       int estimatedtime);
    void themeChanged(int theme);
    void clear();

private:
    void initUI();
    void initConnect();
private:
    QWidget *iconWidget { nullptr };
    QLabel *titileLabel { nullptr };
    QLabel *fileLabel { nullptr };
    QLabel *displayLabel { nullptr };
    QLabel *timeLabel { nullptr };
    ProgressBarLabel *progressLabel { nullptr };
    QFrame *fileNameFrame { nullptr };
    ProcessWindow *processWindow { nullptr };
    QStringList finishJobs;
    bool isVisible = false;
};

class ProgressBarLabel : public QLabel
{
public:
    ProgressBarLabel(QWidget *parent = nullptr);
    void setProgress(int progress);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_progress;
};

#endif
