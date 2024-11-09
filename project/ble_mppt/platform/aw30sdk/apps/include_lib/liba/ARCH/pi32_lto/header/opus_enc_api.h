
#ifndef _OPUS_ENCODE_API_H_
#define _OPUS_ENCODE_API_H_

#ifndef u16
#define  u32  unsigned int
#define  u16  unsigned short
#define  s16  short
#define  u8  unsigned char
#endif


// typedef struct _OPUS_EN_FILE_IO_
// {
// 	void *priv;
// 	u16 (*input_data)(void *priv,s16 *buf,u8 channel,u16 len);   //short
// 	u32 (*output_data)(void *priv,u8 *buf,u16 len);  //bytes
// }OPUS_EN_FILE_IO;


typedef struct __OPUS_ENC_OPS {
    u32(*need_buf)(u16 samplerate);       //samplerate=16k   ignore
    u32(*open)(u8 *ptr, EN_FILE_IO *audioIO, u8 quality, u16 sample_rate);
    //1. quality:(bit3_bit0)bitrate     0:16kbps    1:32kbps    2:64kbps
    //   quality: MSB_2:(bit7_bit6)     format_mode    //0:百度_无头.                   1:酷狗_eng+range.       2:ogg封装.
    //   quality:LMSB_2:(bit5_bit4)     low_complexity //0:高复杂度,高质量.兼容之前库.  1:低复杂度,低质量.
    //2. sample_rate         sample_rate=16k         ignore
    u32(*run)(u8 *ptr);
} OPUS_ENC_OPS;


extern OPUS_ENC_OPS *get_opus_enc_ops(void);



/*
u16 mm_input(void* priv, s16* buf, u16 len)
{
	int  rlen = 0;
	rlen = fread(buf,1,len*2,fin);
	return (rlen >> 1);
}
*/




#endif


