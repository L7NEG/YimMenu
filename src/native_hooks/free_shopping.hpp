#pragma once
#include "native_hooks.hpp"
#include "types/netshop/netCatalog.hpp"

#include "memory/module.hpp"
#include "memory/pattern.hpp"

#include <climits>

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

	constexpr rage::joaat_t free_purchase_coupon = "PO_COUPON_CAR_XMAS2017"_J;

	inline bool is_property_action(rage::joaat_t action)
	{
		return action == "NET_SHOP_ACTION_BUY_PROPERTY"_J || action == "NET_SHOP_ACTION_BUY_WAREHOUSE"_J;
	}

	inline void ensure_catalog()
	{
		if (g_net_catalog)
			return;

		static bool tried = false;
		if (tried)
			return;
		tried = true;

		const auto mod = memory::module("GTA5.exe");
		if (auto res = mod.scan(memory::pattern("0F 82 55 FF FF FF 44 89 7C 24 30")))
			g_net_catalog = res->add(0xB).add(3).rip().as<rage::netCatalog*>();

		if (!g_net_catalog)
			LOG(WARNING) << "free_shopping: NetCatalog not found, property buys will use coupon fallback";
	}

	inline int get_stat_value_from_hash(rage::joaat_t item_hash)
	{
		ensure_catalog();

		auto catalog = g_net_catalog;
		if (!catalog)
			return -1;

		int result = -1;
		catalog->ForEachItem([&](const rage::netCatalogBaseItem& item) {
			if (result != -1)
				return;
			if (item.m_Hash == item_hash)
				result = item.m_StatValue;
		});

		return result;
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

		const auto item_id = static_cast<rage::joaat_t>(item_data->key);
		const int price    = item_data->price;

		if (is_property_action(current_basket_action) && price > 0)
		{
			const int stat_value = get_stat_value_from_hash(static_cast<rage::joaat_t>(item_data->item));

			if (stat_value >= 0)
			{
				auto catalog            = g_net_catalog;
				rage::joaat_t best_hash = 0;
				int best_price          = INT_MAX;

				if (catalog)
				{
					catalog->ForEachItem([&](const rage::netCatalogBaseItem& entry) {
						if (entry.m_StatValue != stat_value)
							return;
						if (entry.m_Price < 0)
							return;
						if (entry.m_Price >= best_price)
							return;
						best_hash  = entry.m_Hash;
						best_price = entry.m_Price;
					});
				}

				if (best_hash)
				{
					item_data->item  = best_hash;
					item_data->price = best_price;
				}

				src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BASKET_ADD_ITEM(reinterpret_cast<Any*>(item_data), quantity));
				return;
			}
		}

		if (is_discount_modifier(item_id))
		{
			src->set_return_value<BOOL>(TRUE);
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
			coupon_data.item       = item_id;
			coupon_data.price      = 0;
			coupon_data.stat_value = item_data->stat_value;

			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BASKET_ADD_ITEM(reinterpret_cast<Any*>(&coupon_data), quantity));
			return;
		}

		src->set_return_value<BOOL>(item_added);
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
