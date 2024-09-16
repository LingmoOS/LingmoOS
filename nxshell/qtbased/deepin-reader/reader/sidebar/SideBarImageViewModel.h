// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARIMAGEVIEWMODEL_H
#define SIDEBARIMAGEVIEWMODEL_H

#include <QAbstractListModel>
#include <QMap>

namespace deepin_reader {
class Annotation;
}

typedef enum E_SideBar {
    SIDE_THUMBNIL = 0,
    SIDE_BOOKMARK,
    SIDE_CATALOG,
    SIDE_NOTE,
    SIDE_SEARCH
} E_SideBar;

typedef enum ImageinfoType_e {
    IMAGE_PIXMAP       = Qt::UserRole,
    IMAGE_BOOKMARK     = Qt::UserRole + 1,
    IMAGE_ROTATE       = Qt::UserRole + 2,
    IMAGE_INDEX_TEXT   = Qt::UserRole + 3,
    IMAGE_CONTENT_TEXT = Qt::UserRole + 4,
    IMAGE_SEARCH_COUNT = Qt::UserRole + 5,
    IMAGE_PAGE_SIZE    = Qt::UserRole + 6,
} ImageinfoType_e;

typedef struct ImagePageInfo_t {
    int pageIndex;

    QString strcontents;
    QString strSearchcount;
    deepin_reader::Annotation *annotation = nullptr;

    explicit ImagePageInfo_t(int index = -1);

    bool operator == (const ImagePageInfo_t &other) const;

    bool operator < (const ImagePageInfo_t &other) const;

    bool operator > (const ImagePageInfo_t &other) const;
} ImagePageInfo_t;
Q_DECLARE_METATYPE(ImagePageInfo_t);

class DocSheet;
/**
 * @brief The SideBarImageViewModel class
 * ImageVIew Model
 */
class SideBarImageViewModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SideBarImageViewModel(DocSheet *sheet, QObject *parent = nullptr);

public:
    /**
     * @brief resetData
     * 清空数据
     */
    void resetData();

    /**
     * @brief initModelLst
     * 初始化MODEL数据
     * @param pagelst
     * @param sort
     */
    void initModelLst(const QList<ImagePageInfo_t> &pagelst, bool sort = false);

    /**
     * @brief setBookMarkVisible
     * 设置书签数据缓存状态
     * @param pageIndex
     * @param visible
     * @param updateIndex
     */
    void setBookMarkVisible(int pageIndex, bool visible, bool updateIndex = true);

    /**
     * @brief insertPageIndex
     * 插入指定页码数据
     * @param pageIndex
     */
    void insertPageIndex(int pageIndex);

    /**
     * @brief insertPageIndex
     * 插入缩略图数据
     * @param tImagePageInfo
     */
    void insertPageIndex(const ImagePageInfo_t &tImagePageInfo);

    /**
     * @brief removePageIndex
     * 删除指定页码数据
     * @param pageIndex
     */
    void removePageIndex(int pageIndex);

    /**
     * @brief removeItemForAnno
     * 删除指定注释数据
     * @param annotation
     */
    void removeItemForAnno(deepin_reader::Annotation *annotation);

    /**
     * @brief changeModelData
     * 重置model数据
     * @param pagelst
     */
    void changeModelData(const QList<ImagePageInfo_t> &pagelst);

    /**
     * @brief getModelIndexForPageIndex
     * 根据页码获取对应MODEL INDEX
     * @param pageIndex
     * @return
     */
    QList<QModelIndex> getModelIndexForPageIndex(int pageIndex);

    /**
     * @brief getModelIndexImageInfo
     * 根据指定页码,获取ImagePageInfo_t数据
     * @param modelIndex
     * @param tImagePageInfo
     */
    void getModelIndexImageInfo(int modelIndex, ImagePageInfo_t &tImagePageInfo);

    /**
     * @brief getPageIndexForModelIndex
     * 根据节点索引值,获取对应页码
     * @param row
     * @return
     */
    int getPageIndexForModelIndex(int row);

    /**
     * @brief findItemForAnno
     * 通过注释对象获取对应节点索引值
     * @param annotation
     * @return
     */
    int findItemForAnno(deepin_reader::Annotation *annotation);

    /**
     * @brief handleThumbnail
     * 处理缩略图
     * @param index
     * @param pixmap
     */
    void handleRenderThumbnail(int index, QPixmap pixmap);

public slots:
    /**
     * @brief onUpdateImage
     * 通知指定页码数据变更
     * @param index
     */
    void onUpdateImage(int index);

protected:
    /**
     * @brief columnCount
     * view列数
     * @return
     */
    int columnCount(const QModelIndex &) const override;

    /**
     * @brief rowCount
     * view行数
     * @param parent
     * @return
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief data
     * view显示数据
     * @param index
     * @param role
     * @return
     */
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief setData
     * 设置model数据
     * @param index
     * @param data
     * @param role
     * @return
     */
    bool setData(const QModelIndex &index, const QVariant &data, int role) override;

private:
    QObject *m_parent = nullptr;
    DocSheet *m_sheet = nullptr;
    QList<ImagePageInfo_t> m_pagelst;
    QMap<int, bool> m_cacheBookMarkMap;
    static QMap<QObject *, QVector<QPixmap>> g_sheetPixmapMap;
};

#endif // SIDEBARIMAGEVIEWMODEL_H
