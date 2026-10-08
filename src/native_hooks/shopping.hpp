#pragma once
#include "core/data/shopping_catalog.hpp"
#include "gta/joaat.hpp"
#include "natives.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace big::shopping
{
	// Basket item layout at script level (see decompiled shop scripts + Unlock-All.lua):
	// 4 x 64-bit slots (32 bytes): [0]=item, [1]=variation/extra, [2]=price, [3]=value.
	// Slot order is passed through as-is to the internal basket (your C++ slots 0..3),
	// so itemId is always slot 0 and variation is always slot 1. Do NOT swap them:
	// e.g. a hangar basket is [MP_STAT_MCKENZIE_HANGAR_OWNED, FIELD_HANGAR_INDEX_1, price, ...]
	// and the stat lookup must use slot 1 (FIELD_HANGAR_INDEX_1, stat 1).
	// NOTE: slot 2's high 32 bits can carry data (hangar baskets show high32=693),
	// so only ever read/write the LOW 32 bits of the price slot and preserve the high half.
	enum BasketSlot64 : std::size_t
	{
		kSlotItemId    = 0,
		kSlotVariation = 1,
		kSlotPrice     = 2,
		kSlotValue     = 3,
		kBasketSlotCount = 4
	};

	inline constexpr std::uint64_t kLow32Mask  = 0xFFFFFFFFull;
	inline constexpr std::uint64_t kHigh32Mask = 0xFFFFFFFF00000000ull;

	inline std::uint32_t low32(std::uint64_t v)
	{
		return static_cast<std::uint32_t>(v & kLow32Mask);
	}

	inline void set_low32(std::uint64_t& slot, std::uint32_t v)
	{
		slot = (slot & kHigh32Mask) | v;
	}

	inline constexpr rage::joaat_t kFreePurchaseCoupon = "PO_COUPON_CAR_XMAS2017"_J;

	inline constexpr rage::joaat_t kActionBuyProperty  = "NET_SHOP_ACTION_BUY_PROPERTY"_J;
	inline constexpr rage::joaat_t kActionBuyWarehouse = "NET_SHOP_ACTION_BUY_WAREHOUSE"_J;

	inline bool is_property_action(rage::joaat_t action)
	{
		return action == kActionBuyProperty || action == kActionBuyWarehouse;
	}

	inline constexpr std::array<rage::joaat_t, 55> kDiscountModifiers = {
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
	    "PM_WEAPON_PIM_AMMO_INCREASE"_J,
	};

	inline bool is_discount_modifier(rage::joaat_t hash)
	{
		for (const auto h : kDiscountModifiers)
		{
			if (h == hash)
				return true;
		}
		return false;
	}

	// Current basket context captured from BASKET_START. Script VM runs hooks sequentially.
	inline rage::joaat_t g_current_basket_action   = 0;
	inline rage::joaat_t g_current_basket_category = 0;

	inline void on_basket_start(rage::joaat_t category, rage::joaat_t action)
	{
		g_current_basket_category = category;
		g_current_basket_action   = action;
	}

	// rage::NetCatalog layout from the game (for reference / future live-catalog use).
	// The embedded Catalog_v94.json derived table below is used at runtime instead,
	// so no pointer scanning is required.
	struct NetCatalogBaseItem
	{
		void* m_vft{};
		std::uint32_t m_hash{};
		std::uint32_t m_category_hash{};
		std::int32_t m_price{};
		std::int32_t m_membership_price{};
		std::int32_t m_stat_value{};
		std::uint32_t m_unknown{};
	};
	static_assert(sizeof(NetCatalogBaseItem) == 0x20);

	struct NetCatalogNode
	{
		std::uint32_t m_key_hash{};
		std::uint32_t m_unknown04{};
		void* m_unknown08{};
		NetCatalogNode* m_next{};
		std::uint8_t m_unknown18[0x08]{};
		NetCatalogBaseItem m_item{};

		NetCatalogBaseItem* item()
		{
			return &m_item;
		}
		const NetCatalogBaseItem* item() const
		{
			return &m_item;
		}
	};
	static_assert(offsetof(NetCatalogNode, m_next) == 0x10);
	static_assert(offsetof(NetCatalogNode, m_item) == 0x20);

	inline void NET_GAMESERVER_BASKET_START(rage::scrNativeCallContext* src)
	{
		auto* transaction_id = src->get_arg<int*>(0);
		const auto category  = src->get_arg<rage::joaat_t>(1);
		const auto action    = src->get_arg<rage::joaat_t>(2);
		const auto flags     = src->get_arg<int>(3);

		on_basket_start(category, action);

		if (g.shopping.log_transactions)
		{
			LOG(INFO) << std::format("[FreeShopping][BASKET_START] category={:08X} action={:08X} flags={}", category, action, flags);
		}

		src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BASKET_START(transaction_id, category, action, flags));
	}

	inline void NET_GAMESERVER_BASKET_ADD_ITEM(rage::scrNativeCallContext* src)
	{
		auto* item     = src->get_arg<std::uint64_t*>(0);
		const auto qty = src->get_arg<int>(1);

		if (!g.shopping.free_shopping || item == nullptr)
		{
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BASKET_ADD_ITEM((Any*)item, qty));
			return;
		}

		const auto category = g_current_basket_category;
		const auto action   = g_current_basket_action;

		const auto item_id   = static_cast<rage::joaat_t>(low32(item[kSlotItemId]));
		const auto variation = static_cast<rage::joaat_t>(low32(item[kSlotVariation]));
		const auto price     = static_cast<std::int32_t>(low32(item[kSlotPrice]));
		const auto price_hi  = static_cast<std::uint32_t>((item[kSlotPrice] >> 32) & 0xFFFFFFFFu);
		const auto value_slot = item[kSlotValue];

		if (g.shopping.log_transactions)
		{
			LOG(INFO) << std::format("[FreeShopping][ADD_ITEM] cat={:08X} act={:08X} [0]={:08X} [1]={:08X} price={} price_hi={} [3]={} qty={}", category, action, item_id, variation, price, price_hi, value_slot, qty);
		}

		// Properties/warehouses can't use the coupon path; swap to the cheapest catalog entry sharing the same statValue.
		if (is_property_action(action) && price > 0)
		{
			bool swapped = false;
			if (const auto stat = shopping_lookup_stat_value(variation))
			{
				if (const auto* cheapest = shopping_find_cheapest(*stat, category))
				{
					if (g.shopping.log_transactions)
					{
						LOG(INFO) << std::format("[FreeShopping][PropertySwap] {:08X} -> {:08X} (price {} -> {})", variation, cheapest->m_hash, price, cheapest->m_price);
					}

					set_low32(item[kSlotVariation], cheapest->m_hash);
					set_low32(item[kSlotPrice], static_cast<std::uint32_t>(cheapest->m_price));
					swapped = true;
				}
				else if (g.shopping.log_transactions)
				{
					LOG(INFO) << std::format("[FreeShopping][PropertyNoCheapest] stat={} variation={:08X}", *stat, variation);
				}
			}
			else if (g.shopping.log_transactions)
			{
				LOG(INFO) << std::format("[FreeShopping][PropertyNoEntry] variation={:08X} item={:08X}", variation, item_id);
			}

			if (!swapped && g.shopping.log_transactions)
			{
				LOG(INFO) << std::format("[FreeShopping][PropertyPassthrough] full price {} kept", price);
			}

			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BASKET_ADD_ITEM((Any*)item, qty));
			return;
		}

		// Skip discount/modifier bookkeeping entries entirely.
		if (is_discount_modifier(item_id) || is_discount_modifier(variation))
		{
			if (g.shopping.log_transactions)
				LOG(INFO) << std::format("[FreeShopping][DiscountSkip] {:08X}", item_id);
			src->set_return_value<BOOL>(TRUE);
			return;
		}

		const bool apply_coupon = price > 0;
		if (apply_coupon)
			set_low32(item[kSlotPrice], 0);

		BOOL added = NETSHOPPING::NET_GAMESERVER_BASKET_ADD_ITEM((Any*)item, qty);

		if (apply_coupon && added)
		{
			// Mirror the game's own coupon pattern (see shop scripts using PM_COUPON_*):
			// coupon = [coupon_hash, orig_itemId, 0, orig_value].
			// NOTE: heap-allocate and intentionally leak: the basket keeps the pointer
			// for checkout (same reason Lua scripts leak 32 bytes per item).
			auto* coupon           = new std::uint64_t[kBasketSlotCount]();
			coupon[kSlotItemId]    = static_cast<std::uint64_t>(kFreePurchaseCoupon);
			coupon[kSlotVariation] = static_cast<std::uint64_t>(item_id);
			coupon[kSlotPrice]     = (item[kSlotPrice] & kHigh32Mask);
			coupon[kSlotValue]     = value_slot;

			if (g.shopping.log_transactions)
			{
				LOG(INFO) << std::format("[FreeShopping][Coupon] [0]={:08X} [1]={:08X} price=0 [3]={} qty={}", low32(coupon[0]), low32(coupon[1]), value_slot, qty);
			}

			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BASKET_ADD_ITEM((Any*)coupon, qty));
			return;
		}

		src->set_return_value<BOOL>(added ? TRUE : FALSE);
	}
}
