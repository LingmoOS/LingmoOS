#ifndef UPLOADFILEWIDGET_H
#define UPLOADFILEWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>

class UploadFileFrame : public QFrame
{
    Q_OBJECT

public:
    UploadFileFrame(QWidget *parent = nullptr);
    ~UploadFileFrame() override;

    void uploadFile();
    QString getZipFilePath() const;
    void themeChanged(int theme);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

signals:
    void updateUI(int status);

private:
    void initUI();
    void initFileFrame();

private:
    QString zipFilePath;
    QFrame *fileFrame { nullptr };
    bool lightTheme { true };
    QPushButton *closeBtn { nullptr };
};

enum uploadStatus {
    Initial = 0,
    valid,
    formaterror
};

class UploadFileWidget : public QFrame
{
    Q_OBJECT

public:
    UploadFileWidget(QWidget *parent = nullptr);
    ~UploadFileWidget();

    bool checkBackupFile(const QString &filePath);
    void clear();

public slots:
    void nextPage();
    void backPage();
    void themeChanged(int theme);

private:
    void initUI();
Q_SIGNALS:
    void Initial();

private:
    QPushButton *backButton { nullptr };
    QPushButton *nextButton { nullptr };
    QLabel *tipLabel { nullptr };
    UploadFileFrame *uploadFileFrame { nullptr };
};

#endif
