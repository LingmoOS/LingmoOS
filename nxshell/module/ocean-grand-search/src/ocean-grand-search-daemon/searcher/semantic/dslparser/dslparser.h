// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DSLPARSER_H
#define DSLPARSER_H

#include <QObject>
#include <QVariant>

#include <antlr4-runtime.h>
#include <querylangParser.h>
#include <querylangLexer.h>

#include "../fileresultshandler.h"
#include "../semanticworker_p.h"

#include "../database/anythingquery.h"
#include "../database/fulltextquery.h"
#include "../database/featurequery.h"
#include "../database/vectorquery.h"

namespace GrandSearch {
class SemanticWorkerPrivate;
// 对应RulePrimary
class BaseCond : public QObject {
    Q_OBJECT
public:
    explicit BaseCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                      SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    virtual ~BaseCond();
    void setCond(const QString &text) { m_cond = text; }
    QString getCond() { return m_cond; }
    int getCondType() { return m_condType; }
    void setParent(BaseCond *cond) { m_parent = cond; }
    BaseCond *getParent() { return m_parent; }
    void addAndCond(BaseCond *cond);
    void addOrCond(BaseCond *cond);
    // 调试使用
    virtual bool isMatch(const QString &text);
    // 某些条件没有存在的必要
    virtual bool isValid() { return m_isValid; }
    // 结构优化调整
    void adjust();
    // 合并Binary层级的条件
    virtual void mergeBinary();
    // 合并Base层级的条件
    virtual void mergeBase();
    // 合并各引擎的条件
    virtual void merge4Engine();
    // 为OR条件调整条件组合
    void adjust4OrCond();
    // 装载检索条件进入检索引擎
    virtual void loadCond();

    virtual QList<SemanticEntity> entityList();
    virtual void addMatchedItems(const MatchedItems &items);
    virtual const MatchedItems &getMatchedItems();
    virtual QString toString(int spaceCounts = 0);

public:
    // mergeBase环节，用于记录收敛情况
    static bool kIsNeedAdjustBase;
    QString m_cond;
    int m_condType = querylangParser::RulePrimary;
    QList<SemanticWorkerPrivate::QueryFunction> *m_querys = nullptr;
    SemanticWorkerPrivate *m_worker = nullptr;
    BaseCond *m_parent = nullptr;
    MatchedItems m_matchedItems;
    QList<BaseCond *> m_andCondList;
    QList<BaseCond *> m_orCondList;
    bool m_isValid = true;
    SemanticEntity m_entity;

public:
    enum {
        RuleAnything = 100,
        RuleFeature,
        RuleFulltext,
    };
};

// 对应RuleBinaryExpression
class BinaryCond : public BaseCond {
public:
    explicit BinaryCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                        SemanticWorkerPrivate *worker, QObject *parent = nullptr) : BaseCond(text, querys, worker, parent) {
        m_condType = querylangParser::RuleBinaryExpression;
    }
};

// 对应RuleDateSearchinfo  DATE <= CURRENT - "16 hour"
class DateInfoCond : public BaseCond {
public:
    explicit DateInfoCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    void loadCond() override;

public:
    QString m_compType;
    qint64 m_timestamp;
    qint64 m_timestamp2;

private:
    AnythingQuery m_anything;
};

// 对应RulePathSearch  PATH IS "Downloads"
class PathCond : public BaseCond {
public:
    explicit PathCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                      SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    void loadCond() override;

public:
    bool m_isTruePath = true;
    QString m_pathName;

private:
    AnythingQuery m_anything;
};

// 对应RuleNameSearch  NAME CONTAINS "名字"
class NameCond : public BaseCond {
public:
    explicit NameCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                      SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    void loadCond() override;

public:
    QString m_name;

private:
    AnythingQuery m_anything;
};

// 对应RuleSizeSearch  SIZE <= "4GB"
class SizeCond : public BaseCond {
public:
    explicit SizeCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                      SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    void loadCond() override;

public:
    QString m_compType;
    qint64 m_fileSize;

private:
    AnythingQuery m_anything;
};

// 对应RuleTypeSearch  TYPE IS "txt"
class TypeCond : public BaseCond {
public:
    explicit TypeCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                      SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    void loadCond() override;

public:
    bool m_isTrueType = true;
    QString m_typeName;

private:
    AnythingQuery m_anything;
};

// 对应RuleDurationSearch  DURATION > \"3 minute\"
class DurationCond : public BaseCond {
public:
    explicit DurationCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    static QString formatTime(qint64 msec);
    void loadCond() override;

public:
    QString m_compType;
    QString m_duration;

private:
    FeatureQuery m_feature;
};

// 对应RuleMetaSearch  META_TYPE IS \"ARTIST\" AND META_VALUE IS \"xxx\"
class MetaCond : public BaseCond {
public:
    explicit MetaCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                      SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    void loadCond() override;

public:
    bool m_isTrue = true;
    QString m_metaType;
    QString m_metaValue;

private:
    FeatureQuery m_feature;
};

// 对应RuleQuantityCondition  QUANTITY = 10
class QuantityCond : public BaseCond {
public:
    explicit QuantityCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    int getQuantity() { return m_quantity; }

private:
    int m_quantity = 0;
};

// 对应RuleContentSearch  CONTENT CONTAINS "read"
class ContentCond : public BaseCond {
public:
    explicit ContentCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                         SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    void loadCond() override;

public:
    QString m_content;

private:
    FullTextQuery m_fulltext;
};

// 对应RuleFilename
class FileNameCond : public BaseCond {
public:
    explicit FileNameCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent = nullptr) : BaseCond(text, querys, worker, parent) {}
};

// 对应合并后的Anything
class AnythingCond : public BaseCond {
public:
    explicit AnythingCond(QList<BaseCond *> *andList, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    bool copyCondOut(BaseCond *cond);
    void loadCond() override;

private:
    AnythingQuery m_anything;
};

// 对应合并后的Feature
class FeatureCond : public BaseCond {
public:
    explicit FeatureCond(QList<BaseCond *> *andList, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    bool copyCondOut(BaseCond *cond);
    void loadCond() override;

private:
    FeatureQuery m_feature;
};

// 对应合并后的Fulltext
class FulltextCond : public BaseCond {
public:
    explicit FulltextCond(QList<BaseCond *> *andList, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    bool copyCondOut(BaseCond *cond);
    void loadCond() override;

private:
    FullTextQuery m_fulltext;
};

class DslParserListener : public antlr4::tree::ParseTreeListener {
public:
    static QString ruleContextId2String(int id) {
        switch (id) {
        case querylangParser::RuleQuery:
            return "RuleQuery";
        case querylangParser::RuleInvalideQuery:
            return "RuleInvalideQuery";
        case querylangParser::RulePrimary:
            return "RulePrimary";
        case querylangParser::RuleBinaryExpression:
            return "RuleBinaryExpression";
        case querylangParser::RuleSearchCondition:
            return "RuleSearchCondition";
        case querylangParser::RuleDateSearch:
            return "RuleDateSearch";
        case querylangParser::RuleBinaryDateSearch:
            return "RuleBinaryDateSearch";
        case querylangParser::RuleDateSearchinfo:
            return "RuleDateSearchinfo";
        case querylangParser::RuleAbsolutedate:
            return "RuleAbsolutedate";
        case querylangParser::RuleRelativelydate:
            return "RuleRelativelydate";
        case querylangParser::RulePathSearch:
            return "RulePathSearch";
        case querylangParser::RuleNameSearch:
            return "RuleNameSearch";
        case querylangParser::RuleSizeSearch:
            return "RuleSizeSearch";
        case querylangParser::RuleTypeSearch:
            return "RuleTypeSearch";
        case querylangParser::RuleDurationSearch:
            return "RuleDurationSearch";
        case querylangParser::RuleMetaSearch:
            return "RuleMetaSearch";
        case querylangParser::RuleQuantityCondition:
            return "RuleQuantityCondition";
        case querylangParser::RuleContentSearch:
            return "RuleContentSearch";
        case querylangParser::RuleComparison_type:
            return "RuleComparison_type";
        case querylangParser::RuleFilename:
            return "RuleFilename";
        case querylangParser::RuleString:
            return "RuleString";
        case querylangParser::RuleIs_or_not:
            return "RuleIs_or_not";
        default:
            return "";
        }
    }

    explicit DslParserListener(BaseCond *cond) {
        m_cond    = cond;
        m_tempPtr = cond;
    }
    void visitTerminal(antlr4::tree::TerminalNode *node) override {}
    void visitErrorNode(antlr4::tree::ErrorNode *node) override {}
    void enterEveryRule(antlr4::ParserRuleContext *ctx) override;
    void exitEveryRule(antlr4::ParserRuleContext *ctx) override;

private:
    QString m_prefix;
    BaseCond *m_cond    = nullptr;
    BaseCond *m_tempPtr = nullptr;
};

class DslParser : public QObject {
    Q_OBJECT
public:
    explicit DslParser(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys, FileResultsHandler *fileHandler,
                       SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    ~DslParser() {}
    bool isMatch(const QString &text);
    QList<SemanticEntity> entityList() { return m_cond.entityList(); }
    const MatchedItems &getMatchedItems() { return m_cond.getMatchedItems(); }

private:
    BaseCond m_cond;
};
}

#endif // DSLPARSER_H
