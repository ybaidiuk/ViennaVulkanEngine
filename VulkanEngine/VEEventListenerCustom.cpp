/**
* The Vienna Vulkan Engine
*
* (c) bei Yevhen Baidiuk, University of Vienna
*
*/

#include "VEInclude.h"
#include "VEEventListenerCustom.h"


namespace ve {

    VEEventListenerCustom::VEEventListenerCustom(std::string name) : VEEventListener(name) {
        extent = getWindowPointer()->getExtent();
        encoder.initContext(extent.width, extent.height);
        udpSender.init("localhost", 8888);
    };

    VEEventListenerCustom::~VEEventListenerCustom() {
        udpSender.closeSock();
    }

    void VEEventListenerCustom::onFrameEnded(veEvent event) {
        if (id_udp_send) {
            uint32_t imageSize = extent.width * extent.height * 4;
            VkImage image = getRendererPointer()->getSwapChainImage();

            auto *dataImage = new uint8_t[imageSize];

            vh::vhBufCopySwapChainImageToHost(getRendererPointer()->getDevice(),
                                              getRendererPointer()->getVmaAllocator(),
                                              getRendererPointer()->getGraphicsQueue(),
                                              getRendererPointer()->getCommandPool(),
                                              image, VK_FORMAT_R8G8B8A8_UNORM,
                                              VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                              dataImage, extent.width, extent.height, imageSize);

//          SEND UDP
            AVPacket *pkt = encoder.convertFrameToMPEG(dataImage);
            udpSender.send(reinterpret_cast<char *>(pkt->data), 1400);
//            av_packet_unref(pkt);
        }

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
            case GLFW_KEY_KP_4 : //rotation y right
            case GLFW_KEY_4 :
                angle = (float) event.dt * -1.0f;
                rot4 = glm::vec4(0.0, 1.0, 0.0, 1.0);
                break;
            case GLFW_KEY_KP_6 : //rotation y right
            case GLFW_KEY_6 :
                angle = (float) event.dt * 1.0f;
                rot4 = glm::vec4(0.0, 1.0, 0.0, 1.0);
                break;
            case GLFW_KEY_R : // start/stop recording
                if (event.idata3 == GLFW_PRESS) {
                    id_udp_send = !id_udp_send;
                    if (id_udp_send) {
                        std::cout << "id_udp_send start" << std::endl;
                    } else {
                        std::cout << "id_udp_send stop" << std::endl;
                    }
                }
                break;
            default:
                return false;
        }

        VESceneNode *plane = getSceneManagerPointer()->getSceneNode("Plane");
        float speed = 5.f;

        //rotation
        glm::vec3 old_pos = plane->getPosition();
        plane->setPosition(glm::vec3(0, 0, 0));
        glm::vec3 rot3 = glm::vec3(rot4.x, rot4.y, rot4.z);
        plane->multiplyTransform(glm::rotate(angle, rot3));
        plane->setPosition(old_pos);

        //moving
        glm::vec3 trans = speed * glm::vec3(translate.x, translate.y, translate.z);
        const glm::mat newPosition = glm::translate((float) event.dt * trans);
        plane->multiplyTransform(newPosition);

        return true;
    }


}








