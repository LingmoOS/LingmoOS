#include <QDebug>
#include <QIcon>
#include <QPixmap>

#include "basesearchedit.h"
#include "globalsizedata.h"

BaseSearchEdit::BaseSearchEdit(QString iconPath, QString value)
{
    m_width = 240;
    m_height = 32;
    m_iconPath = iconPath;
    m_value = value;

    this->init();
    this->changeFontSize();
}

BaseSearchEdit::~BaseSearchEdit()
{
}

void BaseSearchEdit::init(void)
{
    this->setMinimumSize(m_width, m_height);

    m_searchFontEdit = new QLineEdit(this);
    m_iconLab = new QLabel(m_searchFontEdit);

    QIcon searchIcon = QIcon::fromTheme(m_iconPath);
    m_iconLab->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(20 , 20))));
    m_iconLab->setProperty("isWindowButton", 0x1);
    m_iconLab->setProperty("useIconHighlightEffect", 0x2);
    m_iconLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    m_iconLab->setAttribute(Qt::WA_TranslucentBackground, true);

    m_searchFontEdit->setMinimumSize(240, 32);
    m_searchFontEdit->setMaximumSize(240, 37);
    m_searchFontEdit->setPlaceholderText(m_value);   /* 提示文字 */
    m_searchFontEdit->setMaxLength(30);              /* 字数限制 */
    m_searchFontEdit->setTextMargins(m_iconLab->width(), 1, 1 , 1);   /* 设置输入区 */
    m_searchFontEdit->setContextMenuPolicy(Qt::NoContextMenu);        /* 禁用右键 */

    this->installEventFilter(this);
    m_searchFontEdit->installEventFilter(this);
    m_searchFontEdit->clearFocus();

    connect(m_searchFontEdit, &QLineEdit::textChanged, this, &BaseSearchEdit::slotSearchFont);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigFontChange, this, &BaseSearchEdit::changeFontSize);

    return;
}

bool BaseSearchEdit::eventFilter(QObject *watch, QEvent *e)
{
    if ((e->type() == QEvent::MouseButtonPress && watch != m_searchFontEdit) ||
        (e->type() == QEvent::FocusOut && watch == m_searchFontEdit)) {
        if (m_searchFontEdit->text() == "") {
            QFontMetrics lineFontMetr = QFontMetrics(m_searchFontEdit->font());
            m_iconLab->setFixedSize(m_searchFontEdit->width() / 2 - lineFontMetr.width(tr("Search")) / 2 , 32);
            m_iconLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            m_searchFontEdit->setTextMargins(m_iconLab->width() , 1 , 1 , 1);
            m_searchFontEdit->setPlaceholderText(m_value);   /* 提示文字 */
        }
        m_searchFontEdit->clearFocus();
    } else if ((e->type() == QEvent::MouseButtonPress && watch ==  m_searchFontEdit) ||
               (e->type() == QEvent::FocusIn && watch == m_searchFontEdit)){
        m_iconLab->setFixedSize(25, 32);
        m_searchFontEdit->setTextMargins(m_iconLab->width() , 1 , 1 , 1);
        m_searchFontEdit->setFocus();
        m_searchFontEdit->setPlaceholderText(m_value);
    }
    return QObject::eventFilter(watch , e);
}

void BaseSearchEdit::changeFontSize()
{
    QFont font14;
    font14.setPointSizeF(GlobalSizeData::getInstance()->g_font14pxToPt);
    m_searchFontEdit->setFont(font14);
    if (m_searchFontEdit->text() == "") {
        QFontMetrics lineFontMetr = QFontMetrics(font14);
        m_iconLab->setFixedSize(m_searchFontEdit->width() / 2 - lineFontMetr.width(tr("Search")) / 2 , 32);
        m_searchFontEdit->setTextMargins(m_iconLab->width() , 1 , 1 , 1);
    }

    return;
}

void BaseSearchEdit::searchFocusOut()
{
    m_searchFontEdit->clearFocus();
    return;
}

void BaseSearchEdit::slotSearchFont(QString searchText)
{
    /* 添加 搜索字体 SDK的打点 */
    GlobalSizeData::getInstance()->searchFontSDKPoint();
    
    emit sigSearchFont(searchText);
    return;
}
