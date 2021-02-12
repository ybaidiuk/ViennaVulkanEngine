#ifndef VEECS_H
#define VEECS_H

#include <limits>
#include <typeinfo>
#include <typeindex>
#include <variant>
#include "VGJS.h"
#include "VEUtil.h"
#include "VETypeList.h"
#include "VEComponent.h"

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
			, VeComponentRigidBody
			//, ...
		>
		, VeComponentTypeListUser	//components defined by the user
	>;
	using VeComponentPtr = tl::variant_type<tl::to_ptr<VeComponentTypeList>>;


	//-------------------------------------------------------------------------
	//entity type list and pointer

	using VeEntityTypeNode		= VeEntityType<VeComponentPosition, VeComponentOrientation, VeComponentTransform>;
	using VeEntityTypeDraw		= VeEntityType<VeComponentMaterial, VeComponentGeometry>;
	using VeEntityTypeAnimation = VeEntityType<VeComponentAnimation>;
	//...

	using VeEntityTypeList = tl::cat< tl::type_list<
			VeEntityType<>
			, VeEntityTypeNode
			, VeEntityTypeDraw
			, VeEntityTypeAnimation
			// ,... 
		>
		, VeEntityTypeListUser >;


	template <typename... Ts>
	struct VeEntity {
		std::tuple<Ts...> m_components;
	};

	using VeEntityPtr = tl::variant_type<tl::to_ptr<tl::transfer<VeEntityTypeList, VeEntity>>>;
	

	//-------------------------------------------------------------------------
	//entity handle

	template<typename E>
	struct VeHandle_t {
		index_t		m_entity_index{};	//the slot of the entity in the entity list
		counter_t	m_generation_counter{};		//generation counter
	};

	using VeHandle = tl::variant_type<tl::transform<VeEntityTypeList, VeHandle_t>>;


	//-------------------------------------------------------------------------
	//component vector

	class VeEntityManager;

	template<typename C>
	class VeComponentVector : public VeMonostate {
	protected:
		friend VeEntityManager;

		struct entry_t {
			C			m_component;
			VeHandle	m_handle;
			index_t*	m_map_pointer;
		};

		static inline std::vector<entry_t> m_component_vector;

	public:
		VeComponentVector() = default;
		index_t	add( VeHandle h, C&& component );
		void	erase(VeHandle&& h, index_t comp_index);
	};


	template<typename C>
	inline index_t VeComponentVector<C>::add(VeHandle h, C&& component) {
		m_component_vector.push_back({ component, h, nullptr });
		return index_t{ static_cast<typename index_t::type_name>(m_component_vector.size() - 1) };
	}


	template<typename C>
	inline void VeComponentVector<C>::erase(VeHandle&& h, index_t comp_index) {
		if (m_component_vector.size() == 0) return;
		if (comp_index.value < m_component_vector.size()-1) {
			assert(h == m_component_vector[comp_index.value].m_handle);
			m_component_vector[comp_index.value] = m_component_vector[m_component_vector.size() - 1];
			*m_component_vector[comp_index.value].m_map_pointer = comp_index;
		}
		m_component_vector.pop_back();
	}


	using VeComponentVectorPtr = tl::variant_type<tl::to_ptr<tl::transform<VeComponentTypeList, VeComponentVector>>>;

	inline VeComponentVector<VeComponentPosition> p1;
	inline VeComponentVectorPtr pp1{ &p1 };


	//-------------------------------------------------------------------------
	//references to components - each entity has them

	template<int I>
	class VeComponentMapTable : public VeMonostate {
	protected:
		using tuple_type = typename tl::N_tuple<index_t, I >::type; 

		struct entry_t {
			tuple_type	m_index_tuple;
			index_t		m_next{};
		};

		static inline std::vector<entry_t>	m_index_component;
		static inline index_t				m_first_free{};

	public:
		VeComponentMapTable(size_t r = 1 << 10);

		tuple_type& add(VeHandle h);
		tuple_type&	get(index_t index);
		void		erase(index_t idx);
	};


	template<int I>
	inline VeComponentMapTable<I>::VeComponentMapTable(size_t r) {
		if (!this->init()) return;
		m_index_component.reserve(r);
	};


	template<int I>
	typename VeComponentMapTable<I>::tuple_type& VeComponentMapTable<I>::add(VeHandle h) {
		index_t idx{};
		if (!m_first_free.is_null()) {
			idx = m_first_free;
			m_first_free = m_index_component[m_first_free.value].m_next;
		}
		else {
			idx.value = static_cast<typename index_t::type_name>(m_index_component.size());			//
			m_index_component.push_back({ {}, {} });	//
		}
		return m_index_component[idx.value].m_index_tuple;
	};


	template<int I>
	inline typename VeComponentMapTable<I>::tuple_type& VeComponentMapTable<I>::get(index_t index) {
		return m_index_component[index.value].m_index_tuple;
	}


	template<int I>
	void VeComponentMapTable<I>::erase(index_t index) {
		m_index_component[index.value].m_next = m_first_free;
		m_first_free = index;
	}


	//-------------------------------------------------------------------------
	//system

	template<typename T, typename VeSystemComponentTypeList = tl::type_list<>>
	class VeSystem : public VeMonostate, public VeCRTP<T, VeSystem> {
	protected:
	public:
		VeSystem() = default;
	};


	//-------------------------------------------------------------------------
	//entity manager is a system, and thus uses CRTP

	class VeEntityManager : public VeSystem<VeEntityManager> {
	protected:

		struct entry_t {
			counter_t			m_generation_counter{0};	//generation counter starts with 0
			index_t				m_next_free_or_map_index{};	//next free slot or index of reference table
			VeReadWriteMutex	m_mutex;					//per entity synchronization

			entry_t() {};
			entry_t(const entry_t& other) {};
			entry_t& operator=(const entry_t& other) {};
		};

		static inline std::vector<entry_t>	m_entity_table;
		static inline index_t				m_first_free{};

	public:
		VeEntityManager(size_t reserve = 1 << 10);

		template<typename E, typename... Ts>
		requires tl::is_same<E, Ts...>::value
		VeHandle create(E&& e, Ts&&... args);

		void erase(VeHandle& h);
	};
	

	inline VeEntityManager::VeEntityManager(size_t r) : VeSystem() {
		if (!this->init()) return;
		m_entity_table.reserve(r);
	}


	template<typename E, typename... Ts>
	requires tl::is_same<E, Ts...>::value
	inline VeHandle VeEntityManager::create(E&& e, Ts&&... args) {
		index_t idx{};
		if (!m_first_free.is_null()) {
			idx = m_first_free;
			m_first_free = m_entity_table[m_first_free.value].m_next_free_or_map_index;
		}
		else {
			idx.value = static_cast<typename index_t::type_name>(m_entity_table.size());	//index of new entity
			m_entity_table.emplace_back();		//start with counter 0
		}

		VeHandle h{ VeHandle_t<E>{ idx, m_entity_table[idx.value].m_generation_counter } };

		auto map = VeComponentMapTable<tl::size_of<E>::value>().add(h); //reference to map entry, a tuple of index_t
		auto tup = std::make_tuple( VeComponentVector<Ts>().add( h, std::forward<Ts>(args) )... ); //tuple with indices of the components

		tl::static_for<int, 0, sizeof...(Ts) - 1 >(
			[&](auto i) { 
				std::get<i>(map) = std::get<i>(tup);
				using type = typename std::tuple_element<i, std::tuple<Ts...>>::type;
				VeComponentVector<type>().m_component_vector[std::get<i>(tup).value].m_map_pointer = &std::get<i>(map);
			} 
		);

		return h;
	};


	inline void VeEntityManager::erase(VeHandle& handle) {


		/*auto erase_components = [&]<typename... Ts>( 
			typename tl::N_tuple<index_t, sizeof...(Ts)>::type& indextup, VeHandle_t<VeEntityType<Ts...>> &h) {

			tl::static_for<size_t, 0, sizeof...(Ts) - 1 >(
				[&](auto i) {
					( VeComponentVector<Ts>().erase( VeHandle{ h }, std::get<i>(indextup) ), ... );
				}
			);

		};*/

		auto erase_indices_lambda = [this]<typename E>( VeHandle_t<E> h ) {
			VeComponentMapTable<E> map;

			auto mapidx = m_entity_table[h.m_entity_index.value].m_next_free_or_map_index;

			//VeHandle hand{ h };
			//erase_comp<E>().erase(map.get(mapidx), hand);

			//erase_components( map.get(idx), h );

			map.erase(mapidx);
			return;
		};

	}


}

#endif
