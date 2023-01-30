#ifndef MEMEDIT_H
#define MEMEDIT_H
#include"Windows.h"
#include<cstdint>

// Little memory editing class from Uc

int filter( unsigned int code, struct _EXECPTION_POINTERS* ep )
{
	//handle only AV
	return code == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;

}

class MemEdit
{
public:

	template<typename T>
	T read( uintptr_t Address, const T& def_val = T() )
	{
		__try
		{
			return *(T*) (Address);
		}
		__except (filter( GetExceptionCode(), GetExceptionInformation() ))
		{
			return def_val;
		}


	}

	template<typename T>
	bool  write( uintptr_t Address, T Value )
	{
		__try
		{
			 *(T*) (Address)=Value;
		}
		__except (filter( GetExceptionCode(), GetExceptionInformation() ))
		{
			return false;
		}
		return true;
	}

	template<typename T>
	bool add( uintptr_t Address, T Value )
	{
		__try
		{
			*(T*) Address += Value;
		}
		__except (filter( GetExceptionCode(), GetExceptionInformation() ))
		{
			return false;
		}
		return true;
	}

	template <typename T>
	bool sub( uintptr_t Address, T Value )
	{
		__try
		{
			*(T*) Address -= Value;
		}
		__except (filter( GetExceptionCode(), GetExceptionInformation() ))
		{
			return false;
		}
		return true;
	}

	template <typename T>
	bool multiply( uintptr_t Address, T Value )
	{
		__try
		{
			*(T*) Address *= Value;
		}
		__except (filter( GetExceptionCode(), GetExceptionInformation() ))
		{
			return false;
		}
		return true;
	}

	template <class T>
	bool divide( uintptr_t Address, T Value )
	{
		__try
		{
			*(T*) Address /= Value;
		}
		__except (filter( GetExceptionCode(), GetExceptionInformation() ))
		{
			return false;
		}
		return true;
	}

	template <typename T>
	T* pointMemory( LPVOID addrs )
	{
		__try
		{
			return ((T*) addrs);
		}
		__except( filter( GetExceptionCode(), GetExceptionInformation() ) )
		{
			return def_val;
		}

	}

	uintptr_t BaseAddress()
	{
		return (uintptr_t)::GetModuleHandle( NULL );
	}
};

// MemEdit memedit;
//Reading memmory:
//int hue= memedit.read<int>(0x1337,1);
//where "0x1337" is the address and "1" is the default value
//Writing memory:
//memedit.write<float>( 0x1337, 0.1f );
//memedit.add<int>( 0x1337, 2 );
#endif