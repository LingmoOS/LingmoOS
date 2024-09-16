// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageinfowidget.h"

#include "unionimage/imageutils.h"
#include "widgets/formlabel.h"
#include "accessibility/ac-desktop-define.h"
#include "service/commonservice.h"
#include "service/mtpfileproxy.h"

#include <DApplicationHelper>
#include <DArrowLineDrawer>
#include <DDialogCloseButton>
#include <DFontSizeManager>
#include <QApplication>
#include <QBoxLayout>
#include <QDateTime>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QPainterPath>
#include <QPushButton>
#include <QScrollBar>
#include <QString>
#include <QtDebug>
#include <QPainterPath>

namespace {

// const
int TITLE_MAXCNWIDETH = 80;      //中文Title宽度
int TITLE_MAXOTHERWIDETH = 105;  //其他语言Title宽度
//因为qrc改变,icon资源路径改变
const QString ICON_CLOSE_DARK = ":/dark/images/close_normal.svg";
const QString ICON_CLOSE_LIGHT = ":/light/images/close_normal .svg";

#define ArrowLineExpand_HIGHT 30
#define ArrowLineExpand_SPACING 10
#define DIALOG_TITLEBAR_HEIGHT 60

struct MetaData
{
    QString key;
    const char *name;
};

static MetaData MetaDataBasics[] = {{"FileName", QT_TRANSLATE_NOOP("MetadataName", "Name")},
                                    {"DateTimeOriginal", QT_TRANSLATE_NOOP("MetadataName", "Date captured")},
                                    {"DateTimeDigitized", QT_TRANSLATE_NOOP("MetadataName", "Date modified")},
                                    {"FileFormat", QT_TRANSLATE_NOOP("MetadataName", "Type")},
                                    {"Dimension", QT_TRANSLATE_NOOP("MetadataName", "Dimensions")},
                                    {"FileSize", QT_TRANSLATE_NOOP("MetadataName", "File size")},
                                    {"Tag", QT_TRANSLATE_NOOP("MetadataName", "Tag")},
                                    {"", ""}};

static MetaData MetaDataDetails[] = {{"ColorSpace", QT_TRANSLATE_NOOP("MetadataName", "Colorspace")},
                                     {"ExposureMode", QT_TRANSLATE_NOOP("MetadataName", "Exposure mode")},
                                     {"ExposureProgram", QT_TRANSLATE_NOOP("MetadataName", "Exposure program")},
                                     {"ExposureTime", QT_TRANSLATE_NOOP("MetadataName", "Exposure time")},
                                     {"Flash", QT_TRANSLATE_NOOP("MetadataName", "Flash")},
                                     {"ApertureValue", QT_TRANSLATE_NOOP("MetadataName", "Aperture")},
                                     {"FocalLength", QT_TRANSLATE_NOOP("MetadataName", "Focal length")},
                                     {"ISOSpeedRatings", QT_TRANSLATE_NOOP("MetadataName", "ISO")},
                                     {"MaxApertureValue", QT_TRANSLATE_NOOP("MetadataName", "Max aperture")},
                                     {"MeteringMode", QT_TRANSLATE_NOOP("MetadataName", "Metering mode")},
                                     {"WhiteBalance", QT_TRANSLATE_NOOP("MetadataName", "White balance")},
                                     {"FlashExposureComp", QT_TRANSLATE_NOOP("MetadataName", "Flash compensation")},
                                     {"Model", QT_TRANSLATE_NOOP("MetadataName", "Camera model")},
                                     {"LensType", QT_TRANSLATE_NOOP("MetadataName", "Lens model")},
                                     {"", ""}};

static int maxTitleWidth()
{
    int maxWidth = 0;
    for (const MetaData *i = MetaDataBasics; !i->key.isEmpty(); ++i) {
        maxWidth =
            qMax(maxWidth + 1, Libutils::base::stringWidth(DFontSizeManager::instance()->get(DFontSizeManager::T8), i->name));
    }
    for (const MetaData *i = MetaDataDetails; !i->key.isEmpty(); ++i) {
        maxWidth =
            qMax(maxWidth + 1, Libutils::base::stringWidth(DFontSizeManager::instance()->get(DFontSizeManager::T8), i->name));
    }

    return maxWidth;
}

}  // namespace

class ViewSeparator : public QLbtoDLabel
{
    Q_OBJECT
public:
    explicit ViewSeparator(QWidget *parent = nullptr)
        : QLbtoDLabel(parent)
    {
        setFixedHeight(1);
    }
};

class DFMDArrowLineExpand : public DArrowLineDrawer
{
public:
    DFMDArrowLineExpand() {}
    ~DFMDArrowLineExpand() override;

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);
        QRectF bgRect;
        bgRect.setSize(size());
        const QPalette pal = QGuiApplication::palette();  // this->palette();
        QColor bgColor = pal.color(QPalette::Background);

        QPainterPath path;
        path.addRoundedRect(bgRect, 8, 8);
        // drawbackground color
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillPath(path, bgColor);
        painter.setRenderHint(QPainter::Antialiasing, false);
    }
};

DFMDArrowLineExpand::~DFMDArrowLineExpand() {}

#include "imageinfowidget.moc"

LibImageInfoWidget::LibImageInfoWidget(const QString &darkStyle, const QString &lightStyle, QWidget *parent)
    : QFrame(parent)
    , m_maxTitleWidth(maxTitleWidth())
    , m_maxFieldWidth(0)
    , m_currentFontSize(0)
{
#ifdef OPENACCESSIBLE
    setObjectName(IMAGE_WIDGET);
    setAccessibleName(IMAGE_WIDGET);
#endif
    Q_UNUSED(darkStyle);
    Q_UNUSED(lightStyle);
    setFixedWidth(300);
    setFrameStyle(QFrame::NoFrame);

    // Info field
    m_exif_base = new QFrame(this);
    m_exif_base->setFixedWidth(280);

    m_exif_details = new QFrame(this);
    m_exif_details->setFixedWidth(280);

    m_exifLayout_base = new QFormLayout(m_exif_base);
    m_exifLayout_base->setVerticalSpacing(7);
    m_exifLayout_base->setHorizontalSpacing(10);
    m_exifLayout_base->setContentsMargins(10, 1, 7, 10);
    m_exifLayout_base->setLabelAlignment(Qt::AlignLeft);

    m_exifLayout_details = new QFormLayout(m_exif_details);
    m_exifLayout_details->setVerticalSpacing(7);
    m_exifLayout_details->setHorizontalSpacing(16);
    m_exifLayout_details->setContentsMargins(10, 1, 7, 10);
    m_exifLayout_details->setLabelAlignment(Qt::AlignLeft);

    m_exif_base->setLayout(m_exifLayout_base);
    m_exif_details->setLayout(m_exifLayout_details);

    m_mainLayout = new QVBoxLayout(this);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(10);

    m_mainLayout->addWidget(m_exif_base);
    m_mainLayout->addWidget(m_exif_details);
    this->setLayout(m_mainLayout);
}

LibImageInfoWidget::~LibImageInfoWidget()
{
    clearLayout(m_exifLayout_base);
    clearLayout(m_exifLayout_details);
}

void updateFileTime(QMap<QString, QString> &data, const QString &path)
{
    QFileInfo info(path);
    if (data.contains("DateTime")) {
        QDateTime time = QDateTime::fromString(data["DateTime"], "yyyy:MM:dd hh:mm:ss");
        data["DateTimeOriginal"] = time.toString("yyyy/MM/dd hh:mm");
    } else {
        data.insert("DateTimeOriginal", info.lastModified().toString("yyyy/MM/dd HH:mm"));
    }
    data.insert("DateTimeDigitized", info.lastModified().toString("yyyy/MM/dd HH:mm"));
}

void LibImageInfoWidget::setImagePath(const QString &path, bool forceUpdate)
{
    // MTP文件需调整文件路径
    bool needMtpProxy = MtpFileProxy::instance()->contains(path);

    // 根据forceUpdate标志判断使用本函数进行整个image info弹窗的整体强制重刷
    if (!forceUpdate && m_path == path) {
        auto metaData = Libutils::image::getAllMetaData(path);
        if (needMtpProxy) {
            updateFileTime(metaData, MtpFileProxy::instance()->mapToOriginFile(path));
        }

        // 检测数据是否存在变更
        if (m_metaData == metaData) {
            return;
        }
    } else {
        m_path = path;
        m_metaData = Libutils::image::getAllMetaData(path);
        if (needMtpProxy) {
            updateFileTime(m_metaData, MtpFileProxy::instance()->mapToOriginFile(path));
        }
    }

    m_isBaseInfo = false;
    m_isDetailsInfo = false;
    updateInfo();

    QStringList titleList;
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(this->layout());
    if (nullptr != layout) {
        QLayoutItem *child;
        while ((child = layout->takeAt(0)) != nullptr) {
            if (child->widget()) {
                child->widget()->setParent(nullptr);
            }
            delete child;
        }
    }

    m_exif_base->setParent(this);
    m_exif_details->setParent(this);
    qDeleteAll(m_expandGroup);

    m_expandGroup.clear();

    bool firstExpand = true;
    // Wayland 下延迟首次动画展示
    if (Libutils::base::checkWayland()) {
        firstExpand = isVisible();
    }

    if (m_isBaseInfo == true && m_isDetailsInfo == true) {
        titleList << tr("Basic info");
        titleList << tr("Details");
        m_expandGroup = addExpandWidget(titleList);
        m_expandGroup.at(0)->setContent(m_exif_base);
        m_expandGroup.at(0)->setExpand(firstExpand);
        m_expandGroup.at(1)->setContent(m_exif_details);
        m_expandGroup.at(1)->setExpand(firstExpand);

    } else if (m_isBaseInfo == true && m_isDetailsInfo == false) {
        titleList << tr("Basic info");
        m_expandGroup = addExpandWidget(titleList);
        m_expandGroup.at(0)->setContent(m_exif_base);
        m_expandGroup.at(0)->setExpand(firstExpand);
    }

    layout->addStretch(1);
}

void LibImageInfoWidget::showEvent(QShowEvent *e)
{
    QFrame::showEvent(e);

    // Note: Wayland 下默认动画和应用动画重叠，同时可能导致 wayland 未正确取得
    //  控件信息，显示花屏，调整应用动画在 wayland 动画后
    if (Libutils::base::checkWayland()) {
        const int waylandAnimationDuration = 250;
        QTimer::singleShot(waylandAnimationDuration, this, [this]() {
            for (auto expand : m_expandGroup) {
                expand->setExpand(true);
            }
        });
    }
}

void LibImageInfoWidget::resizeEvent(QResizeEvent *e)
{
    DWidget::resizeEvent(e);
}

void LibImageInfoWidget::timerEvent(QTimerEvent *e)
{
    QWidget::timerEvent(e);
}

// LMH0609解决31498 【看图】【5.6.3.9】【sp2】更改字体大小后，图片信息窗口文字布局展示异常
void LibImageInfoWidget::paintEvent(QPaintEvent *event)
{
    QFont font;
    int currentSize = DFontSizeManager::instance()->fontPixelSize(font);
    // LMH0609判断与上次自体的大小是否一样，不一样则刷新
    if (currentSize != m_currentFontSize) {
        m_currentFontSize = currentSize;
        TITLE_MAXCNWIDETH = currentSize * 4;
        updateInfo();
    }
    QWidget::paintEvent(event);
}

void LibImageInfoWidget::clearLayout(QLayout *layout)
{
    QFormLayout *fl = static_cast<QFormLayout *>(layout);
    if (fl) {
        // FIXME fl->rowCount() will always increase
        for (int i = 0; i < fl->rowCount(); i++) {
            QLayoutItem *li = fl->itemAt(i, QFormLayout::LabelRole);
            QLayoutItem *fi = fl->itemAt(i, QFormLayout::FieldRole);
            if (li) {
                if (li->widget())
                    delete li->widget();
                fl->removeItem(li);
            }
            if (fi) {
                if (fi->widget())
                    delete fi->widget();
                fl->removeItem(fi);
            }
        }
    }
}

const QString LibImageInfoWidget::trLabel(const char *str)
{
    return qApp->translate("MetadataName", str);
}

void LibImageInfoWidget::updateInfo()
{
    // Minus layout margins
    //    m_maxFieldWidth = width() - m_maxTitleWidth - 20*2;
    // solve bug 1623 根据中英文系统语言设置Title宽度  shuwenzhi   20200313
    QLocale local;
    bool CNflag;
    QLocale::Language lan = local.language();
    if (lan == QLocale::Language::Chinese) {
        m_maxFieldWidth = width() - TITLE_MAXCNWIDETH /* - 20 * 2 */ - 10 * 2 - 10;
        CNflag = true;
    } else {
        m_maxFieldWidth = width() - TITLE_MAXOTHERWIDETH /* - 20 * 2 */ - 10 * 2 - 10;
        CNflag = false;
    }

    updateBaseInfo(m_metaData, CNflag);
    updateDetailsInfo(m_metaData, CNflag);
}

void LibImageInfoWidget::updateBaseInfo(const QMap<QString, QString> &infos, bool CNflag)
{
    using namespace Libutils::image;
    using namespace Libutils::base;
    clearLayout(m_exifLayout_base);

    auto info = LibCommonService::instance()->getImgInfoByPath(m_path);

    QFileInfo fi(m_path);
    QString suffix = fi.suffix();
    for (MetaData *i = MetaDataBasics; !i->key.isEmpty(); i++) {
        QString value = infos.value(i->key);
        if (value.isEmpty()) {
            continue;
        }

        if ((i->key == "DateTimeOriginal" || i->key == "DateTimeDigitized") && value.left(1) == QString("0")) {
            continue;
        }

        //部分图片采用meta data的形式无法正确读取大小，此处改成使用已缓存的图片大小数据
        if (i->key == "Dimension") {
            value = QString().sprintf("%dx%d", info.imgOriginalWidth, info.imgOriginalHeight);
        }

        /*lmh0825真实格式，没有真格式采用后缀名*/
        if (i->key == "FileFormat" && !suffix.isEmpty() && infos.value(i->key).isNull()) {
            value = fi.suffix();
        }
        // value必须为小写
        if (i->key == "FileFormat") {
            value = value.toLower();
        }

        m_isBaseInfo = true;
        SimpleFormField *field = new SimpleFormField;
        field->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        DFontSizeManager::instance()->bind(field, DFontSizeManager::T8);
        DPalette pa1 = DApplicationHelper::instance()->palette(field);
        pa1.setBrush(DPalette::WindowText, pa1.color(DPalette::TextTitle));
        field->setPalette(pa1);

        // hujianbo 修改图片信息中文修改格式为年月日，英文不变，修复bug24447  备注：随系统升级有时拿到的是年月日格式 ，有时是/格式
        if (i->key == "DateTimeOriginal" || i->key == "DateTimeDigitized") {
            if (CNflag) {
                QDateTime tmpTime = QDateTime::fromString(value, "yyyy/MM/dd hh:mm:ss");
                if (!tmpTime.isNull())
                    value = tmpTime.toString("yyyy年MM月dd日 hh:mm:ss");
            }
        }

        field->setText(SpliteText(value, field->font(), m_maxFieldWidth));

        SimpleFormLabel *title = new SimpleFormLabel(trLabel(i->name) + ":");
        title->setMinimumHeight(field->minimumHeight());
        //        title->setFixedWidth(qMin(m_maxTitleWidth, TITLE_MAXWIDTH));
        if (CNflag) {
            title->setFixedWidth(TITLE_MAXCNWIDETH);
        } else {
            title->setFixedWidth(TITLE_MAXOTHERWIDETH);
        }
        title->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        DFontSizeManager::instance()->bind(title, DFontSizeManager::T8);
        DPalette pa2 = DApplicationHelper::instance()->palette(title);
        pa2.setBrush(DPalette::WindowText, pa2.color(DPalette::TextTitle));
        title->setPalette(pa2);
        if (CNflag) {
            title->setText(SpliteText(trLabel(i->name) + ":", title->font(), TITLE_MAXCNWIDETH, true));
        } else {
            title->setText(SpliteText(trLabel(i->name) + ":", title->font(), TITLE_MAXOTHERWIDETH, true));
        }
        QFontMetrics fm(title->font());
        QStringList list = title->text().split("\n");

        title->setFixedHeight(fm.height() * list.size());

        m_exifLayout_base->addRow(title, field);
    }
}

void LibImageInfoWidget::updateDetailsInfo(const QMap<QString, QString> &infos, bool CNflag)
{
    using namespace Libutils::image;
    using namespace Libutils::base;
    clearLayout(m_exifLayout_details);
    for (MetaData *i = MetaDataDetails; !i->key.isEmpty(); i++) {
        QString value = infos.value(i->key);
        if (value.isEmpty())
            continue;

        m_isDetailsInfo = true;

        SimpleFormField *field = new SimpleFormField;
        field->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        DFontSizeManager::instance()->bind(field, DFontSizeManager::T8);
        DPalette pa1 = DApplicationHelper::instance()->palette(field);
        pa1.setBrush(DPalette::WindowText, pa1.color(DPalette::TextTitle));
        field->setPalette(pa1);
        field->setText(SpliteText(value, field->font(), m_maxFieldWidth));

        SimpleFormLabel *title = new SimpleFormLabel(trLabel(i->name) + ":");
        title->setMinimumHeight(field->minimumHeight());
        if (CNflag) {
            title->setFixedWidth(TITLE_MAXCNWIDETH);
        } else {
            title->setFixedWidth(TITLE_MAXOTHERWIDETH);
        }
        title->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        DFontSizeManager::instance()->bind(title, DFontSizeManager::T8);
        DPalette pa2 = DApplicationHelper::instance()->palette(title);
        pa2.setBrush(DPalette::WindowText, pa2.color(DPalette::TextTitle));
        title->setPalette(pa2);
        if (CNflag) {
            title->setText(SpliteText(trLabel(i->name) + ":", title->font(), TITLE_MAXCNWIDETH, true));
        } else {
            title->setText(SpliteText(trLabel(i->name) + ":", title->font(), TITLE_MAXOTHERWIDETH, true));
        }

        m_exifLayout_details->addRow(title, field);
    }
}

QList<DDrawer *> LibImageInfoWidget::addExpandWidget(const QStringList &titleList)
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(this->layout());
    QList<DDrawer *> group;

    for (const QString &title : titleList) {
        //        DFMDArrowLineExpand *expand = new DFMDArrowLineExpand;  // DArrowLineExpand;
        DArrowLineDrawer *expand = new DArrowLineDrawer;  // DArrowLineExpand;
        expand->setTitle(title);
        initExpand(layout, expand);
        group.push_back(expand);
    }

    return group;
}

void LibImageInfoWidget::initExpand(QVBoxLayout *layout, DDrawer *expand)
{
    expand->setFixedHeight(ArrowLineExpand_HIGHT);
    QMargins cm = layout->contentsMargins();
    //修复style问题
    expand->setFixedWidth(contentsRect().width() - cm.left() - cm.right());
    expand->setExpandedSeparatorVisible(false);
    expand->setSeparatorVisible(false);
    layout->addWidget(expand, 0, Qt::AlignTop);
    //修复style问题
    DEnhancedWidget *hanceedWidget = new DEnhancedWidget(expand, expand);
    connect(hanceedWidget, &DEnhancedWidget::heightChanged, hanceedWidget, [=]() {
        QRect rc1 = geometry();
        rc1.setHeight(contentHeight() + ArrowLineExpand_SPACING * 2);
        setGeometry(rc1);

        emit extensionPanelHeight(contentHeight() /*+ ArrowLineExpand_SPACING*/);
    });
}

int LibImageInfoWidget::contentHeight() const
{
    int expandsHeight = ArrowLineExpand_SPACING;
    foreach (const DDrawer *expand, m_expandGroup) {
        expandsHeight += expand->height();
    }
    if (m_expandGroup.size() == 2)
        expandsHeight += 10;

    return (DIALOG_TITLEBAR_HEIGHT + expandsHeight + contentsMargins().top() + contentsMargins().bottom());
}
