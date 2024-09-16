#ifndef CREATEBACKUPFILEWIDGET_H
#define CREATEBACKUPFILEWIDGET_H

#include <QFrame>
#include <QMap>
#include <QLineEdit>
class QListView;
class QLabel;
class QStorageInfo;
class QStandardItem;
class QToolButton;
class QPushButton;

class NameLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    NameLineEdit(QWidget *parent = nullptr);
    ~NameLineEdit();
signals:
    void out();

protected:
    void focusOutEvent(QFocusEvent *event) override;
};
class LineEditWidget : public QFrame
{
    Q_OBJECT

public:
    LineEditWidget(QWidget *parent = nullptr);
    ~LineEditWidget();
    void clear();
    void setBackupFileName(QString name);
    QString getBackupFileName();
private slots:
    void enterEditMode();

    void exitEditMode();

    void adjustButtonPosition();

private:
    NameLineEdit *lineEdit;
    QToolButton *editButton;
};

class CreateBackupFileWidget : public QFrame
{
    Q_OBJECT
public:
    CreateBackupFileWidget(QWidget *parent = nullptr);
    ~CreateBackupFileWidget();

    void sendOptions();
    void clear();
    void setBackupFileName(QString name);

private:
    void initUI();
    void initDiskListView();
    void checkDisk();

public slots:
    void nextPage();
    void backPage();
    void updateuserSelectFileSize(const QString &sizeStr);
    void updaeBackupFileSize();
    void getUpdateDeviceSingla();
    void updateDevice(const QStorageInfo &device, const bool &isAdd);

private:
    quint64 userSelectFileSize{ 0 };
    quint64 allSize{ 0 };
    QString backupFileName{ "" };
    QLabel *promptLabel{ nullptr };
    QLabel *backupFileSizeLabel{ nullptr };
    QListView *diskListView{ nullptr };
    //    QLineEdit *fileNameInput{ nullptr };
    LineEditWidget *fileNameInput{ nullptr };
    QPushButton *determineButton{ nullptr };
    QList<QStorageInfo> deviceList;

    QMap<QStandardItem *, quint64> diskCapacity;
};

#endif // CREATEBACKUPFILEWIDGET_H
