// H264Decode.cpp: 定义控制台应用程序的入口点。
//
/*
#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include "Nalu2BGR.h"
#include "H264.h"
#include <chrono>

using namespace cv;

//g++ H264.cpp main.cpp Nalu2BGR.cpp -o decode `pkg-config opencv --cflags --libs sdl2` -lavformat -lavcodec -lavutil  -lz -lm -lpthread -I'/usr/local/include' -lswresample -lswresample -lswscale

int main(int argc, char* argv[]) {
	char filename[] = "out.h264";

	AVFormatContext *fmt_ctx = nullptr;
	avformat_open_input(&fmt_ctx, filename, nullptr, nullptr);//打开一个视频
	avformat_find_stream_info(fmt_ctx, nullptr);
	AVStream *stream = fmt_ctx->streams[0];
	

	OpenBitstreamFile(filename);
	NALU_t *nal;
	char fName[300];
	int Frame = 0;
	nal = AllocNALU(9000000);//为结构体nalu_t及其成员buf分配空间。返回值为指向nalu_t存储空间的指针 
	Nalu2BGR_Info * pNalu2BGR_Info = H264_Init(stream);

	unsigned char *outputbuf = (unsigned char *)calloc(WIDTH * HEIGHT * 3, sizeof(char));
	unsigned int outsize = 0;
	unsigned char *m_pData = (unsigned char *)calloc(WIDTH * HEIGHT * 3, sizeof(char));
	int sizeHeBing = 0;
	long int begin = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	while (!feof(getFile())) {
		GetAnnexbNALU(nal);//每执行一次，文件的指针指向本次找到的NALU的末尾，
						   //下一个位置即为下个NALU的起始码0x000001 
		dump(nal);//输出NALU长度和TYPE 
		sprintf(fName, "dump[Len=%d][%d].txt", nal->len, Frame);
		memset(m_pData, 0, 4);
		m_pData[3] = 1;
		memcpy(m_pData + 4, nal->buf, nal->len);
		sizeHeBing = nal->len + 4;
		Frame++;
		int ret = H264_2_RGB(pNalu2BGR_Info, (char *)m_pData, sizeHeBing, outputbuf, &outsize);
		if (ret != 0) continue;
		cv::Mat image = cv::Mat(pNalu2BGR_Info->pCodecCtx->height, pNalu2BGR_Info->pCodecCtx->width, CV_8UC3);
		memcpy(image.data, outputbuf, pNalu2BGR_Info->pCodecCtx->height * pNalu2BGR_Info->pCodecCtx->width * 3);
		cv::imshow("xxx", image);
		cv::waitKey(40);
	}
	long int end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - begin;
	std::cout << end << std::endl;
	FreeNALU(nal);
	return 0;
}


*/

/*
* Copyright (c) 2001 Fabrice Bellard
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

/**
* @file
* video decoding with libavcodec API example
*
* @example decode_video.c
*/




#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opencv2/opencv.hpp"
#include "windows.h"
#include "commdlg.h"


#define INBUF_SIZE 4096

#ifdef __cplusplus
extern "C"
{ 
#pragma comment (lib, "avcodec.lib")
#pragma comment (lib, "avdevice.lib")
#pragma comment (lib, "avfilter.lib")
#pragma comment (lib, "avformat.lib")
#pragma comment (lib, "avutil.lib")
#pragma comment (lib, "swresample.lib")
#pragma comment (lib, "swscale.lib")
#include <libavcodec/avcodec.h>
#include "libavformat/avformat.h" 
#include "libswscale/swscale.h" 
#include "libavutil/avutil.h" 
};

#endif

cv::VideoWriter *writer;

std::string TCHAR2STRING(TCHAR *STR) {
	int iLen = WideCharToMultiByte(CP_ACP, 0,STR, -1, NULL, 0, NULL, NULL);

	char* chRtn = new char[iLen * sizeof(char)];

	WideCharToMultiByte(CP_ACP, 0, STR, -1, chRtn, iLen, NULL, NULL);

	std::string str(chRtn);

	return str;
}

std::string GetFileName() {
	OPENFILENAME ofn;      // 公共对话框结构。     
	TCHAR szFile[MAX_PATH]; // 保存获取文件名称的缓冲区。               
							// 初始化选择文件对话框。     
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = (LPCWSTR)"All(*.*)\0*.*\0Text(*.h264)\0*.h264\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	//ofn.lpTemplateName =  MAKEINTRESOURCE(ID_TEMP_DIALOG);    
	// 显示打开选择文件对话框。
	if (GetOpenFileName(&ofn))
	{
		//显示选择的文件。 
		std::cout << szFile << std::endl;
		//OutputDebugString(szFile);    
		//OutputDebugString((LPCWSTR)"\r\n");
	}

	std::string res = TCHAR2STRING(szFile);

	return res;
}

static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
	char *filename)
{
	FILE *f;
	int i;

	f = fopen(filename, "w");
	//fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
	for (i = 0; i < ysize; i++)
		fwrite(buf + i * wrap, 1, xsize, f);

	printf("%d", sizeof(buf));

	//cv::Mat image(Size(xsize, ysize), CV_8UC3);
	//image.data = (uchar*)
	//memcpy(image.data, buf, xsize * ysize * 3);
	//if (!image.empty())
	//cv::imshow("xxx", image);
	//cv::waitKey(40);
	
	fclose(f);
}

bool pause = false;

static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt,
	const char *filename)
{
	char buf[1024];
	int ret;

	ret = avcodec_send_packet(dec_ctx, pkt);
	if (ret < 0) {
		fprintf(stderr, "Error sending a packet for decoding\n");
		exit(1);
	}

	while (ret >= 0) {
		ret = avcodec_receive_frame(dec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			fprintf(stderr, "Error during decoding\n");
			exit(1);
		}

		printf("saving frame %3d\n", dec_ctx->frame_number);
		fflush(stdout);

		/* the picture is allocated by the decoder. no need to
		free it */
		snprintf(buf, sizeof(buf), "%s-%d", filename, dec_ctx->frame_number);

		int width = frame->width, height = frame->height;
		cv::Mat tmp_img = cv::Mat::zeros(height * 3 / 2, width, CV_8UC1);
		memcpy(tmp_img.data, frame->data[0], width*height);
		memcpy(tmp_img.data + width * height, frame->data[1], width*height / 4);
		memcpy(tmp_img.data + width * height * 5 / 4, frame->data[2], width*height / 4);

		//cv::imshow("", tmp_img);
		cv::Mat bgr;
		cv::cvtColor(tmp_img, bgr, CV_YUV2BGR_I420);
		int si = sizeof(bgr.data);
		std::cout << "size : " << si << std::endl;
		//if (!si)
		//cv::imwrite("test.jpg", bgr);
		//if (si > 100)
		writer->write(bgr);
		cv::imshow("Video", bgr);
		int key = cv::waitKey(10);
		if (key == 's') {
			if (pause)
			cv::waitKey();
			else pause = !pause;
		}

		//pgm_save(frame->data[0], frame->linesize[0],frame->width, frame->height, buf);
	}
}

int main(int argc, char **argv)
{
	std::string filename, outfilename;
	const AVCodec *codec;
	AVCodecParserContext *parser;
	AVCodecContext *c = NULL;
	FILE *f;
	AVFrame *frame;
	uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
	uint8_t *data;
	size_t   data_size;
	int ret;
	AVPacket *pkt;
	cv::namedWindow("Video", CV_WINDOW_AUTOSIZE);
	filename = GetFileName().c_str();
//	if (argc <= 2) {
//		fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
//		exit(0);
//	}
	//filename = argv[1];
	//outfilename = argv[2];
	//filename = "out.h264";
	std::cout << "0:" << filename << std::endl;
	outfilename = filename.substr(0, filename.find_last_of("\\")) + "\\decoded.avi";
	//CV_FOURCC('M', 'J', 'P', 'G')
	writer = new cv::VideoWriter();
	int ret1 = writer->open(outfilename, -1, 30.0, cv::Size(1280, 720));
	//int ret1 = writer->open(outfilename, CV_FOURCC('D', 'I', 'V', '3'), 30.0, cv::Size(1280, 720));
	

	if (!writer->isOpened()) {
		std::cout << "No open";
	}

 	pkt = av_packet_alloc();
	if (!pkt)
		exit(1);

	/* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
	memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

	/* find the MPEG-1 video decoder */
	codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec) {
		fprintf(stderr, "Codec not found\n");
		exit(1);
	}
	std::cout << "1:" << filename << std::endl;

	parser = av_parser_init(codec->id);
	if (!parser) {
		fprintf(stderr, "parser not found\n");
		exit(1);
	}
	std::cout << "2:" << filename << std::endl;

	c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	/* For some codecs, such as msmpeg4 and mpeg4, width and height
	MUST be initialized there because this information is not
	available in the bitstream. */
	std::cout << "3:" << filename << std::endl;

	/* open it */
	if (avcodec_open2(c, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		exit(1);
	}
	std::cout << "4:" << filename << std::endl;

	//std::cout << filename << std::endl;
	f = fopen(filename.c_str(), "rb");
	if (!f) {
		fprintf(stderr, "Could not open %s\n", filename);
		exit(1);
	}

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}

	while (!feof(f)) {
		/* read raw data from the input file */
		data_size = fread(inbuf, 1, INBUF_SIZE, f);
		if (!data_size)
			break;

		/* use the parser to split the data into frames */
		data = inbuf;
		while (data_size > 0) {
			ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
				data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
			if (ret < 0) {
				fprintf(stderr, "Error while parsing\n");
				exit(1);
			}
			printf("%d\n", pkt->size);

			data += ret;
			data_size -= ret;

			if (pkt->size)
				decode(c, frame, pkt, outfilename.c_str());
		}
	}

	/* flush the decoder */
	decode(c, frame, NULL, outfilename.c_str());

	fclose(f);

	av_parser_close(parser);
	avcodec_free_context(&c);
	av_frame_free(&frame);
	av_packet_free(&pkt);

	writer->release();

	return 0;
}
