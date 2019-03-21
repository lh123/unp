#include "unp.h"
#include <limits.h>
#include <fcntl.h>

char *px_ipc_name(const char *name) {
    char *dst = malloc(PATH_MAX);
    if (dst == NULL) {
        return NULL;
    }
    char *dir = getenv("PX_IPC_NAME");
    if (dir == NULL) {
        dir = "/tmp";
    }
    char *slash = dir[strlen(dir) - 1] == '/' ? "" : "/";
    snprintf(dst, PATH_MAX, "%s%s%s", dir, slash, name);
    return dst;
}
