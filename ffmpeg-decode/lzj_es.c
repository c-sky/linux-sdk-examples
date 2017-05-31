#include <stdio.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

int _main(int argc, char **argv)
{
	av_register_all();
	avformat_network_init();

	AVFormatContext *pFormatCtx;
	pFormatCtx = avformat_alloc_context();

	// Open video file
	if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) != 0)
	{
		printf("Couldn't open input stream.\n");
  		return -1; // Couldn't open file
	}

	if(avformat_find_stream_info(pFormatCtx,NULL)<0)
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}


	// Find the first video stream
	int i,videoStream;
	videoStream = -1;
	for(i=0; i < pFormatCtx->nb_streams; i++)
	{
		if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStream = i;
			break;
		}
	}
	if(videoStream == -1)
	{
		printf("Didn't find a video stream.\n");
		return -1;
	}

	AVCodecContext *pCodecCtx;
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;

	AVCodec *pCodec;
	// Find the decoder for the video stream
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL)
	{
		printf("Codec not found.\n");
		return -1;
	}
	// Open codec
	if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		printf("Could not open codec.\n");
		return -1;
	}

// store data from stream
	AVFrame *pFrame, *pFrameYUV;
	pFrame = av_frame_alloc(); //avcodec_alloc_frame();
	pFrameYUV = av_frame_alloc();

	uint8_t *buffer;
	int numBytes;
	numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
// Assign appropriate parts of buffer to image planes in pFrameRGB
// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
// of AVPicture
	avpicture_fill((AVPicture *)pFrameYUV, buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);


	int frameFinished,ret;
	int  frame_cnt=0;
	AVPacket packet;
	//packet=(AVPacket *)av_malloc(sizeof(AVPacket));
	
	struct SwsContext *img_convert_ctx, *pSwsCtx;
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,   
        pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
  	//FILE * f_hyuv=fopen("zhoujielun_1920x1080.yuv","wb+"); 
	FILE * fp_es = fopen("800x480_24fps.es", "wb+");
	//Dump information about file onto standard error
	printf("--------------- File Information ----------------\n");  
	av_dump_format(pFormatCtx,0,argv[1],0);
	printf("----------------hello---------------------------------\n");
	printf(" ==    hello\n");

	//AVBitStreamFilterContext* bsfc = av_bitstream_filter_init("h264_mp4toannexb");
	uint8_t sps[100];
	uint8_t pps[100];
	int spsLength=0;
	int ppsLength=0;
	spsLength=pFormatCtx->streams[0]->codec->extradata[6]*0xFF+pFormatCtx->streams[0]->codec->extradata[7];

	ppsLength=pFormatCtx->streams[0]->codec->extradata[8+spsLength+1]*0xFF+pFormatCtx->streams[0]->codec->extradata[8+spsLength+2];
	i = 0;
	for (i=0;i<spsLength;i++)
	{
		sps[i]=pFormatCtx->streams[0]->codec->extradata[i+8];
	}
	
	for (i=0;i<ppsLength;i++)
	{
		pps[i]=pFormatCtx->streams[0]->codec->extradata[i+8+2+1+spsLength];
	}

    //unsigned char *dummy=NULL;   //输入的指针
    //dummy = (unsigned char *)malloc(32);

	//int dummy_len;
	
	//av_bitstream_filter_filter(bsfc, pCodecCtx, NULL, &dummy, &dummy_len, NULL, 0, 0);
	//AVPacket pkt = packet;
  	//av_bitstream_filter_filter(bsfc, pCodecCtx, NULL, &pkt.data, &pkt.size, packet.data, packet.size, (packet.flags) & AV_PKT_FLAG_KEY);
	fwrite(pCodecCtx->extradata, pCodecCtx->extradata_size, 1, fp_es);  
	printf(" ==    hello\n");
	
	//av_bitstream_filter_close(bsfc);
	//free(dummy);
	char nal_start[]={0,0,0,1};  
	int flag = 1;
	while(av_read_frame(pFormatCtx, &packet) >= 0)
	{
		printf("Reading frame ...\n");
		if(packet.stream_index == videoStream) // Is this a packet from the video stream
		{
			// int a = av_bitstream_filter_filter(bsfc, codecctx, NULL, &pkt.data, &pkt.size, audiopack.data, audiopack.size, audiopack.flags & AV_PKT_FLAG_KEY);
   //          if(a > 0)
   //          {
   //              audiopack = pkt;
   //          }

			// AVPacket pkt = packet;
			// int a = av_bitstream_filter_filter(bsfc, pCodecCtx, NULL, &pkt.data, &pkt.size, packet.data, packet.size, (packet.flags) & AV_PKT_FLAG_KEY);
   //          if(a > 0)
   //          {
   //              packet = pkt;
   //          }
			//fwrite(pkt.data, pkt.size, 1, fp_vides);

			
			if (flag)
			{
				fwrite(nal_start,4,1,fp_es);
				fwrite(sps,spsLength,1,fp_es);
				fwrite(nal_start,4,1,fp_es);
				fwrite(pps,ppsLength,1,fp_es);

				packet.data[0]=0x00;
				packet.data[1]=0x00;
				packet.data[2]=0x00;
				packet.data[3]=0x01;
				fwrite(packet.data,packet.size,1,fp_es);

				flag=0;
			}
			else
			{

				packet.data[0]=0x00;
				packet.data[1]=0x00;
				packet.data[2]=0x00;
				packet.data[3]=0x01;
				fwrite(packet.data,packet.size,1,fp_es);
			}
			frame_cnt ++;
			printf("%d", frame_cnt);
			if(frame_cnt >= 100) break;
			

			/*
			//Decode
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, packet);
			if(ret < 0)
			{
				printf("Decode Error.\n");
				return -1;
			}
			if(frameFinished)
			{
				// Convert the image from its native format to YUV
//      		sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0, 
//            		pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
      			sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,   
                    pFrameYUV->data, pFrameYUV->linesize);
      			if(frame_cnt >= 500 && frame_cnt <= 1250)
      			{
      			printf("Decoded frame index: %d\n",frame_cnt);


                fwrite(pFrameYUV->data[0],pCodecCtx->width*pCodecCtx->height  , 1,f_hyuv);
                fwrite(pFrameYUV->data[1],pCodecCtx->width*pCodecCtx->height/4, 1,f_hyuv);
                fwrite(pFrameYUV->data[2],pCodecCtx->width*pCodecCtx->height/4, 1,f_hyuv);
      			}
                
                frame_cnt++;
			}
			*/
		}
  		
  		// Free the packet that was allocated by av_read_frame
  		av_free_packet(&packet);
	}


	fclose(fp_es);
	sws_freeContext(img_convert_ctx);

	av_free(buffer);
	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);

	// Close the video file
	avformat_close_input(&pFormatCtx);

}
