#pragma once

#include "KJK_Engine/Events/Event.h"

namespace KJk
{
	//Struct representing properties necessary for creating a window
	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;

		WindowProps(const std::string& title = "KJK Engine", unsigned int width = 1280, unsigned int height = 720)
			: Title(title), Width(width), Height(height) {}
	};

	//Interface representing a desktop system window
	class Window
	{
	public:
		//Function type for event callbacks
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		//Called every frame to update the window
		virtual void OnUpdate() = 0;

		//Getter for window width
		virtual unsigned int GetWidth() const = 0;
		//Getter for window height
		virtual unsigned int GetHeight() const = 0;

		//Setter for the event callback function
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		//Setter for the VSync state
		virtual void SetVSync(bool enabled) = 0;
		//Getter for the VSync state
		virtual bool IsVSync() const = 0;
		
		//Factory method for creating a window
		static Window* Create(const WindowProps& props = WindowProps());
	}
}