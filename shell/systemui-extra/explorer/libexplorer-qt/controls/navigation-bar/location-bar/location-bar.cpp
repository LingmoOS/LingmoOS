/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "location-bar.h"

#include "path-bar-model.h"
#include "file-utils.h"

#include "search-vfs-uri-parser.h"

#include "fm-window.h"

#include "file-info.h"
#include "file-info-job.h"
#include "file-enumerator.h"
#include "global-settings.h"

#include <QGSettings>
#include <QUrl>
#include <QMenu>

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionFocusRect>
#include <QStyleOptionFrame>
#include <QLineEdit>

#include <QStandardPaths>
#include <QApplication>
#include <QClipboard>

#include <QHBoxLayout>
#include <QToolButton>
#include <QPainter>
#include <QPainterPath>

#include <QProxyStyle>
#include <QStyleOptionToolButton>
#include <syslog.h>

using namespace Peony;

class LocationBarButtonStyle;
class IndicatorToolButton;

static LocationBarButtonStyle *buttonStyle = nullptr;

class LocationBarButtonStyle : public QProxyStyle
{
public:
    explicit LocationBarButtonStyle() : QProxyStyle() {}
    static LocationBarButtonStyle *getStyle() {
        if (!buttonStyle) {
            buttonStyle = new LocationBarButtonStyle;
        }
        return buttonStyle;
    }

    void polish(QWidget *widget) override;
    void unpolish(QWidget *widget) override;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
};

class IndicatorToolButton : public QToolButton
{
public:
    explicit IndicatorToolButton(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
};

LocationBar::LocationBar(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_Hover);
    setMouseTracking(true);

    auto currentStyleName = GlobalSettings::getInstance()->getValue("widgetThemeName").toString();
    if (currentStyleName == "classical") {
        m_is_classical = true;
    }

    connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=](const QString &key){
        if (key == "widgetThemeName") {
            auto currentStyleName = GlobalSettings::getInstance()->getValue("widgetThemeName").toString();
            if (currentStyleName == "classical") {
                m_is_classical = true;
            } else {
                m_is_classical = false;
            }
        }
        this->update();
    });

    //comment to fix button text show incomplete issue, link to bug#72080
//    setStyleSheet("padding-right: 15;"
//                  "margin-left: 2");
    m_styled_edit = new QLineEdit;
    qDebug()<<sizePolicy();
    //connect(this, &LocationBar::groupChangedRequest, this, &LocationBar::setRootUri);

    m_layout = new QHBoxLayout;
    setLayout(m_layout);

    m_indicator = new IndicatorToolButton(this);
    m_indicator->setObjectName("explorer_location_bar_indicator");
    m_indicator->setFocusPolicy(Qt::FocusPolicy(m_indicator->focusPolicy() & ~Qt::TabFocus));
    m_indicator->setAutoRaise(true);
    //m_indicator->setStyle(LocationBarButtonStyle::getStyle());
    m_indicator->setPopupMode(QToolButton::InstantPopup);
    m_indicator->setArrowType(Qt::RightArrow);
    m_indicator->setCheckable(true);
    m_indicator->setFixedSize(this->height() - 2, this->height() - 2);
    m_indicator->move(0, 1);

    m_indicator_menu = new QMenu(m_indicator);
    m_indicator->setMenu(m_indicator_menu);
    m_indicator->setArrowType(Qt::RightArrow);

    connect(m_indicator_menu, &QMenu::aboutToShow, this, [=](){
        m_indicator->setArrowType(Qt::DownArrow);
    });

    connect(m_indicator_menu, &QMenu::aboutToHide, this, [=](){
        m_indicator->setArrowType(Qt::RightArrow);
    });

    //fix bug 40503, button text not show completely issue when fontsize is very big
    if (QGSettings::isSchemaInstalled("org.lingmo.style"))
    {
        //font monitor
        QGSettings *fontSetting = new QGSettings(FONT_SETTINGS, QByteArray(), this);
        connect(fontSetting, &QGSettings::changed, this, [=](const QString &key){
            if (key == "systemFontSize") {
                // note that updateButtons() will cost more time.
                // there is no need to query file infos again here.
                // use doLayout() is enough.
                // btw, Bug#76858 is directly caused by info querying
                // due to updateButtons().

                //updateButtons();
                doLayout();
            }
        });
    }
}

LocationBar::~LocationBar()
{
    m_styled_edit->deleteLater();
}

void LocationBar::setRootUri(const QString &uri)
{
    m_indicator->setFixedSize(this->height() - 2, this->height() - 2);

    Q_EMIT aboutToSetRootUri();

    bool isSamePath = false;
    if (FileUtils::isSamePath(m_current_uri, uri)  && m_buttons.count() > 0) {
        isSamePath = true;
    }
    m_current_uri = uri;
    //clear buttons
    if (m_current_uri.startsWith("search://") || m_current_uri.startsWith("label://")) {
        clearButtons();
        //m_indicator->setArrowType(Qt::NoArrow);
        addButton(m_current_uri, false, false);
        //fix bug 94229, show button
        doLayout();
        return;
    }

    m_current_info = FileInfo::fromUri(uri);
    m_buttons_info.clear();
    auto tmpUri = uri;
    while (!tmpUri.isEmpty() && tmpUri != "") {
        m_buttons_info.prepend(FileInfo::fromUri(tmpUri));
//        if(tmpUri.startsWith("kmre:///") && tmpUri != "kmre:///"){
//            m_buttons_info.prepend(FileInfo::fromUri("kmre:///"));
//        }
//        if(tmpUri.startsWith("mult:///") && tmpUri != "mult:///"){
//            m_buttons_info.prepend(FileInfo::fromUri("mult:///"));
//        }
        tmpUri = FileUtils::getParentUri(tmpUri);
    }

    //when is the same uri and has buttons return
    if (isSamePath && m_buttons.count() == m_buttons_info.count()) {
        int i = 0 ;
        for (auto button : m_buttons) {
            QString uri = m_buttons_info[i].get()->uri().toLocal8Bit();
            if (uri != button->property("uri").toString()) {
                isSamePath = false;
            }
            i++;
        }

        if (isSamePath)
            return;
    }

    m_querying_buttons_info = m_buttons_info;

    for (auto info : m_buttons_info) {
        auto infoJob = new FileInfoJob(info);
        //infoJob->setAutoDelete();
        // enumerate buttons info directory
        auto enumerator = new FileEnumerator;
        //comment to fix kydroid path show abnormal issue
        //enumerator->setEnumerateWithInfoJob();
        connect(this, &LocationBar::aboutToSetRootUri, enumerator, [=]{
            enumerator->setProperty("isCancelled", true);
            enumerator->cancel();
        });
        connect(enumerator, &FileEnumerator::enumerateFinished, this, [=](bool successed){
            m_querying_buttons_info.removeOne(info);
            if (successed) {
                auto infos = enumerator->getChildren();
                m_infos_hash.insert(info.get()->uri(), infos);
                if (m_querying_buttons_info.isEmpty()) {
                    // add buttons
                    clearButtons();
                    for (auto info : m_buttons_info) {
                        addButton(info.get()->uri().toLocal8Bit(), true, true);
                    }
                    doLayout();
                }
            } else {
                // 避免上一次的取消操作影响此次的结果，这个通常发生在极短时间内进行连续跳转的情况下
                // 从explorer的交互来看基本不会触发，但是文件对话框的流程可能会触发这种情况
                if (!enumerator->property("isCancelled").toBool()) {
                    if (m_querying_buttons_info.isEmpty()) {
                        // add buttons
                        clearButtons();
                        for (auto info : m_buttons_info) {
                            addButton(info.get()->uri().toLocal8Bit(), true, true);
                        }
                        doLayout();
                    }
                }
            }

            enumerator->deleteLater();
        });
        connect(this, &LocationBar::aboutToSetRootUri, infoJob, [=]{
            infoJob->setProperty("isCancelled", true);
            infoJob->cancel();
        });
        connect(this, &LocationBar::destroyed, infoJob, [=]{
            infoJob->setProperty("isCancelled", true);
            infoJob->cancel();
        });
        connect(infoJob, &FileInfoJob::queryAsyncFinished, this, [=](bool successed){
            if (!successed && !info.get()->uri().startsWith("label://")) {
                qWarning()<<"can not query file:"<<info->uri();
                // 避免上一次的取消操作影响此次的结果，这个通常发生在极短时间内进行连续跳转的情况下
                // 从explorer的交互来看基本不会触发，但是文件对话框的流程可能会触发这种情况
                if (!infoJob->property("isCancelled").toBool()) {
                    m_querying_buttons_info.removeOne(info);
                    m_buttons_info.removeOne(info);
                }
                enumerator->deleteLater();
                infoJob->deleteLater();
                return;
            }

            enumerator->setEnumerateDirectory(info.get()->uri());
            enumerator->enumerateAsync();
            infoJob->deleteLater();
        });
        infoJob->queryAsync();
    }

    return;
}

void LocationBar::updateTrashIcon()
{
    if (m_current_uri.startsWith("trash:///")) {
        auto info  = FileInfo::fromUri(m_current_uri);
        auto infoJob = new FileInfoJob(info);
        connect(infoJob, &Peony::FileInfoJob::queryAsyncFinished, this, [=](){
            infoJob->deleteLater();

            QIcon icon = QIcon::fromTheme(Peony::FileUtils::getFileIconName(m_current_uri), QIcon::fromTheme("folder"));
            auto button = m_buttons.value(m_current_uri);
            if (button)
                button->setIcon(icon);
        });
        infoJob->queryAsync();
    }
}

void LocationBar::clearButtons()
{
    for (auto button : m_buttons) {
        button->hide();
        button->deleteLater();
    }

    m_buttons.clear();
}

void LocationBar::updateButtons()
{
    //clear buttons
    clearButtons();

    if (m_current_uri.startsWith("search://")) {
        //m_indicator->setArrowType(Qt::NoArrow);
        addButton(m_current_uri, false, false);
        return;
    }

    auto uri = m_current_uri;
    m_current_info = FileInfo::fromUri(uri);
    m_buttons_info.clear();
    while (!uri.isEmpty() && uri != "") {
        m_buttons_info.prepend(FileInfo::fromUri(uri));
        uri = FileUtils::getParentUri(uri);
    }

    m_querying_buttons_info = m_buttons_info;

    for (auto info : m_buttons_info) {
        auto infoJob = new FileInfoJob(info);
        //infoJob->setAutoDelete();
        connect(infoJob, &FileInfoJob::queryAsyncFinished, this, [=](){
            // enumerate buttons info directory
            auto enumerator = new FileEnumerator;
            enumerator->setEnumerateDirectory(info.get()->uri());
            enumerator->setEnumerateWithInfoJob();

            connect(enumerator, &FileEnumerator::enumerateFinished, this, [=](bool successed){
                m_querying_buttons_info.removeOne(info);
                if (successed) {
                    auto infos = enumerator->getChildren();
                    m_infos_hash.insert(info.get()->uri(), infos);
                    if (m_querying_buttons_info.isEmpty()) {
                        // add buttons
                        clearButtons();
                        for (auto info : m_buttons_info) {
                            addButton(info.get()->uri(), true, true);
                        }
                        doLayout();
                    }
                } else {
                    if (m_querying_buttons_info.isEmpty()) {
                        // add buttons
                        clearButtons();
                        for (auto info : m_buttons_info) {
                            addButton(info.get()->uri(), true, true);
                        }
                        doLayout();
                    }
                }

                enumerator->deleteLater();
            });

            enumerator->enumerateAsync();
            infoJob->deleteLater();
        });
        infoJob->queryAsync();
    }
}

void LocationBar::addButton(const QString &uri, bool setIcon, bool setMenu)
{
    setIcon = true;
    QToolButton *button = new QToolButton(this);
    button->setFocusPolicy(Qt::FocusPolicy(button->focusPolicy() & ~Qt::TabFocus));
    button->setAutoRaise(true);
    button->setStyle(LocationBarButtonStyle::getStyle());
    button->setProperty("uri", uri);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setPopupMode(QToolButton::MenuButtonPopup);

    auto completeName = FileUtils::getFileDisplayName(uri);
    QString displayName = completeName;
    m_buttons.insert(QUrl(uri).toEncoded(), button);
    if (m_current_uri.startsWith("search://")) {
        QString nameRegexp = SearchVFSUriParser::getSearchUriNameRegexp(m_current_uri);
        QString targetDirectory = SearchVFSUriParser::getSearchUriTargetDirectory(m_current_uri);
        button->setIcon(QIcon::fromTheme("edit-find-symbolic"));
        displayName = tr("Search \"%1\" in \"%2\"").arg(nameRegexp).arg(targetDirectory);
        button->setText(displayName);
        button->setFixedWidth(button->sizeHint().width());
        button->setContextMenuPolicy(Qt::CustomContextMenu);
        return;
    }

    if (m_current_uri.startsWith("label:///")) {
        auto displayName = Peony::FileUtils::getFileDisplayName(m_current_uri);
        button->setIcon(QIcon::fromTheme("edit-find-symbolic"));
        displayName = tr("Search results for all files marked in  \"%1\"  in \"%2\"").arg(displayName).arg(tr("File System"));
        button->setText(displayName);
        button->setContextMenuPolicy(Qt::NoContextMenu);
        button->setPopupMode(QToolButton::InstantPopup);
        return;
    }

    if (setIcon) {
        QIcon icon = QIcon::fromTheme(Peony::FileUtils::getFileIconName(uri), QIcon::fromTheme("folder"));
        button->setIcon(icon);
    }

    //comment to fix button text show incomplete issue, link to bug#72080
    //button->setStyleSheet("QToolButton{padding-left: 13px; padding-right: 13px}");

    //fix bug#84324
    //    QUrl url = uri;
    QUrl url = FileUtils::urlEncode(uri);
    if (!url.fileName().isEmpty())
    {
        button->setText(displayName);
        m_current_uri = uri.left(uri.lastIndexOf("/")+1) + displayName;
    } else {
        if (uri == "file:///") {
//            auto text = FileUtils::getFileDisplayName("computer:///root.link");
//            if (text.isNull()) {
//                text = tr("File System");
//            }
            //fix bug#47597, show as root.link issue
            QString text = tr("File System");
            button->setText(text);
            //comment to fix button text show incomplete issue, link to bug#72080
            //button->setStyleSheet("QToolButton{padding-left: 15px; padding-right: 15px}");
        } else {
            button->setText(displayName);
        }
    }

    //if button text is too long, elide it
    displayName = button->text();
    if (displayName.length() > ELIDE_TEXT_LENGTH)
    {
        int  charWidth = fontMetrics().averageCharWidth();
        displayName = fontMetrics().elidedText(displayName, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
    }
    if (displayName.contains("&")) {
        displayName = Peony::FileUtils::handleSpecialSymbols(displayName);
    }
    button->setText(displayName);

    //comment to fix UI improve bug, link to bug#125255
    //缩略显示的情况下的文件夹，需要提供tips看全文件名，其他情况不显示
    if (completeName != displayName)
    {
       button->setToolTip(completeName);
    }

    connect(button, &QToolButton::clicked, [=]() {
        Q_EMIT this->groupChangedRequest(uri);
    });

    if (setMenu) {
        auto infos = m_infos_hash.value(uri);
        QStringList uris;
        for (auto info : infos) {
            if (info.get()->isDir() && !info.get()->displayName().startsWith("."))
                uris<<info.get()->uri();
        }
        if (uris.isEmpty())
            button->setPopupMode(QToolButton::InstantPopup);
        Peony::PathBarModel m;
        m.setStringList(uris);
        m.sort(0);

        auto suburis = m.stringList();
        if (!suburis.isEmpty()) {
            QMenu *menu = new QMenu;
            connect(button, &QToolButton::destroyed, menu, &QMenu::deleteLater);
            const int WIDTH_EXTEND = 5;
            connect(menu, &QMenu::aboutToShow, this, [=](){
                menu->setMinimumWidth(button->width() + WIDTH_EXTEND);
            });
            QList<QAction *> actions;
            for (auto uri : suburis) {
                QString tmp = uri;
                QIcon icon;
                displayName = Peony::FileUtils::getFileDisplayName(uri);
                if (displayName.length() > ELIDE_TEXT_LENGTH)
                {
                    int  charWidth = fontMetrics().averageCharWidth();
                    displayName = fontMetrics().elidedText(displayName, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
                }
                if(uri.startsWith("filesafe:///")){
                    icon = QIcon::fromTheme(Peony::FileUtils::getFileIconName(uri, false), QIcon::fromTheme("folder"));
                }else{
                    icon = QIcon::fromTheme(Peony::FileUtils::getFileIconName(uri), QIcon::fromTheme("folder"));
                }
                if (displayName.contains("&")) {
                    displayName = Peony::FileUtils::handleSpecialSymbols(displayName);
                }
                QAction *action = new QAction(icon, displayName, this);
                actions<<action;
                connect(action, &QAction::triggered, [=]() {
                    Q_EMIT groupChangedRequest(tmp);
                });
            }
            menu->addActions(actions);

            button->setMenu(menu);
        } else {
            // no subdir directory should not display an indicator arrow.
            button->setPopupMode(QToolButton::InstantPopup);
        }
    }

    button->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(button, &QWidget::customContextMenuRequested, this, [=](const QPoint &pos){
        QMenu menu(button);
        FMWindowIface *windowIface = dynamic_cast<FMWindowIface *>(this->topLevelWidget());
        auto copy = menu.addAction(QIcon::fromTheme("edit-copy-symbolic"), tr("Copy Directory"));

        menu.addAction(QIcon::fromTheme("tab-new-symbolic"), tr("Open In New Tab"), [=](){
            windowIface->addNewTabs(QStringList()<<uri);
        });

        menu.addAction(QIcon::fromTheme("window-new-symbolic"), tr("Open In New Window"), [=](){
            auto newWindow = windowIface->create(uri);
            dynamic_cast<QWidget *>(newWindow)->show();
        });

        if (copy == menu.exec(button->mapToGlobal(pos))) {
            if (uri.startsWith("file://")) {
                QUrl url = uri;
                QApplication::clipboard()->setText(url.path());
            } else {
                QApplication::clipboard()->setText(uri);
            }
        }
    });
}

void LocationBar::mousePressEvent(QMouseEvent *e)
{
    //eat this event.
    //QToolBar::mousePressEvent(e);
    if (e->button() == Qt::LeftButton) {
        Q_EMIT blankClicked();
    }
}

void LocationBar::paintEvent(QPaintEvent *e)
{
    //QToolBar::paintEvent(e);

    QPainter p(this);
    QStyleOptionFocusRect opt;
    opt.initFrom(this);

    QStyleOptionFrame fopt;
    fopt.initFrom(this);
    if (!m_is_classical)
        fopt.state |= QStyle::State_HasFocus;
    //fopt.state.setFlag(QStyle::State_HasFocus);
    fopt.rect.adjust(1, 0, 0, 0);
    opt.rect.adjust(1, 0, 0, 0);
    fopt.palette.setCurrentColorGroup(QPalette::Disabled);
    //auto buttonTextDisabled = fopt.palette.buttonText().color();
    fopt.palette.setCurrentColorGroup(QPalette::Active);
    fopt.palette.setColor(QPalette::Highlight, fopt.palette.button().color());
    fopt.palette.setColor(QPalette::Base, fopt.palette.window().color());

    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &fopt, &p, this);

    style()->drawControl(QStyle::CE_ToolBar, &opt, &p, this);
}

void LocationBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
   if (!m_isAnimation) {
       doLayout();
   }
}

void LocationBar::setAnimationMode(bool isAnimation)
{
    m_isAnimation = isAnimation;
}

void LocationBar::doLayout()
{
    m_indicator->setVisible(false);

    QList<int> sizeHints;

    m_indicator_menu->clear();

    int iconWidth = 0;
    if (!m_buttons.isEmpty()) {
        auto button = m_buttons.first();
        button->setVisible(true);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        iconWidth = button->sizeHint().width();
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        iconWidth = iconWidth - button->sizeHint().width();
        button->setVisible(false);
    }
    int exceptTotalwidth = 0;
    for (auto button : m_buttons) {
        button->setVisible(true);
        // 默认不做自动文字缩略
        button->setProperty("elideText", QVariant());
        button->resize(button->sizeHint().width(), button->height());
        button->setFixedHeight(this->height()); //fixme
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        button->adjustSize();
        int sizeHintWidth = button->sizeHint().width();
        exceptTotalwidth += sizeHintWidth;
        sizeHints<<sizeHintWidth;
        button->setVisible(false);
    }

    int totalWidth = this->width() - iconWidth;
    if (totalWidth < exceptTotalwidth) {
        totalWidth = totalWidth - m_indicator->width() - 2;
    }

    int currentWidth = 0;
    int visibleButtonCount = 0;
    for (int index = sizeHints.count() - 1; index >= 0; index--) {
        int tmp = currentWidth + sizeHints.at(index);
        if (tmp <= totalWidth) {
            visibleButtonCount++;
            currentWidth = tmp;
        } else {
            break;
        }
    }

    int offset = 0;

    bool indicatorVisible = visibleButtonCount < sizeHints.count();
    if (indicatorVisible) {
        m_indicator->setVisible(true);
        offset += m_indicator->width() + 2;
    } else {
        m_indicator->setVisible(false);
    }

    for (int index = sizeHints.count() - visibleButtonCount; index < sizeHints.count(); index++) {
        auto button = m_buttons.values().at(index);
        button->setVisible(true);
        button->move(offset, 0);
        if (index == sizeHints.count() - visibleButtonCount) {
            button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            button->adjustSize();
        }
        offset += button->width();
    }

    if (visibleButtonCount == 0 && !m_buttons.isEmpty()) {
        auto button = m_buttons.values().at(sizeHints.count() - 1);
        button->setVisible(true);
        // 设置自动文字缩略
        button->setProperty("elideText", true);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->move(offset, 0);
        button->resize(totalWidth, button->height());
    }

    int spaceCount = 0;
    QList<QAction *> actions;
    for (auto button : m_buttons) {
        if (button->isVisible()) {
            break;
        }
        auto uri = button->property("uri").toString();
        QString space;
        int i = 0;
        while (i < spaceCount) {
            space.append(' ');
            i++;
        }
        auto action = new QAction(space + button->text(), m_indicator_menu);
        actions.append(action);

        connect(action, &QAction::triggered, this, [=](){
            Q_EMIT groupChangedRequest(uri);
        });
        spaceCount++;
    }
    m_indicator_menu->addActions(actions);
    //add some space for switch to edit
    for (int i = 0; i < 10; i++) {
         m_indicator_menu->addSeparator();
    }
}

void LocationBarButtonStyle::polish(QWidget *widget)
{
    if (widget->objectName() == "explorer_location_bar_indicator") {
        return;
    }
    QProxyStyle::polish(widget);

    widget->setProperty("useIconHighlightEffect", true);
    widget->setProperty("iconHighLightEffectMode", 1);
}

void LocationBarButtonStyle::unpolish(QWidget *widget)
{
    QProxyStyle::unpolish(widget);

    widget->setProperty("useIconHighlightEffect", QVariant());
    widget->setProperty("iconHighLightEffectMode", QVariant());
}

void LocationBarButtonStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    if (control == QStyle::CC_ToolButton) {
        auto toolButton = qstyleoption_cast<const QStyleOptionToolButton *>(option);
        auto opt = *toolButton;
        if (widget && widget->objectName() == "explorer_location_bar_indicator") {
            opt.features.setFlag(QStyleOptionToolButton::HasMenu, false);
            return qApp->style()->drawComplexControl(control, &opt, painter);
        } else {
            opt.rect.adjust(0, 1, 0, -1); //bug#165286 地址栏中“计算机”文字显示不完整，高度减小2，宽度不变
        }
        if (widget) {
            if (widget->property("elideText").toBool()) {
                // 设置文字缩略
                int textWidth = opt.rect.width() - opt.iconSize.width() - (opt.features.testFlag(QStyleOptionToolButton::HasMenu)? 44: 20);
                auto text = opt.text;
                opt.text = opt.fontMetrics.elidedText(text, Qt::ElideRight, textWidth);
            }
        }
        return qApp->style()->drawComplexControl(control, &opt, painter, widget);
    }
    return qApp->style()->drawComplexControl(control, option, painter, widget);
}

void LocationBarButtonStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return qApp->style()->drawControl(element, option, painter, widget);
}

IndicatorToolButton::IndicatorToolButton(QWidget *parent) : QToolButton(parent)
{

}

void IndicatorToolButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    LocationBarButtonStyle::getStyle()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this);
}
