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
    AVCodec *avCodec = nullptr;
    AVCodecContext *avCodecContext = nullptr;
    SwsContext *swsContext = nullptr;
    uint64_t frame_counter = 0;

public:
    Encoder() = default;
    ~Encoder() = default;

    void initContext(uint32_t width, uint32_t height);
    void encode(AVFrame *frame, AVPacket *pkt, FILE *outfile);
    void saveImageVectorToFile(uint8_t *dataImage, FILE *file);
//    void cleanContext();
};


#endif // ENCODER_H