export module VVE:VeTable;

import std.core;
import :VeUtil;
import :VeTypes;
import :VeMap;
import :VeMemory;
import :VeTableChunk;
import :VeTableState;


namespace vve {



};


export namespace vve {

	//----------------------------------------------------------------------------------
	class VeTableBase {
	public:
		VeTableBase() = default;
		~VeTableBase() = default;
	};


	//-------------------------------------------------------------------------------
	//main table class

	template< typename... Types> struct VeTable;

	#define VeTableStateType VeTableState< Typelist < TypesOne... >, Typelist < TypesTwo... > >
	#define VeTableType VeTable< Typelist < TypesOne... >, Typelist < TypesTwo... > >

	template< typename... TypesOne, typename... TypesTwo>
	class VeTableType : VeTableBase {

		using tuple_type = std::tuple<TypesOne...>;
		using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

		enum class VeNumStates { ONE, TWO };
		enum class VeOnSwapDo { CLEAR, COPY };

		VeIndex								d_thread_idx;
		std::unique_ptr<VeTableStateType>	d_current_state;
		std::unique_ptr<VeTableStateType>	d_next_state;
		VeOnSwapDo							d_on_swap_do;

	public:
		VeTable(allocator_type alloc = {});
		VeTable(VeNumStates num_states, allocator_type alloc = {});
		VeTable(VeNumStates num_states, VeOnSwapDo on_swap, allocator_type alloc = {});
		~VeTable() = default;

		//-------------------------------------------------------------------------------
		//read operations

		VeIndex getThreadIdx();
		auto getCurrentState();
		auto getNextState();
		auto find( VeHandle handle );

		//-------------------------------------------------------------------------------
		//write operations

		void setThreadIdx(VeIndex idx);
		void clear();
		auto insert(VeGuid guid, tuple_type entry);
		auto insert(VeGuid guid, tuple_type entry, std::shared_ptr<VeHandle> handle);
		void erase( VeHandle handle );
	};


	template<typename... TypesOne, typename... TypesTwo>
	VeTableType::VeTable(allocator_type alloc) : VeTable(VeNumStates::ONE, VeOnSwapDo::COPY, alloc) {}

	template<typename... TypesOne, typename... TypesTwo>
	VeTableType::VeTable(VeNumStates num_states, allocator_type alloc) : VeTable( num_states, VeOnSwapDo::COPY, alloc) {}

	template<typename... TypesOne, typename... TypesTwo>
	VeTableType::VeTable(VeNumStates num_states, VeOnSwapDo on_swap, allocator_type alloc ) :  d_on_swap_do(on_swap) {
		d_current_state = std::unique_ptr<VeTableStateType>(new VeTableStateType(alloc));
		if (num_states == VeNumStates::ONE) return;
		d_next_state = std::unique_ptr<VeTableStateType>(new VeTableStateType(alloc));
	}

	//-------------------------------------------------------------------------------
	//read operations

	template<typename... TypesOne, typename... TypesTwo>
	VeIndex VeTableType::getThreadIdx() {
		return d_thread_idx;
	}

	template<typename... TypesOne, typename... TypesTwo>
	auto VeTableType::getCurrentState() {
		return d_current_state;
	}

	template<typename... TypesOne, typename... TypesTwo>
	auto VeTableType::getNextState() {
		if( d_next_state ) return d_next_state;
		return d_current_state;
	}


	//-------------------------------------------------------------------------------
	//write operations

	template<typename... TypesOne, typename... TypesTwo>
	void VeTableType::setThreadIdx(VeIndex idx) {
		d_thread_idx = idx;
	}

	template<typename... TypesOne, typename... TypesTwo>
	void VeTableType::clear() {

	}



};
