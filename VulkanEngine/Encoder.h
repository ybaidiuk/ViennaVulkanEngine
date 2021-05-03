//
// Created by Yevhen Baidiuk on 02.05.21.
//

#ifndef ENCODER_H
#define ENCODER_H


extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}
#include <iostream>


class Encoder {
private:
    AVCodecContext *m_avcodec_context;
    SwsContext *m_img_convert_ctx;
    AVCodecID codecId = AV_CODEC_ID_MPEG2VIDEO;
    AVCodec *m_codec;

public:
    Encoder() {};

    ~Encoder() {};

    void initContext(size_t width, size_t height);
//    void encode(AVFrame *frame, AVPacket *pkt, FILE *outfile);
//    void saveImageVectorToFile(uint8_t *dataImage, FILE *f, int position);
//    void cleanContext();
};


#endif