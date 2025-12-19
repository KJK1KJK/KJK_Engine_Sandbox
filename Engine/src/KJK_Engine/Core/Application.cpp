#include "Application.h"

#include "KJK_Engine/Core/Logger.h"
#include <KJK_Engine/Events/ApplicationEvent.h>

namespace KJK
{
	Application::Application()
	{

	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		WindowResizeEvent e(1280, 720);
		KJK_CORE_INFO(e);


		while (true)
		{

		}
	}
}

