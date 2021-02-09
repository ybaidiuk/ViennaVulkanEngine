#ifndef VEECS_H
#define VEECS_H

#include <limits>
#include <typeinfo>
#include <typeindex>
#include <variant>
#include "glm.hpp"
#include "gtc/quaternion.hpp"
#include "VGJS.h"
#include "VEContainer.h"
#include "VEUtil.h"

namespace vve {

	//-------------------------------------------------------------------------
	//components

	template<typename T>
	struct VeComponent : crtp<T, VeComponent> {
	};

	struct VeComponentPosition : VeComponent<VeComponentPosition> {
		glm::vec3 m_position;
	};

	struct VeComponentOrientation : VeComponent<VeComponentOrientation> {
		glm::quat m_orientation;
	};

	struct VeComponentTransform : VeComponent<VeComponentTransform> {
		glm::mat4 m_transform;
	};

	struct VeComponentMaterial : VeComponent<VeComponentMaterial> {
	};

	struct VeComponentGeometry : VeComponent<VeComponentGeometry> {
	};

	struct VeComponentAnimation : VeComponent<VeComponentAnimation> {
	};

	struct VeComponentCollisionShape : VeComponent<VeComponentCollisionShape> {
	};

	struct VeComponentBody : VeComponent<VeComponentBody> {
	};

	//-------------------------------------------------------------------------
	//entities

	template <typename... Ts>
	using VeEntity = tl::type_list<Ts...>;

	template<typename T>
	struct VeHandle_t;
}

//user defined component types and entity types
#include "VEECSUser.h" 


namespace vve {

	//-------------------------------------------------------------------------
	//component type list, pointer, pool

	using VeComponentTypeList = tl::cat<tl::type_list<
			  VeComponentPosition
			, VeComponentOrientation
			, VeComponentTransform
			, VeComponentMaterial
			, VeComponentGeometry
			, VeComponentAnimation
			, VeComponentCollisionShape
			, VeComponentBody 
			//, ...
		>,
		VeComponentTypeListUser
	>;
	using VeComponentPtr = tl::variant_type<tl::to_ptr<VeComponentTypeList>>;


	//-------------------------------------------------------------------------
	//entity type list and pointer

	using VeEntityNode = VeEntity<VeComponentPosition, VeComponentOrientation, VeComponentTransform>;
	using VeEntityDraw = VeEntity<VeComponentMaterial, VeComponentGeometry>;
	using VeEntityAnimation = VeEntity<VeComponentAnimation>;
	//...

	using VeEntityTypeList_1 = tl::type_list<
		VeEntity<>
		, VeEntityNode
		, VeEntityDraw
		, VeEntityAnimation
		// ,... 
	>;

	using VeEntityTypeList = tl::cat< VeEntityTypeList_1, VeEntityTypeListUser >;
	using VeEntityPtr = tl::variant_type<tl::to_ptr<tl::transform<VeEntityTypeList, VeEntity>>>;


	//-------------------------------------------------------------------------
	//entity handle

	template<typename E>
	struct VeHandle_t {
		index_t		m_index{ typeid(std::decay_t<E>).hash_code() };		//the slot of the entity in the entity list
		counter_t	m_counter{};	//generation counter
	};

	using VeHandle = tl::variant_type<tl::transform<VeEntityTypeList, VeHandle_t>>;


	//-------------------------------------------------------------------------
	//component pool

	template<typename C>
	class VeComponentPool : public crtp<VeComponentPool<C>, VeComponentPool> {
	protected:

		struct entry_t {
			C m_component;
			VeHandle m_handle;
		};

		static inline std::vector<entry_t> m_component;

	public:
		VeComponentPool() = default;
		void add(VeHandle&& h, C&& component);
	};

	template<typename C>
	inline void VeComponentPool<C>::add(VeHandle&& h, C&& component) {
		int i = 0;
	}

	using VeComponentPoolPtr = tl::variant_type<tl::to_ptr<tl::transform<VeComponentTypeList, VeComponentPool>>>;

	inline VeComponentPool<VeComponentPosition> p1;
	inline VeComponentPoolPtr pp1{ &p1 };


	//-------------------------------------------------------------------------
	//systems

	template<typename E>
	class VeComponentReferencePool : public crtp<VeComponentReferencePool<E>, VeComponentReferencePool> {
	protected:
		using tuple_type = typename tl::to_ref_tuple<E>::type;

		struct entry_t {
			tuple_type m_entry;
			index_t m_next{};
		};

		static inline std::vector<entry_t>	m_ref_index;
		static inline index_t				m_first_free{};

	public:
		VeComponentReferencePool(size_t r = 1 << 10);

		template<typename U >
		requires std::is_same_v<std::decay_t<U>, typename VeComponentReferencePool<E>::tuple_type>
		index_t add(VeHandle& h, U&& ref);
	};


	template<typename T>
	inline VeComponentReferencePool<T>::VeComponentReferencePool(size_t r) {
		if (!this->init()) return;
		m_ref_index.reserve(r);
	};


	template<typename E>
	template<typename U>
	requires std::is_same_v<std::decay_t<U>, typename VeComponentReferencePool<E>::tuple_type>
	index_t VeComponentReferencePool<E>::add(VeHandle& h, U&& ref) {
		index_t idx{};
		if (!m_first_free.is_null()) {
			idx = m_first_free;
			m_first_free = m_ref_index[m_first_free.value].m_next;
		}
		else {
			idx.value = m_ref_index.size();	//index of new entity
			m_ref_index.push_back({}); //start with counter 0
		}
		return idx;
	};



	//-------------------------------------------------------------------------
	//system

	template<typename T, typename Seq = tl::type_list<>>
	class VeSystem : public crtp<T, VeSystem> {
	protected:
	public:
		VeSystem() = default;
	};


	//-------------------------------------------------------------------------
	//entity manager

	class VeEntityManager : public VeSystem<VeEntityManager> {
	protected:

		struct entry_t {
			VeHandle	m_handle{};	//entity handle
			index_t		m_index{};	//next free slot or index of reference pool
		};

		static inline std::vector<entry_t>	m_entity;
		static inline index_t				m_first_free{};

		index_t get_ref_pool_index(VeHandle& h);

	public:
		VeEntityManager(size_t reserve = 1 << 10);

		template<typename E, typename... Ts>
		requires tl::is_same<E, Ts...>::value
		VeHandle create(E&& e, Ts&&... args);

		void erase(VeHandle& h);
	};


	inline index_t VeEntityManager::get_ref_pool_index(VeHandle& h) {
		return {};
	}


	template<typename E, typename... Ts>
	requires tl::is_same<E, Ts...>::value
	inline VeHandle VeEntityManager::create(E&& e, Ts&&... args) {
		index_t idx{};
		if (!m_first_free.is_null()) {
			idx = m_first_free;
			m_first_free = m_entity[m_first_free.value].m_index;
		}
		else {
			idx.value = m_entity.size();	//index of new entity
			m_entity.push_back({}); //start with counter 0
		}
		VeHandle_t<E> h{ idx, counter_t{0} };
		(VeComponentPool<Ts>().add(VeHandle{ h }, std::forward<Ts>(args)), ...);
		return { h };
	};


	inline void VeEntityManager::erase(VeHandle& handle) {
		auto erase_handle = [this]<typename E>(VeHandle_t<E> & h) {
			//VeComponentReferencePool<E>().erase(h.m_next);
		};

		std::visit(erase_handle, handle);
	}


}

#endif
