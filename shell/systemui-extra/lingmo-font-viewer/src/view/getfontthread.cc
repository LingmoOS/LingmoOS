#include<QFontMetrics>

#include "getfontthread.h"
#include "globalsizedata.h"

GetFontThread::GetFontThread(QList<ViewData> allFontData, QList<ViewData> collectFontData, int fontSize, QHash<QString, QStandardItem*> fontItemsHash)
{
    m_allFontData = allFontData;
    m_collectFontData = collectFontData;
    m_fontItemsHash = fontItemsHash;
    m_fontSize = fontSize;

    connect(this, &GetFontThread::finished, this, &GetFontThread::deleteLater);
}

void GetFontThread::run()
{
    for (int i = 0; i < m_allFontData.size(); i++) {
        if (!chooseFontFile(m_allFontData[i].path)) {
            continue;
        }

        if (m_fontItemsHash.contains(m_allFontData[i].path)) {
            continue;
        }

        QStandardItem *newItem = new QStandardItem();

        newItem->setData(m_allFontData[i].path, GlobalSizeData::FontPath);
        newItem->setData(m_allFontData[i].family, GlobalSizeData::FontName);
        newItem->setData(m_allFontData[i].style, GlobalSizeData::FontStyle);
        newItem->setData(GlobalSizeData::FamilyType::SystemFont, GlobalSizeData::FontFamily);
        newItem->setData(GlobalSizeData::CollectType::NotCollected, GlobalSizeData::CollectState);

        QFont textFont = QFont(m_allFontData[i].family, m_fontSize);
        QFontMetrics fontmet = QFontMetrics(textFont);
        int fontHeight = fontmet.lineSpacing();   /* 展示字体的高度 */
        newItem->setData(fontHeight, GlobalSizeData::FontHeight);

        if (!Core::getInstance()->isSystemFont(m_allFontData[i].path)) {
            newItem->setData(GlobalSizeData::FamilyType::MyFont, GlobalSizeData::FontFamily);
        }

        for (int n = 0; n < m_collectFontData.size(); n++) {
            if (m_allFontData[i].path == m_collectFontData[n].path) {
                newItem->setData(GlobalSizeData::CollectType::IsCollected, GlobalSizeData::CollectState);

            }
        }

        emit sigCreatItem(m_allFontData[i].path, newItem);
    }
    return;
}

bool GetFontThread::chooseFontFile(QString path)
{
    QStringList list = path.split(".");
    QString fontTypeTTF = "ttf";
    QString fontTypeOTF = "otf";
    QString fontType = list.at(list.size()-1);

    if (fontType.compare(fontTypeTTF, Qt::CaseInsensitive) == 0) {
        return true;
    } else if (fontType.compare(fontTypeOTF, Qt::CaseInsensitive) == 0) {
        return true;
    }

    return false;
}
