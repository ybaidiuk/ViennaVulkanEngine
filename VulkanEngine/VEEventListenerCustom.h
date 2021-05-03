/**
	@author Yevhen Baidiuk
	@version 1.0 
	@date 2021
*/

#ifndef VEEVENTLISTENERCUSTOM_H
#define VEEVENTLISTENERCUSTOM_H


#include "Encoder.h"

namespace ve {

    class VEEventListenerCustom : public VEEventListener {

    private:
        std::vector<uint8_t *> frames_vector;
        uint32_t recording_counter = 0;
        bool is_recorded = false;
        Encoder encoder;

        void onFrameEnded(veEvent event) override;
        bool onKeyboard(veEvent event) override;

    public:
        ///Constructor
        VEEventListenerCustom(std::string name) : VEEventListener(name) {};

        ///Destructor
        ~VEEventListenerCustom() override {};
    };
}


#endif
