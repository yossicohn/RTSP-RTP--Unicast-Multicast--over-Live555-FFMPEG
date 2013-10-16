#pragma once
#include <math.h>
#include <stdint.h>
extern "C"
{
	
	#include <libavformat/avformat.h>
	#include <libavutil/opt.h>
	#include <libavcodec/avcodec.h>
	#include <libavutil/channel_layout.h>
	#include <libavutil/common.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/samplefmt.h>
}

class FFMPEGDecode
{
public:
	FFMPEGDecode(void);
	virtual ~FFMPEGDecode(void);

	virtual bool InitFFMPEG();
	virtual bool ReleaseFFMPEGResources();
	virtual bool PrepareMediaParams();
	virtual bool DecodeFrame();
	virtual bool DecodeFrame(AVPacket& packet);
	virtual bool DecodeBuffer(unsigned char* buffer, int Size);
	
protected:

	static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize, char *filename);

	int					m_VideoStreamIndex;
	unsigned int		m_uiFrameCount;
	AVFormatContext*	m_pAVFrmtCntxt;
	AVDictionary*		m_pAVOptionDictionary;
	AVCodec*			m_pAVCodec;
	AVCodecContext*		m_pAVCodecCntxt;
	AVFrame*			m_pAVFrame;
	AVPacket			m_AVPacket;
};

