#include <QIcon>
#include <QPixmap>
#include <QPalette>

#include "baseiconbutton.h"
#include "globalsizedata.h"

BaseIconButton::BaseIconButton(QString iconPath, QString value, bool state)
{
    m_iconPath = iconPath;
    m_value = value;
    m_state = state;
    m_width = 180;
    m_height = 45;
    m_space = 16;

    this->init();
    this->slotChangeState(state);
    this->changeFontSize();
}

BaseIconButton::~BaseIconButton()
{
}

void BaseIconButton::init(void)
{
    this->setFlat(true);
    this->setFocusPolicy(Qt::NoFocus);
    this->setMinimumSize(m_width , m_height);

    m_iconLab = new QLabel(this);   /* 按钮图标 */
    m_iconLab->setPixmap(QIcon::fromTheme(m_iconPath).pixmap(QSize(20, 20)));

    m_valueLab = new QLabel(this);   /* 按钮内容 */
    m_valueLab->setText(m_value);

    QFont font;
    font.setPixelSize(14);
    m_valueLab->setFont(font);

    m_pictureToWhite = new PictureToWhite(this);   /* 图标泛白类 */

    m_hlayout = new QHBoxLayout();
    m_hlayout->addSpacing(m_space);
    m_hlayout->addWidget(m_iconLab);
    m_hlayout->addSpacing(m_space / 2);
    m_hlayout->addWidget(m_valueLab);
    m_hlayout->addStretch();

    this->setLayout(m_hlayout);

    this->setAutoExclusive(true);   /* 启用互斥 */
    this->setCheckable(true);       /* 启用checked功能 */
    this->changeStyle();

    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigFontChange, this, &BaseIconButton::changeFontSize);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigFontNameChange, this, &BaseIconButton::slotChangeFontName);

    return;
}

void BaseIconButton::changeStyle()
{
    /* 深浅主题控件样式 */
    /* 浅色主题且按钮未点击，按钮内容黑色 */
    if (GlobalSizeData::THEME_COLOR == GlobalSizeData::LINGMOLight && m_state == false) {
        m_iconLab->setPixmap(QIcon::fromTheme(m_iconPath).pixmap(QSize(20, 20)));
        m_valueLab->setStyleSheet("color:#000000");
    } else {
        m_iconLab->setPixmap(m_pictureToWhite->drawSymbolicColoredPixmap(QIcon::fromTheme(m_iconPath).pixmap(QSize(20, 20))));
        m_valueLab->setStyleSheet("color:#FFFFFF");
    }
    return;
}

void BaseIconButton::changeFontSize()
{
    QFont font14;
    font14.setPointSizeF(GlobalSizeData::getInstance()->g_font14pxToPt);
    m_valueLab->setFont(font14);
    return;
}

void BaseIconButton::slotChangeState(bool state)
{
    m_state = state;
    this->changeStyle();
    return;
}

void BaseIconButton::slotChangeFontName()
{
    this->update();
    return;
}