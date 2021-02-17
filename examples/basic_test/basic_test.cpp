
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
    std::cout << vtl::size<VeEntityTypeList>::value << std::endl;

    VeHandle h1 = et.insert(VeComponentPosition{ glm::vec3{9.0f, 2.0f, 3.0f} }, VeComponentOrientation{}, VeComponentTransform{});
    std::cout << typeid(VeEntityNode).hash_code() << " " << typeid(VeEntityNode).name() << std::endl;

    auto data1  = et.entity(h1);
    auto data1b = et.entity<VeEntityNode>(h1).value();
    auto comp1_1 = data1b.component<VeComponentPosition>();

    auto comp1_2 = et.component<VeComponentPosition>(h1);
    auto comp1_3 = et.component<VeComponentMaterial>(h1);

    et.update<VeComponentPosition>(h1, VeComponentPosition{ glm::vec3{-9.0f, -2.0f, -3.0f} });
    auto comp1_4 = et.component<VeComponentPosition>(h1);

    data1b.update<VeComponentPosition>(VeComponentPosition{ glm::vec3{-999.0f, -2.0f, -3.0f} });
    et.update(h1, data1b);
    auto comp1_5 = et.component<VeComponentPosition>(h1);

    et.erase(h1);


    auto data1c = et.entity<VeEntityNode>(h1);

    VeHandle h2 = et.insert(VeComponentMaterial{ 99 }, VeComponentGeometry{});
    std::cout << typeid(VeEntityDraw).hash_code() << " " << typeid(VeEntityDraw).name() << std::endl;

    auto data2 = et.entity(h2);
    auto data2b = et.entity<VeEntityDraw>(h2);
    auto comp2_1 = data2b.value().component<VeComponentMaterial>();
    auto comp2_2 = et.component<VeComponentMaterial>(h2);

    et.erase(h2);


    imgui();
    std::cout << "Hello world\n";
    return 0;
}

