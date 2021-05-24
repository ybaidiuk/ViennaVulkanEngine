
#include "VEInclude.h"

namespace ve {
	

	void EventListenerGUI::onDrawOverlay(veEvent evt) {
		std::vector < VESceneNode* > list = getSceneManagerPointer()->getRootSceneNode()->getChildrenList();
		VESceneNode* firstNode = list[0];
		

		VESubrenderFW_Nuklear* pSubrender = (VESubrenderFW_Nuklear*)getRendererPointer()->getOverlay();
		if (pSubrender == nullptr) return;

		nk_context* ctx = pSubrender->getContext();
		
		
		if (nk_begin(ctx, "Show", nk_rect(50, 50, 220, 220), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_SCALABLE)) {
			nk_layout_row_static(ctx, 30, 80, 1); 
			
			
				for (auto node : list) {
					nk_label(ctx, node->getName().c_str(), NK_TEXT_LEFT);
				}
		


		}
		nk_end(ctx);

	}

}