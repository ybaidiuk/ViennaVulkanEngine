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
    AVCodecID avCodecId = AV_CODEC_ID_MPEG2VIDEO;
    AVCodec *avCodec;
    AVCodecContext *avCodecContext;
    SwsContext *swsContext;
    uint64_t frame_counter = 0;

public:
    Encoder() = default;

    ~Encoder() = default;

    void initContext(size_t width, size_t height);
    void encode(AVFrame *frame, AVPacket *pkt, FILE *outfile);
    void saveImageVectorToFile(uint8_t *dataImage, FILE *f);
    void cleanContext();
};


#endif // ENCODER_H