
#include <iostream>
#include <utility>
#include "glm.hpp"
#include "gtc/quaternion.hpp"
#include "vulkan/vulkan.h"
#include "vve.h"
#include "basic_test.h"

int imgui();

using namespace vve;

int main() {
    
    VeEntityTable et;

    std::cout << sizeof(VeHandle) << " "  << sizeof(VeHandle_t<VeEntityType<>>) << " " << sizeof(index_t) << std::endl;
    std::cout << tl::size_of<VeEntityTypeList>::value << std::endl;

    VeHandle h1 = et.insert(VeComponentPosition{ glm::vec3{1.0f, 2.0f, 3.0f} }, VeComponentOrientation{}, VeComponentTransform{});
    std::cout << typeid(VeEntityTypeNode).hash_code() << " " << typeid(VeEntityTypeNode).name() << std::endl;

    auto data1  = et.entity(h1);
    auto data1b = et.entity( std::get<VeHandle_t<VeEntityTypeNode>>(h1));

    et.erase(h1);

    VeHandle h2 = et.insert( VeComponentMaterial{}, VeComponentGeometry{});
    std::cout << typeid(VeEntityTypeDraw).hash_code() << " " << typeid(VeEntityTypeDraw).name() << std::endl;

    auto data2 = et.entity(h2);
    auto data2b = et.entity(std::get<VeHandle_t<VeEntityTypeDraw>>(h2));

    et.erase(h2);


    imgui();
    std::cout << "Hello world\n";
    return 0;
}

