#include "redismodule.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Read everything popen() gives us into a dynamically grown buffer. */
static char *read_all(FILE *fp, size_t *out_len) {
    size_t cap = 4096;
    size_t len = 0;
    char *buf = RedisModule_Alloc(cap);
    if (buf == NULL) return NULL;

    size_t n;
    char chunk[4096];
    while ((n = fread(chunk, 1, sizeof(chunk), fp)) > 0) {
        if (len + n > cap) {
            while (len + n > cap) cap *= 2;
            char *grown = RedisModule_Realloc(buf, cap);
            if (grown == NULL) {
                RedisModule_Free(buf);
                return NULL;
            }
            buf = grown;
        }
        memcpy(buf + len, chunk, n);
        len += n;
    }

    *out_len = len;
    return buf;
}

/* system.exec <command> */
int SystemExec_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc != 2) {
        return RedisModule_WrongArity(ctx);
    }

    size_t cmd_len;
    const char *cmd = RedisModule_StringPtrLen(argv[1], &cmd_len);

    /* popen needs a NUL-terminated string; RedisModuleString may not be. */
    char *cmd_z = RedisModule_Alloc(cmd_len + 1);
    if (cmd_z == NULL) {
        return RedisModule_ReplyWithError(ctx, "ERR out of memory");
    }
    memcpy(cmd_z, cmd, cmd_len);
    cmd_z[cmd_len] = '\0';

    FILE *fp = popen(cmd_z, "r");
    if (fp == NULL) {
        int e = errno;
        RedisModule_Free(cmd_z);
        return RedisModule_ReplyWithError(ctx, strerror(e));
    }

    size_t out_len = 0;
    char *out = read_all(fp, &out_len);
    int status = pclose(fp);
    RedisModule_Free(cmd_z);

    if (out == NULL) {
        return RedisModule_ReplyWithError(ctx, "ERR failed to read command output");
    }

    (void)status;

    RedisModule_ReplyWithStringBuffer(ctx, out, out_len);
    RedisModule_Free(out);
    return REDISMODULE_OK;
}

/* Entry point. Redis calls this when the .so is loaded. */
int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    REDISMODULE_NOT_USED(argv);
    REDISMODULE_NOT_USED(argc);

    if (RedisModule_Init(ctx, "system", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, "system.exec",
                                  SystemExec_RedisCommand,
                                  "readonly", 0, 0, 0) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}
