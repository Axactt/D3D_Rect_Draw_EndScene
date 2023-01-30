#ifndef HOOKTEMPLATE_H
#define HOOKTEMPLATE_H
#include<Windows.h>

// patch bytes function
class HooknPatch
{
private:
	// This gatway variable stores the original bytes of function
	char* m_gateWay;

public:
	
	template<int LENGTH> // length is expresion parameter here
	void saveOrigBytes( char* lpOriginalFuncAddrs )
	{
		// Do not redefine m_gateWay here otherwise variable shadowing will occur 
		//Only assign so this way the variable m_gateWay can be reused to patch back original bytes later
		m_gateWay = (char*) VirtualAlloc( 0, LENGTH + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );

		RtlMoveMemory( m_gateWay, lpOriginalFuncAddrs, LENGTH );
	}

	//template non-type parameters used for template functions
	// mid-function Detour and Hook
	// usage: hP.midDetour<LENGTH>(lpOriginalFuncAddrs,lpFinalHookaddrs)
	template<int LENGTH>
	bool midDetour( char* lpOriginalFuncAddrs, char* lpFinalHookaddrs )
	{
		if (LENGTH < 5)
			return false;
		DWORD oProc;
		VirtualProtect( lpOriginalFuncAddrs, LENGTH, PAGE_EXECUTE_READWRITE, &oProc );
		RtlFillMemory( lpOriginalFuncAddrs, LENGTH, 0x90 );
		uintptr_t relAddy = (uintptr_t) (lpFinalHookaddrs - lpOriginalFuncAddrs - 5);
		*lpOriginalFuncAddrs = (char) 0xE9;
		*(uintptr_t*) (lpOriginalFuncAddrs + 1) = (uintptr_t) relAddy;
		VirtualProtect( lpOriginalFuncAddrs, LENGTH, oProc, &oProc );
		return true;
	}

	// trampoline hook function for saving register state
	// usage:hP.trampHook<LENGTH>(lpOriginalFuncAddrs,lpFinalHookaddrs)
	//(tEndScene)(hP.trampHook<7>((char*)d3d9Device[42], (char*)hkEndScene));

	template <int LENGTH>
	char* trampHook( char* lpOriginalFuncAddrs, char* lpFinalHookaddrs )
	{
		if (LENGTH < 5)
			return nullptr;
		saveOrigBytes<LENGTH>( lpOriginalFuncAddrs ); // Do not put this is in midDetour function otherwise order of execution will mess up

		uintptr_t jumpAddress = (uintptr_t) (lpOriginalFuncAddrs - m_gateWay - 5);
		*(m_gateWay + LENGTH) = (char) 0xE9;
		*(uintptr_t*) (m_gateWay + LENGTH + 1) = jumpAddress;
		if (midDetour<LENGTH>( lpOriginalFuncAddrs, lpFinalHookaddrs )) // midDeotour another templated member function called here
		{
			return m_gateWay;
		}
		else return nullptr;
	}

	//	Use of Patch or writememory Function: To patch Back original Bytes for Unhook
	// Create class HooknPatch hP ;
	//use hP.patchByte<LENGTH>((char*)lpOriginalFuncAddrs)
	//hP.patchByte<7>((BYTE*)d3d9Device[42], );

	template<int LENGTH>
	void patchByte( char* lpOriginalFuncAddrs )
	{
		DWORD oProc;
		VirtualProtect( lpOriginalFuncAddrs, LENGTH, PAGE_EXECUTE_READWRITE, &oProc );
		RtlMoveMemory( lpOriginalFuncAddrs, m_gateWay, LENGTH );
		VirtualProtect( lpOriginalFuncAddrs, LENGTH, oProc, &oProc );
	}

};


#endif