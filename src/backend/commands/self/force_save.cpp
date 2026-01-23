#include "backend/command.hpp"
#include "natives.hpp"

namespace big
{
	class force_save : command
	{
		using command::command;

		virtual void execute(const command_arguments&, const std::shared_ptr<command_context> ctx) override
		{
			STATS::STAT_SAVE(0, 0, 3, 0);
		}
	};

	force_save g_force_save("forcesave", "Force Save", "Forces the game to save to cloud.", 0);
}
