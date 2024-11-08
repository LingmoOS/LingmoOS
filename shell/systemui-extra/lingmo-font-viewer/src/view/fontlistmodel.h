#ifndef FONTLISTMODEL_H
#define FONTLISTMODEL_H

#include <QObject>
#include <QStandardItemModel>
#include <QTimer>

#include "include/core.h"

class FontListModel  : public QStandardItemModel
{
    Q_OBJECT
public:
    ~FontListModel();
    static FontListModel* getInstance();          /* 单例，初始化返回指针 */
    void getFontModel();                          /* 获取所有字体 */
    bool isCorrectFontFile(QString path);         /* 判断字体文件类型 */
    QStringList addFont(QStringList fontList);    /* 添加字体 */
    QStandardItem *getItemByPath(QString path);   /* 获取字体item */
    void removeFont(QString path, int row);       /* 卸载字体 */
    void collectFont(QString path);               /* 收藏字体 */
    void previewChangeSize(int size);             /* 预览字号改变 */
    void changeFontSize();                        /* 改变字号高度 */
    bool fontSizeExist();                         /* 当前字号高度是否存在 */
    void getFontSize();                           /* 计算字号高度 */
    void newFontGetSize(QString fontPath);        /* 新字体获取字体大小 */
    QStringList getInstallSuccessList();          /* 获取安装成功的字体path列表 */

private:
    FontListModel();
    bool isFontFile(QString path);
    void updateListViewItem();

    Core *m_core = nullptr;

    QTimer *m_timer = nullptr;

    int m_fontSize = 24;

    QList<ViewData> m_allFontData;       /* 全部字体 */
    QList<ViewData> m_collectFontData;   /* 收藏字体 */
 
    QHash<QString, QString> m_incorrectFontFileHash;      /* 不正确的字体文件 */
    QHash<QString, QStandardItem*> m_fontItemsHash;       /* 字体hash <字体path, list的item>*/
    QHash<QString, QHash<int,int>> m_fontSizeHash;        /* <字体path, 字体高度hash<字号, 字体高度>> */
    QVector<QString> m_newFont;                           /* 安装新字体数组 */

    bool m_isAddFont = false;
    bool m_isFirstAddItem = true;

    QStringList m_installSuccess;

signals:
    void sigGetFontSize();
    void sigGetAllFontOver();
    void sigInstallFontOver();

private slots:
    void slotAddItem(QString path, QStandardItem* item);
    void slotAddNewItem();
    void slotAGetAllFontOver();
};

#endif // FONTLISTMODEL_H
