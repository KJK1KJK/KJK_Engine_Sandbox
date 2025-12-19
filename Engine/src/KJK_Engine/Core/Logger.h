#pragma once

#ifndef SPDLOG_ACTIVE_LEVEL
	#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h" //Allows logging custom types

namespace KJK
{
	//Logger class for engine and client logging
	class Logger
	{
	public:
		//Initialize the loggers
		static void Init();

		//Getters for the loggers
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		//Static pointers to the loggers
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

#ifdef KJK_MINSIZE
	//Core logger macros
	#define KJK_CORE_CRITICAL(...)
	#define KJK_CORE_ERROR(...)
	#define KJK_CORE_WARN(...)
	#define KJK_CORE_INFO(...)
	#define KJK_CORE_TRACE(...)
	//Client logger macros
	#define KJK_CRITICAL(...)
	#define KJK_ERROR(...)
	#define KJK_WARN(...)
	#define KJK_INFO(...)
	#define KJK_TRACE(...)
#else
	//Core logger macros
	#define KJK_CORE_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(::KJK::Logger::GetCoreLogger(), __VA_ARGS__)
	#define KJK_CORE_ERROR(...)    SPDLOG_LOGGER_ERROR(::KJK::Logger::GetCoreLogger(), __VA_ARGS__)
	#define KJK_CORE_WARN(...)     SPDLOG_LOGGER_WARN(::KJK::Logger::GetCoreLogger(), __VA_ARGS__)
	#define KJK_CORE_INFO(...)     SPDLOG_LOGGER_INFO(::KJK::Logger::GetCoreLogger(), __VA_ARGS__)
	#define KJK_CORE_TRACE(...)    SPDLOG_LOGGER_TRACE(::KJK::Logger::GetCoreLogger(), __VA_ARGS__)

	//Client logger macros
	#define KJK_CRITICAL(...)      SPDLOG_LOGGER_CRITICAL(::KJK::Logger::GetClientLogger(), __VA_ARGS__)
	#define KJK_ERROR(...)         SPDLOG_LOGGER_ERROR(::KJK::Logger::GetClientLogger(), __VA_ARGS__)
	#define KJK_WARN(...)          SPDLOG_LOGGER_WARN(::KJK::Logger::GetClientLogger(), __VA_ARGS__)
	#define KJK_INFO(...)          SPDLOG_LOGGER_INFO(::KJK::Logger::GetClientLogger(), __VA_ARGS__)
	#define KJK_TRACE(...)         SPDLOG_LOGGER_TRACE(::KJK::Logger::GetClientLogger(), __VA_ARGS__)
#endif