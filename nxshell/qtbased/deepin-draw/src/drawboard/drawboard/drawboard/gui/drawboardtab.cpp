// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drawboardtab.h"

#include <QMouseEvent>
#include <QApplication>
#include <QStyleFactory>
#include <QGuiApplication>
#include <QDebug>
#include <QStandardPaths>

#include "pagecontext.h"
#include "drawboard.h"
#include "DataHanderInterface.h"
#include "setting.h"

const QSize TabBarMiniSize = QSize(220, 36);

TabBarWgt::TabBarWgt(DrawBoard *parent)
    : TABBAR(parent)
{
    setWgtAccesibleName(this, "MultipTabBarWidget");
    this->setMovable(true);
    this->setTabsClosable(true);
#ifdef USE_DTK
    this->setVisibleAddButton(true);
    this->setEnabledEmbedStyle(true);
#endif
    this->setElideMode(Qt::ElideMiddle);
    this->setFocusPolicy(Qt::NoFocus);
    this->setExpanding(true);

#ifdef USE_DTK
    connect(this, &TabBarWgt::tabAddRequested, this, [ = ]() {
        pageManager()->addPage();
    });
#endif

    connect(this, &TabBarWgt::tabCloseRequested, this, [ = ](int index) {
        auto key = this->key(index);
        pageManager()->closePage(key);
    });

    connect(this, &TabBarWgt::currentChanged, this, [ = ](int index) {
        auto key = this->key(index);
        pageManager()->setCurrentPage(key);
    });

#ifdef USE_DTK
    connect(this, &TabBarWgt::tabIsInserted, this, &TabBarWgt::onTabCountChanged);
    connect(this, &TabBarWgt::tabIsRemoved, this, &TabBarWgt::onTabCountChanged);
#endif

    hide();
}

TabBarWgt::~TabBarWgt()
{

}

DrawBoard *TabBarWgt::pageManager() const
{
    return qobject_cast<DrawBoard *>(parentWidget());
}

void TabBarWgt::addItem(const QString &name, const QString &key)
{
    int index = addTab(name);
    setTabData(index, key);

#ifdef USE_DTK
    // 对标签页名称做长度限制
    this->setTabMinimumSize(index, QSize(220, 36));
#endif
}

void TabBarWgt::removeItem(const QString &key)
{
    int index = this->index(key);
    removeTab(index);
}

int TabBarWgt::index(const QString &key) const
{
    for (int i = 0; i < this->count(); ++i) {
        if (key == this->tabData(i).toString()) {
            return i;
        }
    }
    return -1;
}

QString TabBarWgt::key(int index) const
{
    return tabData(index).toString();
}

void TabBarWgt::onTabCountChanged(int index)
{
    this->setVisible(this->count() > 1);
}

QMenu *TabBarWgt::menu() const
{
    static QMenu *s_menu = nullptr;
    if (s_menu == nullptr) {
        s_menu = new QMenu(const_cast<TabBarWgt *>(this));
        QAction *actionA = new QAction(tr("Close tab"), s_menu);
        connect(actionA, &QAction::triggered, this, [ = ]() {
            emit const_cast<TabBarWgt *>(this)->tabCloseRequested(currentIndex());
        });

        QAction *actionB = new QAction(tr("Close other tabs"), s_menu);
        connect(actionB, &QAction::triggered, this, [ = ]() {
            auto currentKey = key(currentIndex());
            //bool refuse = false;
            Page *loopPage = pageManager()->firstPage();
            while (loopPage != nullptr) {
                if (loopPage->key() != currentKey) {
                    if (!pageManager()->closePage(loopPage)) {
                        //refuse = true;
                        break;
                    }
                    loopPage = pageManager()->firstPage();
                } else {
                    loopPage = pageManager()->nextPage(loopPage);
                }
            }
        });
        s_menu->addAction(actionA);
        s_menu->addAction(actionB);
    }
    return s_menu;
}

void TabBarWgt::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        this->setCurrentIndex(this->tabAt(event->pos()));
        menu()->move(mapToGlobal(event->pos()));
        menu()->exec();
        return;
    }
    TABBAR::mousePressEvent(event);
}


FileSelectDialog::FileSelectDialog(DrawBoard *parent): QFileDialog(parent)
{
    this->setObjectName("DDFSaveDialog");

    //设置文件对话框为保存模式
    this->setAcceptMode(QFileDialog::AcceptSave);

    //只显示文件夹
    this->setOptions(QFileDialog::DontResolveSymlinks /*| QFileDialog::Option(DontUseNativeDialog)*/);

    //设置显示模式
    this->setViewMode(QFileDialog::List);

    this->setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
}

DrawBoard *FileSelectDialog::borad() const
{
    return qobject_cast<DrawBoard *>(this->parent());
}

int FileSelectDialog::exec()
{
    _resultFile.clear();
    int ret = QDialog::Rejected;
begin:
    ret = QFileDialog::exec();

    //not 0 mean success in DFileDialog,but we also should check again.
    if (ret != QDialog::Rejected) {
        QStringList selectedFiles = this->selectedFiles();
        if (!selectedFiles.isEmpty()) {
            QString path = selectedFiles.first();
            if (!FileHander::isLegalFile(path)) {
                //不支持的文件名
                int result = MessageDlg::execMessage(tr("The file name must not contain \\/:*?\"<>|"), this);
                if (0 == result) {
                    //Failed reson: user clicked "OK" mean input file name again,so we jump to 'begin' table.
                    goto begin;
                } else {
                    //Failed reson: user not clicked "OK" mean quit.
                    ret = QDialog::Rejected;
                }
            } else {
                //再判断该文件是否正在被打开着的如果是那么就要提示不能覆盖
                if (borad()->getPageByFile(path) != nullptr) {
                    MessageDlg dia(this);
                    dia.setObjectName("OpenedDialog");
                    dia.setMessage(SMessage(tr("Cannot save it as %1, since the file in that name is open now."
                                               "\nPlease save it in another name or close that file and try again.")
                                            .arg(QFileInfo(path).fileName())));
                    dia.exec();


                    //Failed reson: we have loaded one file named that, so user should change to another.
                    ret = QDialog::Rejected;
                }
            }

            if (ret != QDialog::Rejected) {
                _resultFile = path;
                saveSetting();
            }
        }
    }
    return ret;
}

QString FileSelectDialog::resultFile() const
{
    return _resultFile;
}


void FileSelectDialog::saveSetting()
{
    QFileInfo info(resultFile());
    Setting::instance()->setDefaultFileDialogPath(info.absolutePath());
    Setting::instance()->setDefaultFileDialogNameFilter(QFileDialog::selectedNameFilter());
}

PageStackedWidget::~PageStackedWidget()
{
    //显示首先析构所有的子page对象再析构layout，保证page析构造成的DrawBoard::currentPage调用不会访问到已经被删除的layout
    //由于QObject添加child的保存方式是list，在父QObject析构时会从添加的顺序析构child，导致layout会第一个被析构掉，而widget
    //后析构，这种其实不太合理，因为按照栈(heap)的析构顺序来看，应该是先添加的孩子后析构
    foreach (auto child, children()) {
        if (child != layout()) {
            delete child;
        }
    }
    delete layout();
}
