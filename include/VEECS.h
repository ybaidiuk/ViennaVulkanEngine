#ifndef VEECS_H
#define VEECS_H

#include <limits>
#include <typeinfo>
#include <typeindex>
#include <variant>
#include <array>
#include <memory>
#include <optional>
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
	using VeComponentTypePtr = tl::variant_type<tl::to_ptr<VeComponentTypeList>>;


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


	//-------------------------------------------------------------------------
	//entity handle

	template<typename E>
	struct VeHandle_t {
		index_t		m_entity_index{};			//the slot of the entity in the entity list
		counter_t	m_generation_counter{};		//generation counter
	};

	using VeHandle = tl::variant_type<tl::transform<VeEntityTypeList, VeHandle_t>>;


	template <typename E>
	struct VeEntity_t {
		using tuple_type = typename tl::to_tuple<E>::type;
		VeHandle_t<E>	m_handle;
		tuple_type		m_component_data;

		VeEntity_t(VeHandle_t<E>& h, tuple_type& tup) : m_handle{ h }, m_component_data{ tup } {};

		template<typename C>
		std::optional<C&> get() {
			if constexpr (tl::has_type<E,C>::value) {
				{ std::get<tl::index_of<C, E>::value>(m_component_data); }
			}
			return {};
		};

		template<typename C>
		void set(C&& comp ) {
			if constexpr (tl::has_type<E, C>::value) {
				std::get<tl::index_of<C, E>::value>(m_component_data) = comp;
			}
			return;
		};

		std::string name() {
			return typeid(E).name();
		};
	};

	using VeEntityTypePtr = tl::variant_type<tl::to_ptr<tl::transform<VeEntityTypeList, VeEntity_t>>>;

	using VeEntity = tl::variant_type<tl::transform<VeEntityTypeList, VeEntity_t>>;


	//-------------------------------------------------------------------------
	//system

	template<typename T, typename VeSystemComponentTypeList = tl::type_list<>>
	class VeSystem : public VeMonostate, public VeCRTP<T, VeSystem> {
	protected:
	public:
		VeSystem() = default;
	};


	//-------------------------------------------------------------------------
	//component vector - each entity type has them

	template<typename E>
	class VeComponentVector : public VeMonostate {
	public:
		using tuple_type = typename tl::to_tuple<E>::type;

	protected:
		struct entry_t {
			VeHandle_t<E>	m_handle;
			tuple_type		m_component_data;
		};

		static inline std::vector<entry_t>	m_components;

	public:
		VeComponentVector(size_t r = 1 << 10);

		index_t								insert(VeHandle_t<E>& handle, tuple_type&& tuple);
		entry_t&							at(const index_t index);
		std::tuple<VeHandle_t<E>, index_t>	erase(const index_t idx);
	};


	template<typename E>
	inline VeComponentVector<E>::VeComponentVector(size_t r) {
		if (!this->init()) return;
		m_components.reserve(r);
	};


	template<typename E>
	inline index_t VeComponentVector<E>::insert(VeHandle_t<E>& handle, tuple_type&& tuple) {
		m_components.emplace_back(handle, tuple);
		return index_t{ static_cast<typename index_t::type_name>(m_components.size() - 1) };
	};


	template<typename E>
	inline typename VeComponentVector<E>::entry_t& VeComponentVector<E>::at(const index_t index) {
		assert(index.value < m_components.size());
		return m_components[index.value];
	}


	template<typename E>
	inline std::tuple<VeHandle_t<E>, index_t> VeComponentVector<E>::erase(const index_t index) {
		assert(index.value < m_components.size());
		if (index.value < m_components.size() - 1) {
			std::swap(m_components[index.value], m_components[m_components.size() - 1]);
			m_components.pop_back();
			return std::make_pair(m_components[index.value].m_handle, index);
		}
		m_components.pop_back();
		return std::make_tuple(VeHandle_t<E>{}, index_t{});
	}


	//-------------------------------------------------------------------------
	//entity table base class

	class VeEntityTableBaseClass : public VeMonostate {
	protected:

		struct entry_t {
			counter_t			m_generation_counter{ 0 };		//generation counter starts with 0
			index_t				m_next_free_or_comp_index{};	//next free slot or index of component table
			VeReadWriteMutex	m_mutex;						//per entity synchronization

			entry_t() {};
			entry_t(const entry_t& other) {};
			entry_t& operator=(const entry_t& other) {};
		};

		static inline std::vector<entry_t>	m_entity_table;
		static inline index_t				m_first_free{};

		std::array<std::unique_ptr<VeEntityTableBaseClass>, tl::size_of<VeEntityTypeList>::value> m_dispatch;

		virtual std::optional<VeEntity> entityE(const VeHandle& handle) { return {}; };

		virtual bool updateE(const VeHandle& handle, VeEntity& ent) { return false; };

	public:
		VeEntityTableBaseClass( size_t r = 1 << 10 );

		template<typename... Ts>
		VeHandle insert(Ts&&... args);

		std::optional<VeEntity> entity( const VeHandle &handle);

		template<typename E>
		std::optional<VeEntity_t<E>> entity(const VeHandle_t<E>& handle);

		template<typename C>
		requires tl::has_type<VeComponentTypeList, C>::value
		std::optional<C> component(const VeHandle& handle);

		bool update(const VeHandle& handle, VeEntity& ent);

		template<typename E>
		bool update(const VeHandle_t<E>& handle, VeEntity_t<E>& ent);

		template<typename C>
		//requires tl::has_type<VeComponentTypeList, C>::value
		bool update(const VeHandle& handle, C& comp);

		virtual bool contains(const VeHandle& handle);

		virtual void erase(const VeHandle& handle);
	};

	template<typename... Ts>
	inline VeHandle VeEntityTableBaseClass::insert(Ts&&... args) {
		static_assert(tl::is_same<VeEntityType<Ts...>, Ts...>::value);
		return VeEntityTable<VeEntityType<Ts...>>().insert(std::forward<Ts>(args)...);
	}

	inline std::optional<VeEntity> VeEntityTableBaseClass::entity(const VeHandle& handle) {
		return m_dispatch[handle.index()]->entityE(handle);
	}

	template<typename C>
	requires tl::has_type<VeComponentTypeList, C>::value
	inline std::optional<C> VeEntityTableBaseClass::component(const VeHandle& handle) {
		return m_dispatch[handle.index()]->component<C>(handle);
	}

	inline bool VeEntityTableBaseClass::update(const VeHandle& handle, VeEntity& ent) {
		return m_dispatch[handle.index()]->updateE(handle, ent);
	}

	template<typename C>
	//requires tl::has_type<VeComponentTypeList, C>::value
	inline bool VeEntityTableBaseClass::update(const VeHandle& handle, C& comp) {
		return m_dispatch[handle.index()]->update<C>(handle, comp);
	}

	inline bool VeEntityTableBaseClass::contains(const VeHandle& handle) {
		return m_dispatch[handle.index()]->contains(handle);
	}

	inline void VeEntityTableBaseClass::erase(const VeHandle& handle) {
		m_dispatch[handle.index()]->erase(handle);
	}


	//-------------------------------------------------------------------------
	//entity table

	template<typename E = void>
	class VeEntityTable : public VeEntityTableBaseClass {
	protected:
		virtual std::optional<VeEntity> entityE(const VeHandle& handle);

		virtual bool updateE(const VeHandle& handle, VeEntity& ent);

	public:
		VeEntityTable(size_t r = 1 << 10);

		template<typename... Cs>
		requires tl::is_same<E, Cs...>::value
		VeHandle insert(Cs&&... args);

		std::optional<VeEntity_t<E>> entity(const VeHandle& h);

		template<typename C>
		std::optional<C> component(const VeHandle& handle);

		bool update(const VeHandle& handle, VeEntity_t<E>& ent);

		template<typename C>
		//requires tl::has_type<VeComponentTypeList, C>::value
		bool update(const VeHandle& handle, C& comp);

		bool contains(const VeHandle& handle);

		void erase(const VeHandle& handle);
	};


	template<typename E>
	inline VeEntityTable<E>::VeEntityTable(size_t r) : VeEntityTableBaseClass(r) {}

	template<typename E>
	inline std::optional<VeEntity> VeEntityTable<E>::entityE(const VeHandle& handle) {
		std::optional<VeEntity_t<E>> ent = entity(handle);
		if (ent.has_value()) return { VeEntity{ *ent } };
		return {};
	}
	
	template<typename E>
	inline bool VeEntityTable<E>::updateE(const VeHandle& handle, VeEntity& ent) {
		return update( std::get<VeHandle_t<E>>(handle), std::get<VeEntity_t<E>>(ent));
	}

	template<typename E>
	template<typename... Cs>
	requires tl::is_same<E, Cs...>::value
	inline VeHandle VeEntityTable<E>::insert(Cs&&... args) {
		index_t idx{};
		if (!m_first_free.is_null()) {
			idx = m_first_free;
			m_first_free = m_entity_table[m_first_free.value].m_next_free_or_comp_index;
		}
		else {
			idx.value = static_cast<typename index_t::type_name>(m_entity_table.size());	//index of new entity
			m_entity_table.emplace_back();		//start with counter 0
		}

		VeHandle_t<E> handle{ idx, m_entity_table[idx.value].m_generation_counter };
		index_t compidx = VeComponentVector<E>().insert(handle, std::make_tuple(args...));	//add data as tuple
		m_entity_table[idx.value].m_next_free_or_comp_index = compidx;						//index in component vector 
		return { handle };
	};

	template<typename E>
	inline bool VeEntityTable<E>::contains(const VeHandle& handle) {
		VeHandle_t<E> h = std::get<VeHandle_t<E>>(handle);
		if (h.m_generation_counter != m_entity_table[h.m_entity_index.value].m_generation_counter) return false;
		return true;
	}

	template<typename E>
	inline std::optional<VeEntity_t<E>> VeEntityTable<E>::entity(const VeHandle& handle) {
		if (!contains(handle)) return {};
		VeHandle_t<E> h = std::get<VeHandle_t<E>>(handle);
		VeEntity_t<E> res( h, VeComponentVector<E>().at(m_entity_table[h.m_entity_index.value].m_next_free_or_comp_index).m_component_data );
		return { res };
	}

	template<typename E>
	template<typename C>
	inline std::optional<C> VeEntityTable<E>::component(const VeHandle& handle) {
		if constexpr (!tl::has_type<E,C>::value) { return {}; }
		if (!contains(handle)) return {};
		VeHandle_t<E> h = std::get<VeHandle_t<E>>(handle);

		auto compidx = m_entity_table[h.m_entity_index.value].m_next_free_or_comp_index;
		auto tuple = VeComponentVector<E>().at(compidx).m_component_data;
		return std::get<tl::index_of<C, E>::value>(tuple);
	}

	template<typename E>
	inline bool VeEntityTable<E>::update(const VeHandle& handle, VeEntity_t<E>& ent) {
		if (!contains(handle)) return false;
		VeHandle_t<E> h = std::get<VeHandle_t<E>>(handle);
		auto entry = VeComponentVector<E>().at(h.m_entity_index);
		entry.m_component_data = ent.m_component_data;
		return true;
	}

	template<typename E>
	template<typename C>
	//requires tl::has_type<VeComponentTypeList, C>::value
	inline bool VeEntityTable<E>::update(const VeHandle& handle, C& comp) {
		if (!contains(handle)) return false;
		return true;
	}

	template<typename E>
	inline void VeEntityTable<E>::erase(const VeHandle& handle) {
		if (!contains(handle)) return;
		VeHandle_t<E> h = std::get<VeHandle_t<E>>(handle);
		auto hidx = h.m_entity_index.value;

		auto [corr_hndl, corr_index] = VeComponentVector<E>().erase(m_entity_table[hidx].m_next_free_or_comp_index);
		if (!corr_index.is_null()) { m_entity_table[corr_hndl.m_entity_index.value].m_next_free_or_comp_index = corr_index; }

		m_entity_table[hidx].m_generation_counter.value++;				//>invalidate the entity handle
		m_entity_table[hidx].m_next_free_or_comp_index = m_first_free;	//>put old entry into free list
		m_first_free = h.m_entity_index;
	}


	//-------------------------------------------------------------------------
	//entity table specialization for void

	template<>
	class VeEntityTable<void> : public VeEntityTableBaseClass {
	public:
		VeEntityTable(size_t r = 1 << 10) : VeEntityTableBaseClass(r) {};
	};


	//-------------------------------------------------------------------------
	//entity table base class implementations needing the entity table derived classes

	inline VeEntityTableBaseClass::VeEntityTableBaseClass(size_t r) {
		if (!this->init()) return;
		m_entity_table.reserve(r);

		tl::static_for<size_t, 0, tl::size_of<VeEntityTypeList>::value >(
			[this](auto i) {
				using type = tl::Nth_type<i, VeEntityTypeList>;
				m_dispatch[i] = std::make_unique<VeEntityTable<type>>();
			}
		);
	}


	template<typename E>
	inline std::optional<VeEntity_t<E>> VeEntityTableBaseClass::entity(const VeHandle_t<E>& handle) {
		return VeEntityTable<E>().entity({handle});
	}

	template<typename E>
	bool VeEntityTableBaseClass::update(const VeHandle_t<E>& handle, VeEntity_t<E>& ent) {
		VeEntityTable<E>().update({ handle }, ent);
	}


}

#endif
