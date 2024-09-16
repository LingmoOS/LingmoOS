// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "content.h"

#include <DApplication>
#include <DSuggestButton>
#include <DCommandLinkButton>
#include <DFontSizeManager>

#include <QScrollArea>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QFile>
#include <QTimer>
#include <QProcess>
#include <QDebug>
#include <QStandardPaths>
#include <QFontMetrics>
#include <QScroller>
#include <QFuture>
#include <QtConcurrent>
#include <QThread>
#include <QTranslator>

#include <unistd.h>

DWIDGET_USE_NAMESPACE

Content::Content(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(new QScrollArea)
    , m_acceptCheck(new QCheckBox)
    , m_cancelBtn(new QPushButton)
    , m_acceptBtn(new DSuggestButton)
    , m_source(new QLabel)
    , m_languageBtn(new DButtonBox)
    , m_isCn(false)
    , m_hasCn(false)
    , m_hasEn(false)
{
    m_scrollArea->setAccessibleName("ScrollArea");
    m_acceptCheck->setAccessibleName("AcceptCheck");
    m_cancelBtn->setAccessibleName("CancelBtn");
    m_acceptBtn->setAccessibleName("AcceptBtn");
    m_source->setAccessibleName("SourceLabel");
    m_languageBtn->setAccessibleName("LanguageBtn");
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);

    setLayout(layout);

    DButtonBoxButton *btnChinese = new DButtonBoxButton("中文");
    btnChinese->setAccessibleName("ChineseBtn");
    btnChinese->setFixedSize(90, 36);
    DButtonBoxButton *btnEnginsh = new DButtonBoxButton("English");
    btnEnginsh->setAccessibleName("EnglishBtn");
    btnEnginsh->setFixedSize(90, 36);
    QList<DButtonBoxButton *> btnlist;
    btnlist.append(btnChinese);
    btnlist.append(btnEnginsh);
    m_languageBtn->setButtonList(btnlist, true);

    m_languageBtn->setId(btnChinese, 1);
    m_languageBtn->setId(btnEnginsh, 0);

    layout->addWidget(m_languageBtn, 0, Qt::AlignHCenter);

    m_scrollArea->setMinimumSize(468, 300);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameStyle(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    m_scrollArea->setContentsMargins(0, 0, 0, 0);
    QScroller::grabGesture(m_scrollArea->viewport(), QScroller::LeftMouseButtonGesture);

    QWidget *sourceWidget = new QWidget(this);
    sourceWidget->setAccessibleName("SourceWidget");
    QVBoxLayout *sourceLayout = new QVBoxLayout;
    sourceWidget->setLayout(sourceLayout);
    sourceLayout->addWidget(m_source);

    m_cancelBtn->setFixedSize(90, 36);
    m_acceptBtn->setFixedSize(90, 36);

    m_source->setTextFormat(Qt::MarkdownText);
    m_source->setWordWrap(true);
    m_source->setOpenExternalLinks(true);
    // 左右边距20
    m_source->setContentsMargins(20, 0, 20, 0);

    m_scrollArea->setWidget(sourceWidget);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->setMargin(0);
    bottomLayout->setSpacing(0);
    bottomLayout->addSpacing(10);
    bottomLayout->addWidget(m_acceptCheck, 0, Qt::AlignVCenter);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_cancelBtn, 0, Qt::AlignVCenter);
    bottomLayout->addSpacing(10);
    bottomLayout->addWidget(m_acceptBtn, 0, Qt::AlignVCenter);
    bottomLayout->addSpacing(10);

    m_bottom = new QWidget(this);
    m_bottom->setFixedHeight(65);
    m_bottom->setLayout(bottomLayout);
    m_bottom->setAccessibleName("ContentBottomWidget");

    layout->addWidget(m_scrollArea);
    layout->addSpacing(20);
    layout->addWidget(m_bottom);

    m_acceptBtn->setDisabled(true);

    // 当父进程退出后，此对话框自动退出
    __pid_t idp;
    idp = getppid();

    static bool quit = false;
    QFuture<void> future = QtConcurrent::run([ = ] () {
        while(!quit) {
            if (!QFile(QString("/proc/%1/exe").arg(idp)).exists()) {
                qApp->quit();
                break;
            }

            QThread::msleep(100);
        }
    });

    connect(qApp, &QApplication::aboutToQuit, this, [ = ] {
        quit = true;
    });

    connect(m_cancelBtn, &QPushButton::clicked, this, [ = ] {
        qApp->exit(-1);
    });

    connect(m_acceptBtn, &QPushButton::clicked, this, [ = ] {
        qApp->exit(96);
    });

    connect(m_acceptCheck, &QCheckBox::toggled, m_acceptBtn, &QPushButton::setEnabled);

    connect(m_languageBtn, &DButtonBox::buttonClicked, [this](QAbstractButton * value) {
        m_isCn = m_languageBtn->id(value);
        updateContent();
    });

    m_isCn = QLocale::system().language() == QLocale::Chinese;
    updateContent();
    updateLanguageBtn();

    DFontSizeManager::instance()->bind(m_acceptCheck, DFontSizeManager::SizeType::T8, 70);
}

void Content::setHideBottom(const bool &status)
{
    m_bottom->setVisible(!status);
}

int Content::calWidgetWidth()
{
    QPainter p(m_acceptCheck);
    QFontMetrics fm = p.fontMetrics();
    int width = fm.horizontalAdvance(m_enallow) + m_acceptBtn->width() * 2 + 40;
    return width;
}

void Content::setSource(const QString &source)
{
    if (source.isEmpty())
        return;

    m_source->setText(source);

    updateWindowHeight();
}

void Content::setAllowCheckBoxText(const QString &text)
{
    m_allow = text;
}

void Content::setEnAllowCheckBoxText(const QString &text)
{
    m_enallow = text;
}

void Content::setCnSource(const QString &source)
{
    m_cn = source;
    setSource(source);
    updateLanguageBtn();
    m_languageBtn->button(1)->setChecked(true);
}

void Content::setEnSource(const QString &source)
{
    m_en = source;
    setSource(source);
    updateLanguageBtn();
    m_languageBtn->button(0)->setChecked(true);
}

// 中英文都存在,选中本地语言
void Content::updateLocaleSource()
{
    if (!m_hasCn && !m_hasEn)
        return;
    if (QLocale::system().language() == QLocale::Chinese) {
        m_isCn = true;
        m_languageBtn->button(1)->setChecked(true);
    } else {
        m_isCn = false;
        m_languageBtn->button(0)->setChecked(true);
    }

    updateContent();
}

void Content::updateLanguageBtn()
{
    m_hasCn = QFile::exists(m_cn);
    m_hasEn = QFile::exists(m_en);
    m_languageBtn->setVisible(m_hasCn && m_hasEn);
}

void Content::updateContent()
{
    QTranslator trans;
    setSource(m_isCn ? m_cn : m_en);
    m_acceptCheck->setText(m_isCn ? m_allow : m_enallow);
    m_acceptCheck->setVisible(m_isCn ? !m_allow.isEmpty() : !m_enallow.isEmpty());
    if (m_hasCn && m_hasEn) {
        trans.load(QString("/usr/share/dde-session-ui/translations/dde-session-ui_%1").arg(m_isCn ? "zh_CN" : "en_US"));
    } else {
        trans.load(QString("/usr/share/dde-session-ui/translations/dde-session-ui_%1").arg(QLocale::system().name()));
    }
    qApp->installTranslator(&trans);
    m_cancelBtn->setText(tr("Cancel"));
    m_acceptBtn->setText(tr("Confirm"));
    m_acceptBtn->setEnabled(m_allow.isEmpty() || (!m_allow.isEmpty() && m_acceptCheck->isChecked()));

    Q_EMIT sourceChanged(m_isCn);
}

// 根据文本多少，调整窗口大小
void Content::updateWindowHeight()
{
    QRect rc = m_source->rect();
    rc.setWidth(rc.width() - 40);
    QSize sz = m_source->fontMetrics().boundingRect(rc, Qt::TextWordWrap, m_source->text()).size();

    int minHeight = qBound(300, sz.height(), 491);
    m_scrollArea->setMinimumHeight(minHeight);
}
