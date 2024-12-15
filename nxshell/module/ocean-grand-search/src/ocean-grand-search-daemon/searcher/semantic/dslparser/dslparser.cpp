// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dslparser.h"
#include <tree/ParseTreeWalker.h>
#include <global/searchhelper.h>

#include <QDebug>

using namespace GrandSearch;
using namespace antlr4;

bool BaseCond::kIsNeedAdjustBase = false;

BaseCond::BaseCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                   SemanticWorkerPrivate *worker, QObject *parent) : QObject(parent) {
    setCond(text);
    m_querys = querys;
    m_worker = worker;
}

BaseCond::~BaseCond() {
    for (BaseCond *cond : m_andCondList) {
        delete cond;
    }

    for (BaseCond *cond : m_orCondList) {
        delete cond;
    }
}

void BaseCond::addAndCond(BaseCond *cond) {
    //qDebug() << QString("BaseCond type(%1) add AND(%2)").arg(m_condType).arg(cond->getCondType());
    m_andCondList.append(cond);
    cond->setParent(this);
}

void BaseCond::addOrCond(BaseCond *cond) {
    //qDebug() << QString("BaseCond type(%1) add OR(%2)").arg(m_condType).arg(cond->getCondType());
    m_orCondList.append(cond);
    cond->setParent(this);
}

bool BaseCond::isMatch(const QString &text) {
    bool ret = true;
    qDebug() << QString("BaseCond type(%1) isMatch(%2) m_cond(%3)").arg(m_condType).arg(text).arg(m_cond);
    if (m_andCondList.empty() && m_orCondList.empty()) {
        ret = false;
        goto _EXIT;
    }

    if (!m_andCondList.empty()) {
        for (BaseCond *cond : m_andCondList) {
            if (!cond->isMatch(text)) {
                ret = false;
                goto _EXIT;
            }
        }

        ret = true;
        goto _EXIT;
    }

    for (BaseCond *cond : m_orCondList) {
        if (cond->isMatch(text)) {
            ret = true;
            goto _EXIT;
        }
    }

    ret = false;

_EXIT:
    qInfo() << QString("BaseCond type(%1) isMatch(%2) ret(%3) m_cond(%4)").arg(m_condType).arg(text).arg(ret).arg(m_cond);
    return ret;
}

void BaseCond::adjust() {
    // 消除冗余的Binary层级
    this->mergeBinary();
    // 消除冗余的Base层级，因为Base层级可以嵌套，所以可能要多次调用（如果存在层级变化，使用kIsNeedAdjustBase进行标记，直到不需变化）
    do {
        BaseCond::kIsNeedAdjustBase = false;
        this->mergeBase();
    } while (BaseCond::kIsNeedAdjustBase);
    qInfo() << QString("after merge:\n%1").arg(this->toString()).toUtf8().toStdString().c_str();

    this->merge4Engine();
    this->adjust4OrCond();
    //this->loadCond();
}

void BaseCond::mergeBinary() {
    // 合并DateInfo类型，原理：一般都是成对出现的
    if (m_condType == querylangParser::RuleBinaryExpression && m_andCondList.size() >= 2
            && m_andCondList[0]->getCondType() == querylangParser::RuleDateSearchinfo) {
        bool isAllDate = true;
        for (int i = 0; i < m_andCondList.size(); i++) {
            if (m_andCondList[i]->getCondType() != querylangParser::RuleDateSearchinfo) {
                isAllDate = false;
                break;
            }
        }

        if (isAllDate) {
            DateInfoCond *allCond = new DateInfoCond("", this->m_querys, this->m_worker);
            allCond->m_compType = "==";
            for (int i = m_andCondList.size() - 1; i >= 0; i--) {
                DateInfoCond *cond = dynamic_cast<DateInfoCond *>(m_andCondList[i]);
                if (cond->m_compType.contains(">")) {
                    allCond->m_timestamp = cond->m_timestamp;
                    allCond->m_cond     += cond->m_cond + "  ";
                } else {
                    allCond->m_timestamp2 = cond->m_timestamp;
                    allCond->m_cond      += cond->m_cond + "  ";
                }
                delete m_andCondList[i];
                m_andCondList.removeAt(i);
            }
            m_andCondList.append(allCond);
            allCond->setParent(this);
        }
    }

    if (m_condType == querylangParser::RulePrimary || m_condType == querylangParser::RuleBinaryExpression) {
        if (!m_andCondList.isEmpty()) {
            for (int i = m_andCondList.size() - 1; i >= 0; i--) {
                m_andCondList[i]->mergeBinary();
            }
        } else {
            for (int i = m_orCondList.size() - 1; i >= 0; i--) {
                m_orCondList[i]->mergeBinary();
            }
        }
    }

    // 调整的策略是：如果parent只有一个child，就把child给parent的parent，减少一级冗余
    if (m_condType != querylangParser::RuleBinaryExpression && getParent() && getParent()->getCondType() == querylangParser::RuleBinaryExpression) {
        BaseCond *parent       = getParent();
        BaseCond *adviseParent = parent->getParent();
        // parent下只有一个child，才可以调整
        if (parent->m_andCondList.size() != 1 && parent->m_orCondList.size() != 1) {
            return;
        }

        int idx = adviseParent->m_andCondList.indexOf(parent);
        if (idx != -1) {
            adviseParent->m_andCondList[idx] = this;
        } else {
            idx = adviseParent->m_orCondList.indexOf(parent);
            if (idx == -1) {
                return;
            }

            adviseParent->m_orCondList[idx] = this;
        }
        setParent(adviseParent);
        parent->m_andCondList.clear();
        delete parent;
    }
}

void BaseCond::mergeBase() {
    // 合并DateInfo类型，原理：一般都是成对出现的
    if (m_condType == querylangParser::RulePrimary && m_andCondList.size() >= 2
            && m_andCondList[0]->getCondType() == querylangParser::RuleDateSearchinfo) {
        bool isAllDate = true;
        for (int i = 0; i < m_andCondList.size(); i++) {
            if (m_andCondList[i]->getCondType() != querylangParser::RuleDateSearchinfo) {
                isAllDate = false;
                break;
            }
        }

        if (isAllDate) {
            DateInfoCond *allCond = new DateInfoCond("", this->m_querys, this->m_worker);
            allCond->m_compType = "==";
            for (int i = m_andCondList.size() - 1; i >= 0; i--) {
                DateInfoCond *cond = dynamic_cast<DateInfoCond *>(m_andCondList[i]);
                if (cond->m_compType.contains(">")) {
                    allCond->m_timestamp = cond->m_timestamp;
                    allCond->m_cond     += cond->m_cond + "  ";
                } else {
                    allCond->m_timestamp2 = cond->m_timestamp;
                    allCond->m_cond      += cond->m_cond + "  ";
                }
                delete m_andCondList[i];
                m_andCondList.removeAt(i);
            }
            m_andCondList.append(allCond);
            allCond->setParent(this);
        }
    }

    if (m_condType == querylangParser::RulePrimary) {
        if (!m_andCondList.isEmpty()) {
            for (int i = m_andCondList.size() - 1; i >= 0; i--) {
                m_andCondList[i]->mergeBase();
            }
        } else {
            for (int i = m_orCondList.size() - 1; i >= 0; i--) {
                m_orCondList[i]->mergeBase();
            }
        }
    }

    // 调整的策略是：如果parent只有一个child，就把child给parent的parent，减少一级冗余
    if (m_condType != querylangParser::RulePrimary && getParent() && getParent()->getCondType() == querylangParser::RulePrimary
            && getParent()->getParent()) {
        BaseCond *parent       = getParent();
        BaseCond *adviseParent = parent->getParent();
        // parent下只有一个child，才可以调整
        if (parent->m_andCondList.size() != 1 && parent->m_orCondList.size() != 1) {
            return;
        }

        int idx = adviseParent->m_andCondList.indexOf(parent);
        if (idx != -1) {
            adviseParent->m_andCondList[idx] = this;
        } else {
            idx = adviseParent->m_orCondList.indexOf(parent);
            if (idx == -1) {
                return;
            }

            adviseParent->m_orCondList[idx] = this;
        }
        setParent(adviseParent);
        parent->m_andCondList.clear();
        delete parent;
        BaseCond::kIsNeedAdjustBase = true;
    }
}

void BaseCond::merge4Engine() {
    if (m_condType == querylangParser::RulePrimary || m_condType == querylangParser::RuleBinaryExpression) {
        if (m_andCondList.isEmpty()) {
            for (int i = 0; i < m_orCondList.size(); i++) {
                m_orCondList[i]->merge4Engine();
            }
            return;
        }

        bool hasFeature = false, hasFulltext = false;
        for (int i = 0; i < m_andCondList.size(); i++) {
            if (m_andCondList[i]->getCondType() == querylangParser::RuleMetaSearch
                    || m_andCondList[i]->getCondType() == querylangParser::RuleDurationSearch) {
                hasFeature = true;
                break;
            }

            if (m_andCondList[i]->getCondType() == querylangParser::RuleContentSearch) {
                hasFulltext = true;
                break;
            }
        }

        if (hasFeature) {
            FeatureCond *cond = new FeatureCond(&m_andCondList, m_querys, m_worker);
            if (cond->isValid()) {
                m_andCondList.append(cond);
                cond->setParent(this);
            } else {
                delete cond;
            }
        } else if (hasFulltext) {
            FulltextCond *cond = new FulltextCond(&m_andCondList, m_querys, m_worker);
            if (cond->isValid()) {
                m_andCondList.append(cond);
                cond->setParent(this);
            } else {
                delete cond;
            }
        } else {
            AnythingCond *cond = new AnythingCond(&m_andCondList, m_querys, m_worker);
            if (cond->isValid()) {
                m_andCondList.append(cond);
                cond->setParent(this);
            } else {
                delete cond;
            }
        }
    }
}

void BaseCond::adjust4OrCond() {
    if (m_andCondList.size() == 2 && m_andCondList[0]->getCondType() == querylangParser::RulePrimary
            && !m_andCondList[0]->m_orCondList.isEmpty()
            && m_andCondList[1]->getCondType() == BinaryCond::RuleAnything) {
        bool isAllSimpleCond = true;
        for (int i = 0; i < m_andCondList[0]->m_orCondList.size(); i++) {
            BaseCond *cond = m_andCondList[0]->m_orCondList[i];
            if (cond->getCondType() != querylangParser::RuleTypeSearch) {
                isAllSimpleCond = false;
                break;
            }
        }

        if (isAllSimpleCond) {
            for (int i = 0; i < m_andCondList[0]->m_orCondList.size(); i++) {
                BaseCond *cond = m_andCondList[0]->m_orCondList[i];
                AnythingCond *anyCond = dynamic_cast<AnythingCond *>(m_andCondList[1]);
                if (!anyCond->copyCondOut(cond)) {
                    return;
                }
            }

            delete m_andCondList[1];
            m_andCondList.removeAt(1);
            return;
        }
    }

    if (m_andCondList.size() == 2 && m_andCondList[0]->getCondType() == querylangParser::RulePrimary
            && !m_andCondList[0]->m_orCondList.isEmpty()
            && m_andCondList[1]->getCondType() == BinaryCond::RuleFeature) {
        bool isAllSimpleCond = true;
        for (int i = 0; i < m_andCondList[0]->m_orCondList.size(); i++) {
            BaseCond *cond = m_andCondList[0]->m_orCondList[i];
            if (cond->getCondType() != querylangParser::RuleTypeSearch) {
                isAllSimpleCond = false;
                break;
            }
        }

        if (isAllSimpleCond) {
            for (int i = 0; i < m_andCondList[0]->m_orCondList.size(); i++) {
                BaseCond *cond = m_andCondList[0]->m_orCondList[i];
                FeatureCond *anyCond = dynamic_cast<FeatureCond *>(m_andCondList[1]);
                if (!anyCond->copyCondOut(cond)) {
                    return;
                }
            }

            delete m_andCondList[1];
            m_andCondList.removeAt(1);
            return;
        }
    }

    if (m_andCondList.size() == 2 && m_andCondList[0]->getCondType() == querylangParser::RulePrimary
            && !m_andCondList[0]->m_orCondList.isEmpty()
            && m_andCondList[1]->getCondType() == BinaryCond::RuleFulltext) {
        bool isAllSimpleCond = true;
        for (int i = 0; i < m_andCondList[0]->m_orCondList.size(); i++) {
            BaseCond *cond = m_andCondList[0]->m_orCondList[i];
            if (cond->getCondType() != querylangParser::RuleTypeSearch) {
                isAllSimpleCond = false;
                break;
            }
        }

        if (isAllSimpleCond) {
            for (int i = 0; i < m_andCondList[0]->m_orCondList.size(); i++) {
                BaseCond *cond = m_andCondList[0]->m_orCondList[i];
                FulltextCond *anyCond = dynamic_cast<FulltextCond *>(m_andCondList[1]);
                if (!anyCond->copyCondOut(cond)) {
                    return;
                }
            }

            delete m_andCondList[1];
            m_andCondList.removeAt(1);
            return;
        }
    }

    if (m_andCondList.size() == 1 && m_andCondList[0]->getCondType() == querylangParser::RulePrimary) {
        m_andCondList[0]->adjust4OrCond();
    }
}

void BaseCond::loadCond() {
    if (!m_andCondList.isEmpty()) {
        for (int i = 0; i < m_andCondList.size(); i++) {
            m_andCondList[i]->loadCond();
        }
    } else {
        for (int i = 0; i < m_orCondList.size(); i++) {
            m_orCondList[i]->loadCond();
        }
    }
}

QList<SemanticEntity> BaseCond::entityList() {
    QList<SemanticEntity> list;
    if (m_condType == querylangParser::RulePrimary || m_condType == querylangParser::RuleBinaryExpression) {
        if (!m_andCondList.isEmpty()) {
            list.append(m_andCondList[0]->entityList());
            return list;
        } else {
            for (int i = 0; i < m_orCondList.size(); i++) {
                list.append(m_orCondList[i]->entityList());
            }
            return list;
        }
    }

    list.append(m_entity);
    return list;
}

void BaseCond::addMatchedItems(const MatchedItems &items) {
    m_matchedItems.append(items);
}

const MatchedItems &BaseCond::getMatchedItems() {
    if (m_condType != querylangParser::RulePrimary && m_condType != querylangParser::RuleBinaryExpression) {
        return m_matchedItems;
    }

    if (!m_andCondList.isEmpty()) {
        int quantity = -1;
        bool isGetItems = false;
        for (int i = 0; i < m_andCondList.size(); i++) {
            if (m_andCondList[i]->getCondType() == querylangParser::RuleQuantityCondition) {
                quantity = dynamic_cast<QuantityCond *>(m_andCondList[i])->getQuantity();
                continue;
            }

            if (!isGetItems) {
                m_matchedItems = m_andCondList[i]->getMatchedItems();
                isGetItems = true;
                continue;
            }

            // 存在删除操作，所以采用倒序
            for (int j = m_matchedItems.size() - 1; j >= 0; j--) {
                if (m_andCondList[i]->getMatchedItems().contains(m_matchedItems[j])) {
                    continue;
                }

                m_matchedItems.removeAt(j);
            }
        }
        if (quantity > 0 && m_matchedItems.size() > quantity) {
            while (m_matchedItems.size() > quantity) {
                m_matchedItems.removeLast();
            }
        }
        return m_matchedItems;
    }

    for (int i = 0; i < m_orCondList.size(); i++) {
        const MatchedItems &items = m_orCondList[i]->getMatchedItems();
        for (int j = 0; j < items.size(); j++) {
            if (m_matchedItems.contains(items[j])) {
                continue;
            }

            m_matchedItems.append(items[j]);
        }
    }
    return m_matchedItems;
}

QString BaseCond::toString(int spaceCounts) {
    // 控制缩进
    QString space;
    for (int i = 0; i < spaceCounts; i++) {
        space.append(" ");
    }
    spaceCounts += 2;

    QString str;
    for (BaseCond *cond : m_andCondList) {
        str.append(QString("%1AND: %2(%3)\n").arg(space).arg(cond->getCondType()).arg(cond->getCond()));
        str.append(cond->toString(spaceCounts));
    }
    for (BaseCond *cond : m_orCondList) {
        str.append(QString("%1OR: %2(%3)\n").arg(space).arg(cond->getCondType()).arg(cond->getCond()));
        str.append(cond->toString(spaceCounts));
    }
    return str;
}

DateInfoCond::DateInfoCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                   SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleDateSearchinfo;

    const QDateTime curDT = QDateTime::currentDateTime();
    const QDate curDate = curDT.date();

    m_timestamp = 0;
    int pos = m_cond.indexOf("\"") + 1;
    if (pos == 0) {
        m_compType = ">";
        return;
    }

    QString temp = m_cond.mid(pos, m_cond.length() - pos - 1);
    if (m_cond.contains(">")) {
        m_compType = ">";
    } else {
        m_compType = "<";
    }
    static QRegExp reg("([\\d\\.]+) ?year", Qt::CaseInsensitive);
    static QRegExp reg1("([\\d\\.]+) ?month", Qt::CaseInsensitive);
    static QRegExp reg2("([\\d\\.]+) ?week", Qt::CaseInsensitive);
    static QRegExp reg3("([\\d\\.]+) ?day", Qt::CaseInsensitive);
    static QRegExp reg4("([\\d\\.]+) ?hour", Qt::CaseInsensitive);
    static QRegExp reg5("([\\d\\.]+) ?(min|minute)", Qt::CaseInsensitive);
    if (reg.indexIn(temp) != -1) { // 年
        int offset = reg.cap(1).toInt() - 1;
        auto anchor = QDateTime(QDate(curDate.year(), 1, 1), QTime(0, 0, 0));
        m_timestamp2 = anchor.addSecs(-1).toSecsSinceEpoch();
        m_timestamp = anchor.addYears(-1 * offset).toSecsSinceEpoch();
    } else if (reg1.indexIn(temp) != -1) { // 月
        int offset = reg1.cap(1).toInt() - 1;
        auto anchor = QDateTime(QDate(curDate.year(), curDate.month(), 1), QTime(0, 0, 0));
        m_timestamp2 = anchor.addSecs(-1).toSecsSinceEpoch();
        m_timestamp = anchor.addMonths(-1 * offset).toSecsSinceEpoch();
    } else if (reg2.indexIn(temp) != -1) {// 周
        int offset = reg2.cap(1).toInt() - 1;
        auto anchor = QDateTime(QDate(curDate.addDays(1 - curDate.dayOfWeek())), QTime(0, 0, 0));
        m_timestamp2 = anchor.addSecs(-1).toSecsSinceEpoch();
        m_timestamp = anchor.addDays(-1 * offset * 7).toSecsSinceEpoch();
    } else if (reg3.indexIn(temp) != -1) { // 天
        int offset = reg3.cap(1).toInt() - 1;
        auto anchor = QDateTime(curDate, QTime(0, 0, 0));
        m_timestamp2 = anchor.addSecs(-1).toSecsSinceEpoch();
        m_timestamp = anchor.addDays(-1 * offset).toSecsSinceEpoch();
    } else if (reg4.indexIn(temp) != -1) { // 时
        int offset = reg4.cap(1).toInt();
        m_timestamp2 = curDT.toSecsSinceEpoch();
        m_timestamp = curDT.addSecs(-1 * offset * 60 * 60).toSecsSinceEpoch();
    } else if (reg5.indexIn(temp) != -1) { // 分
        int offset = reg5.cap(1).toInt();
        m_timestamp2 = curDT.toSecsSinceEpoch();
        m_timestamp = curDT.addSecs(-1 * offset * 60).toSecsSinceEpoch();
    } else {
        m_timestamp = curDT.toSecsSinceEpoch();
    }
    qDebug() << QString("DateInfoCond(%1): temp(%2) m_compType(%3) m_timestamp(%4)").arg(m_cond).arg(temp).arg(m_compType).arg(m_timestamp);

    if (m_compType == ">") {
        m_entity.times.append(QPair<qint64, qint64>(m_timestamp, m_timestamp2));
    } else {
        m_entity.times.append(QPair<qint64, qint64>(0, m_timestamp));
    }
}

void DateInfoCond::loadCond() {
    m_anything.setEntity({m_entity});
    SemanticWorkerPrivate::QueryFunction func = {&m_anything, &AnythingQuery::run, this};
    m_querys->append(func);
}

PathCond::PathCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                   SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RulePathSearch;
    if (m_cond.contains("IS NOT")) {
        m_isTruePath = false;
    }

    int pos = m_cond.indexOf("\"") + 1;
    m_pathName = m_cond.mid(pos, m_cond.length() - pos - 1);
    if (m_pathName == "Download") {
        m_pathName = "Downloads";
    } else if (m_pathName == "Document") {
        m_pathName = "Documents";
    } else if (m_pathName == "Picture") {
        m_pathName = "Pictures";
    } else if (m_pathName == "Video") {
        m_pathName = "Videos";
    }
    qDebug() << QString("PathCond(%1): m_isTrue(%2) m_pathName(%3)").arg(m_cond).arg(m_isTruePath).arg(m_pathName);

    m_entity.partPath      = m_pathName;
    m_entity.isContainPath = m_isTruePath;
}

void PathCond::loadCond() {
    m_anything.setEntity({m_entity});
    SemanticWorkerPrivate::QueryFunction func = {&m_anything, &AnythingQuery::run, this};
    m_querys->append(func);
}

NameCond::NameCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                   SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleNameSearch;
    int pos = m_cond.indexOf("\"") + 1;
    m_name = m_cond.mid(pos, m_cond.length() - pos - 1);
    qDebug() << QString("NameCond(%1): m_name(%2)").arg(m_cond).arg(m_name);

    m_entity.keys.append(m_name);
}

void NameCond::loadCond() {
    m_anything.setEntity({m_entity});
    SemanticWorkerPrivate::QueryFunction func = {&m_anything, &AnythingQuery::run, this};
    m_querys->append(func);
}

SizeCond::SizeCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                   SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleSizeSearch;
    int pos = m_cond.indexOf("\"") + 1;
    QString temp = m_cond.mid(pos, m_cond.length() - pos - 1);
    if (m_cond.contains(">")) {
        m_compType = ">";
    } else if (m_cond.contains("<")) {
        m_compType = "<";
    } else if (m_cond.contains("!=")) {
        m_compType = "!=";
    } else {
        m_compType = "=";
    }
    static QRegExp reg("([\\d\\.]+) ?gb", Qt::CaseInsensitive);
    static QRegExp reg1("([\\d\\.]+) ?mb", Qt::CaseInsensitive);
    static QRegExp reg2("([\\d\\.]+) ?kb", Qt::CaseInsensitive);
    static QRegExp reg3("([\\d\\.]+) ?(b|byte|bytes)", Qt::CaseInsensitive);
    if (reg.indexIn(temp) != -1) {
        m_fileSize = static_cast<qint64>(reg.cap(1).toFloat() * 1024 * 1024 * 1024);
    } else if (reg1.indexIn(temp) != -1) {
        m_fileSize = static_cast<qint64>(reg1.cap(1).toFloat() * 1024 * 1024);
    } else if (reg2.indexIn(temp) != -1) {
        m_fileSize = static_cast<qint64>(reg2.cap(1).toFloat() * 1024);
    } else if (reg3.indexIn(temp) != -1) {
        m_fileSize = static_cast<qint64>(reg3.cap(1).toFloat());
    } else {
        m_fileSize = 0;
    }
    qDebug() << QString("SizeCond(%1): temp(%2) m_compType(%3) m_fileSize(%4)").arg(m_cond).arg(temp).arg(m_compType).arg(m_fileSize);

    m_entity.fileCompType = m_compType;
    m_entity.fileSize     = m_fileSize;
}

void SizeCond::loadCond() {
    m_anything.setEntity({m_entity});
    SemanticWorkerPrivate::QueryFunction func = {&m_anything, &AnythingQuery::run, this};
    m_querys->append(func);
}

TypeCond::TypeCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                   SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleTypeSearch;
    if (m_cond.contains("IS NOT")) {
        m_isTrueType = false;
    }

    int pos = m_cond.indexOf("\"") + 1;
    m_typeName = m_cond.mid(pos, m_cond.length() - pos - 1);
    qDebug() << QString("TypeCond(%1): m_isTrue(%2) m_typeName(%3)").arg(m_cond).arg(m_isTrueType).arg(m_typeName);

    /*
     * #define DOCUMENT_GROUP      "text"
     * #define PICTURE_GROUP       "img"
     * #define AUDIO_GROUP         "msc"
     * #define VIDEO_GROUP         "vdo"
     * #define FILE_GROUP          "file"
     * #define FOLDER_GROUP        "fld"
     * #define APPLICATION_GROUP   "app"
     */
    // 优先匹配为类别，其实是扩展名
    if (m_typeName == "song" || m_typeName == "music") {
        m_entity.types.append(AUDIO_GROUP);
    } else if (m_typeName == "paper" || m_typeName == "document") {
        m_entity.types.append(DOCUMENT_GROUP);
    } else if (m_typeName == "picture") {
        m_entity.types.append(PICTURE_GROUP);
    } else if (m_typeName == "video") {
        m_entity.types.append(VIDEO_GROUP);
    } else if (m_typeName == "file") {
        m_entity.types.clear();
    } else {
        m_entity.suffix = m_typeName;
    }
    m_entity.isContainType = m_isTrueType;
}

void TypeCond::loadCond() {
    m_anything.setEntity({m_entity});
    SemanticWorkerPrivate::QueryFunction func = {&m_anything, &AnythingQuery::run, this};
    m_querys->append(func);
}

DurationCond::DurationCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleDurationSearch;
    // DURATION>"3 minute"
    if (m_cond.contains(">")) {
        m_compType = ">";
    } else if (m_cond.contains("<")) {
        m_compType = "<";
    } else if (m_cond.contains("!=")) {
        m_compType = "!=";
    } else {
        m_compType = "=";
    }
    static QRegExp reg("([\\d\\.]+) ?year", Qt::CaseInsensitive);
    static QRegExp reg1("([\\d\\.]+) ?mouth", Qt::CaseInsensitive);
    static QRegExp reg2("([\\d\\.]+) ?week", Qt::CaseInsensitive);
    static QRegExp reg3("([\\d\\.]+) ?day", Qt::CaseInsensitive);
    static QRegExp reg4("([\\d\\.]+) ?hour", Qt::CaseInsensitive);
    static QRegExp reg5("([\\d\\.]+) ?(min|minute)", Qt::CaseInsensitive);
    static QRegExp reg6("([\\d\\.]+) ?(sec|second)", Qt::CaseInsensitive);
    int pos = m_cond.indexOf("\"") + 1;
    QString temp = m_cond.mid(pos, m_cond.length() - pos - 1);
    if (reg.indexIn(temp) != -1) {
        m_duration = formatTime(static_cast<qint64>(reg.cap(1).toFloat() * 60 * 60 * 24 * 365) * 1000);
    } else if (reg1.indexIn(temp) != -1) {
        m_duration = formatTime(static_cast<qint64>(reg1.cap(1).toFloat() * 60 * 60 * 24 * 30) * 1000);
    } else if (reg2.indexIn(temp) != -1) {
        m_duration = formatTime(static_cast<qint64>(reg2.cap(1).toFloat() * 60 * 60 * 24 * 7) * 1000);
    } else if (reg3.indexIn(temp) != -1) {
        m_duration = formatTime(static_cast<qint64>(reg3.cap(1).toFloat() * 60 * 60 * 24) * 1000);
    } else if (reg4.indexIn(temp) != -1) {
        m_duration = formatTime(static_cast<qint64>(reg4.cap(1).toFloat() * 60 * 60) * 1000);
    } else if (reg5.indexIn(temp) != -1) {
        m_duration = formatTime(static_cast<qint64>(reg5.cap(1).toFloat() * 60) * 1000);
    } else {
        m_duration = formatTime(static_cast<qint64>(reg5.cap(1).toFloat()) * 1000);
    }
    qDebug() << QString("DurationCond(%1): temp(%2) compType(%3) duration(%4)").arg(m_cond).arg(temp).arg(m_compType).arg(m_duration);

    m_entity.types.append(AUDIO_GROUP);
    m_entity.types.append(VIDEO_GROUP);
    m_entity.durationCompType = m_compType;
    m_entity.duration         = m_duration;
}

QString DurationCond::formatTime(qint64 msec) {
    auto hours = msec / (1000 * 60 * 60);
    int minutes = (msec % (1000 * 60 * 60)) / (1000 * 60);
    int seconds = ((msec % (1000 * 60 * 60)) % (1000 * 60)) / 1000;
    QString formattedDuration = QString("%1%2%3")
                                        .arg(hours, 2, 10, QLatin1Char('0'))
                                        .arg(minutes, 2, 10, QLatin1Char('0'))
                                        .arg(seconds, 2, 10, QLatin1Char('0'));
    return formattedDuration;
}

void DurationCond::loadCond() {
    m_feature.setEntity({m_entity});
    SemanticWorkerPrivate::QueryFunction func = {&m_feature, &FeatureQuery::run, this};
    m_querys->append(func);
}

MetaCond::MetaCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleMetaSearch;
    // META_TYPEIS"ARTIST"ANDMETA_VALUEIS"xxx"
    // META_TYPEIS"ALBUM"ANDMETA_VALUEIS"xxx"
    // META_TYPEIS"RESOLUTION"ANDMETA_VALUEIS"4K"
    if (m_cond.contains("META_VALUEIS NOT")) {
        m_isTrue = false;
    }

    static QRegExp reg("META_TYPEIS\"(.+)\"ANDMETA_VALUEIS\"(.+)\"", Qt::CaseInsensitive);
    static QRegExp reg1("META_TYPEIS\"(.+)\"ANDMETA_VALUEIS NOT\"(.+)\"", Qt::CaseInsensitive);
    if (m_isTrue) {
        if (reg.indexIn(m_cond) != -1) {
            m_metaType  = reg.cap(1);
            m_metaValue = reg.cap(2);
        }
    } else {
        if (reg1.indexIn(m_cond) != -1) {
            m_metaType  = reg1.cap(1);
            m_metaValue = reg1.cap(2);
        }
    }
    if (m_metaType == "RESOLUTION") {
        if (m_metaValue == "720P" || m_metaValue == "720p") {
            m_metaValue = "1280*720";
        } else if (m_metaValue == "1080P" || m_metaValue == "1080p") {
            m_metaValue = "1920*1080";
        } else if (m_metaValue == "4K" || m_metaValue == "4k") {
            m_metaValue = "3840*2160";
        } else if (m_metaValue == "8K" || m_metaValue == "8k") {
            m_metaValue = "7680*4320";
        }
    }
    qDebug() << QString("MetaCond(%1): isTrue(%2) metaType(%3) metaValue(%4)").arg(m_cond).arg(m_isTrue).arg(m_metaType).arg(m_metaValue);

    if (m_metaType == "ARTIST") {
        m_entity.types.append(AUDIO_GROUP);
        m_entity.author = m_metaValue;
    } else if (m_metaType == "ALBUM") {
        m_entity.types.append(AUDIO_GROUP);
        m_entity.album = m_metaValue;
    } else if (m_metaType == "RESOLUTION") {
        m_entity.types.append(VIDEO_GROUP);
        m_entity.resolution = m_metaValue;
    }
}

void MetaCond::loadCond() {
    if (m_metaType.isEmpty() || m_metaValue.isEmpty()) {
        return;
    }

    m_feature.setEntity({m_entity});
    SemanticWorkerPrivate::QueryFunction func = {&m_feature, &FeatureQuery::run, this};
    m_querys->append(func);
}

QuantityCond::QuantityCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleQuantityCondition;
    int pos = m_cond.indexOf("=") + 1;
    QString temp = m_cond.mid(pos);
    m_quantity = temp.toInt();
    qDebug() << QString("QuantityCond(%1): temp(%2) m_quantity(%3)").arg(m_cond).arg(temp).arg(m_quantity);
}

ContentCond::ContentCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                         SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleContentSearch;
    int pos = m_cond.indexOf("\"") + 1;
    m_content = m_cond.mid(pos, m_cond.length() - pos - 1);
    qDebug() << QString("ContentCond(%1): m_content(%2)").arg(m_cond).arg(m_content);

    m_entity.keys.append(m_content);
}

void ContentCond::loadCond() {
    m_fulltext.setEntity({m_entity});
    SemanticWorkerPrivate::QueryFunction func = {&m_fulltext, &FullTextQuery::run, this};
    m_querys->append(func);
}

AnythingCond::AnythingCond(QList<BaseCond *> *andList, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                           SemanticWorkerPrivate *worker, QObject *parent) : BaseCond("", querys, worker, parent) {
    const qint64 _NOW = QDateTime::currentSecsSinceEpoch();
    m_condType = BaseCond::RuleAnything;
    m_isValid = false;

    // 支持：DateInfo Path Name Size Type
    // Date的条件，一般都是成对出现的
    qint64 timestamp1 = 0, timestamp2 = _NOW;
    // 使用倒序的原因：含有删除操作
    for (int i = andList->size() - 1; i >= 0; i--) {
        if ((*andList)[i]->getCondType() == querylangParser::RuleDateSearchinfo) {
            m_isValid = true;
            DateInfoCond *cond = dynamic_cast<DateInfoCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            if (cond->m_compType == "==") {
                timestamp1 = cond->m_timestamp;
                timestamp2 = cond->m_timestamp2;
            } else if (cond->m_compType.contains(">")) {
                timestamp1 = cond->m_timestamp;
            } else {
                timestamp2 = cond->m_timestamp;
            }
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        if ((*andList)[i]->getCondType() == querylangParser::RulePathSearch) {
            m_isValid = true;
            PathCond *cond = dynamic_cast<PathCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.isContainPath = cond->m_isTruePath;
            m_entity.partPath      = cond->m_pathName;
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        if ((*andList)[i]->getCondType() == querylangParser::RuleNameSearch) {
            m_isValid = true;
            NameCond *cond = dynamic_cast<NameCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.keys.append(cond->m_name);
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        if ((*andList)[i]->getCondType() == querylangParser::RuleSizeSearch) {
            m_isValid = true;
            SizeCond *cond = dynamic_cast<SizeCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.fileCompType = cond->m_compType;
            m_entity.fileSize     = cond->m_fileSize;
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        if ((*andList)[i]->getCondType() == querylangParser::RuleTypeSearch) {
            m_isValid = true;
            TypeCond *cond = dynamic_cast<TypeCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            /*
             * #define DOCUMENT_GROUP      "text"
             * #define PICTURE_GROUP       "img"
             * #define AUDIO_GROUP         "msc"
             * #define VIDEO_GROUP         "vdo"
             * #define FILE_GROUP          "file"
             * #define FOLDER_GROUP        "fld"
             * #define APPLICATION_GROUP   "app"
             */
            // 优先匹配为类别，其实是扩展名
            if (cond->m_typeName == "song" || cond->m_typeName == "music") {
                m_entity.types.append(AUDIO_GROUP);
            } else if (cond->m_typeName == "paper" || cond->m_typeName == "document") {
                m_entity.types.append(DOCUMENT_GROUP);
            } else if (cond->m_typeName == "picture") {
                m_entity.types.append(PICTURE_GROUP);
            } else if (cond->m_typeName == "video") {
                m_entity.types.append(VIDEO_GROUP);
            } else if (cond->m_typeName == "file") {
                m_entity.types.clear();
            } else {
                m_entity.suffix = cond->m_typeName;
            }
            m_entity.isContainType = cond->m_isTrueType;
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        (*andList)[i]->merge4Engine();
    }
    if (timestamp1 != 0 || timestamp2 != _NOW) {
        m_entity.times.append(QPair<qint64, qint64>(timestamp1, timestamp2));
    }
}

bool AnythingCond::copyCondOut(BaseCond *cond) {
    if (cond->getCondType() == querylangParser::RuleTypeSearch) {
        cond->m_condType = BaseCond::RuleAnything;
        cond->m_cond     = m_cond + cond->m_cond;
        SemanticEntity entity = m_entity;
        entity.types.append(cond->m_entity.types);
        entity.suffix = cond->m_entity.suffix;
        cond->m_entity   = entity;
        return true;
    }

    return false;
}

void AnythingCond::loadCond() {
    if (!m_isValid) {
        return;
    }

    m_anything.setEntity({m_entity});
    SemanticWorkerPrivate::QueryFunction func = {&m_anything, &AnythingQuery::run, this};
    m_querys->append(func);
}

FeatureCond::FeatureCond(QList<BaseCond *> *andList, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                           SemanticWorkerPrivate *worker, QObject *parent) : BaseCond("", querys, worker, parent) {
    const qint64 _NOW = QDateTime::currentSecsSinceEpoch();
    m_condType = BaseCond::RuleFeature;
    m_isValid = false;

    // 支持：Meta Duration DateInfo Path Name Size Type
    // Date的条件，一般都是成对出现的
    qint64 timestamp1 = 0, timestamp2 = _NOW;
    // 使用倒序的原因：含有删除操作
    for (int i = andList->size() - 1; i >= 0; i--) {
        if ((*andList)[i]->getCondType() == querylangParser::RuleMetaSearch) {
            m_isValid = true;
            MetaCond *cond = dynamic_cast<MetaCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            if (cond->m_metaType == "ARTIST") {
                if (!m_entity.types.contains(AUDIO_GROUP)) {
                    m_entity.types.append(AUDIO_GROUP);
                }
                m_entity.author = cond->m_metaValue;
            } else if (cond->m_metaType == "ALBUM") {
                if (!m_entity.types.contains(AUDIO_GROUP)) {
                    m_entity.types.append(AUDIO_GROUP);
                }
                m_entity.album = cond->m_metaValue;
            } else if (cond->m_metaType == "RESOLUTION") {
                if (!m_entity.types.contains(VIDEO_GROUP)) {
                    m_entity.types.append(VIDEO_GROUP);
                }
                m_entity.resolution = cond->m_metaValue;
            }
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        if ((*andList)[i]->getCondType() == querylangParser::RuleDurationSearch) {
            m_isValid = true;
            DurationCond *cond = dynamic_cast<DurationCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.durationCompType = cond->m_compType;
            m_entity.duration         = cond->m_duration;
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        if ((*andList)[i]->getCondType() == querylangParser::RuleDateSearchinfo) {
            m_isValid = true;
            DateInfoCond *cond = dynamic_cast<DateInfoCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            if (cond->m_compType == "==") {
                timestamp1 = cond->m_timestamp;
                timestamp2 = cond->m_timestamp2;
            } else if (cond->m_compType.contains(">")) {
                timestamp1 = cond->m_timestamp;
            } else {
                timestamp2 = cond->m_timestamp;
            }
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        if ((*andList)[i]->getCondType() == querylangParser::RulePathSearch) {
            m_isValid = true;
            PathCond *cond = dynamic_cast<PathCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.isContainPath = cond->m_isTruePath;
            m_entity.partPath      = cond->m_pathName;
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        if ((*andList)[i]->getCondType() == querylangParser::RuleNameSearch) {
            m_isValid = true;
            NameCond *cond = dynamic_cast<NameCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.keys.append(cond->m_name);
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        if ((*andList)[i]->getCondType() == querylangParser::RuleSizeSearch) {
            m_isValid = true;
            SizeCond *cond = dynamic_cast<SizeCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.fileCompType = cond->m_compType;
            m_entity.fileSize     = cond->m_fileSize;
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        if ((*andList)[i]->getCondType() == querylangParser::RuleTypeSearch) {
            m_isValid = true;
            TypeCond *cond = dynamic_cast<TypeCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            /*
             * #define DOCUMENT_GROUP      "text"
             * #define PICTURE_GROUP       "img"
             * #define AUDIO_GROUP         "msc"
             * #define VIDEO_GROUP         "vdo"
             * #define FILE_GROUP          "file"
             * #define FOLDER_GROUP        "fld"
             * #define APPLICATION_GROUP   "app"
             */
            // 优先匹配为类别，其实是扩展名
            if (cond->m_typeName == "song" || cond->m_typeName == "music") {
                m_entity.types.append(AUDIO_GROUP);
            } else if (cond->m_typeName == "paper" || cond->m_typeName == "document") {
                m_entity.types.append(DOCUMENT_GROUP);
            } else if (cond->m_typeName == "picture") {
                m_entity.types.append(PICTURE_GROUP);
            } else if (cond->m_typeName == "video") {
                m_entity.types.append(VIDEO_GROUP);
            } else if (cond->m_typeName == "file") {
                m_entity.types.clear();
            } else {
                m_entity.suffix = cond->m_typeName;
            }
            m_entity.isContainType = cond->m_isTrueType;
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        (*andList)[i]->merge4Engine();
    }
    if (timestamp1 != 0 || timestamp2 != _NOW) {
        m_entity.times.append(QPair<qint64, qint64>(timestamp1, timestamp2));
    }
}

bool FeatureCond::copyCondOut(BaseCond *cond) {
    if (cond->getCondType() == querylangParser::RuleTypeSearch) {
        cond->m_condType = BaseCond::RuleFeature;
        cond->m_cond     = m_cond + cond->m_cond;
        SemanticEntity entity = m_entity;
        entity.types.append(cond->m_entity.types);
        entity.suffix = cond->m_entity.suffix;
        cond->m_entity   = entity;
        return true;
    }

    return false;
}

void FeatureCond::loadCond() {
    if (!m_isValid) {
        return;
    }

    m_feature.setEntity({m_entity});
    SemanticWorkerPrivate::QueryFunction func = {&m_feature, &FeatureQuery::run, this};
    m_querys->append(func);
}

FulltextCond::FulltextCond(QList<BaseCond *> *andList, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                           SemanticWorkerPrivate *worker, QObject *parent) : BaseCond("", querys, worker, parent) {
    const qint64 _NOW = QDateTime::currentSecsSinceEpoch();
    m_condType = BaseCond::RuleFulltext;
    m_isValid = false;

    // 支持：Content DateInfo Path Name Size Type
    // Date的条件，一般都是成对出现的
    qint64 timestamp1 = 0, timestamp2 = _NOW;
    // 使用倒序的原因：含有删除操作
    for (int i = andList->size() - 1; i >= 0; i--) {
        if ((*andList)[i]->getCondType() == querylangParser::RuleContentSearch) {
            m_isValid = true;
            ContentCond *cond = dynamic_cast<ContentCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.keys.append(cond->m_content);
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        if ((*andList)[i]->getCondType() == querylangParser::RuleDateSearchinfo) {
            m_isValid = true;
            DateInfoCond *cond = dynamic_cast<DateInfoCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            if (cond->m_compType == "==") {
                timestamp1 = cond->m_timestamp;
                timestamp2 = cond->m_timestamp2;
            } else if (cond->m_compType.contains(">")) {
                timestamp1 = cond->m_timestamp;
            } else {
                timestamp2 = cond->m_timestamp;
            }
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        if ((*andList)[i]->getCondType() == querylangParser::RulePathSearch) {
            m_isValid = true;
            PathCond *cond = dynamic_cast<PathCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.isContainPath = cond->m_isTruePath;
            m_entity.partPath      = cond->m_pathName;
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        if ((*andList)[i]->getCondType() == querylangParser::RuleNameSearch) {
            m_isValid = true;
            NameCond *cond = dynamic_cast<NameCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.keys.append(cond->m_name);
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        if ((*andList)[i]->getCondType() == querylangParser::RuleSizeSearch) {
            m_isValid = true;
            SizeCond *cond = dynamic_cast<SizeCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.fileCompType = cond->m_compType;
            m_entity.fileSize     = cond->m_fileSize;
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        if ((*andList)[i]->getCondType() == querylangParser::RuleTypeSearch) {
            m_isValid = true;
            TypeCond *cond = dynamic_cast<TypeCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            /*
             * #define DOCUMENT_GROUP      "text"
             * #define PICTURE_GROUP       "img"
             * #define AUDIO_GROUP         "msc"
             * #define VIDEO_GROUP         "vdo"
             * #define FILE_GROUP          "file"
             * #define FOLDER_GROUP        "fld"
             * #define APPLICATION_GROUP   "app"
             */
            // 优先匹配为类别，其实是扩展名
            if (cond->m_typeName == "song" || cond->m_typeName == "music") {
                m_entity.types.append(AUDIO_GROUP);
            } else if (cond->m_typeName == "paper" || cond->m_typeName == "document") {
                m_entity.types.append(DOCUMENT_GROUP);
            } else if (cond->m_typeName == "picture") {
                m_entity.types.append(PICTURE_GROUP);
            } else if (cond->m_typeName == "video") {
                m_entity.types.append(VIDEO_GROUP);
            } else if (cond->m_typeName == "file") {
                m_entity.types.clear();
            } else {
                m_entity.suffix = cond->m_typeName;
            }
            m_entity.isContainType = cond->m_isTrueType;
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        (*andList)[i]->merge4Engine();
    }
    if (timestamp1 != 0 || timestamp2 != _NOW) {
        m_entity.times.append(QPair<qint64, qint64>(timestamp1, timestamp2));
    }
}

bool FulltextCond::copyCondOut(BaseCond *cond) {
    if (cond->getCondType() == querylangParser::RuleTypeSearch) {
        cond->m_condType = BaseCond::RuleFulltext;
        cond->m_cond     = m_cond + cond->m_cond;
        SemanticEntity entity = m_entity;
        entity.types.append(cond->m_entity.types);
        entity.suffix = cond->m_entity.suffix;
        cond->m_entity   = entity;
        return true;
    }

    return false;
}

void FulltextCond::loadCond() {
    if (!m_isValid) {
        return;
    }

    m_fulltext.setEntity({m_entity});
    SemanticWorkerPrivate::QueryFunction func = {&m_fulltext, &FullTextQuery::run, this};
    m_querys->append(func);
}

void DslParserListener::enterEveryRule(antlr4::ParserRuleContext *ctx) {
    QString text = QString().fromStdString(ctx->getText());
    switch (ctx->getRuleIndex()) {
    case querylangParser::RulePrimary:
    case querylangParser::RuleBinaryExpression:
    case querylangParser::RuleDateSearchinfo:
    case querylangParser::RulePathSearch:
    case querylangParser::RuleNameSearch:
    case querylangParser::RuleSizeSearch:
    case querylangParser::RuleTypeSearch:
    case querylangParser::RuleDurationSearch:
    case querylangParser::RuleMetaSearch:
    case querylangParser::RuleQuantityCondition:
    case querylangParser::RuleContentSearch:
    case querylangParser::RuleFilename:
        m_prefix += "  ";
        //qDebug() << QString("%1=> enter(%2): %3").arg(m_prefix).arg(ruleContextId2String((int)ctx->getRuleIndex())).arg(text);
        break;
    }

    BaseCond *ptr = nullptr;
    if (ctx->getRuleIndex() == querylangParser::RulePrimary) {
        if (m_prefix.length() == 2) {
            m_cond->setCond(text);
            return;
        }
    }

    bool isAnd = true, isOr = true;
    switch (ctx->getRuleIndex()) {
    case querylangParser::RulePrimary:
        ptr = new BaseCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleBinaryExpression:
        ptr = new BinaryCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        isAnd = (m_cond->getCond().contains(QString("AND") + text));
        isOr  = (m_cond->getCond().contains(QString("OR") + text));
        if (isAnd) {
            m_tempPtr->addAndCond(ptr);
        } else if (isOr) {
            m_tempPtr->addOrCond(ptr);
        } else if (m_cond->getCond().contains(text + QString("OR"))) {
            m_tempPtr->addOrCond(ptr);
        } else {
            m_tempPtr->addAndCond(ptr);
        }
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleDateSearchinfo:
        ptr = new DateInfoCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RulePathSearch:
        ptr = new PathCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleNameSearch:
        ptr = new NameCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleSizeSearch:
        ptr = new SizeCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleTypeSearch:
        ptr = new TypeCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleDurationSearch:
        ptr = new DurationCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleMetaSearch:
        ptr = new MetaCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleQuantityCondition:
        ptr = new QuantityCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleContentSearch:
        ptr = new ContentCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleFilename:
        ptr = new FileNameCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    }
}

void DslParserListener::exitEveryRule(antlr4::ParserRuleContext *ctx) {
    QString text = QString().fromStdString(ctx->getText());
    switch (ctx->getRuleIndex()) {
    case querylangParser::RulePrimary:
    case querylangParser::RuleBinaryExpression:
    case querylangParser::RuleDateSearchinfo:
    case querylangParser::RulePathSearch:
    case querylangParser::RuleNameSearch:
    case querylangParser::RuleSizeSearch:
    case querylangParser::RuleTypeSearch:
    case querylangParser::RuleDurationSearch:
    case querylangParser::RuleMetaSearch:
    case querylangParser::RuleQuantityCondition:
    case querylangParser::RuleContentSearch:
    case querylangParser::RuleFilename:
        //qDebug() << QString("%1<= exit(%2): %3").arg(m_prefix).arg(ruleContextId2String((int)ctx->getRuleIndex())).arg(text);
        if (m_prefix.length() >= 2) {
            m_prefix = m_prefix.mid(0, m_prefix.length() - 2);
        }

        if (m_tempPtr) {
            m_tempPtr = m_tempPtr->getParent();
        }
        break;
    }
}

DslParser::DslParser(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys, FileResultsHandler *fileHandler,
                     SemanticWorkerPrivate *worker, QObject *parent) : QObject(parent), m_cond("", querys, worker) {
    qDebug() << QString("DslParser DslParser(%1)").arg(text);
    ANTLRInputStream input(text.toUtf8().toStdString());
    querylangLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    tokens.fill();
    querylangParser parser(&tokens);
    DslParserListener listener(&m_cond);
    tree::ParseTreeWalker walker;
    walker.walk(&listener, parser.query());
    //qInfo() << QString("before merge:\n%1").arg(m_cond.toString()).toUtf8().toStdString().c_str();
    m_cond.adjust();
    qInfo() << QString("after adjust:\n%1").arg(m_cond.toString()).toUtf8().toStdString().c_str();
}

bool DslParser::isMatch(const QString &text) {
    qInfo() << QString("DslParser isMatch(%1)").arg(text);
    bool ret = m_cond.isMatch(text);
    qInfo() << QString("DslParser isMatch(%1) ret(%2)").arg(text).arg(ret);
    return ret;
}
