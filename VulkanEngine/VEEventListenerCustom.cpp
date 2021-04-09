/**
* The Vienna Vulkan Engine
*
* (c) bei Yevhen Baidiuk, University of Vienna
*
*/

#include "VEInclude.h"

namespace ve {

    void VEEventListenerCustom::onFrameEnded(veEvent event) {
//        m_time_sum += (float) event.dt;
//        if (m_time_sum >= 1) {
//            m_screenshot_current_second = 0;
//            m_time_sum = 0;
//        }
//
//        if (m_screenshot_current_second < m_max_screenshot_per_sec) {
//
//            VkExtent2D extent = getWindowPointer()->getExtent();
//            uint32_t imageSize = extent.width * extent.height * 4;
//            VkImage image = getRendererPointer()->getSwapChainImage();
//
//            auto *dataImage = new uint8_t[imageSize];
//
//            vh::vhBufCopySwapChainImageToHost(getRendererPointer()->getDevice(),
//                                              getRendererPointer()->getVmaAllocator(),
//                                              getRendererPointer()->getGraphicsQueue(),
//                                              getRendererPointer()->getCommandPool(),
//                                              image, VK_FORMAT_R8G8B8A8_UNORM,
//                                              VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//                                              dataImage, extent.width, extent.height, imageSize);
//
//            m_numScreenshot++;
//            m_screenshot_current_second++;
//
//            std::string name("media/screenshots/screenshot" + std::to_string(m_numScreenshot - 1) + ".jpg");
//            stbi_write_jpg(name.c_str(), extent.width, extent.height, 4, dataImage, 4 * extent.width);
//            delete[] dataImage;
    }

    bool VEEventListenerCustom::onKeyboard(veEvent event) {
        glm::vec4 translate = glm::vec4(0.0, 0.0, 0.0, 1.0);    //total translation
        glm::vec4 rot4 = glm::vec4(1.0);                        //total rotation around the axes, is 4d !
        float angle = 0.0;

        switch (event.idata1) {
            case GLFW_KEY_KP_8 : //forward
            case GLFW_KEY_8 :
                translate = glm::vec4(0.0, 0.0, 1.0, 1.0);
                break;
            case GLFW_KEY_KP_2 : //backward
            case GLFW_KEY_2 :
                translate = glm::vec4(0.0, 0.0, -1.0, 1.0);
                break;
            case GLFW_KEY_KP_4 : //rotation y right todo
            case GLFW_KEY_4 :
                angle = (float) event.dt * -1.0f;
                rot4 = glm::vec4(0.0, 1.0, 0.0, 1.0);
                break;
            case GLFW_KEY_KP_6 : //rotation y right todo
            case GLFW_KEY_6 :
                angle = (float) event.dt * 1.0f;
                rot4 = glm::vec4(0.0, 1.0, 0.0, 1.0);
                break;
            default:
                return false;
        }

        VESceneNode *plane = getSceneManagerPointer()->getSceneNode("Plane");
        float speed = 5.f;

        //moving
        glm::vec3 trans = speed * glm::vec3(translate.x, translate.y, translate.z);
        plane->multiplyTransform(glm::translate(glm::mat4(1.0f), (float) event.dt * trans));

        //rotation
        glm::vec3 rot3 = glm::vec3(rot4.x, rot4.y, rot4.z);
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0), angle, rot3);
        plane->multiplyTransform(rotate);

        return true;
    }


}








