#ifndef DELETEDIALOG_H
#define DELETEDIALOG_H

#include <QDialog>
#include <QUrl>

class QLabel;
class QPushButton;

class DeleteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeleteDialog(const QList<QUrl> &urls, QWidget *parent = nullptr);

private slots:
    void accept() override;

private:
    QList<QUrl> m_urls;
    QLabel *m_label;
    QPushButton *m_deleteButton;
    QPushButton *m_cancelButton;
};

#endif // DELETEDIALOG_H 