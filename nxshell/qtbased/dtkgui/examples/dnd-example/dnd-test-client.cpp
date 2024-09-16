// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfiledragclient.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QFrame>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QApplication>
#include <QDebug>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>

#include <QDBusConnectionInterface>

DGUI_USE_NAMESPACE

static DFileDragClient *c;
static QProgressBar *p;

class DropArea : public QFrame
{
    Q_OBJECT
public:
    explicit DropArea(QString s)
        : lb(new QLabel(s, this))
        , le(new QLineEdit(this))
    {
        setLayout(new QVBoxLayout);
        setFrameShape(Shape::Box);
        layout()->addWidget(lb);
        layout()->addWidget(le);
        lb->setAlignment(Qt::AlignmentFlag::AlignCenter);
        le->setText("/tmp");
        le->setPlaceholderText(QString("do not accept drop when edit is empty"));
        setAcceptDrops(true);
    }
protected:
    void dragEnterEvent(QDragEnterEvent *e)
    {
        if (DFileDragClient::checkMimeData(e->mimeData())) {
            if (le->text().isEmpty())
                e->ignore(); // or e->setDropAction(Qt::IgnoreAction);
            else
                e->acceptProposedAction();

            DFileDragClient::setTargetUrl(e->mimeData(), QUrl(le->text()));
        }
    }
    void dragMoveEvent(QDragMoveEvent *e)
    {
        if (DFileDragClient::checkMimeData(e->mimeData())) {
            e->acceptProposedAction();
            DFileDragClient::setTargetUrl(e->mimeData(), QUrl(le->text()));
        }
    }
    void dropEvent(QDropEvent *e)
    {
        if (DFileDragClient::checkMimeData(e->mimeData())) {
            e->acceptProposedAction();
            DFileDragClient::setTargetUrl(e->mimeData(), QUrl(le->text()));
            c = new DFileDragClient(e->mimeData());
            connect(c, &DFileDragClient::progressChanged, p, &QProgressBar::setValue);
        }
    }
private:
    QLabel *lb;
    QLineEdit *le;
};

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    QDialog d;
    d.show();

    QHBoxLayout *lo = new QHBoxLayout();
    lo->addWidget(new DropArea("area 51"));
    lo->addWidget(new DropArea("area 61"));

    QScopedPointer<QProgressBar> pp (new QProgressBar());
    p = pp.data();
    p->setMinimum(0);
    p->setMaximum(100);

    QVBoxLayout *hlo = new QVBoxLayout();
    d.setLayout(hlo);
    hlo->addLayout(lo);
    hlo->addWidget(p);

    a.exec();
}

#include "dnd-test-client.moc"
