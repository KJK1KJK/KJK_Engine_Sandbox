#include <KJK>

#include <iostream>

//Test file for implementing game engine features usage

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