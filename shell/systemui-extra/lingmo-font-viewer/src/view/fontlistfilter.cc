#include <QDebug>
#include <QCollator>

#include "fontlistfilter.h"
#include "fontlistmodel.h"
#include "globalsizedata.h"

FontListFilter::FontListFilter(QSortFilterProxyModel *parent)
    : QSortFilterProxyModel(parent)
{
}
FontListFilter::~FontListFilter()
{

}

bool FontListFilter::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    /* 按照首字母 a-z 顺序 */
    QString nameLeft = source_left.data(GlobalSizeData::FontRoles::FontName).toString();
    QString nameRight = source_right.data(GlobalSizeData::FontRoles::FontName).toString();
    QString styleLeft = source_left.data(GlobalSizeData::FontRoles::FontStyle).toString();
    QString styleRight = source_right.data(GlobalSizeData::FontRoles::FontStyle).toString();

    QString dateLeft = nameLeft + " " + styleLeft;
    QString dateRight = nameRight + " " + styleRight;

    if (compareString(dateRight, dateLeft)) {
        return true;
    }

    return false;
}

bool FontListFilter::filterAcceptsRow(int sourceRow , const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow , 0 , sourceParent);
    QString fontName = index.data(GlobalSizeData::FontRoles::FontName).toString();
    QString fontStyle = index.data(GlobalSizeData::FontRoles::FontStyle).toString();
    QString fontInfo = fontName + " " + fontStyle;

    int fontFamily = index.data(GlobalSizeData::FontRoles::FontFamily).toInt();
    int fontCollect = index.data(GlobalSizeData::FontRoles::CollectState).toInt();

    /* 判断当前查看字体类别 */
    switch (GlobalSizeData::getInstance()->g_fontType) {
    case FontType::AllFont :
        if (fontInfo.contains(filterRegExp())) {
            return true;
        }
        return false;
        break;
    case FontType::SystemFont :
        if (fontFamily == GlobalSizeData::FamilyType::SystemFont) {
            if (fontInfo.contains(filterRegExp())) {
            return true;
           }
           return false;
        }
        break;
    case FontType::MyFont :
        if (fontFamily == GlobalSizeData::FamilyType::MyFont) {
            if (fontInfo.contains(filterRegExp())) {
            return true;
           }
           return false;
        }
        break;
    case FontType::CollectFont :
        if (fontCollect == GlobalSizeData::CollectType::IsCollected) {
            if (fontInfo.contains(filterRegExp())) {
            return true;
           }
           return false;
        }
        break;
    default :
        return false;
    }

    return false;
}

bool FontListFilter::compareString(QString rightStr, QString leftStr) const
{
    QLocale local(QLocale::Chinese);
    QCollator collator(local);
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);

    QStringList stringList;
    stringList << leftStr << rightStr;
    qSort(stringList.begin(), stringList.end(), collator);

    if (stringList[0] != leftStr) {
        return false;
    }

    return true;
}