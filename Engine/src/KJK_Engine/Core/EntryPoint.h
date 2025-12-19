#pragma once

//File allowing entry to the game engine from outside

#ifdef KJK_PLATFORM_WINDOWS

#include "KJK_Engine/Core/Application.h"
#include "KJK_Engine/Core/Logger.h"

//Get the application created in the client code
extern KJK::Application* KJK::CreateApplication();

#ifndef TEST_NO_ENTRYPOINT

int main(int argc, char** argv)
{
	KJK::Logger::Init();

	KJK::Application* app = KJK::CreateApplication();
	app->Run();
	delete app;
}

#endif // TEST_NO_ENTRYPOINT

#endif // KJK_PLATFORM_WINDOWS
