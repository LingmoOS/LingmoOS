#ifndef KSTRINGHANDLERTEST_H
#define KSTRINGHANDLERTEST_H

#include <QObject>

class KStringHandlerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void capwords();
    void tagURLs();
    void perlSplitTextSep();
    void perlSplitRegexSep();
    void obscure();
    void preProcessWrap_data();
    void preProcessWrap();
    void logicalLength_data();
    void logicalLength();
    void lsqueeze();
    void lsqueeze_data();
    void csqueeze();
    void csqueeze_data();
    void rsqueeze();
    void rsqueeze_data();

private:
    static QString test;
};

#endif
