/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "kleftwidget.h"
#include "macro.h"
#include "util.h"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QVariant>
#include <QDebug>
#include <QTimer>
#include <QProcess>
#include <QStringList>
#include <QScrollBar>
#include <glibtop/netload.h>
#include <glibtop/netlist.h>
#include <glibtop/mem.h>
#include <glibtop/swap.h>

#define PERSONALSIE_SCHEMA     "org.lingmo.control-center.personalise"
#define PERSONALSIE_TRAN_KEY   "transparency"
#define CONTAIN_PERSONALSIE_TRAN_KEY   "transparency"

static unsigned long long getCpuTimeData(unsigned long long &workTime)
{
    FILE *file = fopen("/proc/stat", "r");
    if (file == NULL) {
        return 0;
    }

    char buffer[1024] = {0};
    unsigned long long user = 0, nice = 0, system = 0, idle = 0;
    unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guestnice = 0;

    char* ret = fgets(buffer, sizeof(buffer) - 1, file);
    if (ret == NULL) {
        fclose(file);
        return 0;
    }
    fclose(file);

    sscanf(buffer, "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guestnice);
    workTime = user + nice + system;

    return user + nice + system + idle + iowait + irq + softirq + steal;
}

typedef struct _net
{
    guint64 last_in, last_out;
    gint64 time;
} net;
net netinfo;

inline void getNetworkBytesData(unsigned long long int &receiveBytes, unsigned long long int &sendBytes, unsigned long long int &receiveRateBytes, unsigned long long int &sendRateBytes)
{
    glibtop_netlist netlist;
    char **ifnames;
    guint32 i;
    guint64 in = 0, out = 0;
    gint64 time;
    guint64 din = 0, dout = 0;
    ifnames = glibtop_get_netlist(&netlist);

    for (i = 0; i < netlist.number; ++i) {
        glibtop_netload netload;
        glibtop_get_netload(&netload, ifnames[i]);

        if (netload.if_flags & (1 << GLIBTOP_IF_FLAGS_LOOPBACK))
            continue;

        /* Skip interfaces without any IPv4/IPv6 address (or
           those with only a LINK ipv6 addr) However we need to
           be able to exclude these while still keeping the
           value so when they get online (with NetworkManager
           for example) we don't get a suddent peak.  Once we're
           able to get this, ignoring down interfaces will be
           possible too.  */
        if (not (netload.flags & (1 << GLIBTOP_NETLOAD_ADDRESS6)
                 and netload.scope6 != GLIBTOP_IF_IN6_SCOPE_LINK)
            and not (netload.flags & (1 << GLIBTOP_NETLOAD_ADDRESS)))
            continue;

        /* Don't skip interfaces that are down (GLIBTOP_IF_FLAGS_UP)
           to avoid spikes when they are brought up */

        in  += netload.bytes_in;
        out += netload.bytes_out;
    }

    g_strfreev(ifnames);

    time = g_get_real_time();

    if (in >= netinfo.last_in && out >= netinfo.last_out && netinfo.time != 0) {
        float dtime;
        dtime = (double)(time - netinfo.time) / G_USEC_PER_SEC;
        din   = static_cast<guint64>((in  - netinfo.last_in)  / dtime);
        dout  = static_cast<guint64>((out - netinfo.last_out) / dtime);
    } else {
        /* Don't calc anything if new data is less than old (interface
           removed, counters reset, ...) or if it is the first time */
        din  = 0;
        dout = 0;
    }

    netinfo.last_in  = in;
    netinfo.last_out = out;
    netinfo.time     = time;

    receiveBytes = in;
    sendBytes = out;
    receiveRateBytes = din;
    sendRateBytes = dout;
}

KLeftWidget::KLeftWidget(QWidget *parent)
    : QWidget(parent)
{
    this->setFixedWidth(LEFT_WIDGET_WIDTH);
    this->setMinimumHeight(LEFT_WIDGET_HEIGHT);
    this->installEventFilter(this);
    QGSettings *personalQgsettings = nullptr;
    if (QGSettings::isSchemaInstalled(PERSONALSIE_SCHEMA)) {
        personalQgsettings = new QGSettings(PERSONALSIE_SCHEMA, QByteArray(), this);
        transparency = personalQgsettings->get(PERSONALSIE_TRAN_KEY).toDouble() * 255;
        connect(personalQgsettings,&QGSettings::changed,this,[=](QString changedKey) {  //监听透明度变化
                        if (changedKey == CONTAIN_PERSONALSIE_TRAN_KEY) {
                           transparency = personalQgsettings->get(PERSONALSIE_TRAN_KEY).toDouble() * 255;
                           this->repaint();
                        }
                });
    } else {
        personalQgsettings = nullptr;
        qDebug()<<PERSONALSIE_SCHEMA<<" not installed";
    }
    initUI();
    initConnections();
}

KLeftWidget::~KLeftWidget()
{
    if (m_styleSettings) {
        delete m_styleSettings;
        m_styleSettings = nullptr;
    }
}

void KLeftWidget::initUI()
{
    // 初始化布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_mainLayout->setSpacing(16);
    m_titleLayout = new QHBoxLayout();
    m_titleLayout->setContentsMargins(0,0,0,0);
    m_titleLayout->setSpacing(0);
    m_itemsLayout = new QVBoxLayout();
    m_itemsLayout->setContentsMargins(0,0,0,0);
    m_itemsLayout->setSpacing(0);
    m_resLayout = new QVBoxLayout();
    m_resLayout->setContentsMargins(0,0,0,12);
    m_resLayout->setSpacing(12);
    m_layoutOptions = new QVBoxLayout();
    m_layoutOptions->setContentsMargins(10,0,10,0);
    m_layoutOptions->setSpacing(16);

    // 初始化控件
    m_titleLayout->setContentsMargins(8,8,0,0);
    m_labelTitleIcon  = new QLabel(this);
    m_labelTitle  = new KLabel(tr("Lingmo System Monitor"), this);

    m_labelTitleIcon->setFixedSize(24, 24);
    QIcon titleIcon = QIcon::fromTheme("lingmo-system-monitor");
    m_labelTitleIcon->setPixmap(titleIcon.pixmap(titleIcon.actualSize(QSize(24, 24))));
    m_labelTitle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_titleLayout->addWidget(m_labelTitleIcon);
    m_titleLayout->addSpacing(8);
    m_titleLayout->addWidget(m_labelTitle);
    m_titleLayout->addStretch();

    QWidget *titleWidget = new QWidget(this);
    titleWidget->setFixedHeight(40);
    titleWidget->setLayout(m_titleLayout);
    m_mainLayout->addWidget(titleWidget);

    m_frameOptions = new QFrame();
    m_frameOptions->setLayout(m_layoutOptions);
    m_scrollOptions = new QScrollArea();
    m_scrollOptions->setContentsMargins(0, 0, 0, 0);
    m_scrollOptions->setBackgroundRole(QPalette::Window);
    m_scrollOptions->setAutoFillBackground(true);
    m_scrollOptions->setFrameStyle(0);
    m_scrollOptions->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollOptions->setWidgetResizable(true);
    m_scrollOptions->setWidget(m_frameOptions);
    m_scrollOptions->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::transparent);
    m_scrollOptions->setPalette(pal);
    m_scrollOptions->viewport()->setPalette(pal);

    m_scrollOptions->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_scrollOptions->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    m_itemsLayout->setAlignment(Qt::AlignTop);
    QStandardItem *processItem = new QStandardItem(QIcon::fromTheme("lingmo-xtjs-all-process-symbolic"), tr("Processes"));
    QStandardItem *serviceItem = new QStandardItem(QIcon::fromTheme("lingmo-xtjs-process-symbolic"), tr("Services"));
    QStandardItem *disksItem = new QStandardItem(QIcon::fromTheme("drive-harddisk-symbolic"), tr("Disks"));
    m_buttonBar = new KNavigationBar(this);
    m_buttonBar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_buttonBar->setMinimumHeight(120);
    m_buttonBar->setFixedWidth(180);
    m_buttonBar->addItem(processItem);
    m_buttonBar->addItem(serviceItem);
    m_buttonBar->addItem(disksItem);
    m_buttonBar->listview()->setCurrentIndex(processItem->index());

    m_itemsLayout->addWidget(m_buttonBar);
    m_layoutOptions->addLayout(m_itemsLayout);
    // 资源布局
    m_layoutOptions->addLayout(m_resLayout);
    m_resLayout->setAlignment(Qt::AlignBottom);
    m_cpuHisWidget = new CpuHistoryWidget();
    m_cpuHisWidget->setFixedSize(LEFT_BUTTON_WIDTH,119);
    m_resLayout->addWidget(m_cpuHisWidget);

    m_memHisWidget = new MemHistoryWidget();
    m_memHisWidget->setFixedSize(LEFT_BUTTON_WIDTH,168);
    m_resLayout->addWidget(m_memHisWidget);

    m_netHisWidget = new NetHistoryWidget();
    m_netHisWidget->setFixedSize(LEFT_BUTTON_WIDTH,168);
    m_resLayout->addWidget(m_netHisWidget);

    m_mainLayout->addWidget(m_scrollOptions);
    this->setLayout(m_mainLayout);
    initThemeMode();
    m_updateStatusTimer = new QTimer(this);
}

void KLeftWidget::initConnections()
{
    connect(m_buttonBar->listview(), &QListView::clicked, this, [=](const QModelIndex &index){
        Q_EMIT switchPage(index.row());
    });
    connect(m_cpuHisWidget, &CpuHistoryWidget::clicked, this, [=](){
        Q_EMIT switchPage(m_buttonBar->model()->rowCount());
    });
    connect(m_memHisWidget, &MemHistoryWidget::clicked, this, [=](){
        Q_EMIT switchPage(m_buttonBar->model()->rowCount()+1);
    });
    connect(m_netHisWidget, &NetHistoryWidget::clicked, this, [=](){
        Q_EMIT switchPage(m_buttonBar->model()->rowCount()+2);
    });
    connect(m_updateStatusTimer, SIGNAL(timeout()), this, SLOT(onResourceUpdate()));
    connect(this, &KLeftWidget::updateCpuStatus, m_cpuHisWidget, &CpuHistoryWidget::onUpdateData);
    connect(this, &KLeftWidget::updateMemStatus, m_memHisWidget, &MemHistoryWidget::onUpdateMemData);
    connect(this, &KLeftWidget::updateSwapStatus, m_memHisWidget, &MemHistoryWidget::onUpdateSwapData);
    connect(this, &KLeftWidget::onUpdateNetStatus, m_netHisWidget, &NetHistoryWidget::onUpdateData);
    m_updateStatusTimer->start(2000);
}

void KLeftWidget::onSwichPanel(int nIndex)
{
    m_buttonBar->listview()->setCurrentIndex(m_buttonBar->model()->index(nIndex,0));
}

int KLeftWidget::currentIndex()
{
    return m_buttonBar->listview()->currentIndex().row();
}

void KLeftWidget::initThemeMode()
{
    const QByteArray idd(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(idd)) {
        m_styleSettings = new QGSettings(idd);
    }
    if (m_styleSettings) {
        //监听主题改变
        connect(m_styleSettings, &QGSettings::changed, this, [=](const QString &key)
        {
            if (key == "styleName") {
            } else if ("iconThemeName" == key) {
                if (m_labelTitleIcon) {
                    m_labelTitleIcon->setPixmap(QIcon::fromTheme("lingmo-system-monitor").pixmap(24,24));
                }
            }
        });
    }
}

void KLeftWidget::onResourceUpdate()
{
    m_prevCpuWorkTime = m_cpuworkTime;
    m_prevCpuTotalTime = m_cpuTotalTime;
    m_cpuTotalTime = getCpuTimeData(m_cpuworkTime);
    if (m_prevCpuWorkTime != 0 && m_prevCpuTotalTime != 0) {
        emit updateCpuStatus((m_cpuworkTime - m_prevCpuWorkTime) * 100.0 / (m_cpuTotalTime - m_prevCpuTotalTime));
    } else {
        emit updateCpuStatus(0);
    }

    //swap and memory data get signal
    glibtop_mem mem;
    glibtop_swap swap;

    glibtop_get_mem(&mem);
    glibtop_get_swap(&swap);

    //初始单位为字节，需要修正
    float memUsed = mem.used;
    float memTotal = mem.total;

    float swapUsed = swap.used;
    float swapTotal = swap.total;

    unsigned long long lluSwapSize = 0, lluSwapUsed = 0;
    if (getSwaponData(lluSwapSize, lluSwapUsed)) {
        swapUsed = lluSwapUsed*1024;
        swapTotal = lluSwapSize*1024;
    }

    emit updateMemStatus(memUsed, memTotal);
    emit updateSwapStatus(swapUsed, swapTotal);

    //net work data get signal
    getNetworkBytesData(m_totalRecvBytes, m_totalSentBytes, m_rateRecvBytes, m_rateSentBytes);
    emit onUpdateNetStatus(m_totalSentBytes, m_rateSentBytes, m_totalRecvBytes, m_rateRecvBytes);
}

bool KLeftWidget::getSwaponData(unsigned long long &swapSize, unsigned long long &swapUsed)
{
    bool bSucessed = false;
    QProcess proc;
    QStringList options;
    options << "-s";
    proc.start("swapon", options);
    proc.waitForFinished();
    QString dpkgInfo = proc.readAll();
    QStringList infoList = dpkgInfo.split("\n");
    for (int n = 0; n < infoList.size(); n++) {
        QString strInfoLine = infoList[n];
        if (strInfoLine.contains("swap_file")) {
            QStringList lineInfoList = strInfoLine.split(QRegExp("[\\s]+"));
            if (lineInfoList.size() >= 4) {
                lineInfoList[2] = lineInfoList[2].trimmed();
                lineInfoList[3] = lineInfoList[3].trimmed();
                swapSize = lineInfoList[2].toULongLong();
                swapUsed = lineInfoList[3].toULongLong();
                bSucessed = true;
            }
            break;
        }
    }
    return bSucessed;
}

void KLeftWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
//    QColor color = palette().color(QPalette::Base);
    QColor color = palette().color(QPalette::Window);
    color.setAlpha(transparency);
    QPalette pal(this->palette());
    this->setPalette(pal);
    QBrush brush =QBrush(color);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect,0,0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void KLeftWidget::setTabletMode(bool b)
{
    this->tabletMode = b;
    if (tabletMode) {
        m_labelTitleIcon->setFixedSize(32, 32);
        QIcon titleIcon = QIcon::fromTheme("lingmo-system-monitor");
        m_labelTitleIcon->setPixmap(titleIcon.pixmap(titleIcon.actualSize(QSize(32, 32))));
    } else {
        m_labelTitleIcon->setFixedSize(24, 24);
        QIcon titleIcon = QIcon::fromTheme("lingmo-system-monitor");
        m_labelTitleIcon->setPixmap(titleIcon.pixmap(titleIcon.actualSize(QSize(24, 24))));
    }
}
