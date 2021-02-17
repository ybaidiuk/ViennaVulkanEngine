#ifndef VECOMPONENT_H
#define VECOMPONENT_H

#include "glm.hpp"
#include "gtc/quaternion.hpp"

#include "VTL.h"


namespace vecs {

	//-------------------------------------------------------------------------
	//component types

	struct VeComponent {};

	struct VeComponentPosition {
		glm::vec3 m_position;
	};

	struct VeComponentOrientation {
		glm::quat m_orientation;
	};

	struct VeComponentTransform {
		glm::mat4 m_transform;
	};

	struct VeComponentMaterial {
		int i;
	};

	struct VeComponentGeometry {
	};

	struct VeComponentAnimation {
	};

	struct VeComponentCollisionShape {
	};

	struct VeComponentRigidBody {
	};

	using VeComponentTypeListSystem = vtl::type_list<
		VeComponentPosition
		, VeComponentOrientation
		, VeComponentTransform
		, VeComponentMaterial
		, VeComponentGeometry
		, VeComponentAnimation
		, VeComponentCollisionShape
		, VeComponentRigidBody
		//, ...
	>;

	//-------------------------------------------------------------------------
	//entity types

	template <typename... Ts>
	struct VeEntityType {};


	//-------------------------------------------------------------------------
	//engine entity types

	using VeEntityNode = VeEntityType<VeComponentPosition, VeComponentOrientation, VeComponentTransform>;
	using VeEntityDraw = VeEntityType<VeComponentMaterial, VeComponentGeometry>;
	using VeEntityAnimation = VeEntityType<VeComponentAnimation>;
	//...

	using VeEntityTypeListSystem = vtl::type_list<
		  VeEntityNode
		, VeEntityDraw
		, VeEntityAnimation
		// ,... 
	>;

}

#endif

