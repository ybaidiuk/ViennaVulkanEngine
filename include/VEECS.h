#ifndef VEENGINE_H
#define VEENGINE_H

#include <limits>
#include "glm.hpp"
#include "VGJS.h"
#include "VETable.h"

namespace vve {


	template<typename T>
	struct VeComponent {
	};

	struct VePosition : public VeComponent<VePosition>{
		glm::vec3 m_position;
	};

	struct VeOrientation : public VeComponent<VeOrientation> {

	};


	template<typename T, typename... Ts>
	class VeSystem {
	protected:

	public:

	};


	class VeEntityManager {
	public:
		struct VeEntityData;

		struct VeHandle {
			VeEntityData*	m_id = nullptr;
			uint32_t		m_generation = 0;
		};

		struct VeEntityData {
			uint32_t		m_generation = 0;
			VeEntityData*	m_next = nullptr;
		};

		VeEntityManager( size_t reserve) {
			if (m_init_counter > 0) return;
			auto cnt = m_init_counter.fetch_add(1);
			if (cnt > 0) return;
			m_data.reserve(1<<10);
		};
		VeHandle create();
		void erase(VeHandle& h);

	protected:
		static inline std::atomic<uint64_t> m_init_counter = 0;
		static inline VeTable<VeEntityData> m_data;
		static inline VeEntityData*			m_first_free = nullptr;
		static inline std::atomic<bool>		m_init = false;
	};

	using VeHandle = VeEntityManager::VeHandle;

}

#endif
