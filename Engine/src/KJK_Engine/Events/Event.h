#pragma once

#include <string>
#include <functional>

namespace KJK
{
	//Types of events
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	//Categories of events, saved as bitflags to allows multiple categories to be applied to a single event
	enum EventCategory
	{
		None = 0,
		EventCategoryApplication	= 1 << 0,
		EventCategoryInput          = 1 << 1,
		EventCategoryKeyboard       = 1 << 2,
		EventCategoryMouse          = 1 << 3,
		EventCategoryMouseButton    = 1 << 4
	};

//Macro for defining all virtual methods related to event type
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

//Macro for defining one or more categories for an event type
#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	//Base Event class
	//Signifies an event occurring in the window application
	class Event
	{
		//Associated EventDispatcher which handles events
		friend class EventDispatcher;
	public:
		//Getter for event type
		virtual EventType GetEventType() const = 0;
		//Getter for event category
		virtual int GetCategoryFlags() const = 0;
		//Used for logging event info
		virtual const char* GetName() const = 0;
		//Used for logging detailed event info
		virtual std::string ToString() const { return GetName(); }

		//Check if the event is in a certain category
		inline bool IsInCategory(EventCategory category)
		{
			//Bitwise AND to check category flag
			return GetCategoryFlags() & category;
		}

	protected:
		//Check if the event has been handled
		bool m_Handled = false;
	};

	//Class for dispatching events to the correct event handler functions
	class EventDispatcher
	{
		//Define a function type for event handling functions
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& event)
			: m_Event(event) {}

		//Dispatch the event to the correct event function if the types match
		template<typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.m_Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}
	private:
		//Reference to the event being dispatched
		Event& m_Event;
	};

	//Format event as a string for logging
	inline std::string format_as(const Event& e) {
		return e.ToString();
	}
}