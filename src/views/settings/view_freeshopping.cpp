#include "backend/looped/self/freeshopping_data.hpp"
#include "views/view.hpp"

namespace big
{
	void view::freeshopping()
	{
		ImGui::SeparatorText("FREE_SHOPPING"_T.data());
		ImGui::TextWrapped("FREE_SHOPPING_DESCRIPTION"_T.data());

		ImGui::Checkbox("FREE_SHOPPING_MASTER"_T.data(), &g.freeshopping.enabled);

		ImGui::SeparatorText("FREE_SHOPPING_CATEGORIES"_T.data());

		ImGui::Checkbox("FREE_SHOPPING_WEAPONS"_T.data(), &g.freeshopping.weapons);
		ImGui::Checkbox("FREE_SHOPPING_WEAPON_MODS"_T.data(), &g.freeshopping.weapon_mods);
		ImGui::Checkbox("FREE_SHOPPING_CLOTHING"_T.data(), &g.freeshopping.clothing);
		ImGui::Checkbox("FREE_SHOPPING_VEHICLE"_T.data(), &g.freeshopping.vehicle);
		ImGui::Checkbox("FREE_SHOPPING_SERVICES"_T.data(), &g.freeshopping.services);
		ImGui::Checkbox("FREE_SHOPPING_PROPERTIES"_T.data(), &g.freeshopping.properties);
		ImGui::Checkbox("FREE_SHOPPING_MISC"_T.data(), &g.freeshopping.misc);

		ImGui::SeparatorText("FREE_SHOPPING_UNLOCK_ITEMS"_T.data());
		ImGui::Checkbox("FREE_SHOPPING_UNLOCK_ITEMS"_T.data(), &g.freeshopping.unlock_items);
		ImGui::TextWrapped("FREE_SHOPPING_UNLOCK_ITEMS_DESC"_T.data());

		ImGui::Separator();

		if (components::button("FREE_SHOPPING_RESTORE_ALL"_T))
		{
			g.freeshopping.enabled = false;
		}

		ImGui::Text("FREE_SHOPPING_ITEMS_COUNT"_T.data(), g_freeshopping_item_count);
	}
}