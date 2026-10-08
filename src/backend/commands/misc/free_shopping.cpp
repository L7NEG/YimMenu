#include "backend/bool_command.hpp"

namespace big
{
	bool_command g_free_shopping("freeshopping", "FREE_SHOPPING", "FREE_SHOPPING_DESC", g.shopping.free_shopping);
	bool_command g_free_shopping_log("fshoppinglog", "FREE_SHOPPING_LOG", "FREE_SHOPPING_LOG_DESC", g.shopping.log_transactions);
}
