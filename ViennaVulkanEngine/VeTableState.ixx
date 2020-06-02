export module VVE:VeTableState;

import std.core;
import std.memory;

import :VeTypes;
import :VeUtil;
import :VeMap;
import :VeMemory;
import :VeTableChunk;

export namespace vve {

	//----------------------------------------------------------------------------------
	// Delare VeTableState
	template< typename... Types> struct VeTableState;

	#define VeTableStateType VeTableState< Typelist < TypesOne... >, Typelist < TypesTwo... > >

	//----------------------------------------------------------------------------------
	// Specialization of VeTableState
	template< typename... TypesOne, typename... TypesTwo>
	class VeTableStateType {

		using tuple_type = std::tuple<TypesOne...>;
		using chunk_type = VeTableChunk<TypesOne...>;
		static_assert(sizeof(chunk_type) <= VE_TABLE_CHUNK_SIZE);
		using chunk_ptr  = std::unique_ptr<chunk_type>;
		using map_type = decltype(TupleOfLists<TypesTwo...>());
		using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

		//chunks
		std::pmr::vector<chunk_ptr>	d_chunks;	///pointers to table chunks

		//maps
		VeSlotMap									d_slot_map;
		std::array<VeHashMap,sizeof...(TypesTwo)>	d_maps;
		inline static map_type 						d_indices = TupleOfLists<TypesTwo...>();

	public:

		VeTableState(allocator_type alloc = {});
		~VeTableState() = default;

		//-------------------------------------------------------------------------------
		//read operations

		std::optional<tuple_type> at(VeHandle handle);

		//-------------------------------------------------------------------------------
		//write operations

		VeHandle	insert(VeGuid guid, tuple_type &entry);
		VeHandle	insert(VeGuid guid, tuple_type &entry, std::shared_ptr<VeHandle> handle);
		bool		update(VeHandle handle, tuple_type &entry);
		bool		erase(VeHandle handle);
		void		operator=(const VeTableStateType& rhs);
		void		clear();
	};

	//----------------------------------------------------------------------------------
	template<typename... TypesOne, typename... TypesTwo>
	VeTableStateType::VeTableState(allocator_type alloc) : d_chunks(alloc), d_maps() {
		d_chunks.emplace_back(std::make_unique<chunk_type>());	//create one chunk
	};


	//-------------------------------------------------------------------------------
	//read operations

	template< typename... TypesOne, typename... TypesTwo>
	std::optional<std::tuple<TypesOne...>> VeTableStateType::at(VeHandle handle) {
		VeTableIndex table_index = d_slot_map.at(handle);

		if(	table_index == VeTableIndex::NULL() || 
			!(table_index.d_chunk_index < d_chunks.size()) || 
			!d_chunks[table_index.d_chunk_index] )  { return std::nullopt; }

		return d_chunks[table_index.d_chunk_index]->at(table_index.d_in_chunk_index);
	}


	//-------------------------------------------------------------------------------
	//write operations

	template< typename... TypesOne, typename... TypesTwo>
	VeHandle VeTableStateType::insert(VeGuid guid, tuple_type &entry) {
		VeIndex last = d_chunks.size() - 1;		//index of last chunk
		if (d_chunks[last].full()) {			//if its full we need a new chunk
			d_chunks.emplace_back(std::make_unique<chunk_type>());	//create a new chunk
			last = d_chunks.size() - 1;
		}

		VeInChunkIndex in_chunk_index = d_chunks[last].insert(entry);	//insert data into the chunk
		return { .d_guid = guid, .d_table_index = { .d_chunk_index = last, .d_in_chunk_index = in_chunk_index} };
	}

	template< typename... TypesOne, typename... TypesTwo>
	VeHandle VeTableStateType::insert(VeGuid guid, tuple_type &entry, std::shared_ptr<VeHandle> handle) {
		*handle = insert(guid, entry);
		return *handle;
	}

	template< typename... TypesOne, typename... TypesTwo>
	bool VeTableStateType::update(VeHandle handle, tuple_type &entry) {
		VeTableIndex table_index = d_slot_map.at(handle);
		if(	table_index == VeTableIndex::NULL() ||
			!(table_index.d_chunk_index < d_chunks.size())) { return false; }

		return d_chunks[table_index.d_chunk_index].update( handle, entry);
	}

	template< typename... TypesOne, typename... TypesTwo>
	bool VeTableStateType::erase(VeHandle handle) {
		VeTableIndex table_index = d_slot_map.at(handle);
		if (table_index == VeTableIndex::NULL() ||
			!(table_index.d_chunk_index < d_chunks.size())) {return false;}

		return d_chunks[table_index.d_chunk_index].erase(handle);
	}

	template<typename... TypesOne, typename... TypesTwo>
	void VeTableStateType::operator=(const VeTableStateType& rhs) {

	}

	template<typename... TypesOne, typename... TypesTwo>
	void VeTableStateType::clear() {
		d_chunks.clear();
		d_slot_map.clear();
		for (auto& map : d_maps) { map.clear(); }
	}



};


