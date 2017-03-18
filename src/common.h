#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

#define LOGT(...) printf("T ["MODULE_NAME"]: " __VA_ARGS__)
#define LOGD(...) printf("D ["MODULE_NAME"]: " __VA_ARGS__)
#define LOGW(...) fprintf(stderr, "W ["MODULE_NAME"]: " __VA_ARGS__)
#define LOGE(...) fprintf(stderr, "E ["MODULE_NAME"]: " __VA_ARGS__)

#endif // COMMON_H
