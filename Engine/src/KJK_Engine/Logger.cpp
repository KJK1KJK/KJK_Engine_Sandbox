#include "Logger.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace KJK
{
	//Define the static logger pointers
	std::shared_ptr<spdlog::logger> Logger::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Logger::s_ClientLogger;

	//Initialize the loggers used in the engine and client application
	void Logger::Init()
	{
		//Set the log pattern
		spdlog::set_pattern("%^[%l][%t][%D %H:%M:%S.%e][%n]: %v%$ [%s:%#]");

		//Define the logger used in the engine
		s_CoreLogger = spdlog::stdout_color_mt("KJK_ENGINE");
		s_CoreLogger->set_level(spdlog::level::trace);

		//Define the logger used in the client application
		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}