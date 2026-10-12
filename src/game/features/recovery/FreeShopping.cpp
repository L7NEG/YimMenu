#include "backend/bool_command.hpp"
#include "native_hooks/free_shopping.hpp"

namespace big
{
	bool_command g_free_shopping("freeshopping", "FREE_SHOPPING", "FREE_SHOPPING_DESC", free_shopping::g_enabled);
}
