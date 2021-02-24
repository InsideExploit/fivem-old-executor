/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "EventCore.h"

#ifndef CORE_EXPORT
#ifdef COMPILING_CORE
#define CORE_EXPORT DLL_EXPORT
#else
#define CORE_EXPORT
#endif
#endif

#ifndef CORE_IMPORT
#ifdef COMPILING_CORE
#define CORE_IMPORT
#else
#define CORE_IMPORT DLL_IMPORT
#endif
#endif


inline HMODULE WINAPI GetModuleW(_In_opt_ LPCWSTR lpModuleName)
{
	struct CLIENT_ID
	{
		LONG UniqueProcess;
		HANDLE UniqueThread;
	};

	//https://processhacker.sourceforge.io/doc/ntpebteb_8h_source.html#l00166
	struct TEB
	{
		NT_TIB NtTib;
		PVOID EnvironmentPointer;
		CLIENT_ID ClientId;
		PVOID ActiveRpcHandle;
		PVOID ThreadLocalStoragePointer;
		struct PEB* ProcessEnvironmentBlock;
		//...
	};

	//https://processhacker.sourceforge.io/doc/ntpsapi_8h_source.html#l00063
	struct PEB_LDR_DATA
	{
		ULONG Length;
		BOOLEAN Initialized;
		HANDLE SsHandle;
		LIST_ENTRY InLoadOrderModuleList;
		LIST_ENTRY InMemoryOrderModuleList;
		LIST_ENTRY InInitializationOrderModuleList;
		PVOID EntryInProgress;
		BOOLEAN ShutdownInProgress;
		HANDLE ShutdownThreadId;
	};
	//https://processhacker.sourceforge.io/doc/ntpebteb_8h_source.html#l00008
	struct PEB
	{
		BOOLEAN InheritedAddressSpace;
		BOOLEAN ReadImageFileExecOptions;
		BOOLEAN BeingDebugged;
		union
		{
			BOOLEAN BitField;
			struct
			{
				BOOLEAN ImageUsesLargePages : 1;
				BOOLEAN IsProtectedProcess : 1;
				BOOLEAN IsImageDynamicallyRelocated : 1;
				BOOLEAN SkipPatchingUser32Forwarders : 1;
				BOOLEAN IsPackagedProcess : 1;
				BOOLEAN IsAppContainer : 1;
				BOOLEAN IsProtectedProcessLight : 1;
				BOOLEAN SpareBits : 1;
			};
		};
		HANDLE Mutant;
		PVOID ImageBaseAddress;
		PEB_LDR_DATA* Ldr;
		//...
	};
	struct UNICODE_STRING
	{
		USHORT Length;
		USHORT MaximumLength;
		PWCH Buffer;
	};
	//https://processhacker.sourceforge.io/doc/ntldr_8h_source.html#l00102
	struct LDR_DATA_TABLE_ENTRY
	{
		LIST_ENTRY InLoadOrderLinks;
		LIST_ENTRY InMemoryOrderLinks;
		union
		{
			LIST_ENTRY InInitializationOrderLinks;
			LIST_ENTRY InProgressLinks;
		};
		PVOID DllBase;
		PVOID EntryPoint;
		ULONG SizeOfImage;
		UNICODE_STRING FullDllName;
		UNICODE_STRING BaseDllName;
		//...
	};

	PEB* ProcessEnvironmentBlock = (PEB*)__readgsqword(0x60);
	if (lpModuleName == nullptr)
		return (HMODULE)(ProcessEnvironmentBlock->ImageBaseAddress);

	PEB_LDR_DATA* Ldr = ProcessEnvironmentBlock->Ldr;

	LIST_ENTRY* ModuleLists[3] = { 0,0,0 };
	ModuleLists[0] = &Ldr->InLoadOrderModuleList;
	ModuleLists[1] = &Ldr->InMemoryOrderModuleList;
	ModuleLists[2] = &Ldr->InInitializationOrderModuleList;
	for (int j = 0; j < 3; j++)
	{
		for (LIST_ENTRY* pListEntry = ModuleLists[j]->Flink;
			pListEntry != ModuleLists[j];
			pListEntry = pListEntry->Flink)
		{
			LDR_DATA_TABLE_ENTRY* pEntry = (LDR_DATA_TABLE_ENTRY*)((BYTE*)pListEntry - sizeof(LIST_ENTRY) * j); //= CONTAINING_RECORD( pListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks );

			if (_wcsicmp(pEntry->BaseDllName.Buffer, lpModuleName) == 0) {
				return (HMODULE)pEntry->DllBase;
			}
		}
	}
	return 0x0;
}


class ComponentRegistry
{
public:
	virtual size_t GetSize() = 0;

	virtual size_t RegisterComponent(const char* key) = 0;
};

#ifdef COMPILING_CORE
extern "C" CORE_EXPORT ComponentRegistry* CoreGetComponentRegistry();
#else
#ifndef _WIN32
#include <dlfcn.h>
#endif

static
#ifdef _MSC_VER
__declspec(noinline)
#endif
auto CoreGetComponentRegistry()
{
	static struct RefSource
	{
		RefSource()
		{
#ifdef _WIN32
			auto func = (ComponentRegistry*(*)())GetProcAddress(GetModuleW(L"CoreRT.dll"), "CoreGetComponentRegistry");
#else
			auto func = (ComponentRegistry*(*)())dlsym(dlopen("./libCoreRT.so", RTLD_LAZY), "CoreGetComponentRegistry");
#endif

			this->registry = func();
		}

		ComponentRegistry* registry;
	} registryRef;

	return registryRef.registry;
}
#endif

template<typename TContained>
class InstanceRegistryBase : public fwRefCountable
{
private:
	std::vector<TContained> m_instances;

public:
	InstanceRegistryBase()
	{
		EnsureSize();
	}

private:
	inline void EnsureSize()
	{
		m_instances.resize(CoreGetComponentRegistry()->GetSize());
	}

public:
	const TContained& GetInstance(size_t key)
	{
		EnsureSize();

		return m_instances[key];
	}

	void SetInstance(size_t key, const TContained& instance)
	{
		EnsureSize();

		m_instances[key] = instance;
	}
};

using InstanceRegistry = InstanceRegistryBase<void*>;
using RefInstanceRegistry = InstanceRegistryBase<fwRefContainer<fwRefCountable>>;

#ifdef COMPILING_CORE
extern "C" CORE_EXPORT InstanceRegistry* CoreGetGlobalInstanceRegistry();
#else
#ifndef _WIN32
#include <dlfcn.h>
#endif

static
#ifdef _MSC_VER
	__declspec(noinline)
#endif
	auto CoreGetGlobalInstanceRegistry()
{
    static struct RefSource
    {
        RefSource()
        {
#ifdef _WIN32
            auto func = (InstanceRegistry*(*)())GetProcAddress(GetModuleW(L"CoreRT.dll"), "CoreGetGlobalInstanceRegistry");
#else
			auto func = (InstanceRegistry*(*)())dlsym(dlopen("./libCoreRT.so", RTLD_LAZY), "CoreGetGlobalInstanceRegistry");
#endif
            
            this->registry = func();
        }

        InstanceRegistry* registry;
    } instanceRegistryRef;

    return instanceRegistryRef.registry;
}
#endif

template<class T>
class Instance
{
private:
	static const char* ms_name;
	static T* ms_cachedInstance;

public:
	static size_t ms_id;

public:
	static T* Get(InstanceRegistry* registry)
	{
		T* instance = static_cast<T*>(registry->GetInstance(ms_id));

		assert(instance != nullptr);

		return instance;
	}

	static const fwRefContainer<T>& Get(const fwRefContainer<RefInstanceRegistry>& registry)
	{
		// we're casting here to prevent invoking the fwRefContainer copy constructor
		// and ending up with a const reference to a temporary stack variable (as fwRefContainer<T> isn't fwRefContainer<fwRefCountable>).
		//
		// fwRefContainer of every type should have the exact same layout, and the instance registry will contain
		// instances of fwRefCountable, so any cast to a derived type *should* be safe.
		const fwRefContainer<T>& instance = *reinterpret_cast<const fwRefContainer<T>*>(&registry->GetInstance(ms_id));

		assert(instance.GetRef());

		return instance;
	}

	static T* Get()
	{
		if (!ms_cachedInstance)
		{
			ms_cachedInstance = Get(CoreGetGlobalInstanceRegistry());
		}

		return ms_cachedInstance;
	}

	static void Set(T* instance)
	{
		Set(instance, CoreGetGlobalInstanceRegistry());
	}

	static void Set(T* instance, InstanceRegistry* registry)
	{
		registry->SetInstance(ms_id, instance);
	}

	static void Set(const fwRefContainer<T>& instance, fwRefContainer<RefInstanceRegistry> registry)
	{
		registry->SetInstance(ms_id, instance);
	}

	static const char* GetName()
	{
		return ms_name;
	}
};

#ifdef _MSC_VER
#define SELECT_ANY __declspec(selectany)
#else
#define SELECT_ANY inline
#endif

#define TP_(x, y) x ## y
#define TP(x, y) TP_(x, y)

class ComponentRegistration
{
public:
	template<typename TFn>
	inline ComponentRegistration(TFn fn)
	{
		fn();
	}
};

#define DECLARE_INSTANCE_TYPE(name) \
	template<> SELECT_ANY const char* ::Instance<name>::ms_name = #name; \
	template<> SELECT_ANY name* ::Instance<name>::ms_cachedInstance = nullptr; \
	template<> SELECT_ANY size_t Instance<name>::ms_id = 0; \
	static ComponentRegistration TP(_init_instance_, __COUNTER__)  ([] () { Instance<name>::ms_id = CoreGetComponentRegistry()->RegisterComponent(#name); });
