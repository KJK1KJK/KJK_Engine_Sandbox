#pragma once

#include "Event.h"

namespace KJK
{
	//Event class for key mount events
	class KeyEvent : public Event
	{
	public:
		inline int GetKeyCode() const { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(int keycode)
			: m_KeyCode(keycode) {}

		//Key code of the key event
		int m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(int keycode, int repeatCount)
			: KeyEvent(keycode), m_RepeatCount(repeatCount) {
		}

		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int m_RepeatCount;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int keycode)
			: KeyEvent(keycode) {
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(int keycode, char32_t character)
			: KeyEvent(keycode), m_KeyCharacter(character) {
		}

		inline char32_t GetCharacter() const { return m_KeyCharacter; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << static_cast<uint32_t>(m_KeyCharacter);

			//Check if the character is printable ASCII
			if (m_KeyCharacter <= 0x7F && std::isprint(static_cast<char>(m_KeyCharacter)))
				ss << " ('" << static_cast<char>(m_KeyCharacter) << "')";


			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	private:
		char32_t m_KeyCharacter;
	};
}