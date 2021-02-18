#ifndef VECS_H
#define VECS_H

#include <limits>
#include <typeinfo>
#include <typeindex>
#include <variant>
#include <array>
#include <memory>
#include <optional>
#include "VGJS.h"
#include "VECSUtil.h"
#include "VTL.h"
#include "VECSComponent.h"

//user defined component types and entity types
#include "VECSUser.h" 


namespace vecs {

	//-------------------------------------------------------------------------
	//component type list and pointer

	using VeComponentTypeList = vtl::cat< VeComponentTypeListSystem, VeComponentTypeListUser >;
	using VeComponentPtr = vtl::variant_type<vtl::to_ptr<VeComponentTypeList>>;


	//-------------------------------------------------------------------------
	//entity type list

	using VeEntityTypeList = vtl::cat< VeEntityTypeListSystem, VeEntityTypeListUser >;


	//-------------------------------------------------------------------------
	//entity handle

	/**
	* \brief Handles are IDs of entities. Use them to access entitites.
	* VeHandle_t<E> are used to ID entities opf type E.
	*/

	template<typename E>
	struct VeHandle_t {
		index_t		m_entity_index{};			//the slot of the entity in the entity list
		counter_t	m_generation_counter{};		//generation counter
	};

	/**
	* \brief A summary handle type that can be used to ID any entity type.
	*/

	using VeHandle = vtl::variant_type<vtl::transform<VeEntityTypeList, VeHandle_t>>;

	/**
	* \brief This struct can hold the data of an entity of type E. This includes its handle
	* and all components.
	*/

	template <typename E>
	struct VeEntity_t {
		using tuple_type = typename vtl::to_tuple<E>::type;
		VeHandle_t<E>	m_handle;
		tuple_type		m_component_data;

		VeEntity_t(const VeHandle_t<E>& h, const tuple_type& tup) noexcept : m_handle{ h }, m_component_data{ tup } {};

		template<typename C>
		std::optional<C> component() noexcept {
			if constexpr (vtl::has_type<E,C>::value) {
				return { std::get<vtl::index_of<E,C>::value>(m_component_data) };
			}
			return {};
		};

		template<typename C>
		void update(C&& comp ) noexcept {
			if constexpr (vtl::has_type<E,C>::value) {
				std::get<vtl::index_of<E,C>::value>(m_component_data) = comp;
			}
			return;
		};

		std::string name() {
			return typeid(E).name();
		};
	};

	using VeEntity = vtl::variant_type<vtl::transform<VeEntityTypeList, VeEntity_t>>;
	using VeEntityPtr = vtl::variant_type<vtl::to_ptr<vtl::transform<VeEntityTypeList, VeEntity_t>>>;


	//-------------------------------------------------------------------------
	//component vector - each entity type has them

	class VeEntityTableBaseClass;

	template<typename E>
	class VeEntityTable;

	/**
	* \brief This class stores all components of entities of type E
	*/

	template<typename E>
	class VeComponentVector : public VeMonostate<VeComponentVector<E>> {
		friend class VeEntityTableBaseClass;

		template<typename E>
		friend class VeEntityTable;

	public:
		using tuple_type	 = typename vtl::to_tuple<E>::type;
		using tuple_type_ref = typename vtl::to_ref_tuple<E>::type;
		using tuple_type_vec = typename vtl::to_tuple<vtl::transform<E,std::pmr::vector>>::type;

	protected:
		struct entry_t {
			VeHandle_t<E>	m_handle;
		};

		static inline std::vector<entry_t>	m_handles;
		static inline tuple_type_vec		m_components;

		static inline std::array<std::unique_ptr<VeComponentVector<E>>, vtl::size<VeComponentTypeList>::value> m_dispatch; //one for each component type

		virtual bool updateC(index_t entidx, size_t compidx, void* ptr, size_t size) {
			return m_dispatch[compidx]->updateC(entidx, compidx, ptr, size);
		}

		virtual bool componentE(index_t entidx, size_t compidx, void* ptr, size_t size) { 
			return m_dispatch[compidx]->componentE(entidx, compidx, ptr, size);
		}

	public:
		VeComponentVector(size_t r = 1 << 10);

		index_t			insert(VeHandle_t<E>& handle, tuple_type&& tuple);
		tuple_type		values(const index_t index);
		tuple_type_ref	references(const index_t index);
		bool			update(const index_t index, VeEntity_t<E>&& ent);
		size_t			size() { return m_handles.size(); };

		std::tuple<VeHandle_t<E>, index_t> erase(const index_t idx);
	};

	template<typename E>
	inline index_t VeComponentVector<E>::insert(VeHandle_t<E>& handle, tuple_type&& tuple) {
		m_handles.emplace_back(handle);

		vtl::static_for<size_t, 0, vtl::size<E>::value >(
			[&](auto i) {
				std::get<i>(m_components).push_back(std::get<i>(tuple));
			}
		);

		return index_t{ static_cast<typename index_t::type_name>(m_handles.size() - 1) };
	};


	template<typename E>
	inline typename VeComponentVector<E>::tuple_type VeComponentVector<E>::values(const index_t index) {
		assert(index.value < m_handles.size());

		auto f = [&]<typename... Cs>(std::tuple<std::pmr::vector<Cs>...>& tup) {
			return std::make_tuple(std::get<vtl::index_of<E, Cs>::value>(tup)[index.value]...);
		};

		return f(m_components);
	}

	template<typename E>
	inline typename VeComponentVector<E>::tuple_type_ref VeComponentVector<E>::references(const index_t index) {
		assert(index.value < m_handles.size());

		auto f = [&]<typename... Cs>(std::tuple<std::pmr::vector<Cs>...>& tup) {
			return std::tie( std::get<vtl::index_of<E,Cs>::value>(tup)[index.value]... );
		};

		return f(m_components);
	}

	template<typename E>
	inline bool VeComponentVector<E>::update(const index_t index, VeEntity_t<E>&& ent) {
		vtl::static_for<size_t, 0, vtl::size<E>::value >(
			[&](auto i) {
				using type = vtl::Nth_type<E, i>;
				std::get<i>(m_components)[index.value] = ent.component<type>().value();
			}
		);
		return true;
	}

	template<typename E>
	inline std::tuple<VeHandle_t<E>, index_t> VeComponentVector<E>::erase(const index_t index) {
		assert(index.value < m_handles.size());
		if (index.value < m_handles.size() - 1) {
			std::swap(m_handles[index.value], m_handles[m_handles.size() - 1]);
			m_handles.pop_back();
			return std::make_pair(m_handles[index.value].m_handle, index);
		}
		m_handles.pop_back();
		return std::make_tuple(VeHandle_t<E>{}, index_t{});
	}


	//-------------------------------------------------------------------------
	//comnponent vector derived class

	/**
	* \brief This class is derived from the compoentn vector and is used to update or
	* return components C of entities of type E
	*/

	template<typename E, typename C>
	class VeComponentVectorDerived : public VeComponentVector<E> {
	public:
		VeComponentVectorDerived( size_t res = 1 << 10 ) : VeComponentVector<E>(res) {};

		bool update(const index_t index, C&& comp) {
			if constexpr (vtl::has_type<E, C>::value) {
				std::get< vtl::index_of<E, C>::type::value >(this->m_components)[index.value] = comp;
				return true;
			}
			return false;
		}

		bool updateC(index_t entidx, size_t compidx, void* ptr, size_t size) {
			if constexpr (vtl::has_type<E, C>::value) {
				auto tuple = this->references(entidx);
				memcpy((void*)&std::get<vtl::index_of<E,C>::value>(tuple), ptr, size);
				return true;
			}
			return false;
		};

		bool componentE(index_t entidx, size_t compidx, void* ptr, size_t size) {
			if constexpr (vtl::has_type<E,C>::value) {
				auto tuple = this->references(entidx);
				memcpy(ptr, (void*)&std::get<vtl::index_of<E,C>::value>(tuple), size);
				return true;
			}
			return false;
		};
	};


	template<typename E>
	inline VeComponentVector<E>::VeComponentVector(size_t r) {
		if (!this->init()) return;
		m_handles.reserve(r);

		vtl::static_for<size_t, 0, vtl::size<VeComponentTypeList>::value >(
			[&](auto i) {
				using type = vtl::Nth_type<VeComponentTypeList, i>;
				m_dispatch[i] = std::make_unique<VeComponentVectorDerived<E, type>>(r);
			}
		);
	};



	//-------------------------------------------------------------------------
	//entity table base class

	template<typename... Cs>
	class VeIterator;


	/**
	* \brief This class stores all generalized handles of all entities, and can be used
	* to insert, update, read and delete all entity types.
	*/

	class VeEntityTableBaseClass : public VeMonostate<VeEntityTableBaseClass> {
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

		static inline std::array<std::unique_ptr<VeEntityTableBaseClass>, vtl::size<VeEntityTypeList>::value> m_dispatch;

		virtual std::optional<VeEntity> entityE(const VeHandle& handle) { return {}; };
		virtual bool updateE(const VeHandle& handle, VeEntity&& ent) { return false; };
		virtual bool updateC(const VeHandle& handle, size_t compidx, void* ptr, size_t size) { return false; };
		virtual bool componentE(const VeHandle& handle, size_t compidx, void*ptr, size_t size) { return false; };

	public:
		VeEntityTableBaseClass( size_t r = 1 << 10 );

		//-------------------------------------------------------------------------
		//insert data

		template<typename... Ts>
		VeHandle insert(Ts&&... args) {
			static_assert(vtl::is_same<VeEntityType<Ts...>, Ts...>::value);
			return VeEntityTable<VeEntityType<Ts...>>().insert(std::forward<Ts>(args)...);
		}

		//-------------------------------------------------------------------------
		//get data

		std::optional<VeEntity> entity( const VeHandle &handle) {
			return m_dispatch[handle.index()]->entityE(handle);
		}

		template<typename E>
		std::optional<VeEntity_t<E>> entity(const VeHandle& handle);

		template<typename C>
		requires vtl::has_type<VeComponentTypeList, C>::value
		std::optional<C> component(const VeHandle& handle) {
			C res;
			if (m_dispatch[handle.index()]->componentE(handle, vtl::index_of<VeComponentTypeList, C>::value, (void*)&res, sizeof(C))) {
				return { res };
			}
			return {};
		}

		//-------------------------------------------------------------------------
		//update data

		bool update(const VeHandle& handle, VeEntity&& ent) {
			return m_dispatch[handle.index()]->updateE(handle, std::forward<VeEntity>(ent));
		}

		template<typename E>
		requires vtl::has_type<VeEntityTypeList, E>::value
		bool update(const VeHandle& handle, VeEntity_t<E>&& ent);

		template<typename C>
		requires vtl::has_type<VeComponentTypeList, C>::value
		bool update(const VeHandle& handle, C&& comp) {
			return m_dispatch[handle.index()]->updateC(handle, vtl::index_of<VeComponentTypeList,C>::value, (void*)&comp, sizeof(C));
		}

		//-------------------------------------------------------------------------
		//utility

		template<typename E = void>
		size_t size() { return VeComponentVector<E>().size(); };

		template<>
		size_t size<>();

		template<typename... Cs>
		VeIterator<Cs...> begin();

		template<typename... Cs>
		VeIterator<Cs...> end();

		virtual bool contains(const VeHandle& handle) {
			return m_dispatch[handle.index()]->contains(handle);
		}

		virtual void erase(const VeHandle& handle) {
			m_dispatch[handle.index()]->erase(handle);
		}
	};


	template<>
	size_t VeEntityTableBaseClass::size<void>() {
		size_t sum = 0;
		vtl::static_for<size_t, 0, vtl::size<VeEntityTypeList>::value >(
			[&](auto i) {
				using type = vtl::Nth_type<VeEntityTypeList, i>;
				sum += VeComponentVector<type>().size();
			}
		);
		return sum;
	}



	//-------------------------------------------------------------------------
	//entity table

	/**
	* \brief This class is used as access interface for all entities of type E
	*/

	template<typename E = void>
	class VeEntityTable : public VeEntityTableBaseClass {
	protected:
		std::optional<VeEntity> entityE(const VeHandle& handle);
		bool					updateE(const VeHandle& handle, VeEntity&& ent);
		bool					updateC(const VeHandle& handle, size_t compidx, void* ptr, size_t size);
		bool					componentE(const VeHandle& handle, size_t compidx, void* ptr, size_t size);

	public:
		VeEntityTable(size_t r = 1 << 10);

		//-------------------------------------------------------------------------
		//insert data

		template<typename... Cs>
		requires vtl::is_same<E, Cs...>::value
		VeHandle insert(Cs&&... args);

		//-------------------------------------------------------------------------
		//get data

		std::optional<VeEntity_t<E>> entity(const VeHandle& h);

		template<typename C>
		std::optional<C> component(const VeHandle& handle);

		//-------------------------------------------------------------------------
		//update data

		bool update(const VeHandle& handle, VeEntity_t<E>&& ent);

		template<typename C>
		requires (vtl::has_type<VeComponentTypeList, C>::value)
		bool update(const VeHandle& handle, C&& comp);

		//-------------------------------------------------------------------------
		//utility

		size_t size() { return VeComponentVector<E>().size(); };

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
	inline bool VeEntityTable<E>::updateE(const VeHandle& handle, VeEntity&& ent) {
		return update( std::get<VeHandle_t<E>>(handle), std::get<VeEntity_t<E>>(std::forward<VeEntity>(ent)));
	}

	template<typename E>
	inline bool VeEntityTable<E>::updateC(const VeHandle& handle, size_t compidx, void* ptr, size_t size) {
		if (!contains(handle)) return {};
		VeHandle_t<E> h = std::get<VeHandle_t<E>>(handle);
		return VeComponentVector<E>().updateC(m_entity_table[h.m_entity_index.value].m_next_free_or_comp_index, compidx, ptr, size);
	}

	template<typename E>
	inline bool VeEntityTable<E>::componentE(const VeHandle& handle, size_t compidx, void* ptr, size_t size) {
		if (!contains(handle)) return {};
		VeHandle_t<E> h = std::get<VeHandle_t<E>>(handle);
		return VeComponentVector<E>().componentE( m_entity_table[h.m_entity_index.value].m_next_free_or_comp_index, compidx, ptr, size );
	}

	template<typename E>
	template<typename... Cs>
	requires vtl::is_same<E, Cs...>::value
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
		VeEntity_t<E> res( h, VeComponentVector<E>().values(m_entity_table[h.m_entity_index.value].m_next_free_or_comp_index) );
		return { res };
	}

	template<typename E>
	template<typename C>
	inline std::optional<C> VeEntityTable<E>::component(const VeHandle& handle) {
		if constexpr (!vtl::has_type<E,C>::value) { return {}; }
		if (!contains(handle)) return {};
		VeHandle_t<E> h = std::get<VeHandle_t<E>>(handle);

		auto compidx = m_entity_table[h.m_entity_index.value].m_next_free_or_comp_index;
		auto tuple = VeComponentVector<E>().references(compidx); //
		return { std::get<vtl::index_of<E,C>::value>(tuple) };
	}

	template<typename E>
	inline bool VeEntityTable<E>::update(const VeHandle& handle, VeEntity_t<E>&& ent) {
		if (!contains(handle)) return false;
		VeHandle_t<E> h = std::get<VeHandle_t<E>>(handle);
		VeComponentVector<E>().update(h.m_entity_index, std::forward<VeEntity_t<E>>(ent));
		return true;
	}

	template<typename E>
	template<typename C>
	requires (vtl::has_type<VeComponentTypeList, C>::value)
	inline bool VeEntityTable<E>::update(const VeHandle& handle, C&& comp) {
		if constexpr (!vtl::has_type<E, C>::value) { return false; }
		if (!contains(handle)) return false;
		VeHandle_t<E> h = std::get<VeHandle_t<E>>(handle);
		VeComponentVector<E>().update<C>(h.m_entity_index, std::forward<C>(comp));
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

	/**
	* \brief A specialized class to act as convenient access interface instead of the base class.
	*/

	template<>
	class VeEntityTable<void> : public VeEntityTableBaseClass {
	public:
		VeEntityTable(size_t r = 1 << 10) : VeEntityTableBaseClass(r) {};
	};


	//-------------------------------------------------------------------------
	//iterator

	template<typename... Cs>
	class VeComponentIteratorBase {
	protected:
		index_t m_current_index{ 0 };

	public:
		using value_type = std::tuple<Cs...>;

		VeComponentIteratorBase() {};
		virtual value_type operator*() = 0;
		virtual void operator++() = 0;
		virtual void operator++(int) = 0;
		auto operator<=>(const VeComponentIteratorBase<Cs...>& v) {
			return v.m_current_index <=> m_current_index;
		}
	};


	template<typename E, typename... Cs>
	class VeComponentIterator : public VeComponentIteratorBase<Cs...> {
	protected:

	public:
		VeComponentIterator(bool is_end = false) : VeComponentIteratorBase() {
			if (is_end) this->m_current_index.value = VeComponentVector<E>().size();
		};

		typename VeComponentIteratorBase<Cs...>::value_type operator*() {};
		void operator++() {};
		void operator++(int) {};
	};


	template<typename... Cs>
	class VeIterator {
	protected:
		using value_type = std::tuple<Cs...>;
		using entity_types = vtl::filter2< VeEntityTypeList, vtl::type_list<Cs...> >;

		std::array<std::unique_ptr<VeComponentIteratorBase<Cs...>>, vtl::size<entity_types>::value> m_dispatch;
		index_t m_current_iterator{0};
		bool m_is_end;

	public:
		VeIterator( bool end = true);
		value_type operator*() {};
		void operator++() {};
		void operator++(int) {};
		auto operator<=>(const VeIterator<Cs...>& v) const {
			if (v.m_current_iterator == m_current_iterator) {
				return v.m_dispatch[v.m_current_iterator.value] <=> m_dispatch[m_current_iterator.value];
			}
			return v.m_current_iterator <=> m_current_iterator;
		}
	};

	template<typename... Cs>
	VeIterator<Cs...>::VeIterator(bool is_end) : m_is_end{is_end} {
		vtl::static_for<size_t, 0, vtl::size<entity_types>::value >(
			[&](auto i) {
				using type = vtl::Nth_type<entity_types, i>;
				m_dispatch[i] = std::make_unique<VeComponentIterator<type, Cs...>>(is_end);
			}
		);
	};


	//-------------------------------------------------------------------------
	//entity table base class implementations needing the entity table derived classes

	inline VeEntityTableBaseClass::VeEntityTableBaseClass(size_t r) {
		if (!this->init()) return;
		m_entity_table.reserve(r);

		vtl::static_for<size_t, 0, vtl::size<VeEntityTypeList>::value >(
			[&](auto i) {
				using type = vtl::Nth_type<VeEntityTypeList, i>;
				m_dispatch[i] = std::make_unique<VeEntityTable<type>>();
			}
		);
	}

	template<typename E>
	inline std::optional<VeEntity_t<E>> VeEntityTableBaseClass::entity(const VeHandle& handle) {
		return VeEntityTable<E>().entity(handle);
	}

	template<typename E>
	requires vtl::has_type<VeEntityTypeList, E>::value
	bool VeEntityTableBaseClass::update(const VeHandle& handle, VeEntity_t<E>&& ent) {
		return VeEntityTable<E>().update({ handle }, std::forward<VeEntity_t<E>>(ent));
	}

	template<typename... Cs>
	VeIterator<Cs...> VeEntityTableBaseClass::begin() {
		return VeIterator<Cs...>();
	}

	template<typename... Cs>
	VeIterator<Cs...> VeEntityTableBaseClass::end() {
		return VeIterator<Cs...>(true);
	}

	//-------------------------------------------------------------------------
	//system

	/**
	* \brief Systems can access all components in sequence
	*/

	template<typename T, typename VeSystemComponentTypeList = vtl::type_list<>>
	class VeSystem : public VeMonostate<VeSystem<T>> {
	protected:
	public:
		VeSystem() = default;
	};




}

#endif
