#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)
#include "App.hpp"

#define UNUSED(x) (void)(x);
extern App* g_theApp;

int WINAPI WinMain(_In_ HINSTANCE applicationInstanceHandle, _In_ HINSTANCE preViousInstance, _In_ LPSTR commandLineString, _In_ int nShowCmd)
{
	UNUSED( applicationInstanceHandle );
	UNUSED( preViousInstance );
	UNUSED( commandLineString );
	UNUSED( nShowCmd );

	g_theApp = new App();
	g_theApp->Startup();
	g_theApp->Run();
	g_theApp->Shutdown();
	delete g_theApp;
	g_theApp = nullptr;
	return 0;
}



