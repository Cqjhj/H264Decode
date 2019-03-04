extern "C" 
{ 
#include "libavcodec/avcodec.h" 
#include "libavformat/avformat.h" 
#include "libswscale/swscale.h" 
#include "libavutil/avutil.h" 
}; 


#define WIDTH 1280
#define HEIGHT 720
#define FRAME 25

typedef struct { 
	AVCodec *pCodec = NULL; 
	AVCodecContext *pCodecCtx = NULL; 
	SwsContext *img_convert_ctx = NULL; 
	AVFrame *pFrame = NULL; 
	AVFrame *pFrameBGR = NULL; 
	int first; 
	unsigned char * outBuffer; 
}Nalu2BGR_Info;
 
Nalu2BGR_Info * H264_Init(AVStream *stream);
int H264_2_RGB(Nalu2BGR_Info * pNalu2BGR_Info, char *inputbuf, int frame_size, unsigned char *outputbuf, unsigned int*outsize); 
void H264_Release(Nalu2BGR_Info * pNalu2BGR_Info);
