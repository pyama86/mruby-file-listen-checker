/* ** mrb_filelistencheck.c - FileListenCheck class
**
** Copyright (c) pyama86 2019
**
** See Copyright Notice in LICENSE
*/

#include "mruby.h"
#include "mruby/data.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define BUFSIZE 512
#define DONE mrb_gc_arena_restore(mrb, 0);
typedef struct {
  int port;
  char *addr;
} mrb_filelistencheck_data;

static const struct mrb_data_type mrb_filelistencheck_data_type = {
    "mrb_filelistencheck_data",
    mrb_free,
};

// printf("%s:%d\n", local_string, local_port);
// refs: https://github.com/deployfu/nethogs/blob/master/conninode.cpp
#define LISTEN(ver)                                                                                \
  static mrb_value mrb_is_listen##ver(mrb_state *mrb, mrb_value self)                              \
  {                                                                                                \
    FILE *tcp;                                                                                     \
    char local_addr[INET##ver##_ADDRSTRLEN];                                                       \
    int local_port;                                                                                \
    unsigned int state = 0;                                                                        \
    mrb_filelistencheck_data *data = DATA_PTR(self);                                               \
    struct in6_addr in6_local;                                                                     \
    struct in6_addr result_addr_local;                                                             \
    char local_string[INET##ver##_ADDRSTRLEN];                                                     \
    mrb_value ret = mrb_false_value();                                                             \
    tcp = fopen("/proc/net/tcp" #ver, "r");                                                        \
    if (!tcp)                                                                                      \
      mrb_raise(mrb, E_RUNTIME_ERROR, "can't open /proc/net/tcp" #ver);                            \
                                                                                                   \
    char buf[BUFSIZE];                                                                             \
    while (fgets(buf, BUFSIZE, tcp)) {                                                             \
      int matches = sscanf(buf,                                                                    \
                           "%*d: %64[0-9A-Fa-f]:%X %*64[0-9A-Fa-f]:%*X %X %*X:%*X %*X:%*X %*X "    \
                           "%*d %*d %*ld %*512s\n",                                                \
                           local_addr, &local_port, &state);                                       \
      if (matches != 3) {                                                                          \
        continue;                                                                                  \
      }                                                                                            \
      if (strlen(local_addr) > 8) {                                                                \
        sscanf(local_addr, "%08X%08X%08X%08X", &in6_local.s6_addr32[0], &in6_local.s6_addr32[1],   \
               &in6_local.s6_addr32[2], &in6_local.s6_addr32[3]);                                  \
        if ((in6_local.s6_addr32[0] == 0x0) && (in6_local.s6_addr32[1] == 0x0) &&                  \
            (in6_local.s6_addr32[2] == 0xFFFF0000)) {                                              \
          result_addr_local = *((struct in6_addr *)&(in6_local.s6_addr32[3]));                     \
        } else {                                                                                   \
          result_addr_local = in6_local;                                                           \
        }                                                                                          \
      } else {                                                                                     \
        sscanf(local_addr, "%X", (unsigned int *)&result_addr_local);                              \
      }                                                                                            \
      if (data->port == local_port && state == TCP_LISTEN) {                                       \
        inet_ntop(AF_INET##ver, &result_addr_local, local_string, INET##ver##_ADDRSTRLEN);         \
        if (strcmp(local_string, data->addr) == 0) {                                               \
          ret = mrb_true_value();                                                                  \
          break;                                                                                   \
        }                                                                                          \
      }                                                                                            \
    }                                                                                              \
    fclose(tcp);                                                                                   \
    return ret;                                                                                    \
  }

LISTEN()
LISTEN(6)

static mrb_value mrb_filelistencheck_init(mrb_state *mrb, mrb_value self)
{
  mrb_filelistencheck_data *data;

  data = (mrb_filelistencheck_data *)DATA_PTR(self);
  if (data) {
    mrb_free(mrb, data);
  }
  DATA_TYPE(self) = &mrb_filelistencheck_data_type;
  DATA_PTR(self) = NULL;
  data = (mrb_filelistencheck_data *)mrb_malloc(mrb, sizeof(mrb_filelistencheck_data));
  mrb_get_args(mrb, "zi", &data->addr, &data->port);
  DATA_PTR(self) = data;

  return self;
}

static mrb_value mrb_filelistencheck_hello(mrb_state *mrb, mrb_value self)
{
  mrb_filelistencheck_data *data = DATA_PTR(self);

  return mrb_fixnum_value(data->port);
}

void mrb_mruby_file_listen_checker_gem_init(mrb_state *mrb)
{
  struct RClass *filelistencheck;
  filelistencheck = mrb_define_class(mrb, "FileListenCheck", mrb->object_class);
  mrb_define_method(mrb, filelistencheck, "initialize", mrb_filelistencheck_init, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, filelistencheck, "listen?", mrb_is_listen, MRB_ARGS_NONE());
  mrb_define_method(mrb, filelistencheck, "listen6?", mrb_is_listen6, MRB_ARGS_NONE());
  mrb_define_method(mrb, filelistencheck, "hello", mrb_filelistencheck_hello, MRB_ARGS_NONE());
  DONE;
}

void mrb_mruby_file_listen_checker_gem_final(mrb_state *mrb)
{
}
