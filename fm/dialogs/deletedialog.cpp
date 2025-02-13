#include "deletedialog.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <KIO/DeleteJob>

DeleteDialog::DeleteDialog(const QList<QUrl> &urls, QWidget *parent)
    : QDialog(parent)
    , m_urls(urls)
{
    setWindowTitle(tr("Delete Files"));
    setMinimumWidth(400);

    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QString text;
    if (urls.count() == 1) {
        text = tr("Are you sure you want to permanently delete \"%1\"?")
                .arg(urls.first().fileName());
    } else {
        text = tr("Are you sure you want to permanently delete %1 files?")
                .arg(urls.count());
    }

    m_label = new QLabel(text);
    m_label->setWordWrap(true);
    layout->addWidget(m_label);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    layout->addLayout(btnLayout);

    m_cancelButton = new QPushButton(tr("Cancel"));
    m_deleteButton = new QPushButton(tr("Delete"));
    // m_deleteButton->setStyleSheet("QPushButton { color: red; }");

    btnLayout->addWidget(m_cancelButton);
    btnLayout->addWidget(m_deleteButton);

    connect(m_cancelButton, &QPushButton::clicked, this, &DeleteDialog::reject);
    connect(m_deleteButton, &QPushButton::clicked, this, &DeleteDialog::accept);
}

void DeleteDialog::accept()
{
    KIO::DeleteJob *job = KIO::del(m_urls);
    job->start();

    QDialog::accept();
} 