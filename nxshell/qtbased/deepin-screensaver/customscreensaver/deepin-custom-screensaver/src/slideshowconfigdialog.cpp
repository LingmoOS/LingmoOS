// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "slideshowconfigdialog.h"
#include "slideshowconfig.h"
#include "config/contenttitle.h"

#include <QWindow>
#include <DFileDialog>
#include <DFrame>
#include <DFontSizeManager>
#include <DBackgroundGroup>

#include <QScreen>
#include <QVBoxLayout>
#include <QProcessEnvironment>

#define SPECIFYLINEEDITLENGTH 65

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

static bool isWayland()
{
    bool ok = false;
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    //在wayland平台下设置固定大小，解决属性框最大化问题
    if (XDG_SESSION_TYPE == QLatin1String("wayland") ||
            WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        ok = true;
    }
    return ok;
}

SlideShowConfigDialog::SlideShowConfigDialog(QWidget *parent)
    : DAbstractDialog(false, parent)
{
    setModal(true);

    initUI();
    initValue();
    initConnect();
}

SlideShowConfigDialog::~SlideShowConfigDialog()
{

}

void SlideShowConfigDialog::initUI()
{
    auto icon = QIcon(":/data/deepin-screensaver-config.svg");
    setWindowIcon(icon);

    auto mainlayout = new QVBoxLayout(this);
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->setSpacing(0);

    DFrame *contentFrame = new DFrame;
    contentFrame->setLineWidth(0);
    contentFrame->setContentsMargins(10, 10, 10, 10);

    QVBoxLayout *contentlayout = new QVBoxLayout(contentFrame);
    frameBar = new DTitlebar(this);
    frameBar->setMenuVisible(false);
    frameBar->setTitle(QString());
    frameBar->setIcon(icon);

    contentlayout->setContentsMargins(10, 0, 0, 0);

    mainlayout->addWidget(frameBar);
    mainlayout->addWidget(contentFrame);

    // mian title
    {
        contentlayout->addSpacing(15);
        ContentTitle *title = new ContentTitle(contentFrame);
        title->setTitle(tr("Custom Screensaver"));
        title->label()->setForegroundRole(QPalette::BrightText);
        DFontSizeManager::instance()->bind(title, DFontSizeManager::T4, QFont::Medium);
        contentlayout->addWidget(title);
    }

    auto createGroup = [this, contentlayout, contentFrame]
            (const QString &text, const QList<QWidget*> &contWid) {
        DWidget *wid = new DWidget(contentFrame);

        // title
        auto title = new ContentTitle(wid);
        title->setTitle(text);
        DFontSizeManager::instance()->bind(title, DFontSizeManager::T5, QFont::Medium);

        {
            QHBoxLayout *hLay = new QHBoxLayout(wid);
            hLay->setContentsMargins(10, 0, 0, 0);
            hLay->addWidget(title);
            contentlayout->addWidget(wid);
        }

        QVBoxLayout *bgGpLayout = new QVBoxLayout;
        DBackgroundGroup *bgGroup = new DBackgroundGroup(bgGpLayout);
        bgGroup->setItemSpacing(1);
        bgGroup->setItemMargins(QMargins(0, 0, 0, 0));
        bgGroup->setBackgroundRole(QPalette::Window);
        bgGroup->setUseWidgetBackground(false);
        contentlayout->addWidget(bgGroup);

        for (QWidget *pwid : contWid) {
            QWidget *wrapperWidget = new QWidget(bgGroup);
            QHBoxLayout *hLay = new QHBoxLayout(wrapperWidget);
            hLay->setContentsMargins(10, 6, 10, 6);
            pwid->setParent(wrapperWidget);
            hLay->addWidget(pwid);

            bgGpLayout->addWidget(wrapperWidget);
        }

        QSpacerItem *spaceItem = new QSpacerItem(0, 20, QSizePolicy::Minimum,QSizePolicy::Expanding);
        contentlayout->addItem(spaceItem);
    };

    // path
    contentlayout->addSpacing(15);
    pathWidget = new SelectPathWidget;
    createGroup(tr("Path"), {pathWidget});

    // Slideshow
    intervalWidget = new TimeIntervalWidget;
    shuffleCheck = new DCheckBox;
    shuffleCheck->setText(tr("Shuffle"));
    createGroup(tr("Slideshow"), {intervalWidget, shuffleCheck});

    // reset
    {
        QSpacerItem *spaceItem = new QSpacerItem(0, 30, QSizePolicy::Minimum,QSizePolicy::Expanding);
        contentlayout->addItem(spaceItem);

        QWidget *box = new QWidget();
        QHBoxLayout *box_layout = new QHBoxLayout(box);
        box_layout->setContentsMargins(0, 0, 0, 0);
        auto resetBt = new DPushButton(QObject::tr("Restore Defaults"), box);
        resetBt->setMaximumWidth(300);
        resetBt->setAutoDefault(false);
        box_layout->addWidget(resetBt);
        box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        contentlayout->addWidget(box);

        connect(resetBt, &QPushButton::released, this, &SlideShowConfigDialog::resetSettings);
    }

    if (isWayland()) {
        // 设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    setFixedSize(QSize(682, 532));
    return;
}

void SlideShowConfigDialog::initConnect()
{
    connect(pathWidget, &SelectPathWidget::requsetSetPath, this, &SlideShowConfigDialog::selectPath);
    connect(intervalWidget, &TimeIntervalWidget::valueChanged, this, &SlideShowConfigDialog::intervalChanged);
    connect(shuffleCheck, &QCheckBox::stateChanged, this, &SlideShowConfigDialog::shuffleChanged);
}

void SlideShowConfigDialog::initValue()
{
    //path
    pathWidget->setPath(SlideshowConfig::instance()->slideshowPath());
    intervalWidget->setInterval(SlideshowConfig::instance()->intervalTime());
    shuffleCheck->setChecked(SlideshowConfig::instance()->isShuffle());
}

void SlideShowConfigDialog::startConfig()
{
    setWindowFlags(this->windowFlags() & ~Qt::WindowMaximizeButtonHint);

    setAttribute(Qt::WA_DeleteOnClose);

    // remove dialog flag to let dock show window entry.
    setWindowFlag(Qt::Dialog, false);

    QScreen *screen = qApp->primaryScreen();
    move(screen->geometry().width() / 2 - this->width() / 2,
         screen->geometry().height() / 2 - this->height() / 2);

    setFocus();
    show();
}

void SlideShowConfigDialog::mousePressEvent(QMouseEvent *event)
{
    setFocus();
    DAbstractDialog::mouseMoveEvent(event);
}

void SlideShowConfigDialog::handArguments(const QStringList &)
{
    activateWindow();
}

void SlideShowConfigDialog::selectPath()
{
    QString currentPath = pathWidget->getPath();

    // 如果未选择过，或选择的目录已经不存在，则重置为默认值
    if (currentPath.isEmpty() || !QFile::exists(currentPath)) {
        currentPath = SlideshowConfig::defaultPath();
        if (currentPath.startsWith(QStringLiteral("~")))
            currentPath.replace(QStringLiteral("~"), QDir::homePath());

        // 如果默认值为空或不存在，则强制设置为图片目录
        if (currentPath.isEmpty() || !QFile::exists(currentPath)) {
            currentPath = QDir::homePath() + "/Pictures";
            qWarning() << "The default is empty or does not exist,set path is " << currentPath;
        }
    }

    QString selectPath = DFileDialog::getExistingDirectory(nullptr, tr("Picture path"), currentPath,
                                                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (selectPath.isEmpty())
        return;

    pathWidget->setPath(selectPath);

    SlideshowConfig::instance()->setSlideShowPath(selectPath);
}

void SlideShowConfigDialog::intervalChanged(int v)
{
    SlideshowConfig::instance()->setIntervalTime(v);
}

void SlideShowConfigDialog::shuffleChanged(int v)
{
    SlideshowConfig::instance()->setShuffle(v == Qt::Checked);
}

void SlideShowConfigDialog::resetSettings()
{
    SlideshowConfig::instance()->setSlideShowPath(SlideshowConfig::instance()->defaultPath());
    SlideshowConfig::instance()->setIntervalTime(kDefaultTime);
    SlideshowConfig::instance()->setShuffle(false);
    initValue();
}
