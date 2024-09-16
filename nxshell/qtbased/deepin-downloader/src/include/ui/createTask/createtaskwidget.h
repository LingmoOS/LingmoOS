// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NEWTASKWIDGET_H
#define NEWTASKWIDGET_H

#include "settings.h"
#include <dfilechooseredit.h>
#include <DMainWindow>
#include <DPushButton>
#include <DDialog>
#include <DLabel>
#include <DTextEdit>
#include <DSuggestButton>
#include <DFileDialog>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCloseEvent>

class LinkInfo;
class BtInfoDialog;
class MessageBox;
class BtInfoDelegate;
class BtInfoTableView;
class AnalysisUrl;
class TaskDelegate;
class QStandardItemModel;
class TaskModel;

DWIDGET_USE_NAMESPACE

class CreateTaskWidget : public DDialog
{
    Q_OBJECT
public:
    explicit CreateTaskWidget(QWidget *parent = 0);
    ~CreateTaskWidget();

    /**
     * @brief 设置url
     * @param
    */
    void setUrl(QString url);

    /**
     * @brief 网络错误弹窗
    */
    void showNetErrorMsg();

    /**
     * @brief updateSelectedInfo 更新文件列表选中后的信息（Delegate内部调用）
     */
    void updateSelectedInfo();

    /**
     * @brief 根据索引，设置url名字
    */
    void setUrlName(int index, QString name);

    void setTableReset();

private:
    /**
     * @brief 初始化ui
     */
    void initUi();

    /**
     * @brief 是否为磁力链接
     * @param url 路径
     * @return  true 为磁力链接  false 为不是磁力链接
    */
    bool isMagnet(QString url);

    /**
     * @brief 是否为http链接
     * @param url 路径
     * @return  true 为http链接  false 为不是http链接
    */
    bool isHttp(QString url);

    /**
     * @brief 是否为isFtp链接
     * @param url 路径
     * @return  true 为Ftp链接  false 为不是Ftp链接
    */
    bool isFtp(QString url);

    /**
     * @brief 获取url名字和类型
     * @param url 路径
     * @param name url名字
     * @param type url类型
    */
    void getUrlToName(QString url, QString &name, QString &type);

    /**
     * @brief 设置表格中数据
     * @param index 索引
     * @param name url名字
     * @param type url类型
     * @param size url大小，QString类型
     * @param url url内容
     * @param length url长度 long类型
     * @param trueUrl 解析后url真实链接
    */
    void setData(int index, QString name, QString type, QString size, QString url, long length, QString trueUrl);

    /**
     * @brief 隐藏表格
     */
    void hideTableWidget();

    /**
     * @brief 显示表格
     */
    void showTableWidget();

    /**
     * @brief 判断扩展名是否是常见视频格式
     * @param ext 传入后缀
     * @return  真为是，假为否
    */
    bool isVideo(QString ext);
    /**
     * @brief 判断扩展名是否是常见音频格式
     * @param ext 传入后缀
     * @return  真为是，假为否
    */
    bool isAudio(QString ext);
    /**
     * @brief 判断扩展名是否是常见图片格式
     * @param ext 传入后缀
     * @return  真为是，假为否
    */
    bool isPicture(QString ext);
    /**
     * @brief 判断扩展名是否是常见压缩包格式
     * @param ext 传入后缀
     * @return  真为是，假为否
    */
    bool isZip(QString ext);
    /**
     * @brief 判断扩展名是否是常见文档格式
     * @param ext 传入后缀
     * @return  真为是，假为否
    */
    bool isDoc(QString ext);

    void registCallBack(void *func, QString str, int state);

    /**
     * @brief 获取断网提示
    */
    QString getNetErrTip();

    /**
     * @brief 磁盘大小转换
     * @param 传入字符串
     * @return 传出double大小
    */
    double formatSpeed(QString str);

    /**
     * @brief 获取当前选择文件大小
     * @param 传入字符串
     * @return 传出double大小
    */
    double getSelectSize();

    /**
     * @brief 判断是否存在该类型的后缀名
     * @param 后缀名
     * @return 是否存在
    **/
    bool isExistType(QString type);

private slots:
    /**
     * @brief 打开选择文件窗口按钮
     */
    void onFileDialogOpen();

    /**
     * @brief 打开metalink选择文件窗口按钮
     */
    void onMLFileDialogOpen();

    /**
     * @brief 关闭窗口按钮
     */
    void onCancelBtnClicked();
    /**
     * @brief 确定下载按钮
     */
    void onSureBtnClicked();
    /**
     * @brief url文本框变化
     */
    void onTextChanged();

    /**
     * @brief 选择下载路径
     */
    void onFilechoosed(const QString &);

    /**
     * @brief 更新解析列表
     */
    void updataTabel(LinkInfo *);

    /**
     * @brief 全选按钮
     */
    void onAllCheck();

    /**
     * @brief 视频按钮
     */
    void onVideoCheck();
    /**
     * @brief 音频按钮
     */
    void onAudioCheck();
    /**
     * @brief 图片按钮
     */
    void onPictureCheck();
    /**
     * @brief 压缩包按钮
     */
    void onZipCheck();
    /**
     * @brief 文档按钮
     */
    void onDocCheck();
    /**
     * @brief 其他按钮
     */
    void onOtherCheck();

protected:
    /**
     * @brief 拖拽处理函数
     * @param event 拖拽事件
    */
    void dragEnterEvent(QDragEnterEvent *event);
    /**
     * @brief 拖拽释放处理函数
     * @param event 拖拽事件
    */
    void dropEvent(QDropEvent *event);

    /**
     * @brief 关闭事件
     * @param event 关闭事件
    */
    void closeEvent(QCloseEvent *event);

    void keyPressEvent(QKeyEvent *event);

signals:
    /**
     * @brief 新建http下载任务
     * @param url 信息
     * @param save_path 保存路径
     * @param path_name 文件名字
    */
    void downloadWidgetCreate(QVector<LinkInfo> &urlList, QString savePath);
    /**
     * @brief 新建bt下载任务
     * @param btPath bt下载地址
     * @param opt 选项
     * @param infoName bt文件名字
     * @param infoHash bt文件hash值
    */
    void downLoadTorrentCreate(QString btPath, QMap<QString, QVariant> &opt, QString infoName, QString infoHash);

    /**
     * @brief 新建MetaLink下载任务
     * @param MetaLinkPath MetaLink下载地址
     * @param opt 选项
     * @param infoName bt文件名字
    */
    void downLoadMetaLinkCreate(QString MetaLinkPath, QMap<QString, QVariant> &opt, QString infoName);

private:
    DTextEdit *m_texturl; //url文本框
    DSuggestButton *m_sureButton; //确认按钮
    DFileChooserEdit *m_editDir; //选择下载路径窗口
    QString m_defaultDownloadDir; //默认文件路径

    //QStandardItemModel *m_model; //tableview中的模型，数据交流
    TaskModel *m_model; //tableview中的模型，数据交流
    TaskDelegate *m_delegate; //tableview中选中表格item
    BtInfoTableView *m_tableView; //列表
    DWidget *m_widget; //包裹view

    DCheckBox *m_checkAll; //文件类型全选
    DCheckBox *m_checkVideo; //视频文件类型
    DCheckBox *m_checkAudio; //音频文件类型
    DCheckBox *m_checkPicture; //图片文件类型
    DCheckBox *m_checkDoc; //文档文件类型
    DCheckBox *m_checkZip; //压缩包文件类型
    DCheckBox *m_checkOther; //其他文件类型
    DLabel *m_labelSelectedFileNum; //选中文件数
    DLabel *m_labelFileSize; //总大小标签
    QWidget *m_checkWidget;
    AnalysisUrl *m_analysisUrl;
    DLabel *m_labelCapacityFree; //下载路径所在分区剩余磁盘容量
};

#endif // NEWTASKWIDGET_H
