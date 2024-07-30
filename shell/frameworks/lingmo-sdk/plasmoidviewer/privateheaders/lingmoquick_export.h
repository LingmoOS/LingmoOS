
#ifndef LINGMOQUICK_EXPORT_H
#define LINGMOQUICK_EXPORT_H

#ifdef LINGMOQUICK_STATIC_DEFINE
#define LINGMOQUICK_EXPORT
#define LINGMOQUICK_NO_EXPORT
#else
#ifndef LINGMOQUICK_EXPORT
#ifdef KF6LingmoQuick_EXPORTS
/* We are building this library */
#define LINGMOQUICK_EXPORT __attribute__((visibility("default")))
#else
/* We are using this library */
#define LINGMOQUICK_EXPORT __attribute__((visibility("default")))
#endif
#endif

#ifndef LINGMOQUICK_NO_EXPORT
#define LINGMOQUICK_NO_EXPORT __attribute__((visibility("hidden")))
#endif
#endif

#ifndef LINGMOQUICK_DEPRECATED
#define LINGMOQUICK_DEPRECATED __attribute__((__deprecated__))
#endif

#ifndef LINGMOQUICK_DEPRECATED_EXPORT
#define LINGMOQUICK_DEPRECATED_EXPORT LINGMOQUICK_EXPORT LINGMOQUICK_DEPRECATED
#endif

#ifndef LINGMOQUICK_DEPRECATED_NO_EXPORT
#define LINGMOQUICK_DEPRECATED_NO_EXPORT LINGMOQUICK_NO_EXPORT LINGMOQUICK_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
#define LINGMOQUICK_NO_DEPRECATED
#endif

#endif
