#include "backend/looped_command.hpp"
#include "core/data/free_shopping.hpp"
#include "services/tunables/tunables_service.hpp"

namespace big
{
	class free_shopping_cmd : looped_command
	{
		using looped_command::looped_command;

		std::vector<std::pair<int32_t*, int32_t>> m_int_restore;
		std::vector<std::pair<float*, float>> m_float_restore;
		bool m_backed_up = false;

		virtual void on_tick() override
		{
			if (!g_tunables_service->initialized()) [[unlikely]]
				return;

			if (!m_backed_up) [[unlikely]]
			{
				m_int_restore.reserve(::big::free_shopping::DISCOUNT_INT_HASH_COUNT);
				m_float_restore.reserve(::big::free_shopping::DISCOUNT_FLOAT_HASH_COUNT);

				for (const auto hash : ::big::free_shopping::DISCOUNT_INT_HASHES)
				{
					if (auto tunable = g_tunables_service->get_tunable<int32_t*>(hash))
						m_int_restore.emplace_back(tunable, *tunable);
				}
				for (const auto hash : ::big::free_shopping::DISCOUNT_FLOAT_HASHES)
				{
					if (auto tunable = g_tunables_service->get_tunable<float*>(hash))
						m_float_restore.emplace_back(tunable, *tunable);
				}
				// if nothing was found, the service isn't ready yet, retry next tick
				m_backed_up = !m_int_restore.empty() || !m_float_restore.empty();
			}
			else
			{
				for (auto& [tunable, _] : m_int_restore)
					*tunable = 0;
				for (auto& [tunable, _] : m_float_restore)
					*tunable = 0.0f;
			}
		}

		virtual void on_disable() override
		{
			if (!m_backed_up)
				return;

			for (auto& [tunable, restore] : m_int_restore)
				*tunable = restore;
			for (auto& [tunable, restore] : m_float_restore)
				*tunable = restore;

			m_backed_up = false;
			m_int_restore.clear();
			m_float_restore.clear();
		}
	};

	free_shopping_cmd g_free_shopping("freeshopping", "FREE_SHOPPING", "FREE_SHOPPING_DESC", g.self.free_shopping);
}