#pragma once

//File allowing entry to the game engine from outside

#ifdef KJK_PLATFORM_WINDOWS

#include "KJK_Engine/Application.h"
#include "KJK_Engine/Logger.h"

extern KJK::Application* KJK::CreateApplication();

int main(int argc, char** argv)
{
	KJK::Logger::Init();
	KJK_CORE_WARN("Initialized the Logger!");
	KJK_INFO("Initialized the Logger!");

	KJK::Application* app = KJK::CreateApplication();
	app->Run();
	delete app;
}

#endif // KJK_PLATFORM_WINDOWS
