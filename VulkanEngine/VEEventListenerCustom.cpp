/**
* The Vienna Vulkan Engine
*
* (c) bei Yevhen Baidiuk, University of Vienna
*
*/

#include "VEInclude.h"

namespace ve {

    void VEEventListenerCustom::onFrameEnded(veEvent event) {

        m_time_sum += (float) event.dt;
        if (m_time_sum >= 1) {
            m_screenshot_current_second = 0;
            m_time_sum = 0;
        }

        if (m_screenshot_current_second < m_max_screenshot_per_sec) {

            VkExtent2D extent = getWindowPointer()->getExtent();
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

            m_numScreenshot++;
            m_screenshot_current_second++;

            std::string name("media/screenshots/screenshot" + std::to_string(m_numScreenshot - 1) + ".jpg");
            stbi_write_jpg(name.c_str(), extent.width, extent.height, 4, dataImage, 4 * extent.width);
            delete[] dataImage;
        }
    }


}








