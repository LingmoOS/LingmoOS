#ifndef REFLEX_GLOBAL_H
#define REFLEX_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(REFLEX_LIBRARY)
#  define REFLEXSHARED_EXPORT Q_DECL_EXPORT
#else
#  define REFLEXSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // REFLEX_GLOBAL_H
