/**
	@author Yevhen Baidiuk
	@version 1.0 
	@date 2021
*/

#ifndef VEEVENTLISTENERCUSTOM_H
#define VEEVENTLISTENERCUSTOM_H


#include "Encoder.h"
#include "UDPSend.h"

namespace ve {

    class VEEventListenerCustom : public VEEventListener {

    private:
//        std::vector<uint8_t *> frames_vector;
//        uint32_t recording_counter = 0;
        VkExtent2D extent;
        bool id_udp_send = false;
        Encoder encoder;
        UDPSend udpSender;

        void onFrameEnded(veEvent event) override;
        bool onKeyboard(veEvent event) override;

    public:
        ///Constructor
        VEEventListenerCustom(std::string name);

        ///Destructor
        ~VEEventListenerCustom();
    };
}


#endif
