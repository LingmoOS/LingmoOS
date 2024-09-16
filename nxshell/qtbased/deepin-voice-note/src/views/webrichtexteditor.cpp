// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "webrichtexteditor.h"
#include "common/jscontent.h"
#include "common/actionmanager.h"
#include "common/vnoteitem.h"
#include "common/metadataparser.h"
#include "common/vtextspeechandtrmanager.h"
#include "dialog/imageviewerdialog.h"
#include "common/setting.h"
#include "task/exportnoteworker.h"
#include "dialog/vnotemessagedialog.h"

#include "db/vnoteitemoper.h"

#include <DFileDialog>
#include <DGuiApplicationHelper>

#include <QClipboard>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QWebEngineContextMenuData>
#include <QApplication>
#include <QStandardPaths>
#include <QThreadPool>

static const char webPage[] = WEB_PATH "/index.html";

WebRichTextEditor::WebRichTextEditor(QWidget *parent)
    : QWebEngineView(parent)
{
    qDebug() << "Initializing the rich text editor...";
    qDebug() << "Initializing font information...";
    initFontsInformation();
    qDebug() << "Initializing the Rich Text page...";
    initWebView();
    qDebug() << "Initializing the right-click menu...";
    initRightMenu();
    qDebug() << "Initializing the timing updater...";
    initUpdateTimer();
    qDebug() << "The rich text editor has been initialized";
}

WebRichTextEditor::~WebRichTextEditor()
{
    if (imgView != nullptr) {
        delete imgView;
    }
}

void WebRichTextEditor::initWebView()
{
    QWebChannel *channel = new QWebChannel(this);
    JsContent *content = JsContent::instance();
    channel->registerObject("webobj", content);
    page()->setWebChannel(channel);
    QFileInfo info(webPage);
    //printf("%s \n", webPage);
    load(QUrl::fromLocalFile(info.absoluteFilePath()));
    page()->setBackgroundColor(DGuiApplicationHelper::instance()->applicationPalette().base().color());

    connect(content, &JsContent::textChange, this, &WebRichTextEditor::onTextChange);
    connect(content, &JsContent::setDataFinsh, this, &WebRichTextEditor::onSetDataFinsh);
    connect(content, &JsContent::popupMenu, this, &WebRichTextEditor::saveMenuParam);
    connect(content, &JsContent::textPaste, this, &WebRichTextEditor::onPaste);
    connect(content, &JsContent::loadFinsh, this, &WebRichTextEditor::onThemeChanged);
    connect(content, &JsContent::loadFinsh, this, &WebRichTextEditor::onLoadFinsh);
    connect(content, &JsContent::viewPictrue, this, &WebRichTextEditor::viewPicture);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &WebRichTextEditor::onThemeChanged);

    connect(content, &JsContent::getfontinfo, this, &WebRichTextEditor::onSetFontListInfo);

    if (nullptr != focusProxy()) {
        focusProxy()->installEventFilter(this);
    }
}

void WebRichTextEditor::initFontsInformation()
{
    // 初始化字体服务Dus接口
    m_appearanceDBusInterface = new QDBusInterface(DEEPIN_DAEMON_APPEARANCE_SERVICE,
                                                   DEEPIN_DAEMON_APPEARANCE_PATH,
                                                   DEEPIN_DAEMON_APPEARANCE_INTERFACE,
                                                   QDBusConnection::sessionBus());
    if (m_appearanceDBusInterface->isValid()) {
        qInfo() << "字体服务初始化成功！字体服务： " << DEEPIN_DAEMON_APPEARANCE_SERVICE << " 地址：" << DEEPIN_DAEMON_APPEARANCE_PATH << " 接口：" << DEEPIN_DAEMON_APPEARANCE_INTERFACE;
        //获取默认字体
        QString defaultfont = m_appearanceDBusInterface->property("StandardFont").value<QString>();
        //qInfo() << "默认字体: " << defaultfont;
        //获取字体列表
        QDBusPendingReply<QString> font = m_appearanceDBusInterface->call("List", "standardfont");
        //qInfo() << "字体列表 font: " << font;

        QJsonArray array = QJsonDocument::fromJson(font.value().toLocal8Bit().data()).array();
        QStringList list;
        for (int i = 0; i != array.size(); i++) {
            list << array.at(i).toString();
        }
        //qInfo() << "字体列表: " << list;

        QList<QVariant> arg;
        arg << "standardfont"
            << list;
        //获取带翻译的字体列表
        QDBusPendingReply<QString> font1 = m_appearanceDBusInterface->callWithArgumentList(QDBus::AutoDetect,"Show", arg);

        QJsonArray arrayValue = QJsonDocument::fromJson(font1.value().toLocal8Bit().data()).array();
        //qInfo() << "带翻译的字体列表: " << arrayValue;
        //列表格式转换
        for (int i = 0; i != arrayValue.size(); i++) {
            QJsonObject object = arrayValue.at(i).toObject();
            object.insert("type", QJsonValue("standardfont"));
            m_fontList << object["Name"].toString();
            if (defaultfont == object["Id"].toString()) {
                //根据id 获取带翻译的默认字体
                m_FontDefault = object["Name"].toString();
            }
        }
        //qInfo() << "m_fontList: " << m_fontList;
        //qInfo() << "m_FontDefault: " << m_FontDefault;

        qInfo() << "带翻译的默认字体: " << m_FontDefault;
        // sort for display name
        std::sort(m_fontList.begin(), m_fontList.end(), [ = ](const QString & obj1, const QString & obj2) {
            QCollator qc;
            return qc.compare(obj1, obj2) < 0;
        });
    } else {
        qWarning() << "初始化失败！字体服务 (" << DEEPIN_DAEMON_APPEARANCE_SERVICE << ") 不存在";
    }
}

void WebRichTextEditor::onSetFontListInfo()
{
    Q_EMIT JsContent::instance()->callJsSetFontList(m_fontList, m_FontDefault);
}

void WebRichTextEditor::initRightMenu()
{
    m_pictureRightMenu = ActionManager::Instance()->pictureContextMenu();
    m_voiceRightMenu = ActionManager::Instance()->voiceContextMenu();
    m_txtRightMenu = ActionManager::Instance()->txtContextMenu();
    connect(m_pictureRightMenu, &DMenu::triggered,
            this, &WebRichTextEditor::onMenuActionClicked);
    connect(m_voiceRightMenu, &DMenu::triggered,
            this, &WebRichTextEditor::onMenuActionClicked);
    connect(m_txtRightMenu, &DMenu::triggered,
            this, &WebRichTextEditor::onMenuActionClicked);
    //文本右键菜单隐藏隐藏编辑工具栏
    connect(m_txtRightMenu, &DMenu::aboutToHide, this, &WebRichTextEditor::onHideEditToolbar);
}

void WebRichTextEditor::initUpdateTimer()
{
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(1000);
    connect(m_updateTimer, &QTimer::timeout, this, &WebRichTextEditor::updateNote);
}

void WebRichTextEditor::initData(VNoteItem *data, const QString &reg, bool focus)
{
    //重置鼠标点击位置
    m_mouseClickPos = QPoint(-1, -1);
    m_setFocus = focus;
    //富文本设置异步操作，解决笔记列表不实时刷新
    QTimer::singleShot(0, this, [ = ] {
        setData(data, reg);
    });
}

void WebRichTextEditor::insertVoiceItem(const QString &voicePath, qint64 voiceSize)
{
    VNVoiceBlock data;
    data.ptrVoice->voiceSize = voiceSize;
    data.ptrVoice->voicePath = voicePath;
    data.ptrVoice->createTime = QDateTime::currentDateTime();
    data.ptrVoice->voiceTitle = data.ptrVoice->createTime.toString("yyyyMMdd hh.mm.ss");

    MetaDataParser parse;
    QVariant value;
    parse.makeMetaData(&data, value);
    this->setFocus();
    //关闭应用时，需要同步插入语音并进行后台更新
    if (OpsStateInterface::instance()->isAppQuit()) {
        JsContent::instance()->callJsSynchronous(page(), QString("insertVoiceItem('%1')").arg(value.toString()));
        m_textChange = true;
        update();
        return;
    }
    emit JsContent::instance()->callJsInsertVoice(value.toString());
}

void WebRichTextEditor::updateNote()
{
    if (m_noteData) {
        if (m_textChange) {
            QVariant result = JsContent::instance()->callJsSynchronous(page(), QString("getHtml()"));
            if (result.isValid()) {
                m_noteData->htmlCode = result.toString();
                VNoteItemOper noteOps(m_noteData);
                if (!noteOps.updateNote()) {
                    qInfo() << "Save note error";
                }
            }
            m_textChange = false;
        }
    }
}

void WebRichTextEditor::searchText(const QString &searchKey)
{
    findText(searchKey, QWebEnginePage::FindFlags(), [ = ](const bool & result) {
        if (result == false) {
            emit currentSearchEmpty();
        }
    });
}

void WebRichTextEditor::unboundCurrentNoteData()
{
    //停止更新定时器
    m_updateTimer->stop();
    //手动更新
    updateNote();
    //绑定数据设置为空
    m_noteData = nullptr;
}

void WebRichTextEditor::onTextChange()
{
    if (!m_textChange) {
        m_textChange = true;
        //更新修改时间
        if (nullptr != m_noteData) {
            m_noteData->modifyTime = QDateTime::currentDateTime();
            emit contentChanged();
        }
    }
}

void WebRichTextEditor::saveMenuParam(int type, const QVariant &json)
{
    m_menuType = static_cast<Menu>(type);
    m_menuJson = json;
}

void WebRichTextEditor::onSetDataFinsh()
{
    //清除选中
    page()->triggerAction(QWebEnginePage::Unselect);
    //数据加载完成,需要设置焦点时需要先清除焦点再重新设置，解决编辑器无光标问题
    if (m_setFocus) {
        clearFocus();
        setFocus();
    }
    //只有编辑区内容加载完成才能搜索
    if (!m_searchKey.isEmpty()) {
        findText(m_searchKey);
    }
}

void WebRichTextEditor::showTxtMenu(const QPoint &pos)
{
    ActionManager::Instance()->resetCtxMenu(ActionManager::MenuType::TxtCtxMenu, false); //重置菜单选项
    bool isAlSrvAvailabel = OpsStateInterface::instance()->isAiSrvExist(); //获取语音服务是否可用
    bool TTSisWorking = VTextSpeechAndTrManager::isTextToSpeechInWorking(); //获取语音服务是否正在朗读
    //设置语音服务选项状态
    if (isAlSrvAvailabel) {
        if (TTSisWorking) {
            ActionManager::Instance()->visibleAction(ActionManager::TxtStopreading, true);
            ActionManager::Instance()->visibleAction(ActionManager::TxtSpeech, false);
            ActionManager::Instance()->enableAction(ActionManager::TxtStopreading, true);
        } else {
            ActionManager::Instance()->visibleAction(ActionManager::TxtStopreading, false);
            ActionManager::Instance()->visibleAction(ActionManager::TxtSpeech, true);
        }
    }
    //获取web端编辑标志
    QWebEngineContextMenuData::EditFlags flags = page()->contextMenuData().editFlags();
    //设置普通菜单项状态
    //可全选
    if (flags.testFlag(QWebEngineContextMenuData::CanSelectAll)) {
        ActionManager::Instance()->enableAction(ActionManager::TxtSelectAll, true);
    }
    //可复制
    if (flags.testFlag(QWebEngineContextMenuData::CanCopy)) {
        ActionManager::Instance()->enableAction(ActionManager::TxtCopy, true);
        if (isAlSrvAvailabel) {
            if (VTextSpeechAndTrManager::getTransEnable()) {
                ActionManager::Instance()->enableAction(ActionManager::TxtTranslate, true);
            }
            if (!TTSisWorking && VTextSpeechAndTrManager::getTextToSpeechEnable()) {
                ActionManager::Instance()->enableAction(ActionManager::TxtSpeech, true);
            }
        }
    }
    //可剪切
    if (flags.testFlag(QWebEngineContextMenuData::CanCut)) {
        ActionManager::Instance()->enableAction(ActionManager::TxtCut, true);
    }
    //可删除
    if (flags.testFlag(QWebEngineContextMenuData::CanDelete)) {
        ActionManager::Instance()->enableAction(ActionManager::TxtDelete, true);
    }
    //可粘贴
    if (flags.testFlag(QWebEngineContextMenuData::CanPaste)) {
        ActionManager::Instance()->enableAction(ActionManager::TxtPaste, true);
        if (!TTSisWorking && VTextSpeechAndTrManager::getSpeechToTextEnable()) {
            ActionManager::Instance()->enableAction(ActionManager::TxtDictation, true);
        }
    }
    m_txtRightMenu->popup(pos);
}

/**
 * @brief 图片菜单显示
 */
void WebRichTextEditor::showPictureMenu(const QPoint &pos)
{
    m_pictureRightMenu->popup(pos);
}

/**
 * @brief 语音菜单显示
 */
void WebRichTextEditor::showVoiceMenu(const QPoint &pos)
{
    m_voiceBlock.reset(new VNVoiceBlock);
    MetaDataParser dataParser;
    //解析json数据
    if (!dataParser.parse(m_menuJson, m_voiceBlock.get())) {
        return;
    }

    //语音文件不存在使用弹出提示
    if (!QFile(m_voiceBlock->voicePath).exists()) {
        //异步操作，防止阻塞前端事件
        QTimer::singleShot(0, this, [this] {
            VNoteMessageDialog audioOutLimit(VNoteMessageDialog::VoicePathNoAvail);
            audioOutLimit.exec();
            //删除语音文本
            deleteSelectText();
        });
        return;
    }

    //如果当前有语音处于转换状态就将语音转文字选项置灰
    ActionManager::Instance()->enableAction(ActionManager::VoiceToText, !OpsStateInterface::instance()->isVoice2Text());
    m_voiceRightMenu->popup(pos);
}

void WebRichTextEditor::onMenuActionClicked(QAction *action)
{
    ActionManager::ActionKind kind = ActionManager::Instance()->getActionKind(action);
    switch (kind) {
    case ActionManager::VoiceAsSave:
        //另存语音
        saveMP3As();
        break;
    case ActionManager::VoiceToText:
        //通知主窗口进行转写服务
        emit asrStart(m_voiceBlock.get());
        break;
    case ActionManager::VoiceDelete:
    case ActionManager::PictureDelete:
    case ActionManager::TxtDelete:
        deleteSelectText();
        break;
    case ActionManager::VoiceSelectAll:
    case ActionManager::PictureSelectAll:
    case ActionManager::TxtSelectAll: {
        //模拟全选快捷键ctrl+A
        QKeyEvent event(QEvent::KeyPress, Qt::Key_A, Qt::ControlModifier);
        QApplication::sendEvent(focusProxy(), &event);
        break;
    }
    case ActionManager::VoiceCopy:
    case ActionManager::PictureCopy:
    case ActionManager::TxtCopy:
        //直接调用web端的复制事件
        page()->triggerAction(QWebEnginePage::Copy);
        break;
    case ActionManager::VoiceCut:
    case ActionManager::PictureCut:
    case ActionManager::TxtCut:
        //直接调用web端的剪切事件
        page()->triggerAction(QWebEnginePage::Cut);
        break;
    case ActionManager::VoicePaste:
    case ActionManager::PicturePaste:
    case ActionManager::TxtPaste:
        //粘贴事件，从剪贴板获取数据
        onPaste(isVoicePaste());
        break;
    case ActionManager::PictureView:
        //查看图片
        viewPicture(m_menuJson.toString());
        break;
    case ActionManager::PictureSaveAs:
        //另存图片
        savePictureAs();
        break;
    case ActionManager::TxtSpeech:
        VTextSpeechAndTrManager::onTextToSpeech();
        break;
    case ActionManager::TxtStopreading:
        VTextSpeechAndTrManager::onStopTextToSpeech();
        break;
    case ActionManager::TxtDictation:
        VTextSpeechAndTrManager::onSpeechToText();
        break;
    case ActionManager::TxtTranslate:
        VTextSpeechAndTrManager::onTextTranslate();
        break;
    default:
        break;
    }
}

/**
 * @brief WebRichTextEditor::savePictureAs
 * 另存图片
 */
void WebRichTextEditor::savePictureAs()
{
    QString originalPath = m_menuJson.toString(); //获取原图片路径
    saveAsFile(originalPath, QStandardPaths::writableLocation(QStandardPaths::PicturesLocation), "image");
}

/**
 * @brief WebRichTextEditor::saveMP3As
 * 另存语音
 */
void WebRichTextEditor::saveMP3As()
{
    if (m_voiceBlock.isNull()) {
        return;
    }

    //获取历史使用的路径
    QString historyDir = setting::instance()->getOption(VNOTE_EXPORT_VOICE_PATH_KEY).toString();
    if (historyDir.isEmpty()) {
        historyDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }

    QString newPath = saveAsFile(m_voiceBlock->voicePath, historyDir, m_voiceBlock->voiceTitle);
    if (!newPath.isEmpty()) {
        setting::instance()->setOption(VNOTE_EXPORT_VOICE_PATH_KEY, QFileInfo(newPath).dir().path());
    }
}

/**
 * @brief WebRichTextEditor::saveAsFile
 * 另存文件
 * @param originalPath  原文件路径
 * @param dirPath 默认保存文件夹路径
 * @param defalutName 默认保存文件名
 * @return  保存的文件路径
 */
QString WebRichTextEditor::saveAsFile(const QString &originalPath, QString dirPath, const QString &defalutName)
{
    //存储文件夹默认为桌面
    if (dirPath.isEmpty()) {
        dirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }

    QFileInfo fileInfo(originalPath);
    QString filter = "*." + fileInfo.suffix();
    QString baseName = defalutName.isEmpty() ? fileInfo.baseName() : defalutName;
    QString dir = QString("%1/%2").arg(dirPath).arg(baseName + "." + fileInfo.suffix());
    //获取需要保存的文件位置，默认路径为用户图片文件夹，默认文件名为原文件名
    QString newPath = DFileDialog::getSaveFileName(this, "", dir, filter);
    if (newPath.isEmpty()) {
        return "";
    }
    //添加文件后缀
    if (!newPath.endsWith(fileInfo.suffix())) {
        newPath += ("." + fileInfo.suffix());
    }

    QFileInfo info(newPath);

    if (!QFileInfo(info.dir().path()).isWritable()) {
        //文件夹没有写权限
        VNoteMessageDialog audioOutLimit(VNoteMessageDialog::NoPermission);
        audioOutLimit.exec();
        return "";
    }
    if (info.exists()) {
        //文件已存在，删除原文件
        if (!info.isWritable()) {
            //文件没有写权限
            VNoteMessageDialog audioOutLimit(VNoteMessageDialog::NoPermission);
            audioOutLimit.exec();
            return "";
        }
        QFile::remove(newPath);
    }

    //复制文件
    if (!QFile::copy(originalPath, newPath)) {
        VNoteMessageDialog audioOutLimit(VNoteMessageDialog::SaveFailed);
        audioOutLimit.exec();
        qCritical() << "copy failed:" << originalPath << ";" << newPath;
        return "";
    }
    return newPath;
}

/**
 * @brief WebRichTextEditor::viewPicture
 * 查看图片
 * @param filePath 图片路径
 */
void WebRichTextEditor::viewPicture(const QString &filePath)
{
    if (imgView == nullptr) {
        imgView = new ImageViewerDialog(this);
    }
    //加载图片并显示
    imgView->open(filePath);
}

void WebRichTextEditor::onPaste(bool isVoicePaste)
{
    //语音插件复制不经过系统剪切板,粘贴无法通过剪切板内容判断，直接调用前端粘贴事件
    if (isVoicePaste) {
        return page()->triggerAction(QWebEnginePage::Paste);
    }
    //获取剪贴板信息
    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    //识别语音类数据
    auto html = mimeData->html();
    if (html.contains(QRegularExpression("<div class=\"[^\"]*voiceBox"))) {
        JsContent::instance()->callJsPasteHtml(html);
        return;
    }

    //存在文件url
    if (mimeData->hasUrls()) {
        QStringList paths;
        for (auto url : mimeData->urls()) {
            paths.push_back(url.path());
        }
        JsContent::instance()->insertImages(paths);
    } else if (mimeData->hasImage()) {
        JsContent::instance()->insertImages(qvariant_cast<QImage>(mimeData->imageData()));
    } else {
        //无图片文件，直接调用web端的粘贴事件
        page()->triggerAction(QWebEnginePage::Paste);
    }
}

void WebRichTextEditor::contextMenuEvent(QContextMenuEvent *e)
{
    QPoint globalPos = e->globalPos();
    QPoint pos = e->pos();
    //菜单y坐标为负数时，右键菜单显示在编辑区中间
    if (pos.y() < 0 || pos.y() > height()) {
        pos = rect().center();
        globalPos = mapToGlobal(pos);
    }
    switch (m_menuType) {
    case PictureMenu:
        //图片菜单
        showPictureMenu(globalPos);
        break;
    case VoiceMenu:
        //语音菜单
        showVoiceMenu(globalPos);
        break;
    case TxtMenu:
        //文字菜单
        showTxtMenu(globalPos);
        //显示编辑工具栏
        onShowEditToolbar(mapFromGlobal(m_txtRightMenu->pos()));
        break;
    default:
        break;
    }
}

/**
 * @brief WebRichTextEditor::dragEnterEvent
 * 拖拽移入， 覆盖父类方法
 * @param event
 */
void WebRichTextEditor::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

/**
 * @brief WebRichTextEditor::dragLeaveEvent
 * 拖拽移出， 覆盖父类方法
 * @param event
 */
void WebRichTextEditor::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
}

/**
 * @brief WebRichTextEditor::dragMoveEvent
 * 拖拽移动， 覆盖父类方法
 * @param e
 */
void WebRichTextEditor::dragMoveEvent(QDragMoveEvent *e)
{
    Q_UNUSED(e);
}

void WebRichTextEditor::dropEvent(QDropEvent *event)
{
    // 获取文件路径
    if (event->mimeData()->hasUrls()) {
        QStringList paths;
        //获取文件路径
        for (auto url : event->mimeData()->urls()) {
            paths.push_back(url.path());
        }
        JsContent::instance()->insertImages(paths); //向web端发送路径
    }
}

void WebRichTextEditor::deleteSelectText()
{
    //向焦点代理窗口发送delete按键，删除选中内容
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    QApplication::sendEvent(focusProxy(), &event);
}

void WebRichTextEditor::onThemeChanged()
{
    DGuiApplicationHelper *dAppHelper = DGuiApplicationHelper::instance();
    DPalette dp = dAppHelper->applicationPalette();
    //获取系统高亮色
    QString activeHightColor = dp.color(DPalette::Active, DPalette::Highlight).name();
    QString disableHightColor = dp.color(DPalette::Disabled, DPalette::Highlight).name();
    page()->setBackgroundColor(dp.base().color());
    //获取系统主题类型
    DGuiApplicationHelper::ColorType theme = dAppHelper->themeType();
    emit JsContent::instance()->callJsSetTheme(theme, activeHightColor, disableHightColor, dp.base().color().name());
}

void WebRichTextEditor::onShowEditToolbar(const QPoint &pos)
{
    QPoint menuPoint = pos;
    menuPoint.setX(menuPoint.x() - 9);
    int width = this->width() - menuPoint.x() - 395;
    if (width < 0) {
        menuPoint.setX(menuPoint.x() + width);
    }
    if (menuPoint.y() < 100) { //工具栏显示在右键菜单下方
        menuPoint.setY(menuPoint.y() + 15 + m_txtRightMenu->height());
    } else {
        //窗无口正常显示右键菜单时，菜单左上角坐标=鼠标位置，工具栏显示在右键菜单上方
        menuPoint.setY(menuPoint.y() - 45);
    }
    //记录编辑工具栏的坐标位置
    m_editToolbarRect = QRect(menuPoint, QPoint(menuPoint.x() + 290 + 85, menuPoint.y() + 35));
    emit JsContent::instance()->calllJsShowEditToolbar(menuPoint.x(), menuPoint.y());
}

void WebRichTextEditor::onHideEditToolbar()
{
    //当前鼠标位置不在编辑工具栏上，则隐藏编辑工具栏
    if (!m_editToolbarRect.contains(mapFromGlobal(QCursor::pos()))) {
        emit JsContent::instance()->callJsHideEditToolbar();
    }
}

/**
 * @brief onLoadFinsh
 * 与web端通信建立完成
 */
void WebRichTextEditor::onLoadFinsh()
{
    //再次设置笔记内容
    if (m_noteData && !m_loadFinshSign) {
        if (m_noteData->htmlCode.isEmpty()) {
            emit JsContent::instance()->callJsInitData(m_noteData->metaDataRef().toString());
        } else {
            emit JsContent::instance()->callJsSetHtml(m_noteData->htmlCode);
        }
    }
    m_loadFinshSign = true;
}

void WebRichTextEditor::setData(VNoteItem *data, const QString &reg)
{
    //有焦点先隐藏编辑工具栏
    if (hasFocus()) {
        emit JsContent::instance()->callJsHideEditToolbar();
    }

    if (nullptr == data) {
        this->setVisible(false);
        //无数据时先保存之前数据
        updateNote();
        //解绑当前数据
        unboundCurrentNoteData();
        return;
    }
    this->setVisible(true);
    //清除选中需重新加载，解决匹配字符显示灰色问题
    bool reSet = !m_searchKey.isEmpty() && reg.isEmpty();
    m_searchKey = reg;
    if (m_noteData != data || reSet) { //笔记切换或清除搜索结果时设置笔记内容
        m_updateTimer->stop();
        updateNote();
        m_noteData = data;
        if (m_loadFinshSign) {
            if (data->htmlCode.isEmpty()) {
                emit JsContent::instance()->callJsInitData(data->metaDataRef().toString());
            } else {
                emit JsContent::instance()->callJsSetHtml(data->htmlCode);
            }
        }
        m_updateTimer->start();
    } else { //笔记相同时执行搜索
        findText(reg);
    }
}

bool WebRichTextEditor::isVoicePaste()
{
    //调用web前端接口
    return JsContent::instance()->callJsSynchronous(page(), "returnCopyFlag()").toBool();
}

void WebRichTextEditor::shortcutPopupMenu()
{
    //同步获取菜单类型与参数
    QMap<QString, QVariant> param = JsContent::instance()->callJsSynchronous(page(), "isRangeVoice()").toMap();
    if (2 == param.size()) {
        m_menuType = static_cast<Menu>(param["flag"].toInt());
        m_menuJson = param["info"];
        //根据光标位置显示菜单
        if (m_mouseClickPos.x() < 0 || TxtMenu == m_menuType) {
            //模拟发送菜单键事件
            QKeyEvent event(QEvent::KeyPress, Qt::Key_Menu, Qt::NoModifier);
            QApplication::sendEvent(focusProxy(), &event);
            return;
        }
        //语音与图片菜单，根据鼠标选中图片或语音时的位置显示菜单
        if (VoiceMenu == m_menuType) {
            showVoiceMenu(m_mouseClickPos);
        } else if (PictureMenu == m_menuType) {
            showPictureMenu(m_mouseClickPos);
        }
    }
}

void WebRichTextEditor::clearJSContent()
{
    if (this->isVisible()) {
        connect(JsContent::instance(), &JsContent::setDataFinsh, this, &WebRichTextEditor::onSetDataFinsh);
        emit JsContent::instance()->callJsSetHtml("");

        // 开启100ms事件循环，保证js页面内容被刷新
        QEventLoop eveLoop;
        QTimer::singleShot(100, &eveLoop, SLOT(quit()));
        eveLoop.exec();
        disconnect(JsContent::instance(), &JsContent::setDataFinsh, this, &WebRichTextEditor::onSetDataFinsh);
    }
}

bool WebRichTextEditor::eventFilter(QObject *o, QEvent *e)
{
    if (o == focusProxy()) {
        if (e->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *event = dynamic_cast<QMouseEvent *>(e);
            if (event) {
                m_mouseClickPos = event->globalPos();
            }
        }
    }
    return QWebEngineView::eventFilter(o, e);
}
