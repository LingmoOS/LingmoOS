#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UNKNOWN,
    AVAILABLE = 0,
    NOT_INSTALLED,
    NOT_SUPPORTED
} FeatureStatus;

}

#endif // COMMON_H
