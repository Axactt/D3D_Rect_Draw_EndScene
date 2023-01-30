#include<iostream>
#include<Windows.h>
#include<d3d9.h>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")
#include <d3dx9.h>
#pragma comment(lib, "d3dx9.lib")
#include"hookTemplate.h"

using aliasEndscene = HRESULT( __stdcall* )(IDirect3DDevice9*);
aliasEndscene  EndScenePtr { nullptr };

// using hooking class here
HooknPatch hNP {};

int gameWindowWidth {};
int gameWindowHeight {};

// Get the size of Gamewindow  
void getWindowSize( HWND gamewindow)
{
	RECT rect {};
	if (GetWindowRect( gamewindow, &rect ))
	{
		gameWindowWidth = rect.right - rect.left;
		gameWindowHeight = rect.bottom - rect.top;

	}
}


//drawing filled rectangle function to be called in hookedEndscene
void DrawFillRect( IDirect3DDevice9* pDevice, int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b )
{
	D3DCOLOR rectColor = D3DCOLOR_XRGB( r, g, b );	//No point in using alpha because clear & alpha dont work!
	D3DRECT BarRect = { x, y, x + w, y + h };

	pDevice->Clear( 1, &BarRect, D3DCLEAR_TARGET | D3DCLEAR_TARGET, rectColor, 0, 0 );
}

// Create our hookFunction having prototype as Endscene
HRESULT __stdcall hookEndScene( IDirect3DDevice9* pDevice )
{
	std::cout << " I got hooked. now you can draw.\n";
	// Do our drawing stuff here
	// 
	//random reactangle
	DrawFillRect( pDevice, 25, 25, 100, 100,  255, 255, 255 ) ;

	// crosshair
	  DrawFillRect( pDevice,(gameWindowWidth / 2 - 2), (gameWindowHeight / 2 - 2), 4, 4,255, 255, 255 );
	
	//return back to original function using EndScenePtr function pointer
	return EndScenePtr( pDevice );

}


void* FindEndScene( HWND gameWindow )
{
	constexpr int  endscene_index = 42;// index of Endscene function in VTable. Same for all D3D9.dll
	constexpr int present_index = 17; // index of Present function in Vtable. same for all D3D9.dll
	//function to create a pointer to IDirect3D9 interface
	//methods of IDirect3D9 interface is used to create MS Direct3D objects and set up environment
	// This interace includes methods for enumerating and retrieving cpabilities of device
	//IDirect3D9 interface allows creation of IDirect3DDevice9 objects
	IDirect3D9* pD3D = NULL;
	if (NULL == (pD3D = Direct3DCreate9( D3D_SDK_VERSION )))
	{
		std::cout << " IDirect3D9 interface pointer not returned.\n";
		return nullptr;
	}
	else
	{
		std::cout << " IDirect3D9* pObject--->\t" << std::hex << pD3D << '\n';
	}
	
	if (pD3D)

	{
		// Now initialize value for D3DPRESENT_PARAMETERS structure that is used to create Direct3D device
		D3DPRESENT_PARAMETERS d3dpp {};
		d3dpp.BackBufferFormat = D3DFMT_R5G6B5;
		d3dpp.BackBufferCount = 1;
		d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow = gameWindow;
		d3dpp.Windowed = TRUE;
		d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		//now use IDirect3D9* interface with createdevice() function 
		// This will return address of IDirect3DDevice9* interface in form of pDevice pointer
		//Various implementation methods can be used from IDirect3DDevice9* interface
		//returns fully working device interface,set to the rqd display mode 
		//and allocated with appropriate back-buffers

		IDirect3DDevice9* pDevice {}; // interface pointer to IDirect3Ddevice9
		if (SUCCEEDED( pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, gameWindow, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pDevice ) ))
		{
			
			// These two lines get the address of vTable functions present in IDirect3DDevice9 interface
			// first read value of Vtable from class pointer pDevice(interface class pointer)
			//Then add the desired index to reah the desired function pointer
			// Then read the value of function address from that found in step(3)
			//cast it to void*

			void* presentAddress = (void*) *(std::intptr_t*) (*(std::intptr_t*) pDevice + (present_index * sizeof( std::intptr_t )));

			void* endSceneAddress = (void*) *(std::intptr_t*) (*(std::intptr_t*) pDevice + (endscene_index * sizeof( std::intptr_t )));

			std::cout << " IDirect3DDevice9* pDevice --->\t" << std::hex << pDevice << '\n';
			std::cout << " Present() addrs --->\t" << std::hex << presentAddress << '\n';
			std::cout << " EndScene() addrs --->\t" << std::hex << endSceneAddress << '\n';

			//pDevice->Release(); // release the IDirect3DDevice9* interface pDevice created to prevent memory leaks
			return endSceneAddress;
		}
		
		//pD3D->Release(); // release the IDirect3D9* interface object pD3D created to prevent memory leaks
	}
	return nullptr;
}

DWORD WINAPI MyThreadFunction( HMODULE hinstDLL )
{
	//Create a console 
	AllocConsole();
	FILE* f;
	freopen_s(&f,"CONOUT$","w",stdout);

		// Find the  game window
	HWND game_window = FindWindowA( NULL, "Counter-Strike: Global Offensive - Direct3D 9" );
	if (!game_window)
	{
		std::cout << " Game-Window HWND find failed. Error: " << GetLastError() << '\n';
	}
	else
	{
		// If Game-window is found we will Find the Endscene address
		getWindowSize( game_window);

		// If Game-window is found we will Find the Endscene address
		EndScenePtr = (aliasEndscene) FindEndScene( game_window );
	}
	// and Type-cast it to Endscene function pointer type-alias to EndScenePtr variable
		// This will assign the Real EndScene address to EndScene Pointer
	// Now we will use the EndScenePtr holding value of real endscene function to create a Trampoline hook to our function 
	
	uintptr_t lpOriginalAddress = (uintptr_t) EndScenePtr; // src original function address saved here

	//Hook endscene with our templated hook class
	// 7 is the length of stolen bytes
	EndScenePtr = (aliasEndscene) hNP.trampHook<7>( (char*) lpOriginalAddress, (char*) &hookEndScene );


	while (!GetAsyncKeyState( VK_END ) & 1)
	{
		Sleep( 10 );
	}
	//UnHook
	hNP.patchByte<7>( (char*)lpOriginalAddress );
	FreeLibraryAndExitThread( hinstDLL, 0 );
	CloseHandle( hinstDLL );
	return 0;
}

BOOL WINAPI DllMain( HMODULE hinst__DLL, DWORD fdwReason, LPVOID lpvReserved )
{

	//DWORD ModuleBase = (DWORD)hinst__DLL;
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH: // A process is loading the DLL.
		
		::DisableThreadLibraryCalls( hinst__DLL ); // disable unwanted thread notifications to reduce overhead
		
		CreateThread( nullptr, 0, (LPTHREAD_START_ROUTINE) MyThreadFunction, hinst__DLL, 0, nullptr ); //init our hooks
		break;

		case DLL_PROCESS_DETACH: // A process unloads the DLL.
		
		break;
	}
	return TRUE;
}