#include "VEInclude.h"
#include "VEEventListenerCustom.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include "libavutil/common.h"
#include "libavutil/mathematics.h"
}

namespace ve {

	bool VEEventListenerCustom::onKeyboard(veEvent event) {
		glm::vec4 translate = glm::vec4(0.0, 0.0, 0.0, 1.0);    //total translation
		glm::vec4 rot4 = glm::vec4(1.0);                        //total rotation around the axes, is 4d !
		float angle = 0.0;

		switch (event.idata1) {
		case GLFW_KEY_KP_8: //forward
		case GLFW_KEY_8:
			translate = glm::vec4(0.0, 0.0, 1.0, 1.0);
			break;
		case GLFW_KEY_KP_2: //backward
		case GLFW_KEY_2:
			translate = glm::vec4(0.0, 0.0, -1.0, 1.0);
			break;
		case GLFW_KEY_KP_4: //rotation y right
		case GLFW_KEY_4:
			angle = (float)event.dt * -1.0f;
			rot4 = glm::vec4(0.0, 1.0, 0.0, 1.0);
			break;
		case GLFW_KEY_KP_6: //rotation y right
		case GLFW_KEY_6:
			angle = (float)event.dt * 1.0f;
			rot4 = glm::vec4(0.0, 1.0, 0.0, 1.0);
			break;
		default:
			return false;
		}

		VESceneNode* plane = getSceneManagerPointer()->getSceneNode("Plane");
		float speed = 5.f;

		//rotation
		glm::vec3 old_pos = plane->getPosition();
		plane->setPosition(glm::vec3(0, 0, 0));
		glm::vec3 rot3 = glm::vec3(rot4.x, rot4.y, rot4.z);
		plane->multiplyTransform(glm::rotate(angle, rot3));
		plane->setPosition(old_pos);

		//moving
		glm::vec3 trans = speed * glm::vec3(translate.x, translate.y, translate.z);
		const glm::mat newPosition = glm::translate((float)event.dt * trans);
		plane->multiplyTransform(newPosition);

		return true;
	}


	void VEEventListenerCustom::onFrameEnded(veEvent event) {

		VkExtent2D extent = getWindowPointer()->getExtent();
		uint32_t imageSize = extent.width * extent.height * 4;
		VkImage image = getRendererPointer()->getSwapChainImage();

		auto* dataImage = new uint8_t[imageSize];

		vh::vhBufCopySwapChainImageToHost(getRendererPointer()->getDevice(),
			getRendererPointer()->getVmaAllocator(),
			getRendererPointer()->getGraphicsQueue(),
			getRendererPointer()->getCommandPool(),
			image, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, dataImage, extent.width, extent.height, imageSize);

		avcodec_register_all();

		if (firstTime) {
			firstTime = false;
			encoder.initContext(extent.width, extent.height);
		}
		encoder.convertAndSend(dataImage);

		delete[] dataImage;
	}

}