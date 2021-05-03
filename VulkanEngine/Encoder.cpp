//
// Created by Yevhen Baidiuk on 02.05.21.
//

#include "Encoder.h"

void Encoder::initContext(size_t width, size_t height) {
    std::cout << "init context start" << std::endl;
    avCodec = avcodec_find_encoder(avCodecId);
    avCodecContext = avcodec_alloc_context3(avCodec);

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

    if (avcodec_open2(avCodecContext, avCodec, NULL) < 0) {
        fprintf(stderr, "could not open codec "+ avcodec_open2(avCodecContext, avCodec, NULL));
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

void Encoder::encode(AVFrame *frame, AVPacket *pkt, FILE *outfile)
{
    int ret;

    // send the frame to the encoder */
    ret = avcodec_send_frame(avCodecContext, frame);
    if (ret < 0)
    {
        fprintf(stderr, "error sending a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0)
    {
        int ret = avcodec_receive_packet(avCodecContext, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0)
        {
            fprintf(stderr, "error during encoding\n");
            exit(1);
        }

        //printf("encoded frame %lld (size=%5d)\n", pkt->pts, pkt->size);
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
        av_frame_free(&frame);
    }
}

void Encoder::cleanContext()
{
    avcodec_free_context(&avCodecContext);
    sws_freeContext(swsContext);
}

void Encoder::saveImageVectorToFile(uint8_t *dataImage, FILE *f)
{
    fflush(stdout);

    auto frame = av_frame_alloc();
    frame->format = avCodecContext->pix_fmt;
    frame->width = avCodecContext->width;
    frame->height = avCodecContext->height;

    auto rgbFrame = av_frame_alloc();
    rgbFrame->format = AV_PIX_FMT_RGBA;
    rgbFrame->width = avCodecContext->width;
    rgbFrame->height = avCodecContext->height;

    if (av_frame_get_buffer(frame, 32) < 0)
    {
        fprintf(stderr, "could not alloc the frame data\n");
        exit(1);
    }

    if (av_frame_get_buffer(rgbFrame, 32) < 0)
    {
        fprintf(stderr, "could not alloc the frame data\n");
        exit(1);
    }

    if (av_frame_make_writable(frame) < 0)
    {
        fprintf(stderr, "Cannot make frame writeable\n");
        exit(1);
    }

    if (av_frame_make_writable(rgbFrame) < 0)
    {
        fprintf(stderr, "Cannot make frame writeable: rgb Frame\n");
        exit(1);
    }

    auto pkt = av_packet_alloc();
    if (!pkt)
    {
        fprintf(stderr, "Cannot alloc packet\n");
        exit(1);
    }

    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, dataImage, AV_PIX_FMT_RGBA, avCodecContext->width, avCodecContext->height, 1);

    sws_scale(swsContext, (const uint8_t **)rgbFrame->data, rgbFrame->linesize, 0, avCodecContext->height,
              frame->data, frame->linesize);

    frame->pts = ++frame_counter;
    encode(frame, pkt, f);
}

