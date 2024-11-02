/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "kcommentpanel.h"
#include "themeController.h"
#include <QBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QPainter>

namespace kdk
{
class StarWidget: public QWidget
{
public:
    StarWidget(QWidget*parent);
    void setGrade(StarLevel level);
private:
    void doFresh();
    StarLevel m_starLevel;
    QLabel *m_plbl1;
    QLabel *m_plbl2;
    QLabel *m_plbl3;
    QLabel *m_plbl4;
    QLabel *m_plbl5;
};

StarWidget::StarWidget(QWidget *parent)
    :QWidget(parent),
      m_starLevel(LevelZero)
{
    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->setSpacing(2);
    m_plbl1 = new QLabel(this);
    m_plbl2 = new QLabel(this);
    m_plbl3 = new QLabel(this);
    m_plbl4 = new QLabel(this);
    m_plbl5 = new QLabel(this);
    m_plbl1->setAlignment(Qt::AlignTop);
    m_plbl2->setAlignment(Qt::AlignTop);
    m_plbl3->setAlignment(Qt::AlignTop);
    m_plbl4->setAlignment(Qt::AlignTop);
    m_plbl5->setAlignment(Qt::AlignTop);
    m_plbl1->setFixedSize(14,14);
    m_plbl2->setFixedSize(14,14);
    m_plbl3->setFixedSize(14,14);
    m_plbl4->setFixedSize(14,14);
    m_plbl5->setFixedSize(14,14);
    hLayout->addWidget(m_plbl1);
    hLayout->addWidget(m_plbl2);
    hLayout->addWidget(m_plbl3);
    hLayout->addWidget(m_plbl4);
    hLayout->addWidget(m_plbl5);
}

void StarWidget::setGrade(StarLevel level)
{
    if(level != m_starLevel)
    {
        m_starLevel = level;
        doFresh();
    }
}

void StarWidget::doFresh()
{
    switch(m_starLevel)
    {
    case LevelZero:
        break;
    case LevelOne:
        m_plbl1->setPixmap(QIcon::fromTheme("lingmo-starred-on-symbolic").pixmap(12,12));
        m_plbl2->setPixmap(QIcon::fromTheme("lingmo-starred-symbolic").pixmap(12,12));
        m_plbl3->setPixmap(QIcon::fromTheme("lingmo-starred-symbolic").pixmap(12,12));
        m_plbl4->setPixmap(QIcon::fromTheme("lingmo-starred-symbolic").pixmap(12,12));
        m_plbl5->setPixmap(QIcon::fromTheme("lingmo-starred-symbolic").pixmap(12,12));
        break;
    case LevelTwo:
        m_plbl1->setPixmap(QIcon::fromTheme("lingmo-starred-on-symbolic").pixmap(12,12));
        m_plbl2->setPixmap(QIcon::fromTheme("lingmo-starred-on-symbolic").pixmap(12,12));
        m_plbl3->setPixmap(QIcon::fromTheme("lingmo-starred-symbolic").pixmap(12,12));
        m_plbl4->setPixmap(QIcon::fromTheme("lingmo-starred-symbolic").pixmap(12,12));
        m_plbl5->setPixmap(QIcon::fromTheme("lingmo-starred-symbolic").pixmap(12,12));
        break;
    case LevelThree:
        m_plbl1->setPixmap(QIcon::fromTheme("lingmo-starred-on-symbolic").pixmap(12,12));
        m_plbl2->setPixmap(QIcon::fromTheme("lingmo-starred-on-symbolic").pixmap(12,12));
        m_plbl3->setPixmap(QIcon::fromTheme("lingmo-starred-on-symbolic").pixmap(12,12));
        m_plbl4->setPixmap(QIcon::fromTheme("lingmo-starred-symbolic").pixmap(12,12));
        m_plbl5->setPixmap(QIcon::fromTheme("lingmo-starred-symbolic").pixmap(12,12));
        break;
    case LevelFour:
        m_plbl1->setPixmap(QIcon::fromTheme("lingmo-starred-on-symbolic").pixmap(12,12));
        m_plbl2->setPixmap(QIcon::fromTheme("lingmo-starred-on-symbolic").pixmap(12,12));
        m_plbl3->setPixmap(QIcon::fromTheme("lingmo-starred-on-symbolic").pixmap(12,12));
        m_plbl4->setPixmap(QIcon::fromTheme("lingmo-starred-on-symbolic").pixmap(12,12));
        m_plbl5->setPixmap(QIcon::fromTheme("lingmo-starred-symbolic").pixmap(12,12));
        break;
    case LevelFive:
        m_plbl1->setPixmap(QIcon::fromTheme("lingmo-starred-on-symbolic").pixmap(12,12));
        m_plbl2->setPixmap(QIcon::fromTheme("lingmo-starred-on-symbolic").pixmap(12,12));
        m_plbl3->setPixmap(QIcon::fromTheme("lingmo-starred-on-symbolic").pixmap(12,12));
        m_plbl4->setPixmap(QIcon::fromTheme("lingmo-starred-on-symbolic").pixmap(12,12));
        m_plbl5->setPixmap(QIcon::fromTheme("lingmo-starred-on-symbolic").pixmap(12,12));
        break;
    }

}

class KCommentPanelPrivate :public QObject,public ThemeController
{
     Q_DECLARE_PUBLIC(KCommentPanel)
    Q_OBJECT

public:
    KCommentPanelPrivate(KCommentPanel*parent);
    void changeTheme();

private:
    QLabel* m_pContentLabel;
    QLabel* m_pPicLabel;
    QLabel* m_pTimeLabel;
    QLabel* m_pNameLabel;
    StarWidget* m_pStarWidget;
    QColor m_color;
    KCommentPanel* q_ptr;
};

KCommentPanel::KCommentPanel(QWidget *parent) :
    QWidget(parent),
    d_ptr(new KCommentPanelPrivate(this))
{

}

void KCommentPanel::setIcon(const QIcon &icon)
{
    Q_D(KCommentPanel);
    d->m_pPicLabel->setPixmap(icon.pixmap(50,50));
}

void KCommentPanel::setTime(const QString &str)
{
    Q_D(KCommentPanel);
    d->m_pTimeLabel->setText(str);
}

void KCommentPanel::setName(const QString &str)
{
    Q_D(KCommentPanel);
    d->m_pNameLabel->setText(str);
}

void KCommentPanel::setText(const QString &str)
{
    Q_D(KCommentPanel);
    d->m_pContentLabel->setText(str);
}

void KCommentPanel::setGrade(StarLevel level)
{
    Q_D(KCommentPanel);
    d->m_pStarWidget->setGrade(level);
}

void KCommentPanel::paintEvent(QPaintEvent *event)
{
    Q_D(KCommentPanel);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setPen(Qt::NoPen);
    painter.setBrush(d->m_color);

    QRect rect = this->rect(); rect.setWidth(rect.width() );
    rect.setHeight(rect.height());
    painter.drawRoundedRect(rect, 6, 6);
}

KCommentPanelPrivate::KCommentPanelPrivate(KCommentPanel *parent)
    :q_ptr(parent)
{
    Q_Q(KCommentPanel);
    QVBoxLayout* mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(16,16,16,16);
    QHBoxLayout* controlLayout = new QHBoxLayout;
    m_pPicLabel = new QLabel(q);
    m_pPicLabel->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    m_pPicLabel->setFixedSize(50,50);
    controlLayout->addWidget(m_pPicLabel);

    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->setSpacing(1);
    vLayout->setContentsMargins(0,0,0,0);
    controlLayout->addLayout(vLayout);

    QHBoxLayout*hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);
    m_pNameLabel = new QLabel(q);
    m_pNameLabel->setAlignment(Qt::AlignBottom);
    m_pNameLabel->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    m_pTimeLabel = new QLabel(q);
    m_pTimeLabel->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    hLayout->addWidget(m_pNameLabel);
    hLayout->addStretch();
    hLayout->addWidget(m_pTimeLabel);
    vLayout->addLayout(hLayout);
    hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);
    m_pStarWidget = new StarWidget(q);
    hLayout->addWidget(m_pStarWidget);
    hLayout->addStretch();
    vLayout->addLayout(hLayout);
    m_pContentLabel = new QLabel(q);
    m_pContentLabel->setWordWrap(true);
    m_pContentLabel->setAlignment(Qt::AlignTop);
    m_pContentLabel->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(m_pContentLabel);
    mainLayout->addStretch();
    changeTheme();

    connect(m_gsetting,&QGSettings::changed,this,&KCommentPanelPrivate::changeTheme);

}

void KCommentPanelPrivate::changeTheme()
{
    Q_Q(KCommentPanel);
    initThemeStyle();
    if (ThemeController::themeMode() == LightTheme)
    {
        m_pTimeLabel->setStyleSheet("font-size:14px;color:#8C8C8C;");
        m_pNameLabel->setStyleSheet("font-size:16px;font-weight:500;color:#262626;");
        m_color = QColor("#F5F5F5");
        m_pContentLabel->setStyleSheet("color:#595959");
    }
    else
    {
        m_pTimeLabel->setStyleSheet("font-size:14px;color:#737373;");
        m_pNameLabel->setStyleSheet("font-size:16px;font-weight:500;color:#FFFFFF;");
        m_color = QColor("#232426");
        m_pContentLabel->setStyleSheet("color:#D9D9D9");
    }
}

}
#include "kcommentpanel.moc"
#include "moc_kcommentpanel.cpp"

