#pragma once

#include "Macros.h"

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