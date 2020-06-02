export module VVE:VeTableChunk;

import std.core;
import std.memory;

import :VeUtil;
import :VeTypes;
import :VeMap;
import :VeMemory;

export namespace vve {

	const uint32_t VE_TABLE_CHUNK_SIZE = 1 << 14;

	///----------------------------------------------------------------------------------
	/// \brief table states are cut into chunks of equal size
	///----------------------------------------------------------------------------------

	template<typename... Args>
	class VeTableChunk {
		static const uint32_t data_size = (sizeof(Args) + ...) + sizeof(VeIndex); //d_data + slot_map_array
		static const uint32_t manage_size = sizeof(VeGuid) + sizeof(uint32_t) + sizeof(VeChunkIndex);
		static const uint32_t c_max_size = (VE_TABLE_CHUNK_SIZE - manage_size) / data_size;

		using data_arrays = std::tuple<std::array<Args, c_max_size>...>;	//tuple of arrays
		using tuple_type = std::tuple<Args...>;								//basic tuple type
		using slot_map_array = std::array<VeIndex, c_max_size>;				//index of slot map

		VeGuid			d_guid;				//current GUID reflecting the state of the chunk
		VeChunkIndex	d_chunk_index;		///index of this chunk in the chunks vector of the table state
		uint32_t		d_size;				///number of array slots used currently
		data_arrays		d_data;				///arrays of data items
		slot_map_array	d_slot_map_index;	///index of the slot map entry that points to this slot

	public:
		VeTableChunk();
		VeTableChunk( VeChunkIndex chunk_index );
		VeTableChunk( const VeTableChunk &) = delete;
		~VeTableChunk() = default;

		VeInChunkIndex	insert(VeIndex slot_map_index, tuple_type entry);
		bool			update(VeInChunkIndex in_chunk_index, tuple_type entry);
		bool			update(VeIndex slot_map_index, VeInChunkIndex in_chunk_index, tuple_type entry);
		tuple_type		at(VeInChunkIndex in_chunk_index, VeIndex &slot_map_index);
		void			pop_back();
		auto			data();
		bool			full();
		VeGuid			guid();
		std::size_t		size();
		VeIndex			slotMapIndex(VeInChunkIndex);
	};

	///----------------------------------------------------------------------------------
	/// \brief Default Constructor
	///----------------------------------------------------------------------------------
	template<typename... Args>
	VeTableChunk<Args...>::VeTableChunk() : d_size(0), d_chunk_index(VeChunkIndex::NULL()) {}

	///----------------------------------------------------------------------------------
	// \brief Constructor with chunk index
	///----------------------------------------------------------------------------------
	template<typename... Args>
	VeTableChunk<Args...>::VeTableChunk(VeChunkIndex chunk_index) : d_size(0), d_chunk_index(chunk_index) {}
	
	///----------------------------------------------------------------------------------
	/// \brief Insert a new entry into the chunk
	/// \param[in] slot_map_index The index of the slot map that points to this entry
	/// \param[in] entry The tuple that is to be added to this chunk
	/// \returns the in chunk index of the new entry in this chunk 
	///----------------------------------------------------------------------------------
	template<typename... Args>
	VeInChunkIndex VeTableChunk<Args...>::insert(VeIndex slot_map_index, tuple_type entry) {
		static_for<std::size_t, 0, std::tuple_size_v<tuple_type>>( [&,this](auto i) { //copy tuple into the chunk
			std::get<i>(d_data)[d_size] = std::get<i>(entry); 
		});
		d_slot_map_index[d_size] = slot_map_index;		//index of the slot map that points to this entry
		return VeInChunkIndex(d_size++);				//increase size and return in chunk index
	}

	///----------------------------------------------------------------------------------
	/// \brief Update an existing entry of this chunk
	/// \param[in] in_chunk_index Index of the entry in the data arrays
	/// \param[in] entry The tuple that is to be updated in this chunk
	/// \returns whether the operation was successful
	///----------------------------------------------------------------------------------
	template<typename... Args>
	bool VeTableChunk<Args...>::update(VeInChunkIndex in_chunk_index, tuple_type entry) {
		return update( VeIndex::NULL(), in_chunk_index, entry );
	}

	///----------------------------------------------------------------------------------
	/// \brief Update an existing entry of this chunk
	/// \param[in] slot_map_index New index from the slot map that points to this entry
	/// \param[in] in_chunk_index Index of the entry in the data arrays
	/// \param[in] entry The tuple that is to be updated in this chunk
	/// \returns whether the operation was successful
	///----------------------------------------------------------------------------------
	template<typename... Args>
	bool VeTableChunk<Args...>::update(VeIndex slot_map_index, VeInChunkIndex in_chunk_index, tuple_type entry) {
		if (!(in_chunk_index.value < d_size)) return false;

		static_for<std::size_t, 0, std::tuple_size_v<tuple_type>>([&, this](auto i) { //copy tuple over old chunk entry
			std::get<i>(d_data)[in_chunk_index] = std::get<i>(entry);
			});

		if (slot_map_index != VeIndex::NULL()) { d_slot_map_index[in_chunk_index] = slot_map_index; }
		return true;
	}


	///----------------------------------------------------------------------------------
	/// \brief Retrieve the data at a certain index from this chunk
	/// \param[in] in_chunk_index Index of the entry in the data arrays to be retrieved
	/// \param[out] slot_map_index The slot map index or NULL if the item was not found
	/// \returns the data as tuple or an empty tuple
	///----------------------------------------------------------------------------------
	template<typename... Args>
	std::tuple<Args...> VeTableChunk<Args...>::at(VeInChunkIndex in_chunk_index, VeIndex& slot_map_index) {
		tuple_type tuple;
		slot_map_index = VeIndex::NULL();

		if ( in_chunk_index < d_size) {
			static_for<std::size_t, 0, std::tuple_size_v<tuple_type>>([&, this](auto i) { //copy tuple from arrays
				std::get<i>(tuple) = std::get<i>(d_data)[in_chunk_index];
			});
			slot_map_index = d_slot_map_index[in_chunk_index];
		}
		return tuple;
	}

	///----------------------------------------------------------------------------------
	/// \brief Erase an entry at a certain index from this chunk
	/// \param[in] in_chunk_index Index of the entry in the data arrays to be erased
	/// \returns the slot map index of the entry thas was moved to the empty entry to fill the gap
	///----------------------------------------------------------------------------------
	template<typename... Args>
	void VeTableChunk<Args...>::pop_back() {
		if (d_size == 0) return;
		--d_size;									//reduce size by 1
	}

	///----------------------------------------------------------------------------------
	/// \brief Returns whether the chunk is full or whether it can hold more items
	/// \returns true if chunk is full, else false
	///----------------------------------------------------------------------------------
	template<typename... Args>
	bool VeTableChunk<Args...>::full() {
		return d_size >= c_max_size;
	}

	///----------------------------------------------------------------------------------
	/// \brief Get a pointer to the raw chunk data
	/// \returns a pointer to the tuple containing the arrays of data items of this chunk
	///----------------------------------------------------------------------------------
	template<typename... Args>
	auto VeTableChunk<Args...>::data() {
		return &d_data;
	}

	///----------------------------------------------------------------------------------
	/// \brief Get the current GUID of the chunk
	/// \returns the current GUID reflecting the chunk state
	///----------------------------------------------------------------------------------
	template<typename... Args>
	VeGuid VeTableChunk<Args...>::guid() {
		return d_guid;
	}

	///----------------------------------------------------------------------------------
	/// \brief Get the number of entries in the chunk
	/// \returns the number of entries in this chunk
	///----------------------------------------------------------------------------------
	template<typename... Args>
	std::size_t VeTableChunk<Args...>::size() {
		return d_size;
	}

	///----------------------------------------------------------------------------------
	/// \brief Get the slot map index of a given entry
	/// \returns the slot map index of an entry
	///----------------------------------------------------------------------------------
	template<typename... Args>
	VeIndex	VeTableChunk<Args...>::slotMapIndex(VeInChunkIndex in_chunk_index) {
		if (!(in_chunk_index < d_slot_map_index.size())) { return VeIndex::NULL(); }
		return d_slot_map_index[in_chunk_index];
	}

};

