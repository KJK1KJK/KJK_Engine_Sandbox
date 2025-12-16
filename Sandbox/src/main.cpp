#include <KJK>

#include <iostream>

class SandboxApp : public KJK::Application
{
public:
	SandboxApp()
	{

	}

	~SandboxApp()
	{

	}
};


KJK::Application* KJK::CreateApplication()
{
	return new SandboxApp();
}