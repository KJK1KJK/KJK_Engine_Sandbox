#pragma once

#include "Core.h"

namespace KJK
{
	class Application
	{
		public:
		Application();
		virtual ~Application();

		void Run();
	};

	Application* CreateApplication();
}