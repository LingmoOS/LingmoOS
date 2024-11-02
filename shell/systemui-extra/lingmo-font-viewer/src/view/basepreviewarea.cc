#include <QDebug>
#include <QIcon>
#include <QPixmap>
#include <QPalette>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QPainterPath>

#include "basepreviewarea.h"

BasePreviewArea::BasePreviewArea(QString value, int width, int height, int space)
{
    m_width = width;
    m_height = height;
    m_value = value;
    m_space = space;
    m_size = value.toInt();

    this->init();
}

BasePreviewArea::~BasePreviewArea()
{
}

void BasePreviewArea::init()
{
    this->setMinimumWidth(m_width);
    // this->setMinimumHeight(m_height);
    this->setFocusPolicy(Qt::ClickFocus);

    m_previewEdit = new QLineEdit(this);                 /* 预览编辑框 */
    m_previewEdit->setFixedHeight(40);
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        qDebug()<<"init  qitayuyan";

        m_previewEdit->setAlignment(Qt::AlignRight);
        m_previewEdit->setPlaceholderText(tr("Enter Text Content For Preview"));   /* 提示文字 */
   }else{
       m_previewEdit->setAlignment(Qt::AlignLeft);
        m_previewEdit->setPlaceholderText(tr("Enter Text Content For Preview"));   /* 提示文字 */
    }
    m_previewEdit->setMaxLength(30);                     /* 字数限制 */
    m_previewEdit->clearFocus();

    m_fontSizeSlider = new QSlider(this);                 /* 字号滑动条 */
    m_fontSizeSlider->setOrientation(Qt::Horizontal);     /* 水平方向 */
    m_fontSizeSlider->setMinimum(14);                     /* 最小值 */
    m_fontSizeSlider->setMaximum(60);                     /* 最大值 */
    m_fontSizeSlider->setSingleStep(1);                   /* 步长 */
    m_fontSizeSlider->setValue(m_size);                   /* 设置初始值: 24 */

    m_valueLab = new QLabel(this);                        /* 滑动条当前值 */
    m_valueLab->setText(m_value);
    m_valueLab->setStyleSheet("color:#8C8C8C;");

    m_hlayout = new QHBoxLayout();
    m_hlayout->addSpacing(m_space / 2);
    m_hlayout->addWidget(m_previewEdit, 6);
    m_hlayout->addSpacing(m_space * 2);
    m_hlayout->addWidget(m_fontSizeSlider, 1);
    m_hlayout->addSpacing(m_space);
    m_hlayout->addWidget(m_valueLab);
    m_hlayout->addSpacing(m_space * 2);
    m_hlayout->setSpacing(0);
    m_hlayout->setMargin(0);

    this->setLayout(m_hlayout);

    m_fontSizeSlider->installEventFilter(this);

    switch (GlobalSizeData::getInstance()->m_currentMode)
    {
        case CurrentMode::PCMode:
        slotChangePCSize();        
            break;
        case CurrentMode::HMode:
        slotChangeFaltSize();
            break;
        case CurrentMode::VMode:
        slotChangeFaltSize();
            break;
    }

    connect(m_fontSizeSlider, &QSlider::valueChanged, this, &BasePreviewArea::slotChangSize);
    connect(m_previewEdit, &QLineEdit::textChanged, this, &BasePreviewArea::slotPreviewValue);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigFontNameChange, this, &BasePreviewArea::slotChangeFontName);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigPCMode, this, &BasePreviewArea::slotChangePCSize);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigHFlatMode, this, &BasePreviewArea::slotChangeFaltSize);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigVFlatMode, this, &BasePreviewArea::slotChangeFaltSize);

    return;
}

void BasePreviewArea::previewFocusOut()
{
    m_previewEdit->clearFocus();
    return;
}

bool BasePreviewArea::eventFilter(QObject* watched, QEvent* event)
{

    if (watched == m_fontSizeSlider) {
        if (event->type()==QEvent::MouseButtonPress) {    /* 判断类型 */
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton)   /* 判断左键 */
            {
                QString systemLang = QLocale::system().name();
                if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
                    QSlider* pSlider = (QSlider*)watched;
                    int dur = pSlider->maximum() - pSlider->minimum();
                    int pos = pSlider->maximum() - dur * ((double)mouseEvent->x() / pSlider->width());
                    if(pos != pSlider->sliderPosition())
                    {
                        qDebug()<<"BasePreviewArea  qitayuyan"<<pos;
                        pSlider->setValue(pos);
                }
                }
                   else{
                        QSlider* pSlider = (QSlider*)watched;
                        int dur = pSlider->maximum() - pSlider->minimum();
                        int pos = pSlider->minimum() + dur * ((double)mouseEvent->x() / pSlider->width());
                        if(pos != pSlider->sliderPosition())
                        {
                            qDebug()<<"BasePreviewArea"<<pos;
                            pSlider->setValue(pos);
                        }  
               }
//                    QSlider* pSlider = (QSlider*)watched;
//                    int dur = pSlider->maximum() - pSlider->minimum();
//                    //int pos = pSlider->minimum() + dur * ((double)mouseEvent->x() / pSlider->width());
//                    int pos = pSlider->maximum() - dur * ((double)mouseEvent->x() / pSlider->width());
//                    if(pos != pSlider->sliderPosition())
//                    {
//                        qDebug()<<"BasePreviewArea"<<pos;
//                        pSlider->setValue(pos);
//                    }

                }

        }
    }

    return QObject::eventFilter(watched,event);
}
void BasePreviewArea::slotChangSize(int size)
{
    m_size = size;
    QString fontSize = QString("%1").arg(size);
    m_valueLab->setText(fontSize);
    emit sigFontSize(size);

    return;
}

void BasePreviewArea::slotPreviewValue(QString text)
{
    emit sigPreviewValue(text);
    return;
}

void BasePreviewArea::slotChangFont(QString fontName)
{
    m_valueLab->setFont(QFont(fontName));
    return;
}

void BasePreviewArea::slotChangeFontName()
{
    m_valueLab->setFont(QFont(GlobalSizeData::getInstance()->m_fontName));
    return;
}

void BasePreviewArea::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing); // 反锯齿;

    QPainterPath rectPath;
    rectPath.addRoundedRect(rect(), 0, 0);

    QStyleOption opt;
    opt.init(this);
    p.setPen(Qt::NoPen);
    QColor color = qApp->palette().color(QPalette::Window);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Window, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect, 0, 0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    p.fillPath(rectPath, brush);
}

void BasePreviewArea::slotChangeFaltSize()
{
    this->setFixedHeight(80);
    m_previewEdit->setFixedHeight(60);
    return;
}

void BasePreviewArea::slotChangePCSize()
{
    this->setFixedHeight(50);
    m_previewEdit->setFixedHeight(40);
    return;
}
