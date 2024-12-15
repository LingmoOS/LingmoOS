// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JSCONTENT_H
#define JSCONTENT_H

#include <QObject>
#include <QClipboard>

#include <QtWebEngineWidgets/qwebenginepage.h>

class JsContent : public QObject
{
    Q_OBJECT
public:
    static JsContent *instance();

    /**
     * @brief The AsrFlag enum
     * 语音转写文字标志
     */
    enum AsrFlag {
        Start = 0, //开始转写
        End //转写结束
    };
    Q_ENUM(AsrFlag)

    QVariant callJsSynchronous(QWebEnginePage *page, const QString &funtion);
    /**
     * @brief 插入图片
     * @param filePaths 图片路径
     * @return
     */
    bool insertImages(QStringList filePaths);
    /**
     * @brief 插入图片
     */
    bool insertImages(const QImage &image);

signals:
    void callJsInitData(const QString &jsonData); //调用web前端，设置json格式数据
    void callJsSetHtml(const QString &html); //调用web前端，设置html格式数据
    void callJsInsertVoice(const QString &jsonData); //调用web前端，插入语音
    /**
     * @brief 调用web前端，设置语音转文字结果
     * @param text 文本
     * @param flag 转写标志 参数说明依据AsrFlag枚举
     */
    void callJsSetVoiceText(const QString &text, int asrflag);
    void callJsInsertImages(const QStringList &images); //调用web前端，插入图片
    void callJsSetPlayStatus(int status); //调用web前端, 设置播放状态，0播放中，1暂停中 2.结束播放
    /**
     * @brief 调用web前端，设置系统主题
     * @param theme : 主题类型，0 未知，1浅色主题，2深色主题，参考DGuiApplicationHelper::ColorType
     * @param activeHightColor : 系统活动高亮色
     * @param disableHightColor : 系统活动高亮色置灰颜色
     * @param backgroundColer : 系统控件背景色
     */
    void callJsSetTheme(int theme, const QString &activeHightColor, const QString &disableHightColor, const QString &backgroundColer);
    void callJsClipboardDataChanged(); //剪切板数据发生改变信号

    void calllJsShowEditToolbar(int x, int y); //显示编辑工具栏
    void callJsHideEditToolbar(); //隐藏编辑工具栏
    void callJsSetVoicePlayBtnEnable(bool enable); //设置播放按钮是否可用

    void callJsPasteHtml(const QString &html); //处理粘贴语音类数据

    void textPaste(bool isVoicePaste); //粘贴信号
    void textChange();
    void loadFinsh();
    void popupMenu(int type, const QVariant &json);
    void playVoice(const QVariant &json, bool bIsSame);
    void viewPictrue(const QString &imagePath);
    void createNote();
    /**
     * @brief 编辑区内容加载完成信号
     */
    void setDataFinsh();

    /**
     * @author liuxinping  ut002764
     * @brief  字体列表和默认字体信号发送web
     * @param  list  字体列表
     * @param  font  默认字体
     */
    void callJsSetFontList(const QStringList &list, const QString &font);
    void getfontinfo();  //获取字体列表信息信号

protected:
    JsContent();

public slots:
    /**
     * @brief web前端编辑区内容设置完成后，通知后端
     */
    void jsCallSetDataFinsh();
    void jsCallTxtChange(); //web前端调用后端，通知数据变化
    void jsCallChannleFinish(); //web前端调用后端，通信建立完成
    void jsCallSummernoteInitFinish();  //summernote 加载完成
    void jsCallPopupMenu(int type, const QVariant &json); //web前端调用后端，弹出右键菜单
    void jsCallPlayVoice(const QVariant &json, bool bIsSame); //web前端调用后端，播放语音
    void jsCallPaste(bool isVoicePaste = false); //web前端调用后端，编辑区粘贴功能
    void jsCallViewPicture(const QString &imagePath); //web前端调用后端，进行图片预览
    void jsCallCreateNote(); //web前端调用后端，新建笔记
    void jsCallSetClipData(const QString &text, const QString &html); //web前端调用后端，设置剪切板内容
    QString jsCallGetTranslation(); //web前端调用后端，获取翻译
    void onClipChange(QClipboard::Mode mode);

private:
    const QMimeData *m_clipData {nullptr};
};

#endif // JSCONTENT_H
