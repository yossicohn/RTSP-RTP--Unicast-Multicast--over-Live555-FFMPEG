#include "stdafx.h"
#include "FFMPEGDecode.h"
#include "yuv2rgb.h"

/// <summary>
/// Initializes a new instance of the <see cref="FFMPEGDecode" /> class.
/// </summary>
FFMPEGDecode::FFMPEGDecode(void): m_pAVFrmtCntxt(0), m_pAVOptionDictionary(0), m_pAVCodec(0), m_pAVCodecCntxt(0), m_pAVFrame(0),
	m_uiFrameCount(0), m_VideoStreamIndex(-1)
{
	InitConvertTable();
}


/// <summary>
/// Finalizes an instance of the <see cref="FFMPEGDecode" /> class.
/// </summary>
FFMPEGDecode::~FFMPEGDecode(void)
{
}

/// <summary>
/// Inits the FFMPEG.
/// </summary>
/// <returns></returns>
bool FFMPEGDecode::InitFFMPEG()
{
	bool bReVal = true;

	  //register all the codecs 
    avcodec_register_all();	  
	av_register_all();

	return bReVal;
}




/// <summary>
/// Releases the FFMPEG resources.
/// </summary>
/// <returns></returns>
bool FFMPEGDecode::ReleaseFFMPEGResources()
	{
	bool bReVal = true;

	av_free_packet(&m_AVPacket);
	
	avcodec_close(m_pAVCodecCntxt);
  
    avcodec_free_frame(&m_pAVFrame);
	 
	av_free(m_pAVCodec);

	return bReVal;
}

/// <summary>
/// Prepares the media params.
/// </summary>
/// <returns></returns>
bool FFMPEGDecode::PrepareMediaParams()
	{
	bool bReVal = true;

	
	
	int ret = 0;
  
	av_dict_set(&m_pAVOptionDictionary, "video_size", "640x480", 0);
	av_dict_set(&m_pAVOptionDictionary, "pixel_format", "yuv420", 0);
			

	  /* find the mpeg1 video decoder */
	//m_pAVCodec = avcodec_find_decoder(CODEC_ID_MSMPEG4V2);
	//m_pAVCodec = avcodec_find_decoder(CODEC_ID_MPEG4);
	//m_pAVCodec = avcodec_find_decoder(CODEC_ID_MPEG1VIDEO);
	m_pAVCodec = avcodec_find_decoder(CODEC_ID_H264);
    if (!m_pAVCodec) {
       fprintf(stderr, "Codec not found\n");
       return false;
    }
	m_pAVCodecCntxt = avcodec_alloc_context3(m_pAVCodec);
    if (!m_pAVCodecCntxt) {
        fprintf(stderr, "Could not allocate video codec context\n");
       return false;
    }

    if(m_pAVCodec->capabilities&CODEC_CAP_TRUNCATED)
        m_pAVCodecCntxt->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */

	m_pAVCodecCntxt->flags2 |= CODEC_FLAG2_FAST;
    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */

    /* open it */

	m_pAVCodecCntxt->width = 1280;
	m_pAVCodecCntxt->height = 720;

	//m_pAVCodecCntxt->width = 640;
	//m_pAVCodecCntxt->height = 480;
	//m_pAVCodecCntxt->width = 320;
	//m_pAVCodecCntxt->height = 240;
	//m_pAVCodecCntxt->width = 1280;
	//m_pAVCodecCntxt->height = 720;

	m_pAVCodecCntxt->pix_fmt = PIX_FMT_YUV420P;

    if (avcodec_open2(m_pAVCodecCntxt, m_pAVCodec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
         return false;
    }

	 m_pAVFrame = avcodec_alloc_frame();
    if (!m_pAVFrame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
	avcodec_get_frame_defaults(m_pAVFrame);

	av_init_packet(&m_AVPacket);

	m_VideoStreamIndex = 0;
	return bReVal;
}

bool FFMPEGDecode::DecodeFrame()
{

	bool bReVal = true;

	int len = 0;
	int got_frame = 0;
	 char buf[1024];

	 
	len = avcodec_decode_video2(m_pAVCodecCntxt, m_pAVFrame, &got_frame, &m_AVPacket);
	 if (len < 0) {
        fprintf(stderr, "Error while decoding frame %d\n", m_uiFrameCount);
        return len;
    }

	 if (got_frame)
	{
        printf("Saving %sframe %3d\n", 0 ? "last " : "", m_uiFrameCount);
        fflush(stdout);

		if(true)
			{				
				FILE*  pf2= 0;
				char fname[256]={0};
				sprintf_s(fname, "OriginalYUV%d.raw", m_uiFrameCount);
				fopen_s(&pf2, fname, "wb");
				if(pf2)
				{
					fwrite(m_pAVFrame->data[0], 1, m_pAVFrame->linesize[0] *m_pAVCodecCntxt->coded_height, pf2);
					fwrite(m_pAVFrame->data[1], 1, m_pAVFrame->linesize[1] * m_pAVCodecCntxt->coded_height/2, pf2);
					fwrite(m_pAVFrame->data[2], 1, m_pAVFrame->linesize[2] * m_pAVCodecCntxt->coded_height/2, pf2);
					fflush(pf2);
					fclose(pf2);
				}
			}	

		const int size565 = m_pAVCodecCntxt->coded_height * m_pAVCodecCntxt->coded_width * 16;
		unsigned char* dst_ori = new unsigned char[size565];

		ConvertYUV2RGB565(	m_pAVFrame->data[0],m_pAVFrame->data[1],m_pAVFrame->data[2], dst_ori, m_pAVCodecCntxt->coded_width, m_pAVCodecCntxt->coded_height, m_pAVFrame->linesize[0]);
        /* the picture is allocated by the decoder, no need to free it */
		char* outfilename = "origYUV%d.pgm";
		sprintf_s(buf, sizeof(buf), outfilename, m_uiFrameCount);
        pgm_save(m_pAVFrame->data[0], m_pAVFrame->linesize[0],
                 m_pAVCodecCntxt->width, m_pAVCodecCntxt->height, buf);
        m_uiFrameCount++;
    }
	return bReVal;
}


bool FFMPEGDecode::DecodeFrame(AVPacket& rfAVPacket)
{

	bool bReVal = true;

	int len = 0;
	int got_frame = 0;
	 char buf[1024];

	 m_AVPacket.data = rfAVPacket.data;
	 m_AVPacket.size = rfAVPacket.size;
	 m_AVPacket.stream_index = rfAVPacket.stream_index;

	len = avcodec_decode_video2(m_pAVCodecCntxt, m_pAVFrame, &got_frame, &m_AVPacket);
	 if (len < 0) {
        fprintf(stderr, "Error while decoding frame %d\n", m_uiFrameCount);
        return len;
    }

	 if (got_frame)
	{
        printf("Saving %sframe %3d\n", 0 ? "last " : "", m_uiFrameCount);
        fflush(stdout);

		if(true)
			{				
				FILE*  pf2= 0;
				char fname[256]={0};
				sprintf_s(fname, "OriginalYUV%d.raw", m_uiFrameCount);
				fopen_s(&pf2, fname, "wb");
				if(pf2)
				{
					fwrite(m_pAVFrame->data[0], 1, m_pAVFrame->linesize[0] *m_pAVCodecCntxt->coded_height, pf2);
					fwrite(m_pAVFrame->data[1], 1, m_pAVFrame->linesize[1] * m_pAVCodecCntxt->coded_height/2, pf2);
					fwrite(m_pAVFrame->data[2], 1, m_pAVFrame->linesize[2] * m_pAVCodecCntxt->coded_height/2, pf2);
					fflush(pf2);
					fclose(pf2);
				}
			}			
        /* the picture is allocated by the decoder, no need to free it */
		char* outfilename = "origYUV%d.pgm";
		sprintf_s(buf, sizeof(buf), outfilename, m_uiFrameCount);
        pgm_save(m_pAVFrame->data[0], m_pAVFrame->linesize[0],
                 m_pAVCodecCntxt->width, m_pAVCodecCntxt->height, buf);
        m_uiFrameCount++;
    }
	return bReVal;
}




bool FFMPEGDecode::DecodeBuffer(unsigned char* pBuffer, int nBuffSize)
{

	bool bReVal = true;

	int len = 0;
	int got_frame = 0;
	 char buf[1024];
	 char* nalBuff = new char[nBuffSize+4]();
	 nalBuff[0] = 0;
	 nalBuff[1] = 0;
	 nalBuff[2] = 0;
	 nalBuff[3] = 1;
	 memcpy(nalBuff+4, pBuffer,nBuffSize);
	 bool	bFirst = true;

	
	m_AVPacket.data			= bFirst == true ? (uint8_t*)nalBuff	: NULL;
	m_AVPacket.size			= bFirst == true ? (nBuffSize+4)			: 0	;
	m_AVPacket.stream_index	= m_VideoStreamIndex;						
	// the stream it came from, based on the number in the AVFormatContext
	
	
	
	len = avcodec_decode_video2(m_pAVCodecCntxt, m_pAVFrame, &got_frame, &m_AVPacket);
	if (len < 0) 
	{
		fprintf(stderr, "Error while decoding frame %d\n", m_uiFrameCount);
		return len;
	}
	

	 if (got_frame)
	{
        printf("Saving %sframe %3d\n", 0 ? "last " : "", m_uiFrameCount);
        fflush(stdout);

		if(true)
			{				
				FILE*  pf2= 0;
				char fname[256]={0};
				sprintf_s(fname, "OriginalYUV%d.raw", m_uiFrameCount);
				fopen_s(&pf2, fname, "wb");
				if(pf2)
				{
					fwrite(m_pAVFrame->data[0], 1, m_pAVFrame->linesize[0] *m_pAVCodecCntxt->coded_height, pf2);
					fwrite(m_pAVFrame->data[1], 1, m_pAVFrame->linesize[1] * m_pAVCodecCntxt->coded_height/2, pf2);
					fwrite(m_pAVFrame->data[2], 1, m_pAVFrame->linesize[2] * m_pAVCodecCntxt->coded_height/2, pf2);
					fflush(pf2);
					fclose(pf2);

				}
			}			
		const int size565 = m_pAVCodecCntxt->coded_height * m_pAVCodecCntxt->coded_width * 16;
		unsigned char* dst_ori = new unsigned char[size565];

		if(m_uiFrameCount % 10 == 0)
		{
			ConvertYUV2RGB565(	m_pAVFrame->data[0],m_pAVFrame->data[1],m_pAVFrame->data[2], dst_ori, m_pAVCodecCntxt->coded_width, m_pAVCodecCntxt->coded_height, m_pAVFrame->linesize[0]);
			FILE*  pfrgb= 0;
			char fname[256]={0};
			sprintf_s(fname, "RGB%d.raw", m_uiFrameCount);
			fopen_s(&pfrgb, fname, "wb");
			if(pfrgb)
			{
				fwrite(dst_ori, 1, size565, pfrgb);
		
				fflush(pfrgb);
				fclose(pfrgb);

			}
		}
		
		/* the picture is allocated by the decoder, no need to free it */
		char* outfilename = "origYUV%d.pgm";
		sprintf_s(buf, sizeof(buf), outfilename, m_uiFrameCount);
        pgm_save(m_pAVFrame->data[0], m_pAVFrame->linesize[0],
                 m_pAVCodecCntxt->width, m_pAVCodecCntxt->height, buf);
        m_uiFrameCount++;
    }
	return bReVal;
}


void FFMPEGDecode::pgm_save(unsigned char *buf, int wrap, int xsize, int ysize, char *filename)
{
    FILE *f;
    int i;

    errno_t wrrr = fopen_s(&f, filename,"w");
    fprintf(f,"P5\n%d %d\n%d\n",xsize,ysize,255);
    for(i=0;i<ysize;i++)
        fwrite(buf + i * wrap,1,xsize,f);
    fclose(f);
}
