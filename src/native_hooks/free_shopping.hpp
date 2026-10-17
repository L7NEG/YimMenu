#pragma once
#include "invoker/invoker.hpp"
#include "native_hooks.hpp"
#include "pointers.hpp"
#include "types/netshop/netCatalog.hpp"

#include "memory/module.hpp"

#include <climits>
#include <cstring>

namespace big::free_shopping
{
	inline bool g_enabled = false;
	inline rage::netCatalog* g_net_catalog = nullptr;

	struct basket_item_data
	{
		alignas(8) Hash key;
		alignas(8) Hash item;
		alignas(8) int price;
		alignas(8) int stat_value;
	};
	static_assert(sizeof(basket_item_data) == 32);

	struct catalog_item_info
	{
		int stat_value         = -1;
		rage::joaat_t category = 0;
	};

	constexpr rage::joaat_t free_purchase_coupon        = "PO_COUPON_CAR_XMAS2017"_J;
	constexpr rage::joaat_t category_casino_chips       = "CATEGORY_CASINO_CHIPS"_J;
	constexpr rage::joaat_t category_casino_chip_reason = "CATEGORY_CASINO_CHIP_REASON"_J;
	constexpr rage::joaat_t casino_chips_item           = "CASINO_CHIPS_v0"_J;
	constexpr rage::joaat_t action_casino_chips_spend   = 0xFEAE09E5;
	constexpr rage::joaat_t category_property           = "CATEGORY_PROPERTIE"_J;
	constexpr rage::joaat_t category_property_interior  = "CATEGORY_PROPERTY_INTERIOR"_J;
	constexpr rage::joaat_t category_warehouse          = "CATEGORY_WAREHOUSE"_J;
	constexpr rage::joaat_t category_warehouse_interior = "CATEGORY_WAREHOUSE_INTERIOR"_J;

	inline bool is_property_action(rage::joaat_t action)
	{
		return action == "NET_SHOP_ACTION_BUY_PROPERTY"_J || action == "NET_SHOP_ACTION_BUY_WAREHOUSE"_J;
	}

	inline bool is_property_category(rage::joaat_t category)
	{
		return category == category_property
		    || category == category_property_interior
		    || category == category_warehouse
		    || category == category_warehouse_interior;
	}

	inline bool is_chip_category(rage::joaat_t category)
	{
		return category == category_casino_chips || category == category_casino_chip_reason;
	}

	inline bool catalog_contains(rage::netCatalog* catalog, rage::joaat_t hash)
	{
		if (!catalog)
			return false;

		__try
		{
			const auto count   = catalog->m_BucketCount;
			const auto buckets = catalog->m_BucketArray;
			if (!count || count > 32768 || !buckets)
				return false;

			std::size_t visited = 0;
			for (std::uint16_t i = 0; i < count; ++i)
			{
				for (auto node = buckets[i]; node; node = node->m_Next)
				{
					if (++visited > 150000)
						return false;
					if (node->Item()->m_Hash == hash)
						return true;
				}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
		return false;
	}

	inline memory::module& game_module()
	{
		static memory::module mod("GTA5.exe");
		return mod;
	}

	inline void* try_load_pointer(const void* address)
	{
		void* value = nullptr;
		__try
		{
			value = *reinterpret_cast<void* const*>(address);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			value = nullptr;
		}
		return value;
	}

	struct catalog_scan_stats
	{
		int handlers               = 0;
		int functions              = 0;
		int candidates             = 0;
		rage::netCatalog* best     = nullptr;
		std::uint16_t best_buckets = 0;
		std::uint32_t best_version = 0;
		std::uint32_t best_crc     = 0;
	};

	inline bool catalog_candidate_sane(rage::netCatalog* catalog, std::uint16_t& buckets, std::uint32_t& version, std::uint32_t& crc)
	{
		__try
		{
			buckets = catalog->m_BucketCount;
			version = catalog->m_CatalogVersion;
			crc     = catalog->m_CatalogCRC;
			return catalog->m_BucketArray != nullptr && buckets != 0 && buckets <= 32768;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
	}

	inline rage::netCatalog* scan_handler_for_catalog(rage::scrNativeHandler handler, rage::joaat_t want_hash, catalog_scan_stats& stats)
	{
		const auto& mod = game_module();

		struct code_ref
		{
			std::uintptr_t address;
			std::size_t size;
			int depth;
		};

		code_ref queue[64];
		int queue_size = 0;

		const auto push_range = [&](std::uintptr_t address, std::size_t size, int depth) {
			if (!address || queue_size >= 64 || !mod.contains(memory::handle(address)))
				return;
			for (int i = 0; i < queue_size; ++i)
			{
				if (queue[i].address == address)
					return;
			}
			queue[queue_size++] = {address, size, depth};
		};

		const auto consider_candidate = [&](std::uintptr_t target) -> rage::netCatalog* {
			rage::netCatalog* candidates[2] = {};
			int count                      = 0;

			if (mod.contains(memory::handle(target)))
				candidates[count++] = reinterpret_cast<rage::netCatalog*>(target);

			if (const auto loaded = try_load_pointer(reinterpret_cast<const void*>(target)))
			{
				if (loaded != reinterpret_cast<void*>(target))
					candidates[count++] = reinterpret_cast<rage::netCatalog*>(loaded);
			}

			for (int c = 0; c < count; ++c)
			{
				std::uint16_t buckets = 0;
				std::uint32_t version = 0;
				std::uint32_t crc     = 0;
				if (!catalog_candidate_sane(candidates[c], buckets, version, crc))
					continue;

				stats.candidates++;
				stats.best         = candidates[c];
				stats.best_buckets = buckets;
				stats.best_version = version;
				stats.best_crc     = crc;

				if (catalog_contains(candidates[c], want_hash))
					return candidates[c];
			}
			return nullptr;
		};

		push_range(reinterpret_cast<std::uintptr_t>(handler), 0x800, 0);

		for (int qi = 0; qi < queue_size; ++qi)
		{
			const auto address = queue[qi].address;
			const auto size    = queue[qi].size;
			const auto depth   = queue[qi].depth;
			stats.functions++;

			for (std::size_t i = 0; i + 7 < size; ++i)
			{
				const auto b0 = *reinterpret_cast<const std::uint8_t*>(address + i);
				const auto b1 = *reinterpret_cast<const std::uint8_t*>(address + i + 1);

				if (depth < 2 && (b0 == 0xE8 || b0 == 0xE9))
				{
					std::int32_t rel;
					std::memcpy(&rel, reinterpret_cast<const void*>(address + i + 1), sizeof(rel));
					push_range(address + i + 5 + rel, 0x400, depth + 1);
					i += 4;
					continue;
				}

				if (depth < 2 && b0 == 0xFF && b1 == 0x25)
				{
					std::int32_t rel;
					std::memcpy(&rel, reinterpret_cast<const void*>(address + i + 2), sizeof(rel));
					const auto slot = address + i + 6 + rel;
					if (mod.contains(memory::handle(slot)))
					{
						const auto target = reinterpret_cast<std::uintptr_t>(try_load_pointer(reinterpret_cast<const void*>(slot)));
						push_range(target, 0x400, depth + 1);
					}
					i += 5;
					continue;
				}

				if (b0 < 0x48 || b0 > 0x4F)
					continue;
				if (b1 != 0x8B && b1 != 0x8D && b1 != 0x89 && b1 != 0x39 && b1 != 0x3B)
					continue;
				if ((*reinterpret_cast<const std::uint8_t*>(address + i + 2) & 0xC7) != 0x05)
					continue;

				std::int32_t disp;
				std::memcpy(&disp, reinterpret_cast<const void*>(address + i + 3), sizeof(disp));
				if (auto found = consider_candidate(address + i + 7 + disp))
					return found;
			}
		}

		return nullptr;
	}

	inline void ensure_live_catalog(rage::joaat_t want_hash)
	{
		if (g_net_catalog)
			return;

		static bool failure_logged = false;

		if (!g_pointers->m_gta.m_get_native_handler)
		{
			if (!failure_logged)
			{
				failure_logged = true;
				LOG(WARNING) << "free_shopping: native handler lookup unavailable";
			}
			return;
		}

		constexpr NativeIndex probes[] = {
		    NativeIndex::NET_GAMESERVER_GET_PRICE,
		    NativeIndex::NET_GAMESERVER_CATALOG_ITEM_IS_VALID,
		    NativeIndex::NET_GAMESERVER_GET_CATALOG_CLOUD_CRC,
		    NativeIndex::NET_GAMESERVER_IS_CATALOG_CURRENT,
		    NativeIndex::NET_GAMESERVER_CATALOG_IS_VALID,
		};

		catalog_scan_stats stats;
		for (auto idx : probes)
		{
			const auto handler = g_pointers->m_gta.m_get_native_handler(g_pointers->m_gta.m_native_registration_table, g_crossmap[static_cast<std::size_t>(idx)]);
			if (!handler)
				continue;

			stats.handlers++;
			if (auto found = scan_handler_for_catalog(handler, want_hash, stats))
			{
				g_net_catalog = found;
				LOG(INFO) << "free_shopping: NetCatalog resolved via native handler " << static_cast<int>(idx);
				return;
			}
		}

		if (!failure_logged)
		{
			failure_logged = true;
			LOG(INFO) << "free_shopping: NetCatalog not resolved (handlers " << stats.handlers << " functions " << stats.functions << " candidates " << stats.candidates << " best " << static_cast<void*>(stats.best) << " buckets " << stats.best_buckets << " version " << stats.best_version << " crc " << stats.best_crc << ")";
		}
	}

	inline bool live_item_info_for_hash(rage::joaat_t item_hash, catalog_item_info& out)
	{
		if (!item_hash || !g_net_catalog)
			return false;

		__try
		{
			bool found = false;
			g_net_catalog->ForEachItem([&](const rage::netCatalogBaseItem& item) {
				if (found)
					return;
				if (item.m_Hash == item_hash)
				{
					out.stat_value = item.m_StatValue;
					out.category   = item.m_CategoryHash;
					found          = true;
				}
			});
			return found;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
	}

	inline bool live_cheapest_for_stat(int stat_value, rage::joaat_t category, rage::joaat_t& out_hash, int& out_price)
	{
		if (!g_net_catalog)
			return false;

		__try
		{
			rage::joaat_t stat_hash = 0;
			rage::joaat_t any_hash  = 0;
			int stat_price          = INT_MAX;
			int any_price           = INT_MAX;

			g_net_catalog->ForEachItem([&](const rage::netCatalogBaseItem& entry) {
				if (entry.m_CategoryHash != category)
					return;
				if (entry.m_Price < 0)
					return;
				if (entry.m_StatValue == stat_value && entry.m_Price < stat_price)
				{
					stat_hash  = entry.m_Hash;
					stat_price = entry.m_Price;
				}
				if (entry.m_Price < any_price)
				{
					any_hash  = entry.m_Hash;
					any_price = entry.m_Price;
				}
			});

			if (stat_hash && stat_price <= any_price)
			{
				out_hash  = stat_hash;
				out_price = stat_price;
				return true;
			}
			if (any_hash)
			{
				out_hash  = any_hash;
				out_price = any_price;
				return true;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
		return false;
	}

	inline bool item_info_for_hash(rage::joaat_t item_hash, catalog_item_info& out)
	{
		if (!item_hash)
			return false;

		ensure_live_catalog(item_hash);

		return live_item_info_for_hash(item_hash, out);
	}

	inline bool get_cheapest_for_stat(int stat_value, rage::joaat_t category, rage::joaat_t& out_hash, int& out_price)
	{
		if (!g_net_catalog)
			return false;

		return live_cheapest_for_stat(stat_value, category, out_hash, out_price);
	}

	constexpr rage::joaat_t discount_modifiers[] = {
	    "PM_CARMOD_BUYNOW"_J,
	    "PM_CARMOD_TUNER_OWNER_DISCOUNT"_J,
	    "PM_CARMOD_VINEWOOD_GARAGE_DISCOUNT"_J,
	    "PM_CLOTHING_BIN"_J,
	    "PM_CLOTHING_DESIGNER_FEE"_J,
	    "PM_COUPON_ADD_VEH_MOD_P"_J,
	    "PM_COUPON_CAR_MEET_VEH_P"_J,
	    "PM_COUPON_CAR_SITE"_J,
	    "PM_COUPON_CASINO_BIKE_SITE"_J,
	    "PM_COUPON_CASINO_BOAT_SITE"_J,
	    "PM_COUPON_CASINO_CAR_SITE"_J,
	    "PM_COUPON_CASINO_CAR_SITE2"_J,
	    "PM_COUPON_CASINO_MIL_SITE"_J,
	    "PM_COUPON_CASINO_PLANE_SITE"_J,
	    "PM_COUPON_MIL_SITE"_J,
	    "PM_COUPON_PLANE_SITE"_J,
	    "PM_TATTOO_DISCOUNT_MANSION"_J,
	    "PM_WEAPON_DISCOUNT_BRONZE_DRIVEBY"_J,
	    "PM_WEAPON_DISCOUNT_BRONZE_HEADSHOT"_J,
	    "PM_WEAPON_DISCOUNT_BRONZE_KILLS"_J,
	    "PM_WEAPON_DISCOUNT_BRONZE_MEDAL"_J,
	    "PM_WEAPON_DISCOUNT_FIXER_ARMORY"_J,
	    "PM_WEAPON_DISCOUNT_GOLD_DRIVEBY"_J,
	    "PM_WEAPON_DISCOUNT_GOLD_HEADSHOT"_J,
	    "PM_WEAPON_DISCOUNT_GOLD_KILLS"_J,
	    "PM_WEAPON_DISCOUNT_GOLD_MEDAL"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_0_0"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_0_1"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_0_2"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_0_3"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_0_4"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_0_5"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_0_6"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_0_7"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_0_8"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_0_9"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_1_0"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_1_1"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_1_2"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_1_3"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_1_4"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_2_0"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_2_1"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_2_2"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_2_3"_J,
	    "PM_WEAPON_DISCOUNT_GUN_VAN_2_4"_J,
	    "PM_WEAPON_DISCOUNT_MANSION_ARMORY"_J,
	    "PM_WEAPON_DISCOUNT_PLAT_DRIVEBY"_J,
	    "PM_WEAPON_DISCOUNT_PLAT_HEADSHOT"_J,
	    "PM_WEAPON_DISCOUNT_PLAT_KILLS"_J,
	    "PM_WEAPON_DISCOUNT_SILVER_DRIVEBY"_J,
	    "PM_WEAPON_DISCOUNT_SILVER_HEADSHOT"_J,
	    "PM_WEAPON_DISCOUNT_SILVER_KILLS"_J,
	    "PM_WEAPON_DISCOUNT_SILVER_MEDAL"_J,
	    "PM_WEAPON_PIM_AMMO_INCREASE"_J};

	inline bool is_discount_modifier(rage::joaat_t item_id)
	{
		return std::ranges::contains(discount_modifiers, item_id);
	}

	inline rage::joaat_t current_basket_action   = 0;
	inline rage::joaat_t current_basket_category = 0;

	inline void NET_GAMESERVER_BASKET_START(rage::scrNativeCallContext* src)
	{
		current_basket_category = src->get_arg<rage::joaat_t>(1);
		current_basket_action   = src->get_arg<rage::joaat_t>(2);

		if (g_enabled)
			LOG(INFO) << "free_shopping: basket category " << HEX_TO_UPPER(static_cast<std::uint32_t>(current_basket_category)) << " action " << HEX_TO_UPPER(static_cast<std::uint32_t>(current_basket_action));

		src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BASKET_START(src->get_arg<int*>(0), current_basket_category, current_basket_action, src->get_arg<int>(3)));
	}

	inline void NET_GAMESERVER_BASKET_ADD_ITEM(rage::scrNativeCallContext* src)
	{
		auto item_data     = src->get_arg<basket_item_data*>(0);
		const int quantity = src->get_arg<int>(1);

		if (!g_enabled)
		{
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BASKET_ADD_ITEM(reinterpret_cast<Any*>(item_data), quantity));
			return;
		}

		const auto key_hash  = static_cast<rage::joaat_t>(item_data->key);
		const auto item_hash = static_cast<rage::joaat_t>(item_data->item);
		const int price      = item_data->price;

		if (is_discount_modifier(key_hash))
		{
			src->set_return_value<BOOL>(TRUE);
			return;
		}

		catalog_item_info info;
		const bool have_info = item_info_for_hash(item_hash, info) || item_info_for_hash(key_hash, info);

		if (current_basket_category == category_casino_chips || (have_info && is_chip_category(info.category)))
		{
			if (g_enabled)
				LOG(INFO) << "free_shopping: chip basket line " << HEX_TO_UPPER(static_cast<std::uint32_t>(key_hash)) << '/' << HEX_TO_UPPER(static_cast<std::uint32_t>(item_hash)) << " price " << price << " quantity " << quantity;

			if (current_basket_action == action_casino_chips_spend && price == 1 && quantity > 1 && (key_hash == casino_chips_item || item_hash == casino_chips_item))
			{
				LOG(INFO) << "free_shopping: casino chip spend " << quantity << " -> 1";
				src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BASKET_ADD_ITEM(reinterpret_cast<Any*>(item_data), 1));
				return;
			}

			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BASKET_ADD_ITEM(reinterpret_cast<Any*>(item_data), quantity));
			return;
		}

		if (is_property_action(current_basket_action) || (have_info && is_property_category(info.category)))
		{
			if (price > 0)
			{
				if (have_info)
				{
					rage::joaat_t best_hash = 0;
					int best_price          = 0;
					if (get_cheapest_for_stat(info.stat_value, info.category, best_hash, best_price) && best_hash && best_price < price)
					{
						LOG(INFO) << "free_shopping: property item " << HEX_TO_UPPER(static_cast<std::uint32_t>(key_hash)) << '/' << HEX_TO_UPPER(static_cast<std::uint32_t>(item_hash)) << " category " << HEX_TO_UPPER(static_cast<std::uint32_t>(info.category)) << " stat " << info.stat_value << " price " << price << " -> " << HEX_TO_UPPER(static_cast<std::uint32_t>(best_hash)) << " price " << best_price;
						item_data->item  = best_hash;
						item_data->price = best_price;
					}
					else
					{
						LOG(INFO) << "free_shopping: property item " << HEX_TO_UPPER(static_cast<std::uint32_t>(key_hash)) << '/' << HEX_TO_UPPER(static_cast<std::uint32_t>(item_hash)) << " category " << HEX_TO_UPPER(static_cast<std::uint32_t>(info.category)) << " stat " << info.stat_value << " price " << price << " (no cheaper candidate)";
					}
				}
				else
				{
					LOG(INFO) << "free_shopping: property item " << HEX_TO_UPPER(static_cast<std::uint32_t>(key_hash)) << '/' << HEX_TO_UPPER(static_cast<std::uint32_t>(item_hash)) << " not found in catalog";
				}
			}

			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BASKET_ADD_ITEM(reinterpret_cast<Any*>(item_data), quantity));
			return;
		}

		const bool apply_coupon = price > 0;

		if (apply_coupon)
			item_data->price = 0;

		BOOL item_added = NETSHOPPING::NET_GAMESERVER_BASKET_ADD_ITEM(reinterpret_cast<Any*>(item_data), quantity);

		if (apply_coupon && item_added)
		{
			basket_item_data coupon_data{};
			coupon_data.key        = free_purchase_coupon;
			coupon_data.item       = key_hash ? key_hash : item_hash;
			coupon_data.price      = 0;
			coupon_data.stat_value = item_data->stat_value;

			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BASKET_ADD_ITEM(reinterpret_cast<Any*>(&coupon_data), quantity));
			return;
		}

		src->set_return_value<BOOL>(std::move(item_added));
	}

	inline void USE_FAKE_MP_CASH(rage::scrNativeCallContext* src)
	{
		if (g_enabled)
			return;

		HUD::USE_FAKE_MP_CASH(src->get_arg<BOOL>(0));
	}

	inline void CHANGE_FAKE_MP_CASH(rage::scrNativeCallContext* src)
	{
		if (g_enabled)
			return;

		HUD::CHANGE_FAKE_MP_CASH(src->get_arg<int>(0), src->get_arg<int>(1));
	}
}
