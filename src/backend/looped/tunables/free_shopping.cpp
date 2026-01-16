#include "backend/looped_command.hpp"
#include "core/scr_globals.hpp"
#include "natives.hpp"
#include "services/tunables/tunables_service.hpp"

namespace big
{
	class free_shopping : looped_command
	{
		using looped_command::looped_command;

		const std::vector<Hash> m_tunable_hashes = {
			"MULTIPLIER_SHOP_MOD_PRICES"_J,
			"MP_CHAR_ARMOUR_1_PRICE"_J,
			"MP_CHAR_ARMOUR_2_PRICE"_J,
			"MP_CHAR_ARMOUR_3_PRICE"_J,
			"MP_CHAR_ARMOUR_4_PRICE"_J,
			"MP_CHAR_ARMOUR_5_PRICE"_J,
			"MP_CHAR_CLOTHING_PRICES"_J,
			"MP_CHAR_TATTOO_PRICES"_J,
			"MP_CHAR_HAIRSTYLE_PRICES"_J,
			"MP_CHAR_WEAPON_PRICES"_J,
			"MP_CHAR_WEAPON_MOD_PRICES"_J,
			"MP_CHAR_VEHICLE_PRICES"_J,
			"MP_CHAR_VEHICLE_MOD_PRICES"_J,
			"MP_CHAR_PROPERTY_PRICES"_J,
			"MP_CHAR_INTERIOR_PRICES"_J
		};

		virtual void on_tick() override
		{
			// Bypass transaction limits
			if (scr_globals::transaction_overlimit.is_valid())
			{
				*scr_globals::transaction_overlimit.as<PBOOL>() = FALSE;
			}

			// Set prices to 0
			for (const auto& hash : m_tunable_hashes)
			{
				if (auto tunable = g_tunables_service->get_tunable<int*>(hash))
				{
					*tunable = 0;
				}
			}
		}

		virtual void on_disable() override
		{
			// Prices will be restored by g_tunables_service cache if needed, 
			// but usually tunables are re-downloaded or reset by scripts.
			// YimMenu's tunables service handles restoration if it was backed up.
		}
	};

	free_shopping g_free_shopping("freeshopping", "Free Shopping", "Makes all items in shops and internet free to purchase and bypasses transaction limits.", g.tunables.free_shopping);
}
