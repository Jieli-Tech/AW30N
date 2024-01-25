#ifndef __SRC_H__
#define __SRC_H__

#include "typedef.h"
#include "src_para.h"

#define SRC_MAX_POINT  (32*4)
#define SRC_MAX_LEN    (SRC_MAX_POINT + 10)
#define SRC_MAX_BUFF   (SRC_MAX_LEN + 2)

typedef struct _SRC_INFO {
    u32 phase;
    u32 fltb_offset;
    u32 olen;
    s16 fltb_buf[24];
    // JL_SRC_TypeDef src
} SRC_INFO;

typedef struct _SRC_IO_CONTEXT {
    void *priv;
    int(*output)(void *priv, void *data, int len);
} SRC_IO_CONTEXT;


typedef struct _SRC_PARA_STRUCT_ {
    unsigned short insample;
    unsigned short outsample;
    signed short insample_inc;
    unsigned short max_in;
} SRC_PARA_STRUCT;

typedef struct  _SRC_STUCT_API_ {
    unsigned int(*need_buf)();
    // int *(*open)(unsigned int *ptr, int in_sr, int out_sr, SRC_IO_CONTEXT *rsio);
    int *(*open)(unsigned int *ptr, SRC_PARA_API *para, SRC_IO_CONTEXT *rsio);
    int (*run)(unsigned int *ptr, short *inbuf, int len);
    u32(*config)(void *work_buf, u32 cmd, void *parm);
} SRC_STUCT_API;


typedef struct _SRC_BUFF {
    SRC_IO_CONTEXT *io;
    SRC_PARA_STRUCT para;
    SRC_STUCT_API   *ops;
    SRC_INFO        info;
    u8              buff[SRC_MAX_BUFF * 2];
    u16             rmlen;
    u16            rmcnt;
} SRC_BUFF;

const SRC_STUCT_API *get_src_context(void);

enum SRC_CMD {
    SRC_CMD_NONE = 0,
    SRC_CMD_INSR_SET,
    SRC_CMD_INSR_INC_SET,
    SRC_CMD_INSR_INC_INIT,
    SRC_CMD_GET_INSAMPLE,
    SRC_CMD_GET_OUTSAMPLE,
    SRC_CMD_GET_INSAMPLE_INC,
};


#endif

