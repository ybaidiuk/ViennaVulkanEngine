#include "Encoder.h"

void Encoder::initContext(size_t width, size_t height) {
    std::cout<< "init context"<<std::endl;
    m_codec = avcodec_find_encoder(codecId); //todo
//
//    m_avcodec_context = avcodec_alloc_context3(m_codec);
//
//    m_avcodec_context->bit_rate = 400000;
//
//    m_avcodec_context->width = width;
//    m_avcodec_context->height = height;
//
//    // frames per second
//    m_avcodec_context->time_base.num = 1;
//    m_avcodec_context->time_base.den = 25;
//    m_avcodec_context->framerate.num = 25;
//    m_avcodec_context->framerate.den = 1;
//
//    m_avcodec_context->gop_size = 10; // emit one intra frame every ten frames
//    m_avcodec_context->max_b_frames = 1;
//    m_avcodec_context->pix_fmt = AV_PIX_FMT_YUV420P;
//
//    if (avcodec_open2(m_avcodec_context, m_codec, NULL) < 0) {
//        fprintf(stderr, "could not open codec\n");
//        exit(1);
//    }
//
//    m_img_convert_ctx = sws_getContext(m_avcodec_context->width, m_avcodec_context->height, AV_PIX_FMT_RGBA,
//                                       m_avcodec_context->width, m_avcodec_context->height, AV_PIX_FMT_YUV420P, 0, NULL,
//                                       NULL, NULL);
//    if (!m_img_convert_ctx) {
//        fprintf(stderr, "error creating swsContext");
//        exit(1);
//    }
}

