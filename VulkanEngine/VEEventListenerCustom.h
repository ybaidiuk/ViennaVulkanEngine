#include "UDPSend.h"
#ifndef VEEVENTLISTENERCUSTOM_H
#define VEEVENTLISTENERCUSTOM_H


namespace ve {

    class VEEventListenerCustom : public VEEventListener {

    protected:
        //		bool  m_usePrevCursorPosition = false;			///<Can I use the previous cursor position for moving the camera?
        //		bool  m_rightButtonClicked = false;				///<Is the left button currently clicked?
        //		float m_cursorPrevX = 0;						///<Previous X position of cursor
        //		float m_cursorPrevY = 0;						///<Previous Y position of cursor
        //		bool  m_makeScreenshot = false;					///<Should I make a screeshot after frame is done?
        //		bool  m_makeScreenshotDepth = false;			///<Should I make a screeshot after frame is done?
        uint32_t m_numScreenshot = 0;                    ///<Screenshot ID
        const uint32_t m_max_screenshot_per_sec = 5;
        uint32_t m_screenshot_current_second = 0;
        float m_time_sum = 0;
        UDPSend sender;
        const char* adreess = "127.0.0.1";

        void onFrameEnded(veEvent event) override;
        		virtual bool onKeyboard(veEvent event);
        //		virtual bool onMouseMove(veEvent event);
        //		virtual bool onMouseButton(veEvent event);
        //		virtual bool onMouseScroll(veEvent event);

    public:
        ///Constructor
        VEEventListenerCustom(std::string name) : VEEventListener(name) {
            WSADATA wsaData;
            auto wVersionRequested = MAKEWORD(2, 2);
            auto err = WSAStartup(wVersionRequested, &wsaData);

            if (err != 0) {
                printf("WSAStartup failed with error: %d\n", err);
            }

            sender.init(adreess, 5000);
        };

        ///Destructor
        ~VEEventListenerCustom() override {};
    };
}


#endif
