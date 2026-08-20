#include "backend/looped_command.hpp"
#include "backend/looped/self/freeshopping_data.hpp"
#include "backend/looped/self/freeshopping_unlocks_data.hpp"
#include "script_global.hpp"
#include "services/tunables/tunables_service.hpp"

#include <unordered_map>

namespace big
{
	class freeshopping : looped_command
	{
		using looped_command::looped_command;

		std::unordered_map<int, int> m_original_values;
		std::unordered_map<int, int> m_original_unlocks;

		bool category_enabled(freeshopping_category cat) const
		{
			switch (cat)
			{
			case freeshopping_category::WEAPONS: return g.freeshopping.weapons;
			case freeshopping_category::WEAPON_MODS: return g.freeshopping.weapon_mods;
			case freeshopping_category::CLOTHING: return g.freeshopping.clothing;
			case freeshopping_category::VEHICLE: return g.freeshopping.vehicle;
			case freeshopping_category::SERVICES: return g.freeshopping.services;
			case freeshopping_category::PROPERTY: return g.freeshopping.properties;
			case freeshopping_category::MISC: return g.freeshopping.misc;
			}

			return false;
		}

		int* get_tunable(int offset)
		{
			return script_global(TUNABLE_BASE_ADDRESS).at(offset).as<int*>();
		}

		virtual void on_tick() override
		{
			for (int i = 0; i < g_freeshopping_item_count; i++)
			{
				const auto& item = g_freeshopping_items[i];
				const bool should_zero = category_enabled(item.category);
				const auto it         = m_original_values.find(item.offset);
				const bool is_active  = it != m_original_values.end();

				if (auto* tunable = get_tunable(item.offset))
				{
					if (should_zero)
					{
						if (!is_active)
						{
							m_original_values.emplace(item.offset, *tunable);
						}

						*tunable = 0;
					}
					else if (is_active)
					{
						*tunable = it->second;
						m_original_values.erase(it);
					}
				}
			}

			if (g.freeshopping.unlock_items)
			{
				for (int i = 0; i < g_freeshopping_unlock_count; i++)
				{
					const auto& unlock = g_freeshopping_unlocks[i];
					const auto it      = m_original_unlocks.find(unlock.offset);
					const bool active  = it != m_original_unlocks.end();

					if (auto* tunable = get_tunable(unlock.offset))
					{
						if (!active)
						{
							m_original_unlocks.emplace(unlock.offset, *tunable);
						}

						*tunable = TRUE;
					}
				}
			}
			else if (!m_original_unlocks.empty())
			{
				for (auto& [offset, value] : m_original_unlocks)
				{
					if (auto* tunable = get_tunable(offset))
					{
						*tunable = value;
					}
				}

				m_original_unlocks.clear();
			}
		}

		virtual void on_disable() override
		{
			for (auto& [offset, value] : m_original_values)
			{
				if (auto* tunable = get_tunable(offset))
				{
					*tunable = value;
				}
			}

			m_original_values.clear();

			for (auto& [offset, value] : m_original_unlocks)
			{
				if (auto* tunable = get_tunable(offset))
				{
					*tunable = value;
				}
			}

			m_original_unlocks.clear();
		}
	};

	freeshopping g_freeshopping("freeshopping", "FREE_SHOPPING", "FREE_SHOPPING_DESC", g.freeshopping.enabled);
}