#define __STDC_CONSTANT_MACROS
extern "C"
{
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavdevice/avdevice.h"
#include "libavcodec/avcodec.h"
};
#include "net/NetInterface.h"
#include "xop/RtspServer.h"

#include <cstdio>
#include <string>
#include <ctime>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>

using namespace std;

int main(int argc, char* argv[])
{
	//av_register_all();
	avcodec_register_all();
	avfilter_register_all();
    av_log_set_level(AV_LOG_QUIET);

	int ret;
	int in_width = 640;
	int in_height = 480;
    std::atomic<int64_t> encode_count(0);
	AVCodec *pCodecH264 = NULL;
	AVCodecContext *pCodeCtx = NULL;

	auto ip = xop::NetInterface::getLocalIPAddress();
	xop::EventLoop loop;
	xop::RtspServer rtspServer(&loop, ip, 8554);
	xop::MediaSession* session = xop::MediaSession::createNew("live");
	session->addMediaSource(xop::channel_0, xop::H264Source::createNew());
	auto sessionId = rtspServer.addMeidaSession(session);
	std::thread t([&loop]() {
		loop.loop();
	});
    loop.addTimer([&encode_count]()->bool {
        auto framerate = encode_count.load();
        std::cout << "framerate " << framerate << std::endl;
        encode_count.store(0);
        return true;
    },1000);

	pCodecH264 = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!pCodecH264)
	{
		fprintf(stderr, "h264 codec not found\n");
		exit(1);
	}

	pCodeCtx = avcodec_alloc_context3(pCodecH264);
	pCodeCtx->bit_rate = 3000000;// put sample parameters 
	pCodeCtx->width = in_width;// 
	pCodeCtx->height = in_height;// 

	FILE* fd = fopen("test.h264","wb+");

	// frames per second 
	AVRational rate;
	rate.num = 1;
	rate.den = 25;
	AVRational framerate;
	framerate.num = 25;
	framerate.den = 1;
	pCodeCtx->time_base = rate;//(AVRational){1,25};
	pCodeCtx->gop_size = 25; // emit one intra frame
	pCodeCtx->framerate = framerate;
	pCodeCtx->max_b_frames = 0;
	pCodeCtx->thread_count = 1;
	pCodeCtx->pix_fmt = AV_PIX_FMT_YUV420P;//PIX_FMT_RGB24;
	pCodeCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	av_opt_set(pCodeCtx->priv_data, "preset", "slow", 0);
	av_opt_set(pCodeCtx->priv_data, "tune", "zerolatency", 0);

	//av_opt_set(c->priv_data, /*"preset"*/"libvpx-1080p.ffpreset", /*"slow"*/NULL, 0);
	if (avcodec_open2(pCodeCtx, pCodecH264, NULL) < 0)
	{
		printf("open CodecH264 error");
		return 0;
	}

	//const char *filter_descr = "lutyuv='u=128:v=128'";
	//const char *filter_descr = "boxblur";
	//const char *filter_descr = "hflip";
	//const char *filter_descr = "hue='h=60:s=-3'";
	//const char *filter_descr = "crop=2/3*in_w:2/3*in_h";
	//const char *filter_descr = "drawbox=x=100:y=100:w=100:h=100:color=pink@0.5";
	const char *filter_descr = "drawtext=fontfile=arial.ttf:fontcolor=red:fontsize=100:text='test'";

	while (true)
	{
        auto time1 = std::chrono::steady_clock::now();
        AVPacket* avpkt = av_packet_alloc();
		char sz_filter_descr[256] = { 0 };
		time_t lt = time(NULL);
		struct tm * tm_ptr = localtime(&lt);
		strftime(sz_filter_descr, 256, "drawtext=fontfile=arial.ttf:fontcolor=red:fontsize=50:text='osd %Y/%m/%d %H/%M/%S'", tm_ptr);

		char args[512];
		AVFilterContext *buffersink_ctx;
		AVFilterContext *buffersrc_ctx;
		AVFilterGraph *filter_graph;
		const AVFilter *buffersrc = avfilter_get_by_name("buffer");
		const AVFilter *buffersink = avfilter_get_by_name("buffersink");
		AVFilterInOut *outputs = avfilter_inout_alloc();
		AVFilterInOut *inputs = avfilter_inout_alloc();
		enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
		AVBufferSinkParams *buffersink_params;

		filter_graph = avfilter_graph_alloc();

		/* buffer video source: the decoded frames from the decoder will be inserted here. */
		snprintf(args, sizeof(args),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			in_width, in_height, AV_PIX_FMT_YUV420P,
			1, 25, 1, 1);

		ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
			args, NULL, filter_graph);
		if (ret < 0) {
			printf("Cannot create buffer source\n");
			return ret;
		}

		/* buffer video sink: to terminate the filter chain. */
		buffersink_params = av_buffersink_params_alloc();
		buffersink_params->pixel_fmts = pix_fmts;
		ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
			NULL, buffersink_params, filter_graph);
		av_free(buffersink_params);
		if (ret < 0) {
			printf("Cannot create buffer sink\n");
			return ret;
		}

		/* Endpoints for the filter graph. */
		outputs->name = av_strdup("in");
		outputs->filter_ctx = buffersrc_ctx;
		outputs->pad_idx = 0;
		outputs->next = NULL;

		inputs->name = av_strdup("out");
		inputs->filter_ctx = buffersink_ctx;
		inputs->pad_idx = 0;
		inputs->next = NULL;

		if ((ret = avfilter_graph_parse_ptr(filter_graph, sz_filter_descr,
			&inputs, &outputs, NULL)) < 0)
			return ret;

		if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
			return ret;

		AVFrame *frame_in;
		AVFrame *frame_out;
		unsigned char *frame_buffer_in ;
		//unsigned char *frame_buffer_out;

		frame_in = av_frame_alloc();
		frame_buffer_in = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, in_width, in_height, 1));
		memset(frame_buffer_in,0, av_image_get_buffer_size(AV_PIX_FMT_YUV420P, in_width, in_height, 1));
		av_image_fill_arrays(frame_in->data, frame_in->linesize, frame_buffer_in,
			AV_PIX_FMT_YUV420P, in_width, in_height, 1);

		frame_out = av_frame_alloc();

		frame_in->width = in_width;
		frame_in->height = in_height;
		frame_in->format = AV_PIX_FMT_YUV420P;

		//input Y,U,V
		frame_in->data[0] = frame_buffer_in;
		frame_in->data[1] = frame_buffer_in + in_width * in_height;
		frame_in->data[2] = frame_buffer_in + in_width * in_height * 5 / 4;

		if (av_buffersrc_add_frame_flags(buffersrc_ctx, frame_in, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
			printf("Error while add frame.\n");
			break;
		}
		/* pull filtered pictures from the filtergraph */
		while (1) {
			ret = av_buffersink_get_frame(buffersink_ctx, frame_out);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				break;
			if (ret < 0)
				return 0;
			int u_size = 0;

			avcodec_send_frame(pCodeCtx, frame_out);
			u_size = avcodec_receive_packet(pCodeCtx, avpkt);
			if (u_size == 0)
			{
				AVPacket* pkt = avpkt;
				if (pkt && pkt->size != 0)
				{
					xop::AVFrame vidoeFrame(pkt->size + 1024);
					vidoeFrame.size = 0;
					vidoeFrame.type = xop::VIDEO_FRAME_P;
					vidoeFrame.timestamp = xop::H264Source::getTimeStamp();
					if (pkt->data[4] == 0x65) //0x67:sps ,0x65:IDR
					{
						vidoeFrame.type = xop::VIDEO_FRAME_I;
						memcpy(vidoeFrame.buffer.get() + vidoeFrame.size, pkt->data + 4, pkt->size - 4);
						vidoeFrame.size += (pkt->size - 4);

					}
					else
					{
						memcpy(vidoeFrame.buffer.get() + vidoeFrame.size, pkt->data + 4, pkt->size - 4);
						vidoeFrame.size += (pkt->size - 4);
					}

					if (vidoeFrame.type == xop::VIDEO_FRAME_I) {
						uint8_t* extraData = pCodeCtx->extradata;
						uint8_t extraDatasize = pCodeCtx->extradata_size;
						uint8_t* sps = extraData + 4;
						uint8_t* pps = nullptr;
						for (uint8_t* p = sps; p < (extraData + extraDatasize - 4); p++) {
							if (p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 1) {
								if (p[4] == 0x68) {
									pps = p + 4;
									break;
								}
							}
						}
						if (pps == nullptr) {
							std::cout << " get pps error " << std::endl;
						}

						uint32_t sps_size = pps - sps - 4;
						uint32_t pps_size = extraDatasize - sps_size - 8;
						xop::AVFrame sps_frame(sps_size);
						xop::AVFrame pps_frame(pps_size);
						memcpy(sps_frame.buffer.get(), sps, sps_size);
						memcpy(pps_frame.buffer.get(), pps, pps_size);
						sps_frame.timestamp = vidoeFrame.timestamp;
						pps_frame.timestamp = vidoeFrame.timestamp;

						rtspServer.pushFrame(sessionId, xop::channel_0, sps_frame);
						rtspServer.pushFrame(sessionId, xop::channel_0, pps_frame);
						rtspServer.pushFrame(sessionId, xop::channel_0, vidoeFrame);
						fwrite(pCodeCtx->extradata,pCodeCtx->extradata_size,1,fd);
                        fwrite(pkt->data,pkt->size,1,fd);
                }
					else {
                        fwrite(pkt->data,pkt->size,1,fd);
						rtspServer.pushFrame(sessionId, xop::channel_0, vidoeFrame);
					}
				}
			}
		}

        if (avpkt)
        {
            //av_packet_unref(avpkt);
            av_packet_free(&avpkt);
            avpkt = nullptr;
        }

		av_frame_unref(frame_out);
		av_frame_unref(frame_in);

		av_free(frame_buffer_in);
		//av_free(frame_buffer_out);

		av_frame_free(&frame_in);
		av_frame_free(&frame_out);

		avfilter_inout_free(&inputs);
		avfilter_inout_free(&outputs);
		avfilter_free(buffersrc_ctx);
		avfilter_free(buffersink_ctx);
		avfilter_graph_free(&filter_graph);

        auto time2 = std::chrono::steady_clock::now();
        auto du = std::chrono::duration_cast<std::chrono::milliseconds>(time2 - time1);
        if (du.count() < 40) {
            std::this_thread::sleep_for(std::chrono::milliseconds(40 - du.count()));
        }
        else {
            //std::cout << "code time  " << du.count() << std::endl;
        }
        encode_count++;
	}

	avcodec_close(pCodeCtx);

	return 0;
}