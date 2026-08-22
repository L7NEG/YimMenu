#pragma once
#include "core/scr_globals.hpp"
#include "native_hooks.hpp"

namespace big
{
	namespace free_shop
	{
		// Forces shop scripts down their local-transaction code path (the one normally
		// used in singleplayer): basket -> instant local checkout -> item applied by the
		// script itself. No NETSHOPPING packet is sent to Rockstar, so nothing is
		// validated or deducted server-side.
		void USE_SERVER_TRANSACTIONS(rage::scrNativeCallContext* src)
		{
			src->set_return_value<BOOL>(!g.self.free_shop);
		}
	}
}
