#include "openimage.h"
#include "kyview.h"
#include "sizedate.h"
#include "global/interactiveqml.h"
OpenImage::OpenImage(QWidget *parent) : QWidget(parent)
{
    initInteraction();
    //最外层的大圆形
    m_openInCenter = new QPushButton(this);
    m_openInCenter->setFocusPolicy(Qt::NoFocus);
    m_openInCenter->setFixedSize(OPENINCENTER_SIZE);
    m_openInCenter->move(110, 110);
    m_ft.setPixelSize(14);
    //中间“+”号按钮
    m_addFile = new kdk::KBorderlessButton(m_openInCenter);
    m_addFile->setFocusPolicy(Qt::NoFocus);
    m_addFile->setFixedSize(ADD_FILESIZE);
    m_addFile->setFlat(true);
	m_addFile->move(44, 44);
    m_addFile->setIcon(QIcon(":/res/res/addplus.svg"));
    m_addFile->setIconSize(ICON_SIZE);
    //按钮下的“打开图片”文字
    m_openText = new QLabel(this);
    m_openText->setText(tr("Click or drag to load picture"));
    m_openText->setFixedWidth(OPEN_IMAGEFIX);
    m_openText->setAlignment(Qt::AlignCenter);
    m_openText->move(0, OPEN_IMAGEFIX - 80);
    //    m_openText->setFont(m_ft);

    this->setFixedSize(OPEN_IMAGESIZE);
    this->initconnect();
    this->initGsettings();
}

//信号槽
void OpenImage::initconnect()
{
    connect(m_openInCenter, &QPushButton::clicked, this, &OpenImage::openImagePath);
    connect(m_addFile, &kdk::KBorderlessButton::clicked, this, &OpenImage::openImagePath);
}
//打开图片
void OpenImage::openImagePath()
{
    QFileDialog m_fileDialog;
    m_fileDialog.update();
    QList<QUrl> list = m_fileDialog.sidebarUrls();
    int sidebarNum = 8; //最大添加U盘数，可以自己定义
    QString home = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList filist = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < filist.size(); ++i) {
        QFileInfo fi = filist.at(i);
        //华为990、9a0需要屏蔽最小系统挂载的目录
        if (fi.fileName() == "2691-6AB8")
            continue;
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher fsw(&m_fileDialog);
    fsw.addPath("/media/" + home + "/");
    connect(&fsw, &QFileSystemWatcher::directoryChanged, &m_fileDialog,
            [=, &sidebarNum, &mntUrlList, &list, &m_fileDialog](const QString path) {
                QDir wmntDir(path);
                wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
                QFileInfoList wfilist = wmntDir.entryInfoList();
                mntUrlList.clear();
                for (int i = 0; i < sidebarNum && i < wfilist.size(); ++i) {
                    QFileInfo fi = wfilist.at(i);
                    //华为990、9a0需要屏蔽最小系统挂载的目录
                    if (fi.fileName() == "2691-6AB8")
                        continue;
                    mntUrlList << QUrl("file://" + fi.filePath());
                }
                m_fileDialog.setSidebarUrls(list + mntUrlList);
                m_fileDialog.update();
            });

    connect(&m_fileDialog, &QFileDialog::finished, &m_fileDialog, [=, &list, &m_fileDialog]() {
        m_fileDialog.setSidebarUrls(list);
    });

    //这行要添加，设置左侧导航
    m_fileDialog.setSidebarUrls(list + mntUrlList);
    //设置视图模式
    m_fileDialog.setViewMode(QFileDialog::Detail);
    //设置可以选择多个文件,默认为只能选择一个文件QFileDialog::ExistingFiles
    m_fileDialog.setFileMode(QFileDialog::ExistingFiles);
    m_fileDialog.setOption(QFileDialog::HideNameFilterDetails);

    QString file_path;
    QString format;
    QString openFile = tr("Open Image");
    QString fileType = tr("Image Files(");

    //构造打开条件
    format = "(";
    for (const QString &str : Variable::SUPPORT_FORMATS)
        format += "*." + str + " ";
    format += ");;";
    //所有类型都放在一起，类型名显示不全，故增加分层，将同一类型的放在一起，分层显示
    format = format + Variable::SUPPORT_FORMATS_CLASSIFY;

    //打开文件夹中的图片文件
    file_path = m_fileDialog.getOpenFileName(this, openFile, Variable::getSettings("imagePath").toString(), format);
    //空校验，点击“取消”
    if (file_path.isEmpty()) {
        Q_EMIT openEmptyFile(true);
        return;
    }
    operayteMode = OperayteMode::ChangeImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    //写入配置
    QFileInfo info(file_path);
    Variable::setSettings("imagePath", info.absolutePath());
    Q_EMIT openEmptyFile(false);
    Q_EMIT openImageSignal(file_path);
}

void OpenImage::initInteraction()
{
    connect(Interaction::getInstance(), &Interaction::openFromAlbum, this, &OpenImage::openImagePath); //启动时打开图片
}

void OpenImage::initGsettings()
{
    connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemThemeChange, this, [=]() {
        dealSystemGsettingChange();
    });
    dealSystemGsettingChange();
    return;
}

void OpenImage::dealSystemGsettingChange()
{

    QString themeStyle = kdk::kabase::Gsettings::getSystemTheme().toString();
    if ("lingmo-dark" == themeStyle || "lingmo-black" == themeStyle) {
        m_openInCenter->setStyleSheet("background-color:rgba(0, 0, 0, 0.5);border-radius:64;");
        //        m_openText->setStyleSheet("QLabel{background-color:transparent;color:rgba(255, 255, 255, 1);}");
        //        m_addFile->setStyleSheet("background-color:transparent;");

    } else {
        m_openInCenter->setStyleSheet("background-color:rgba(255, 255, 255, 0.4);border-radius:64;");
        //        m_openText->setStyleSheet("QLabel{background-color:transparent;color:rgba(38, 38, 38, 0.85);}");
    }
}
