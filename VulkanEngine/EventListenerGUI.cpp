#include "EventListenerGUI.h"

namespace ve {


	void EventListenerGUI::onDrawOverlay(veEvent evt) {
		std::vector<std::string> nodeNameList;
		getSceneManagerPointer()->createSceneNodeList(getSceneManagerPointer()->getRootSceneNode(), nodeNameList);

		VESubrenderFW_Nuklear* pSubrender = (VESubrenderFW_Nuklear*)getRendererPointer()->getOverlay();
		if (pSubrender == nullptr) return;

		nk_context* ctx = pSubrender->getContext();


		if (nk_begin(ctx, "Show", nk_rect(0, 0, 400, 800), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_SCALABLE)) {
			nk_layout_row_static(ctx, 30, 80, 1);



			for (auto nodeName : nodeNameList) {
				nk_layout_row_dynamic(ctx, 45, 1);
				sprintf(buf, nodeName.c_str());

				if (nk_button_label(ctx, buf)) {
					if (openedNodeName.compare(nodeName) == 0) {
						openedNodeName = "";
					}
					else {
						openedNodeName = nodeName;
					}
				}
				if (openedNodeName.compare(nodeName) == 0) {
					auto node = getSceneManagerPointer()->getSceneNode(nodeName);
					if (node == nullptr) continue;

					isLightNode(node) ? addLightMenu(ctx, (VELight*)node) : addNodePosMenu(ctx, node);
				}
			}


		}
		nk_end(ctx);

	}


	bool EventListenerGUI::isLightNode(VESceneNode* node) {
		for (auto n : getSceneManagerPointer()->getLights()) {
			if (n->getName().compare(node->getName()) == 0) {
				return true;
			}
		}
		return false;
	}

	void EventListenerGUI::addLightMenu(nk_context* ctx, VELight* light) {
		nk_layout_row_dynamic(ctx, 50, 1);

		auto originalCol = light->m_col_ambient;
		nk_color lightColor = { originalCol.r * 255.0, originalCol.g * 255.0, originalCol.b * 255.0, originalCol.a * 255.0 };
		nk_label(ctx, buf, NK_TEXT_LEFT);
		nk_label(ctx, "Ambient Light Color", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 250, 1);
		nk_color_pick(ctx, &lightColor, NK_RGBA);

		light->m_col_ambient = { float(lightColor.r) / 255, float(lightColor.g) / 255, float(lightColor.b) / 255, float(lightColor.a) / 255 };
	}


	void EventListenerGUI::addNodePosMenu(nk_context* ctx, VESceneNode* node) {
		nk_layout_row_dynamic(ctx, 50, 12);
		auto pos = node->getPosition();

		sprintf(buf, "x:");
		nk_label(ctx, buf, NK_TEXT_LEFT);
		strcpy(buf, std::to_string(node->getPosition().x).c_str());//set x_val to field
		auto event = nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, buf, sizeof(buf) - 1, nk_filter_float);
		sprintf(buf, "+");//create + button
		if (nk_button_label(ctx, buf)) {
			pos.x++;
			node->setPosition(pos);
		}
		sprintf(buf, "-");//create - button
		if (nk_button_label(ctx, buf)) {
			pos.x--;
			node->setPosition(pos);
		}

		sprintf(buf, "y:");
		nk_label(ctx, buf, NK_TEXT_LEFT);
		strcpy(buf, std::to_string(node->getPosition().y).c_str());//set x_val to field
		event = nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, buf, sizeof(buf) - 1, nk_filter_float);
		sprintf(buf, "+");//create + button
		if (nk_button_label(ctx, buf)) {
			pos.y++;
			node->setPosition(pos);
		}
		sprintf(buf, "-");//create - button
		if (nk_button_label(ctx, buf)) {
			pos.y--;
			node->setPosition(pos);
		}

		sprintf(buf, "z:");
		nk_label(ctx, buf, NK_TEXT_LEFT);
		strcpy(buf, std::to_string(node->getPosition().z).c_str());//set x_val to field
		event = nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, buf, sizeof(buf) - 1, nk_filter_float);
		sprintf(buf, "+");//create + button
		if (nk_button_label(ctx, buf)) {
			pos.z++;
			node->setPosition(pos);
		}
		sprintf(buf, "-");//create - button
		if (nk_button_label(ctx, buf)) {
			pos.z--;
			node->setPosition(pos);
		}
	}
}