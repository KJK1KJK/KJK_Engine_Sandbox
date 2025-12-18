#pragma once

//File allowing entry to the game engine from outside

#ifdef KJK_PLATFORM_WINDOWS

#include "KJK_Engine/Application.h"

extern KJK::Application* KJK::CreateApplication();

int main(int aergc, char** argv)
{
	KJK::Application* app = KJK::CreateApplication();
	app->Run();
	delete app;
}

#endif // KJK_PLATFORM_WINDOWS
