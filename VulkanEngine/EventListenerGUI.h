#ifndef EVENTLISTENERGUI_H
#define EVENTLISTENERGUI_H
#include "VEInclude.h"

namespace ve {

	class EventListenerGUI : public VEEventListener {

	private:
		char buf[256];
		std::string openedNodeName;

	protected:
		virtual void onDrawOverlay(veEvent event);
		//virtual bool onKeyboard(veEvent event);

		bool isLightNode(VESceneNode* node);
		void addLightMenu(nk_context* ctx, VELight* light);
		void addNodePosMenu(nk_context* ctx, VESceneNode* node);
		

	public:
		///Constructor
		EventListenerGUI(std::string name) : VEEventListener(name) {};

		///Destructor
		~EventListenerGUI() override {};
	};
}
#endif