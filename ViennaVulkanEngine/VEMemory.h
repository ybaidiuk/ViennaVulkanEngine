#pragma once



namespace mem {

	const uint32_t VE_MEM_IDX_NULL = std::numeric_limits<uint32_t>::max();

	struct TableFindIndex {
		uint32_t									m_key_offset;	///offset in the entry struct that contains the 32 bit key
		std::unordered_multimap<uint32_t, uint32_t> m_index;		///key value pairs - value is the index of the entry in the table

		TableFindIndex(uint32_t key_offset) {
			m_key_offset = key_offset;
		}
	};

	struct TableSortIndex {
		uint32_t							m_key_offset;	///offset in the entry struct that contains the 32 bit key
		std::multimap<uint32_t, uint32_t>	m_index;		///key value pairs, key is sorted, value is the entry idx in the table

		TableSortIndex( uint32_t key_offset ) {
			m_key_offset = key_offset;
		}
	};

	template <typename T>
	struct FixedSizeTable {
		uint32_t					m_thread_id;			///id of thread that accesses to this table are scheduled to
		std::vector<TableFindIndex>	m_find_index;			///vector of hashed indices for quickly finding entries in O(1)
		std::vector<TableSortIndex>	m_sort_index;			///vector of sorted keys for sorted iterating through entries
		std::vector<T>				m_data;					///growable data table

		FixedSizeTable(std::vector<uint32_t>& find_key_offsets, std::vector<uint32_t>& sort_key_offsets) {
			for each (uint32_t offset in find_key_offsets) {
				TableFindIndex tfi(offset);
				m_find_index.emplace_back(tfi);
			};
			for each (uint32_t offset in sort_key_offsets) {
				TableSortIndex tsi(offset);
				m_sort_index.emplace_back(tsi);
			};
		};

		uint32_t getEntryKey(T& entry, uint32_t offset) {
			void* p = &entry + offset;
			return *((uint32_t*)p);
			return 0;
		};

		void addFindIndex(T& te, uint32_t entry_idx) {
			for each (TableFindIndex & tfi in m_find_index) {
				uint32_t key = getEntryKey(te, tfi.m_key_offset);
				tfi.m_index.emplace(key, entry_idx);
			}
		};

		void removeFindIndex( uint32_t key ) {
			for each (TableFindIndex & tfi in m_find_index) {
				tfi.m_index.erase(key);
			}
		}

		void addSortIndex(T& te, uint32_t entry_idx) {
			for each (TableSortIndex& tsi in m_sort_index) {
				uint32_t key = getEntryKey(te, tsi.m_key_offset);
				tsi.m_index.emplace(key, entry_idx);
			}
		};

		void removeSortedIndex(uint32_t key) {
			for each (TableSortIndex & tsi in m_sort_index) {
				tsi.m_index.erase(key);
			}
		}

		void addEntry(T& te) {
			uint32_t idx = m_data.size();
			m_data.emplace_back(te);
			addFindIndex(te, idx);
			addSortIndex(te, idx);
		};

		void removeEntries( uint32_t key, uint32_t index_nr ) {

		}

		auto& getUnorderedIndexIterator(uint32_t key, uint32_t index_nr) {
			auto& map = m_find_index[index_nr].m_index;
			return map.equal_range(key);
		}

		uint32_t findEntry(uint32_t key, uint32_t index_nr, T& entry ) {
			uint32_t result = findEntryIndex(key, index_nr);
			if (result != VE_MEM_IDX_NULL) entry = m_data[result];
			return result;
		}

		uint32_t findEntryIndex(uint32_t key, uint32_t index_nr) {
			auto& map = m_find_index[index_nr].m_index;
			auto result = map.find(key);
			if (result == map.end()) return VE_MEM_IDX_NULL;
			return result->second();
		}

		uint32_t findEntryIndices(uint32_t key, uint32_t index_nr, std::vector<uint32_t>& result) {
			auto ret = m_find_index[index_nr].m_index.equal_range(key);
			uint32_t num_result = 0;
			for ( auto it = ret.first;	it != ret.second; ++it, ++num_result) {
				result.emplace_back(it->second());
			}
			return num_result;
		};

		auto& getIndexForSortedEntryIndices( uint32_t key, uint32_t index_nr ) {
			return m_sort_index[index_nr].m_index;
		}

		uint32_t getSortedEntryIndices(uint32_t key, uint32_t index_nr, std::vector<uint32_t>& result ) {
			auto map = getIndexForSortedEntryIndices(key, index_nr);
			uint32_t num_indices = 0;
			for (auto iter = map.begin(); iter != map.end(); ++iter, ++num_indices) result.emplace_back(iter->second);
			return num_indices;
		}
	};



	///-------------------------------------------------------------------------------
	struct VariableSizeTable {
		TableSortIndex			m_indices;
		std::vector<uint8_t>	m_data;
	};


}


