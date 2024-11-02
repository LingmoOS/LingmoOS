#pragma once

#include <QListView>
#include <QPixmap>
#include <QVariant>

#include "UI/player/searchresult.h"
#include "UIControl/base/musicDataBase.h"
#include "UIControl/base/musicfileinformation.h"

class MusicSearchListModel;
class MusicSearchListDelegate;
class SearchResult;
class MusicSearchListview: public QListView
{
    Q_OBJECT
public:
    explicit MusicSearchListview(QWidget *parent = Q_NULLPTR);
    ~MusicSearchListview() override;

    void setSearchResultWidget(SearchResult *result);
    void setSearchType(SearchType type);
    SearchType getSearchType() const;
    void setSearchText(QString text);
    int rowCount();
    int getIndexInt()const;
    QString getSearchText()const;
    void    setCurrentIndexInt(int row);

private:
    // 单击或者触屏单击时调用，跳转到搜索结果tab页
    void switchToSearchResultTab(const QModelIndex &index);
Q_SIGNALS:
    void sigSearchClear();
    void signalSearchTexts(QString text);
public Q_SLOTS:
    void SearchClear();
    void slotOnClicked(const QModelIndex &index);
    void onReturnPressed();
protected:
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual bool event(QEvent *event) Q_DECL_OVERRIDE;

public:
    SearchType                  m_searchType;
private:
    MusicSearchListModel        *m_model        = nullptr;
    MusicSearchListDelegate     *m_delegate     = nullptr;
    QString                     m_searchText    = "";
    int                         m_CurrentIndex  = -1;
//    QPixmap                     m_defaultIcon   = QPixmap(":/icons/deepin/builtin/actions/cover_max_200px.svg");

    QList<musicDataStruct> m_MusicDataStruct;
    QList<musicDataStruct> m_SingerInfos;
    QList<musicDataStruct> m_AlbumInfos;
    SearchResult                *m_SearchResultWidget = nullptr;
};

