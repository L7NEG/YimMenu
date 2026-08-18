#pragma once
#include <cstdint>
#include "fiber_pool.hpp"
#include "native_hooks.hpp"
#include "core/scr_globals.hpp"
#include "util/math.hpp"

namespace big
{
	namespace shop_controller
	{
		inline std::string format_chips(int value)
		{
			std::string formatted = std::to_string(value);
			for (int i = formatted.size() - 3; i > 0; i -= 3)
			{
				formatted.insert(i, ",");
			}
			return formatted;
		}

		// Free shopping implemented the way Cherax does it:
		//  - Free Shopping (g.self.free_shopping): store purchases (basket flow +
		//    NETWORK_BUY_*/NETWORK_DEDUCT_CASH) + property upgrades/renovations
		//    (the NETWORK_SPENT_*/NETWORK_SPEND_* family).
		//
		// A REAL full-price server transaction always fires — that is what records
		// ownership on Rockstar's backend so it persists across sessions. The money
		// the server charges is then refunded back to the player automatically
		// (NETWORK_REFUND_CASH, queued on the fiber pool so it runs after the charge).
		// Server transactions are forced on so the purchase is never client-side.
		// Can-spend stats are spoofed so the script always lets it through.

		// Queue a refund for the charged amount. Runs on the fiber pool so it fires
		// AFTER the purchase charge has been applied server-side, keeping the purchase
		// legitimate (it persists) while returning the money to the player.
		inline void refund(int amount)
		{
			if (amount > 0)
				g_fiber_pool->queue_job([amount] {
					MONEY::NETWORK_REFUND_CASH(amount, "FREE_SHOPPING", "Free Shopping Refund", true);
				});
		}

		void STAT_GET_INT(rage::scrNativeCallContext* src)
		{
			const auto stat_hash = src->get_arg<Hash>(0);
			auto* out            = src->get_arg<int*>(1);
			const auto p2        = src->get_arg<int>(2);

			src->set_return_value<BOOL>(STATS::STAT_GET_INT(stat_hash, out, p2));
		}

			src->set_return_value<BOOL>(std::move(result));
		}

		void SET_WARNING_MESSAGE_WITH_HEADER(rage::scrNativeCallContext* src)
		{
			HUD::SET_WARNING_MESSAGE_WITH_HEADER(src->get_arg<const char*>(0), src->get_arg<const char*>(1), src->get_arg<int>(2), src->get_arg<const char*>(3), src->get_arg<BOOL>(4), src->get_arg<Any>(5), src->get_arg<Any*>(6), src->get_arg<Any*>(7), src->get_arg<BOOL>(8), src->get_arg<Any>(9));
		}

		void SCALEFORM_MOVIE_METHOD_ADD_PARAM_INT(rage::scrNativeCallContext* src)
		{
			auto arg0 = src->get_arg<int>(0);
			if (g.window.gui.format_money)
			{
				Hash casino_chips = self::char_index ? "MP1_CASINO_CHIPS"_J : "MP0_CASINO_CHIPS"_J;
				int player_chips;

				STATS::STAT_GET_INT(casino_chips, &player_chips, -1);
				if (arg0 == player_chips && player_chips >= 1000)
				{
					auto chips_format = format_chips(player_chips);
					return GRAPHICS::SCALEFORM_MOVIE_METHOD_ADD_PARAM_PLAYER_NAME_STRING(chips_format.c_str());
				}
			}

			GRAPHICS::SCALEFORM_MOVIE_METHOD_ADD_PARAM_INT(arg0);
		}

		void NET_GAMESERVER_GET_PRICE(rage::scrNativeCallContext* src)
		{
			auto itemHash = src->get_arg<Hash>(0);
			auto categoryHash = src->get_arg<Hash>(1);
			auto p2 = src->get_arg<BOOL>(2);
			
			src->set_return_value<int>(NETSHOPPING::NET_GAMESERVER_GET_PRICE(itemHash, categoryHash, p2));
		}

		void NET_GAMESERVER_CATALOG_ITEM_IS_VALID(rage::scrNativeCallContext* src)
		{
			auto name = src->get_arg<const char*>(0);
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_CATALOG_ITEM_IS_VALID(name));
		}

		void NET_GAMESERVER_CATALOG_ITEM_KEY_IS_VALID(rage::scrNativeCallContext* src)
		{
			auto hash = src->get_arg<Hash>(0);
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_CATALOG_ITEM_KEY_IS_VALID(hash));
		}

		void NET_GAMESERVER_CATALOG_IS_VALID(rage::scrNativeCallContext* src)
		{
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_CATALOG_IS_VALID());
		}

		void NET_GAMESERVER_BEGIN_SERVICE(rage::scrNativeCallContext* src)
		{
			auto transactionId = src->get_arg<int*>(0);
			auto categoryHash = src->get_arg<Hash>(1);
			auto itemHash = src->get_arg<Hash>(2);
			auto actionTypeHash = src->get_arg<Hash>(3);
			auto value = src->get_arg<int>(4);
			auto flags = src->get_arg<int>(5);

			// Keep the real price so the server accepts and records the purchase
			// (that is what makes ownership persist); the money is refunded after
			// the deduction natives apply the charge.
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BEGIN_SERVICE(transactionId, categoryHash, itemHash, actionTypeHash, value, flags));
		}

		void NET_GAMESERVER_USE_SERVER_TRANSACTIONS(rage::scrNativeCallContext *src)
		{
			// Force REAL server transactions so the server records ownership (persists).
			if (g.self.free_shopping)
			{
				src->set_return_value<BOOL>(TRUE);
				return;
			}
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_USE_SERVER_TRANSACTIONS());
		}

		void NET_GAMESERVER_END_SERVICE(rage::scrNativeCallContext* src)
		{
			auto transactionId = src->get_arg<int>(0);
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_END_SERVICE(transactionId));
		}

		void NET_GAMESERVER_GET_CATALOG_CLOUD_CRC(rage::scrNativeCallContext* src)
		{
			src->set_return_value<Hash>(NETSHOPPING::NET_GAMESERVER_GET_CATALOG_CLOUD_CRC());
		}

		void NET_GAMESERVER_RETRIEVE_INIT_SESSION_STATUS(rage::scrNativeCallContext* src)
		{
			auto p0 = src->get_arg<int*>(0);
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_RETRIEVE_INIT_SESSION_STATUS(p0));
		}

		void NET_GAMESERVER_START_SESSION(rage::scrNativeCallContext* src)
		{
			auto charSlot = src->get_arg<int>(0);
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_START_SESSION(charSlot));
		}

		void NET_GAMESERVER_START_SESSION_PENDING(rage::scrNativeCallContext* src)
		{
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_START_SESSION_PENDING());
		}

		void NET_GAMESERVER_RETRIEVE_START_SESSION_STATUS(rage::scrNativeCallContext* src)
		{
			auto p0 = src->get_arg<int*>(0);
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_RETRIEVE_START_SESSION_STATUS(p0));
		}

		void NET_GAMESERVER_IS_SESSION_REFRESH_PENDING(rage::scrNativeCallContext* src)
		{
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_IS_SESSION_REFRESH_PENDING());
		}

		void NET_GAMESERVER_GET_SESSION_STATE_AND_STATUS(rage::scrNativeCallContext* src)
		{
			auto p0 = src->get_arg<int*>(0);
			auto p1 = src->get_arg<BOOL*>(1);
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_GET_SESSION_STATE_AND_STATUS(p0, p1));
		}

		void NET_GAMESERVER_TRANSACTION_IN_PROGRESS(rage::scrNativeCallContext* src)
		{
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_TRANSACTION_IN_PROGRESS());
		}

		void NET_GAMESERVER_IS_SESSION_VALID(rage::scrNativeCallContext* src)
		{
			auto charSlot = src->get_arg<int>(0);
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_IS_SESSION_VALID(charSlot));
		}

		void NET_GAMESERVER_RETRIEVE_SESSION_ERROR_CODE(rage::scrNativeCallContext* src)
		{
			auto p0 = src->get_arg<int*>(0);
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_RETRIEVE_SESSION_ERROR_CODE(p0));
		}

		void NET_GAMESERVER_IS_CATALOG_CURRENT(rage::scrNativeCallContext* src)
		{
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_IS_CATALOG_CURRENT());
		}

		void NET_GAMESERVER_RETRIEVE_CATALOG_REFRESH_STATUS(rage::scrNativeCallContext* src)
		{
			auto state = src->get_arg<int*>(0);
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_RETRIEVE_CATALOG_REFRESH_STATUS(state));
		}

		void NET_GAMESERVER_CHECKOUT_START(rage::scrNativeCallContext* src)
		{
			auto transactionId = src->get_arg<int>(0);
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_CHECKOUT_START(transactionId));
		}

		void NET_GAMESERVER_BASKET_START(rage::scrNativeCallContext* src)
		{
			auto transactionId = src->get_arg<int*>(0);
			auto categoryHash = src->get_arg<Hash>(1);
			auto actionHash = src->get_arg<Hash>(2);
			auto flags = src->get_arg<int>(3);
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BASKET_START(transactionId, categoryHash, actionHash, flags));
		}

		void NET_GAMESERVER_BASKET_ADD_ITEM(rage::scrNativeCallContext* src)
		{
			auto itemData = src->get_arg<int*>(0);
			auto quantity = src->get_arg<int>(1);

			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BASKET_ADD_ITEM((Any*)itemData, quantity));
		}

		void NET_GAMESERVER_BASKET_END(rage::scrNativeCallContext* src)
		{
			src->set_return_value<BOOL>(NETSHOPPING::NET_GAMESERVER_BASKET_END());
		}

		void NETWORK_BUY_ITEM(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			auto item = src->get_arg<Hash>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);
			auto p4 = src->get_arg<BOOL>(4);
			auto item_name = src->get_arg<const char*>(5);
			auto p6 = src->get_arg<Any>(6);
			auto p7 = src->get_arg<Any>(7);
			auto p8 = src->get_arg<Any>(8);
			auto p9 = src->get_arg<BOOL>(9);

			if (g.self.free_shopping)
				refund(amount);

			MONEY::NETWORK_BUY_ITEM(amount, item, p2, p3, p4, item_name, p6, p7, p8, p9);
		}

		void NETWORK_BUY_PROPERTY(rage::scrNativeCallContext* src)
		{
			auto cost = src->get_arg<int>(0);
			auto propertyName = src->get_arg<Hash>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<BOOL>(3);

			if (g.self.free_shopping)
				refund(cost);

			MONEY::NETWORK_BUY_PROPERTY(cost, propertyName, p2, p3);
		}

		void NETWORK_DEDUCT_CASH(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			auto p1 = src->get_arg<const char*>(1);
			auto p2 = src->get_arg<const char*>(2);
			auto p3 = src->get_arg<BOOL>(3);
			auto p4 = src->get_arg<BOOL>(4);
			auto p5 = src->get_arg<BOOL>(5);

			if (g.self.free_shopping)
				refund(amount);

			MONEY::NETWORK_DEDUCT_CASH(amount, p1, p2, p3, p4, p5);
		}

		void NETWORK_BUY_HEALTHCARE(rage::scrNativeCallContext* src)
		{
			auto cost = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(cost);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);

			MONEY::NETWORK_BUY_HEALTHCARE(cost, p1, p2);
		}

		void NETWORK_BUY_AIRSTRIKE(rage::scrNativeCallContext* src)
		{
			auto cost = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(cost);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_BUY_AIRSTRIKE(cost, p1, p2, p3);
		}

		void NETWORK_BUY_HELI_STRIKE(rage::scrNativeCallContext* src)
		{
			auto cost = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(cost);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_BUY_HELI_STRIKE(cost, p1, p2, p3);
		}

		void NETWORK_BUY_BOUNTY(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto victim = src->get_arg<Player>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<BOOL>(3);
			auto p4 = src->get_arg<Any>(4);

			MONEY::NETWORK_BUY_BOUNTY(amount, victim, p2, p3, p4);
		}

		void NETWORK_BUY_FAIRGROUND_RIDE(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<BOOL>(3);
			auto p4 = src->get_arg<Any>(4);

			MONEY::NETWORK_BUY_FAIRGROUND_RIDE(amount, p1, p2, p3, p4);
		}

		void NETWORK_SPENT_MOVE_YACHT(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);

			MONEY::NETWORK_SPENT_MOVE_YACHT(amount, p1, p2);
		}

		void NETWORK_SPENT_HANGAR_UTILITY_CHARGES(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);

			MONEY::NETWORK_SPENT_HANGAR_UTILITY_CHARGES(amount, p1, p2);
		}

		void NETWORK_SPENT_HANGAR_STAFF_CHARGES(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);

			MONEY::NETWORK_SPENT_HANGAR_STAFF_CHARGES(amount, p1, p2);
		}

		void NETWORK_GET_VC_BANK_BALANCE(rage::scrNativeCallContext* src)
		{
			src->set_return_value<int>(MONEY::NETWORK_GET_VC_BANK_BALANCE());
		}

		void NETWORK_GET_VC_WALLET_BALANCE(rage::scrNativeCallContext* src)
		{
			src->set_return_value<int>(MONEY::NETWORK_GET_VC_WALLET_BALANCE(src->get_arg<int>(0)));
		}

		void NETWORK_GET_VC_BALANCE(rage::scrNativeCallContext* src)
		{
			src->set_return_value<int>(MONEY::NETWORK_GET_VC_BALANCE());
		}

		void NETWORK_GET_EVC_BALANCE(rage::scrNativeCallContext* src)
		{
			src->set_return_value<int>(MONEY::NETWORK_GET_EVC_BALANCE());
		}

		void NETWORK_GET_PVC_BALANCE(rage::scrNativeCallContext* src)
		{
			src->set_return_value<int>(MONEY::NETWORK_GET_PVC_BALANCE());
		}

		void NETWORK_GET_STRING_WALLET_BALANCE(rage::scrNativeCallContext* src)
		{
			src->set_return_value<const char*>(MONEY::NETWORK_GET_STRING_WALLET_BALANCE(src->get_arg<int>(0)));
		}

		void NETWORK_GET_STRING_BANK_BALANCE(rage::scrNativeCallContext* src)
		{
			src->set_return_value<const char*>(MONEY::NETWORK_GET_STRING_BANK_BALANCE());
		}

		void NETWORK_GET_STRING_BANK_WALLET_BALANCE(rage::scrNativeCallContext* src)
		{
			src->set_return_value<const char*>(MONEY::NETWORK_GET_STRING_BANK_WALLET_BALANCE(src->get_arg<int>(0)));
		}

		void NETWORK_CASINO_BUY_CHIPS(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<int>(1);

			MONEY::NETWORK_CASINO_BUY_CHIPS(amount, p1);
		}

		void NETWORK_SPENT_UPGRADE_OFFICE_PROPERTY(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);
			auto p4 = src->get_arg<Any>(4);

			MONEY::NETWORK_SPENT_UPGRADE_OFFICE_PROPERTY(amount, p1, p2, p3, p4);
		}

		void NETWORK_SPENT_PURCHASE_OFFICE_PROPERTY(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);
			auto p4 = src->get_arg<Any>(4);

			MONEY::NETWORK_SPENT_PURCHASE_OFFICE_PROPERTY(amount, p1, p2, p3, p4);
		}

		void NETWORK_SPENT_UPGRADE_WAREHOUSE_PROPERTY(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_UPGRADE_WAREHOUSE_PROPERTY(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_PURCHASE_WAREHOUSE_PROPERTY(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_PURCHASE_WAREHOUSE_PROPERTY(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_UPGRADE_IMPEXP_WAREHOUSE_PROPERTY(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_UPGRADE_IMPEXP_WAREHOUSE_PROPERTY(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_PURCHASE_IMPEXP_WAREHOUSE_PROPERTY(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any*>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<BOOL>(3);

			MONEY::NETWORK_SPENT_PURCHASE_IMPEXP_WAREHOUSE_PROPERTY(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_PURCHASE_CLUB_HOUSE(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_PURCHASE_CLUB_HOUSE(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_UPGRADE_CLUB_HOUSE(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_UPGRADE_CLUB_HOUSE(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_PURCHASE_BUSINESS_PROPERTY(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_PURCHASE_BUSINESS_PROPERTY(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_UPGRADE_BUSINESS_PROPERTY(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_UPGRADE_BUSINESS_PROPERTY(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_UPGRADE_OFFICE_GARAGE(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			if (g.self.free_shopping)
				refund(amount);

			MONEY::NETWORK_SPENT_UPGRADE_OFFICE_GARAGE(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_PURCHASE_OFFICE_GARAGE(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			if (g.self.free_shopping)
				refund(amount);

			MONEY::NETWORK_SPENT_PURCHASE_OFFICE_GARAGE(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_UPGRADE_HANGAR(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_UPGRADE_HANGAR(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_PURCHASE_HANGAR(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_PURCHASE_HANGAR(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_UPGRADE_TRUCK(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_UPGRADE_TRUCK(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_BUY_TRUCK(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_BUY_TRUCK(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_UPRADE_BUNKER(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_UPRADE_BUNKER(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_BUY_BUNKER(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_BUY_BUNKER(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_UPGRADE_BASE(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_UPGRADE_BASE(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_BUY_BASE(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_BUY_BASE(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_BUY_TILTROTOR(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_BUY_TILTROTOR(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_UPGRADE_TILTROTOR(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_UPGRADE_TILTROTOR(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_UPGRADE_HACKER_TRUCK(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_UPGRADE_HACKER_TRUCK(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_PURCHASE_HACKER_TRUCK(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_PURCHASE_HACKER_TRUCK(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_UPGRADE_NIGHTCLUB_AND_WAREHOUSE(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_UPGRADE_NIGHTCLUB_AND_WAREHOUSE(amount, p1, p2, p3);
		}

		void NETWORK_SPENT_PURCHASE_NIGHTCLUB_AND_WAREHOUSE(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPENT_PURCHASE_NIGHTCLUB_AND_WAREHOUSE(amount, p1, p2, p3);
		}

		void NETWORK_SPEND_UPGRADE_ARENA(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<const char*>(3);

			MONEY::NETWORK_SPEND_UPGRADE_ARENA(amount, p1, p2, p3);
		}

		void NETWORK_SPEND_BUY_ARENA(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<const char*>(3);

			MONEY::NETWORK_SPEND_BUY_ARENA(amount, p1, p2, p3);
		}

		void NETWORK_SPEND_UPGRADE_CASINO(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<Any*>(3);

			MONEY::NETWORK_SPEND_UPGRADE_CASINO(amount, p1, p2, p3);
		}

		void NETWORK_SPEND_BUY_CASINO(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<Any*>(3);

			MONEY::NETWORK_SPEND_BUY_CASINO(amount, p1, p2, p3);
		}

		void NETWORK_SPEND_UPGRADE_ARCADE(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPEND_UPGRADE_ARCADE(amount, p1, p2, p3);
		}

		void NETWORK_SPEND_BUY_ARCADE(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPEND_BUY_ARCADE(amount, p1, p2, p3);
		}

		void NETWORK_SPEND_UPGRADE_SUB(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPEND_UPGRADE_SUB(amount, p1, p2, p3);
		}

		void NETWORK_SPEND_BUY_SUB(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPEND_BUY_SUB(amount, p1, p2, p3);
		}

		void NETWORK_SPEND_UPGRADE_AUTOSHOP(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPEND_UPGRADE_AUTOSHOP(amount, p1, p2, p3);
		}

		void NETWORK_SPEND_BUY_AUTOSHOP(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPEND_BUY_AUTOSHOP(amount, p1, p2, p3);
		}

		void NETWORK_SPEND_UPGRADE_AGENCY(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPEND_UPGRADE_AGENCY(amount, p1, p2, p3);
		}

		void NETWORK_SPEND_BUY_AGENCY(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::NETWORK_SPEND_BUY_AGENCY(amount, p1, p2, p3);
		}

		void _NETWORK_SPEND_BUY_MFGARAGE(rage::scrNativeCallContext* src)
		{
			auto p0 = src->get_arg<Any>(0);
			if (g.self.free_shopping)
				refund(p0);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::_NETWORK_SPEND_BUY_MFGARAGE(p0, p1, p2, p3);
		}

		void _NETWORK_SPEND_UPGRADE_MFGARAGE(rage::scrNativeCallContext* src)
		{
			auto p0 = src->get_arg<Any>(0);
			if (g.self.free_shopping)
				refund(p0);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::_NETWORK_SPEND_UPGRADE_MFGARAGE(p0, p1, p2, p3);
		}

		void _NETWORK_SPEND_BUY_ACID_LAB(rage::scrNativeCallContext* src)
		{
			auto p0 = src->get_arg<Any>(0);
			if (g.self.free_shopping)
				refund(p0);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::_NETWORK_SPEND_BUY_ACID_LAB(p0, p1, p2, p3);
		}

		void _NETWORK_SPEND_BUY_SUPPLIES(rage::scrNativeCallContext* src)
		{
			auto p0 = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(p0);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<int>(3);

			MONEY::_NETWORK_SPEND_BUY_SUPPLIES(p0, p1, p2, p3);
		}

		void _NETWORK_SPEND_UPGRADE_ACID_LAB_EQUIPMENT(rage::scrNativeCallContext* src)
		{
			auto p0 = src->get_arg<Any>(0);
			if (g.self.free_shopping)
				refund(p0);
			auto p1 = src->get_arg<Any>(1);
			auto p2 = src->get_arg<Any>(2);
			auto p3 = src->get_arg<Any>(3);

			MONEY::_NETWORK_SPEND_UPGRADE_ACID_LAB_EQUIPMENT(p0, p1, p2, p3);
		}

		void _NETWORK_SPEND_UPGRADE_ACID_LAB_ARMOR(rage::scrNativeCallContext* src)
		{
			auto p0 = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(p0);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<int>(3);

			MONEY::_NETWORK_SPEND_UPGRADE_ACID_LAB_ARMOR(p0, p1, p2, p3);
		}

		void _NETWORK_SPEND_UPGRADE_ACID_LAB_SCOOP(rage::scrNativeCallContext* src)
		{
			auto p0 = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(p0);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<int>(3);

			MONEY::_NETWORK_SPEND_UPGRADE_ACID_LAB_SCOOP(p0, p1, p2, p3);
		}

		void _NETWORK_SPEND_UPGRADE_ACID_LAB_MINES(rage::scrNativeCallContext* src)
		{
			auto p0 = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(p0);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<int>(3);

			MONEY::_NETWORK_SPEND_UPGRADE_ACID_LAB_MINES(p0, p1, p2, p3);
		}

		void _NETWORK_SPENT_AIR_FREIGHT(rage::scrNativeCallContext* src)
		{
			auto p0 = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(p0);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<int>(3);
			auto p4 = src->get_arg<int>(4);
			auto p5 = src->get_arg<int>(5);

			MONEY::_NETWORK_SPENT_AIR_FREIGHT(p0, p1, p2, p3, p4, p5);
		}

		void _NETWORK_SPENT_STEALTH_MODULE(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<Hash>(3);

			MONEY::_NETWORK_SPENT_STEALTH_MODULE(amount, p1, p2, p3);
		}

		void _NETWORK_SPENT_MISSILE_JAMMER(rage::scrNativeCallContext* src)
		{
			auto amount = src->get_arg<int>(0);
			if (g.self.free_shopping)
				refund(amount);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<Hash>(3);

			MONEY::_NETWORK_SPENT_MISSILE_JAMMER(amount, p1, p2, p3);
		}

		void _NETWORK_SPENT_GENERIC(rage::scrNativeCallContext* src)
		{
			auto price = src->get_arg<int>(0);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto stat = src->get_arg<Hash>(3);
			auto spent = src->get_arg<Hash>(4);
			auto p5 = src->get_arg<const char*>(5);
			auto p6 = src->get_arg<const char*>(6);
			auto data = src->get_arg<Any*>(7);

			if (g.self.free_shopping)
				refund(price);

			MONEY::_NETWORK_SPENT_GENERIC(price, p1, p2, stat, spent, p5, p6, data);
		}

		void NETWORK_CAN_SPEND_MONEY(rage::scrNativeCallContext* src)
		{
			if (g.self.free_shopping)
			{
				src->set_return_value<BOOL>(TRUE);
				return;
			}
			auto p0 = src->get_arg<Any>(0);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<BOOL>(3);
			auto p4 = src->get_arg<Any>(4);
			auto p5 = src->get_arg<Any>(5);
			src->set_return_value<BOOL>(MONEY::NETWORK_CAN_SPEND_MONEY(p0, p1, p2, p3, p4, p5));
		}

		void NETWORK_CAN_SPEND_MONEY2(rage::scrNativeCallContext* src)
		{
			if (g.self.free_shopping)
			{
				src->set_return_value<BOOL>(TRUE);
				return;
			}
			auto p0 = src->get_arg<Any>(0);
			auto p1 = src->get_arg<BOOL>(1);
			auto p2 = src->get_arg<BOOL>(2);
			auto p3 = src->get_arg<BOOL>(3);
			auto p4 = src->get_arg<Any*>(4);
			auto p5 = src->get_arg<Any>(5);
			auto p6 = src->get_arg<Any>(6);
			src->set_return_value<BOOL>(MONEY::NETWORK_CAN_SPEND_MONEY2(p0, p1, p2, p3, p4, p5, p6));
		}

		void NETWORK_GET_CAN_SPEND_FROM_WALLET(rage::scrNativeCallContext* src)
		{
			if (g.self.free_shopping)
			{
				src->set_return_value<BOOL>(TRUE);
				return;
			}
			src->set_return_value<BOOL>(MONEY::NETWORK_GET_CAN_SPEND_FROM_WALLET(src->get_arg<int>(0), src->get_arg<int>(1)));
		}

		void NETWORK_GET_CAN_SPEND_FROM_BANK(rage::scrNativeCallContext* src)
		{
			if (g.self.free_shopping)
			{
				src->set_return_value<BOOL>(TRUE);
				return;
			}
			src->set_return_value<BOOL>(MONEY::NETWORK_GET_CAN_SPEND_FROM_BANK(src->get_arg<int>(0)));
		}

		void NETWORK_GET_CAN_SPEND_FROM_BANK_AND_WALLET(rage::scrNativeCallContext* src)
		{
			if (g.self.free_shopping)
			{
				src->set_return_value<BOOL>(TRUE);
				return;
			}
			src->set_return_value<BOOL>(MONEY::NETWORK_GET_CAN_SPEND_FROM_BANK_AND_WALLET(src->get_arg<int>(0), src->get_arg<int>(1)));
		}
	}
}
