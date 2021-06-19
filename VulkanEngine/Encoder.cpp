
#include "Encoder.h"

void Encoder::initContext(uint32_t width, uint32_t height) {
	std::cout << "init context start" << std::endl;

	avCodecContext->width = width;
	avCodecContext->height = height;

	avCodecContext->bit_rate = 400000;
	avCodecContext->time_base.num = 1;
	avCodecContext->time_base.den = 25;
	avCodecContext->framerate.num = 25;
	avCodecContext->framerate.den = 1;

	avCodecContext->gop_size = 10; // one I-frame every ten frames
	avCodecContext->max_b_frames = 1;
	avCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;

	if (avcodec_open2(avCodecContext, codec, NULL) < 0) {
		fprintf(stderr, "could not open codec " + avcodec_open2(avCodecContext, codec, NULL));
		exit(1);
	}

	swsContext = sws_getContext(avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA,
		avCodecContext->width, avCodecContext->height, AV_PIX_FMT_YUV420P, 0, NULL,
		NULL, NULL);
	if (!swsContext) {
		fprintf(stderr, "error creating swsContext");
		exit(1);
	}
	std::cout << "init context end" << std::endl;
}

void Encoder::_encodeAndSend(AVFrame* frame, AVPacket* pkt) {
	int ret;

	// send the frame to the encoder */
	ret = avcodec_send_frame(avCodecContext, frame);
	if (ret < 0) {
		fprintf(stderr, "error sending a frame for encoding\n");
		exit(1);
	}

	while (ret >= 0) {
		int ret = avcodec_receive_packet(avCodecContext, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			fprintf(stderr, "error during encoding\n");
			exit(1);
		}

		//fwrite(pkt->data, 1, pkt->size, outfile);
		udpSend.send((char*) pkt->data, pkt->size);
		av_packet_unref(pkt);

	}
}

// nachgefragt von Christian.
void Encoder::convertAndSend(uint8_t* dataImage) {
	fflush(stdout);

	auto frameResult = av_frame_alloc();
	frameResult->format = avCodecContext->pix_fmt;
	frameResult->width = avCodecContext->width;
	frameResult->height = avCodecContext->height;

	auto frameRGB = av_frame_alloc();
	frameRGB->format = AV_PIX_FMT_RGBA;
	frameRGB->width = avCodecContext->width;
	frameRGB->height = avCodecContext->height;

	av_frame_get_buffer(frameResult, 32);
	av_frame_make_writable(frameResult);

	av_frame_get_buffer(frameRGB, 32);
	av_frame_make_writable(frameRGB);

	auto pkt = av_packet_alloc();
	if (pkt == NULL) {
		printf("okt is null");
	}

	av_image_fill_arrays(frameRGB->data, frameRGB->linesize, dataImage, AV_PIX_FMT_RGBA, avCodecContext->width,
		avCodecContext->height, 1);

	sws_scale(swsContext, (const uint8_t**)frameRGB->data, frameRGB->linesize, 0, avCodecContext->height,
		frameResult->data, frameResult->linesize);
	
		_encodeAndSend(frameResult, pkt);
	
}