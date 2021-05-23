#ifndef EVENTLISTENERGUI_H
#define EVENTLISTENERGUI_H


namespace ve {

    class EventListenerGUI : public VEEventListener {

    protected:
   
        virtual void onDrawOverlay(veEvent event);

    public:
        ///Constructor
        EventListenerGUI(std::string name) : VEEventListener(name) {};

        ///Destructor
        ~EventListenerGUI() override {};
    };
}


#endif
