/**
* The Vienna Vulkan Engine
*
* (c) bei Yevhen Baidiuk, University of Vienna
*
*/

#include "VEInclude.h"

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


}