#ifndef GETFONTTHREAD_H
#define GETFONTTHREAD_H

#include <QThread>
#include <QStandardItem>

#include "include/core.h"

class GetFontThread : public QThread
{
    Q_OBJECT
public:
    GetFontThread(QList<ViewData> allFontData, QList<ViewData> collectFontData, int fontSize, QHash<QString, QStandardItem*> fontItemsHash);
    void run();

private:
    QList<ViewData> m_allFontData;                    /* 全部字体 */
    QList<ViewData> m_collectFontData;                /* 收藏字体 */
    QHash<QString, QStandardItem*> m_fontItemsHash;   /* 字体hash <字体path, list的item>*/
    int m_fontSize;
    
    bool chooseFontFile(QString path);
   
signals:
    void sigCreatItem(QString path, QStandardItem *newItem);

private slots:
};

#endif // GETFONTTHREAD_H
