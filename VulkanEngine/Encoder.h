#ifndef ENCODER_H
#define ENCODER_H


extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

#include <iostream>
#include "UDPSend.h"


class Encoder {
private:
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MPEG2VIDEO);
    AVCodecContext* avCodecContext = avcodec_alloc_context3(codec);
    SwsContext* swsContext = nullptr;
    UDPSend udpSend;
public:
    const char* extensions = ".mpg";
    Encoder() {
        udpSend.init("127.0.0.1", 23042);
    };
    ~Encoder() = default;

    virtual void initContext(uint32_t width, uint32_t height);
    void _encodeAndSend(AVFrame* frame, AVPacket* pkt);
    void convertAndSend(uint8_t* dataImage);
};


#endif // ENCODER_H