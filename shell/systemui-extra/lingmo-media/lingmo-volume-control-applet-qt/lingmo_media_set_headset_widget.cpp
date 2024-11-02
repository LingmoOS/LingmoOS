/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "lingmo_media_set_headset_widget.h"

#include <QHBoxLayout>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QStyleOption>
#include <QPainter>
#include <QProcess>
#include <QPainterPath>

extern double transparency;
LingmoUIMediaSetHeadsetWidget::LingmoUIMediaSetHeadsetWidget(QWidget *parent) : QWidget (parent)
{
    headphoneIconButton = new QToolButton();
    headsetIconButton = new QToolButton();
    microphoneIconButton = new QToolButton();
    soundSettingButton = new QPushButton(tr("Sound Settings"));
    cancelButton = new QPushButton(tr("Cancel"));
    selectSoundDeviceLabel = new QLabel(tr("Select Sound Device"));

    initSetHeadsetWidget();
    initPanelGSettings();

    headphoneIconButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    connect(headphoneIconButton,SIGNAL(clicked()),this,SLOT(headphoneButtonClickedSlot()));
    connect(headsetIconButton,SIGNAL(clicked()),this,SLOT(headsetButtonClickedSlot()));
    connect(microphoneIconButton,SIGNAL(clicked()),this,SLOT(microphoneButtonClickedSlot()));
    connect(soundSettingButton,SIGNAL(clicked()),this,SLOT(soundSettingButtonClickedSlot()));
    connect(cancelButton,SIGNAL(clicked()),this,SLOT(cancelButtonClickedSlot()));
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::ToolTip);
}

void LingmoUIMediaSetHeadsetWidget::initSetHeadsetWidget()
{
    this->setFixedSize(402,252);
    headphoneIconButton->setFixedSize(96,96);
    headsetIconButton->setFixedSize(96,96);
    microphoneIconButton->setFixedSize(96,96);
    headphoneIconButton->setIconSize(QSize(32,32));
    headsetIconButton->setIconSize(QSize(32,32));
    microphoneIconButton->setIconSize(QSize(32,32));
    selectSoundDeviceLabel->setFixedSize(402,24);
    soundSettingButton->setFixedSize(120,36);
    cancelButton->setFixedSize(107,36);

    soundSettingButton->setContextMenuPolicy(Qt::DefaultContextMenu);

    headphoneIconButton->setText(tr("Headphone"));
    headsetIconButton->setText(tr("Headset"));
    microphoneIconButton->setText(tr("Microphone"));

    headphoneIconButton->setIcon(QIcon::fromTheme("headphones-symbolic"));
    headsetIconButton->setIcon(QIcon::fromTheme("audio-headset-symbolic"));
    microphoneIconButton->setIcon(QIcon::fromTheme("audio-input-microphone"));

    headphoneIconButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    headsetIconButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    microphoneIconButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    selectSoundDeviceLabel->setAlignment(Qt::AlignCenter);

    QWidget *labelWidget = new QWidget;
    labelWidget->setFixedSize(402,24);
    QVBoxLayout *labelLayout = new QVBoxLayout;
    labelLayout->addWidget(selectSoundDeviceLabel);
    labelLayout->setContentsMargins(0,0,0,0);
    labelWidget->setLayout(labelLayout);

    QWidget *headsetButtonWidget = new QWidget();
    headsetButtonWidget->setFixedSize(402,96);
    QHBoxLayout *headsetButtonLayout = new QHBoxLayout;
    headsetButtonLayout->addWidget(headphoneIconButton);
    headsetButtonLayout->addWidget(headsetIconButton);
    headsetButtonLayout->addWidget(microphoneIconButton);
    headsetButtonLayout->setSpacing(24);
    headsetButtonLayout->setContentsMargins(32,0,32,0);
    headsetButtonWidget->setLayout(headsetButtonLayout);

    QWidget *buttonWidget = new QWidget;
    buttonWidget->setFixedSize(402,36);
//    buttonWidget->setStyleSheet("QWidget{background-color:rgb(255,0,0);}");
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(soundSettingButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->setSpacing(108);
    buttonLayout->setContentsMargins(32,0,32,0);
    buttonWidget->setLayout(buttonLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(labelWidget);
    mainLayout->addWidget(headsetButtonWidget);
    mainLayout->addWidget(buttonWidget);
    this->setLayout(mainLayout);
    this->setProperty("useSystemStyleBlur",true);
    this->setWindowTitle("whole window blur");
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setContentsMargins(0,16,0,16);
}

void LingmoUIMediaSetHeadsetWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
//    double transparence = transparency * 255;
    QColor color = palette().color(QPalette::Base);
//    color.setAlpha(transparence);
    QBrush brush = QBrush(color);
    p.setBrush(brush);
    p.setPen(Qt::NoPen);
    QPainterPath path;
    opt.rect.adjust(0,0,0,0);
    path.addRoundedRect(opt.rect,12,12);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.drawRoundedRect(opt.rect,12,12);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

/*
 *  set input and output port.
 *  input: intel mic
 *  output: headphone
*/
void LingmoUIMediaSetHeadsetWidget::headphoneButtonClickedSlot()
{
    isShow = false;
//    cset("name=Capture Source", HW_CARD, "2", 0, 0);
    QDBusMessage message =QDBusMessage::createSignal("/", "org.lingmo.media", "headsetJack");
    message<<"headphone";
    QDBusConnection::sessionBus().send(message);
    this->hide();
}

/*
 *  set input and output port.
 *  input: headsed mic
 *  output: headphone
 */
void LingmoUIMediaSetHeadsetWidget::headsetButtonClickedSlot()
{
    isShow = false;
//    cset("name=Capture Source", HW_CARD, "0", 0, 0);
    QDBusMessage message =QDBusMessage::createSignal("/", "org.lingmo.media", "headsetJack");
    message<<"headset";
    QDBusConnection::sessionBus().send(message);
    this->hide();
}

/*
 * set input and output port.
 * input: headphone mic
 * output: speaker
*/
void LingmoUIMediaSetHeadsetWidget::microphoneButtonClickedSlot()
{
    isShow = false;
//    int ret = cset("name=Capture Source", HW_CARD, "1", 0, 0);
    QDBusMessage message =QDBusMessage::createSignal("/", "org.lingmo.media", "headsetJack");
    message<<"headphone mic" ;
//    if (ret < 0)
//        cset("name=Input Source", HW_CARD, "1", 0, 0);
    QDBusConnection::sessionBus().send(message);
    this->hide();
}

/*
 *  Jump to the control panel
*/
void LingmoUIMediaSetHeadsetWidget::soundSettingButtonClickedSlot()
{
    QProcess *process;
    QStringList args;
    args.append("-m Audio");
    QString command = "lingmo-control-center";
    process = new QProcess();
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start(command, args);

    if (!process->waitForStarted()) {
        qDebug() << "start failed:" << process->errorString();
    } else {
        qDebug() << "start success:";
    }
    this->hide();
    isShow = false;
}

/*
 *  Cancel settings
*/
void LingmoUIMediaSetHeadsetWidget::cancelButtonClickedSlot()
{
    this->hide();
    isShow = false;
}

int LingmoUIMediaSetHeadsetWidget::cset(char * name, char *card, char *c, int roflag, int keep_handle)
{
    int err;
    static snd_ctl_t *handle = NULL;
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_value_t *control;
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_value_alloca(&control);

    printf("name[%s]card[%s]c[%s]", name, card, c);
    if (snd_ctl_ascii_elem_id_parse(id, name)) {
        printf("Wrong control identifier: %\n", name);
        return -1;
    }

    if (handle == NULL &&
            (err = snd_ctl_open(&handle, card, 0)) < 0) {
        printf("Control %s open error: %s", card, snd_strerror(err));
        return err;
    }

    snd_ctl_elem_info_set_id(info, id);

    if ((err = snd_ctl_elem_info(handle, info)) < 0) {
        //if (ignore_error)
        //return 0;
        printf("Cannot find the given element from control %s", card);
        if (! keep_handle) {
            snd_ctl_close(handle);
            handle = NULL;
        }
        return err;
    }

    snd_ctl_elem_info_get_id(info, id);
    if (!roflag) {
        snd_ctl_elem_value_set_id(control, id);

        if ((err = snd_ctl_elem_read(handle, control)) < 0) {
            //if (ignore_error)
            //return 0;
            printf("Cannot read the given element from control %s", card);
            if (! keep_handle) {
                snd_ctl_close(handle);
                handle = NULL;
            }
            return err;
        }
        err = snd_ctl_ascii_value_parse(handle, control, info, c);
        if (err < 0) {
            //if (!ignore_error)
            //error("Control %s parse error: %s\n", card, snd_strerror(err));
            if (!keep_handle) {
                snd_ctl_close(handle);
                handle = NULL;
            }
            //return ignore_error ? 0 : err;
            return err;
        }

        if ((err = snd_ctl_elem_write(handle, control)) < 0) {
            //if (!ignore_error)
            //error("Control %s element write error: %s\n", card, snd_strerror(err));
            if (!keep_handle) {
                snd_ctl_close(handle);
                handle = NULL;
            }
            //return ignore_error ? 0 : err;
            return err;
        }
    }
    if (! keep_handle) {
        snd_ctl_close(handle);
        handle = NULL;
    }

    snd_hctl_t *hctl;
    snd_hctl_elem_t *elem;
    if ((err = snd_hctl_open(&hctl, card, 0)) < 0) {
        printf("Control %s open error: %s", card, snd_strerror(err));
        return err;
    }

    if ((err = snd_hctl_load(hctl)) < 0) {
        printf("Control %s load error: %s", card, snd_strerror(err));
        return err;
    }

    elem = snd_hctl_find_elem(hctl, id);
    if (elem)
        showControl("  ", elem, LEVEL_BASIC | LEVEL_ID);
    else
        printf("Could not find the specified element");
    snd_hctl_close(hctl);

    return 0;
}

void LingmoUIMediaSetHeadsetWidget::showControlId(snd_ctl_elem_id_t *id)
{
    char *str;

    str = snd_ctl_ascii_elem_id_get(id);
    if (str)
        printf("%s", str);
    free(str);
}

int LingmoUIMediaSetHeadsetWidget::showControl(const char *space, snd_hctl_elem_t *elem,
                                            int level)
{
    int err;
    unsigned int item, idx, count, *tlv;
    snd_ctl_elem_type_t type;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_value_t *control;
    snd_aes_iec958_t iec958;
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_value_alloca(&control);
    if ((err = snd_hctl_elem_info(elem, info)) < 0) {
        printf("Control hw snd_hctl_elem_info error: %s\n", snd_strerror(err));
        return err;
    }
    if (level & LEVEL_ID) {
        snd_hctl_elem_get_id(elem, id);
        showControlId(id);
        printf("\n");
    }
    count = snd_ctl_elem_info_get_count(info);
    type = snd_ctl_elem_info_get_type(info);

    switch (type) {
    case SND_CTL_ELEM_TYPE_INTEGER:
        printf(",min=%li,max=%li,step=%li \n",
               snd_ctl_elem_info_get_min(info),
               snd_ctl_elem_info_get_max(info),
               snd_ctl_elem_info_get_step(info));
        break;
    case SND_CTL_ELEM_TYPE_INTEGER64:
        printf(",min=%lli,max=%lli,step=%lli \n",
               snd_ctl_elem_info_get_min64(info),
               snd_ctl_elem_info_get_max64(info),
               snd_ctl_elem_info_get_step64(info));
        break;
    case SND_CTL_ELEM_TYPE_ENUMERATED:
    {
        unsigned int items = snd_ctl_elem_info_get_items(info);
        printf(",items=%u\n", items);
        for (item = 0; item < items; item++) {
            snd_ctl_elem_info_set_item(info, item);
            if ((err = snd_hctl_elem_info(elem, info)) < 0) {
                printf("Control hw element info error: %s\n", snd_strerror(err));
                return err;
            }
            printf("%s; Item #%u '%s' \n", space, item, snd_ctl_elem_info_get_item_name(info));
        }
        break;
    }
    default:
        printf("\n");
        break;
    }

    if (level & LEVEL_BASIC) {
        if (!snd_ctl_elem_info_is_readable(info))
            goto __skip_read;
        if ((err = snd_hctl_elem_read(elem, control)) < 0) {
            printf("Control hw:0 element read error: %s\n", snd_strerror(err));
            return err;
        }

        for (idx = 0; idx < count; idx++) {
            if (idx > 0)
                printf(",\n");
            switch (type) {
            case SND_CTL_ELEM_TYPE_BOOLEAN:
                printf("%s \n", snd_ctl_elem_value_get_boolean(control, idx) ? "on" : "off");
                break;
            case SND_CTL_ELEM_TYPE_INTEGER:
                printf("%li \n", snd_ctl_elem_value_get_integer(control, idx));
                break;
            case SND_CTL_ELEM_TYPE_INTEGER64:
                printf("%lli \n", snd_ctl_elem_value_get_integer64(control, idx));
                break;
            case SND_CTL_ELEM_TYPE_ENUMERATED:
                printf("%u \n", snd_ctl_elem_value_get_enumerated(control, idx));
                break;
            case SND_CTL_ELEM_TYPE_BYTES:
                printf("0x%02x \n", snd_ctl_elem_value_get_byte(control, idx));
                break;
            case SND_CTL_ELEM_TYPE_IEC958:
                snd_ctl_elem_value_get_iec958(control, &iec958);
                printf("[AES0=0x%02x AES1=0x%02x AES2=0x%02x AES3=0x%02x] \n",
                       iec958.status[0], iec958.status[1],
                        iec958.status[2], iec958.status[3]);
                break;
            default:
                printf("? \n");
                break;
            }
        }
__skip_read:
        if (!snd_ctl_elem_info_is_tlv_readable(info))
            goto __skip_tlv;
        /* skip ASoC ext bytes controls that may have huge binary TLV data */
        if (type == SND_CTL_ELEM_TYPE_BYTES &&
                !snd_ctl_elem_info_is_readable(info) &&
                !snd_ctl_elem_info_is_writable(info)) {
            printf("%s; ASoC TLV Byte control, skipping bytes dump \n", space);
            goto __skip_tlv;
        }

        tlv = (unsigned int *)malloc(4096);
        if ((err = snd_hctl_elem_tlv_read(elem, tlv, 4096)) < 0) {
            printf("Control hw:0 element TLV read error: %s \n", snd_strerror(err));
            free(tlv);
            return err;
        }
        //decode_tlv(strlen(space), tlv, 4096);
        free(tlv);
    }
__skip_tlv:
    return 0;
}

void LingmoUIMediaSetHeadsetWidget::showWindow()
{
    QRect rect = caculatePosition();
    QString platform = QGuiApplication::platformName();
    if (!platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)){
        this->setGeometry(QRect((rect.x() - rect.width()*2), rect.y(), rect.width(), rect.height()));
        this->showNormal();
    }
    else {
        this->show();
        kdk::WindowManager::setGeometry(this->windowHandle(), QRect((rect.x() - rect.width()*2), rect.y(), rect.width(), rect.height()));
    }

    isShow = true;
    this->show();
}

void LingmoUIMediaSetHeadsetWidget::initPanelGSettings()
{
    const QByteArray id(PANEL_SETTINGS);
    if (QGSettings::isSchemaInstalled(id)) {
        if (m_panelGSettings == nullptr) {
            m_panelGSettings = new QGSettings(id);
        }
        if (m_panelGSettings->keys().contains(PANEL_POSITION_KEY)) {
            m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
        }
        if (m_panelGSettings->keys().contains(PANEL_SIZE_KEY)) {
            m_panelSize = m_panelGSettings->get(PANEL_SIZE_KEY).toInt();
        }
        connect(m_panelGSettings, &QGSettings::changed, this, [&] (const QString &key) {
            if (key == PANEL_POSITION_KEY) {
                m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
            }
            if (key == PANEL_SIZE_KEY) {
                m_panelSize = m_panelGSettings->get(PANEL_SIZE_KEY).toInt();
            }
            if (this->isVisible()) {
                QRect rect = caculatePosition();
                QString platform = QGuiApplication::platformName();
                if (!platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)){
                    this->setGeometry(QRect((rect.x() - rect.width()*2), rect.y(), rect.width(), rect.height()));
                    this->showNormal();
                }
                else {
                    this->show();
                    kdk::WindowManager::setGeometry(this->windowHandle(), QRect((rect.x() - rect.width()*2), rect.y(), rect.width(), rect.height()));
                }
            }
        });
    }
}

QRect LingmoUIMediaSetHeadsetWidget::caculatePosition()
{
    QRect availableGeo = QGuiApplication::screenAt(QCursor::pos())->geometry();
    int x, y;
    int margin = 8;

    switch (m_panelPosition)
    {
    case PanelPosition::Top:
    {
        x = availableGeo.x() + availableGeo.width() - this->width() - margin;
        y = availableGeo.y() + m_panelSize + margin;
    }
        break;
    case PanelPosition::Bottom:
    {
        x = availableGeo.x() + availableGeo.width() - this->width() - margin;
        y = availableGeo.y() + availableGeo.height() - m_panelSize - this->height() - margin;
    }
        break;
    case PanelPosition::Left:
    {
        x = availableGeo.x() + m_panelSize + margin;
        y = availableGeo.y() + availableGeo.height() - this->height() - margin;
    }
        break;
    case PanelPosition::Right:
    {
        x = availableGeo.x() + availableGeo.width() - m_panelSize - this->width() - margin;
        y = availableGeo.y() + availableGeo.height() - this->height() - margin;
    }
        break;
    }

    return QRect(x, y, this->width(), this->height());
}

LingmoUIMediaSetHeadsetWidget::~LingmoUIMediaSetHeadsetWidget()
{

}

UkmediaHeadsetButtonWidget::UkmediaHeadsetButtonWidget(QString icon, QString text)
{
    iconLabel = new QLabel;
    textLabel = new QLabel(text);

    this->setFixedSize(96,96);
    iconLabel->setFixedSize(32,32);
    textLabel->setFixedHeight(20);
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(iconLabel);
    vLayout->addWidget(textLabel);
    vLayout->setSpacing(7);
    vLayout->setContentsMargins(0,20,0,20);
    this->setLayout(vLayout);

    iconLabel->setPixmap(QIcon::fromTheme(icon).pixmap(32,32));
}

UkmediaHeadsetButtonWidget::~UkmediaHeadsetButtonWidget()
{

}
