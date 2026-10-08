#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace big
{
	struct ShoppingCatalogEntry
	{
		std::uint32_t m_hash{};
		std::int32_t m_stat_value{};
		std::int32_t m_price{};
		std::uint32_t m_category{};
	};

	// Compact property-related catalog derived from Catalog_v94.json (v94, crc 1367690766).
	// Used for the property swap (BUY_PROPERTY / BUY_WAREHOUSE): variation hash -> statValue -> cheapest entry.
	inline const ShoppingCatalogEntry g_shopping_catalog[] = {
#include "shopping_catalog.inc"
	};

	inline constexpr std::size_t g_shopping_catalog_size = sizeof(g_shopping_catalog) / sizeof(g_shopping_catalog[0]);

	inline std::optional<std::int32_t> shopping_lookup_stat_value(std::uint32_t variation_hash)
	{
		for (std::size_t i = 0; i < g_shopping_catalog_size; ++i)
		{
			if (g_shopping_catalog[i].m_hash == variation_hash)
				return g_shopping_catalog[i].m_stat_value;
		}
		return std::nullopt;
	}

	inline const ShoppingCatalogEntry* shopping_find_cheapest(std::int32_t stat_value, std::uint32_t prefer_category = 0)
	{
		const ShoppingCatalogEntry* best_overall      = nullptr;
		const ShoppingCatalogEntry* best_in_category  = nullptr;
		for (std::size_t i = 0; i < g_shopping_catalog_size; ++i)
		{
			const auto& e = g_shopping_catalog[i];
			if (e.m_stat_value != stat_value)
				continue;
			if (!best_overall || e.m_price < best_overall->m_price)
				best_overall = &e;
			if (prefer_category != 0 && e.m_category == prefer_category)
			{
				if (!best_in_category || e.m_price < best_in_category->m_price)
					best_in_category = &e;
			}
		}
		// Prefer same-category cheapest when available, otherwise global cheapest for this statValue.
		return best_in_category ? best_in_category : best_overall;
	}
}
