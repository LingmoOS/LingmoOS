#ifndef SERVERDLL_H
#define SERVERDLL_H
#include <QtCore/qglobal.h>

//定义DLL_SHARE,使用者可以不用再处理符号的导入和导出细节
#ifdef SERVERDLL_LIB
#define SERVERDLL_SHARE Q_DECL_EXPORT
#else
#define SERVERDLL_SHARE Q_DECL_IMPORT
#endif
#endif // SERVERDLL_H
