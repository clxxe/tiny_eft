#pragma once
#include <cstdint>
#include <intrin.h>

#define sapphire_func( mod, type ) reinterpret_cast< decltype( &type ) >( chloe::get_exported_function( chloe::get_sapphire_module( ##mod ), #type ) )
#define sapphire_type( mod, type ) reinterpret_cast< type >( chloe::get_exported_function( chloe::get_sapphire_module( ##mod ), #type ) );

namespace chloe {
#define contains_record( address, type, field ) ( ( type* )( ( char* )( address ) - ( std::uintptr_t )( & ( (type* ) 0 ) -> field ) ) )
#define loword(l) ((std::uint16_t)(((std::uintptr_t)(l)) & 0xffff))

	struct module_t {
		void* base{ };
		const wchar_t* name{ };
		std::uint32_t size{ };
		module_t( void* b, const wchar_t* n, std::uint32_t s ) : base( b ), name( n ), size( s ) {}
		module_t( ) : base( nullptr ), name( 0 ), size( 0 ) {}
	};

	template<typename type_t = char>
	inline bool compare( const type_t* a, const type_t* b )
	{
		if ( !a || !b )
			return !a && !b;

		int ret = 0;
		type_t* p1 = ( type_t* ) a;
		type_t* p2 = ( type_t* ) b;
		while ( !( ret = *p1 - *p2 ) && *p2 )
			++p1, ++p2;

		return ret == 0;
	}

	inline std::uint32_t wcslen( const wchar_t* str )
	{
		auto counter{ 0 };
		if ( !str )
			return 0;
		for ( ; *str != '\0'; ++str )
			++counter;

		return counter;
	}

	namespace nt {
		const struct list_entry
		{
			struct list_entry* Flink;
			struct list_entry* Blink;
		};

		const struct unicode_string
		{
			std::uint16_t Length;
			std::uint16_t MaximumLength;
			wchar_t* Buffer;
		};

		const struct peb_ldr_data
		{
			std::uint32_t Length;
			std::uint32_t Initialized;
			const char* SsHandle;
			list_entry InLoadOrderModuleList;
			list_entry InMemoryOrderModuleList;
			list_entry InInitializationOrderModuleList;
		};

		const struct peb
		{
			std::uint8_t Reserved1[ 2 ];
			std::uint8_t BeingDebugged;
			std::uint8_t Reserved2[ 1 ];
			const char* Reserved3[ 2 ];
			peb_ldr_data* Ldr;
		};

		const struct ldr_data_table_entry
		{
			list_entry InLoadOrderModuleList;
			list_entry InMemoryOrderLinks;
			list_entry InInitializationOrderModuleList;
			void* DllBase;
			void* EntryPoint;
			union {
				std::uint32_t SizeOfImage;
				const char* _dummy;
			};
			unicode_string FullDllName;
			unicode_string BaseDllName;
		};

		const struct image_dos_header
		{
			std::uint16_t e_magic;
			std::uint16_t e_cblp;
			std::uint16_t e_cp;
			std::uint16_t e_crlc;
			std::uint16_t e_cparhdr;
			std::uint16_t e_minalloc;
			std::uint16_t e_maxalloc;
			std::uint16_t e_ss;
			std::uint16_t e_sp;
			std::uint16_t e_csum;
			std::uint16_t e_ip;
			std::uint16_t e_cs;
			std::uint16_t e_lfarlc;
			std::uint16_t e_ovno;
			std::uint16_t e_res[ 4 ];
			std::uint16_t e_oemid;
			std::uint16_t e_oeminfo;
			std::uint16_t e_res2[ 10 ];
			long e_lfanew;
		};

		const struct image_file_header
		{
			std::uint16_t Machine;
			std::uint16_t NumberOfSections;
			std::uint32_t TimeDateStamp;
			std::uint32_t PointerToSymbolTable;
			std::uint32_t NumberOfSymbols;
			std::uint16_t SizeOfOptionalHeader;
			std::uint16_t Characteristics;
		};

		const struct image_export_directory
		{
			std::uint32_t Characteristics;
			std::uint32_t TimeDateStamp;
			std::uint16_t MajorVersion;
			std::uint16_t MinorVersion;
			std::uint32_t Name;
			std::uint32_t Base;
			std::uint32_t NumberOfFunctions;
			std::uint32_t NumberOfNames;
			std::uint32_t AddressOfFunctions;
			std::uint32_t AddressOfNames;
			std::uint32_t AddressOfNameOrdinals;
		};

		const struct image_data_directory
		{
			std::uint32_t VirtualAddress;
			std::uint32_t Size;
		};

		const struct image_optional_header
		{
			std::uint16_t Magic;
			std::uint8_t MajorLinkerVersion;
			std::uint8_t MinorLinkerVersion;
			std::uint32_t SizeOfCode;
			std::uint32_t SizeOfInitializedData;
			std::uint32_t SizeOfUninitializedData;
			std::uint32_t AddressOfEntryPoint;
			std::uint32_t BaseOfCode;
			std::uint64_t ImageBase;
			std::uint32_t SectionAlignment;
			std::uint32_t FileAlignment;
			std::uint16_t MajorOperatingSystemVersion;
			std::uint16_t MinorOperatingSystemVersion;
			std::uint16_t MajorImageVersion;
			std::uint16_t MinorImageVersion;
			std::uint16_t MajorSubsystemVersion;
			std::uint16_t MinorSubsystemVersion;
			std::uint32_t Win32VersionValue;
			std::uint32_t SizeOfImage;
			std::uint32_t SizeOfHeaders;
			std::uint32_t CheckSum;
			std::uint16_t Subsystem;
			std::uint16_t DllCharacteristics;
			std::uint64_t SizeOfStackReserve;
			std::uint64_t SizeOfStackCommit;
			std::uint64_t SizeOfHeapReserve;
			std::uint64_t SizeOfHeapCommit;
			std::uint32_t LoaderFlags;
			std::uint32_t NumberOfRvaAndSizes;
			image_data_directory DataDirectory[ 16 ];
		};

		const struct image_nt_headers {
			std::uint32_t Signature;
			image_file_header FileHeader;
			image_optional_header OptionalHeader;
		};
	}

	inline auto get_sapphire_module( const wchar_t* name ) -> const chloe::module_t {
		const auto peb = reinterpret_cast< chloe::nt::peb* >( __readgsqword( 0x60 ) );
		if ( !peb )
			return {};

		const auto head = peb->Ldr->InMemoryOrderModuleList;

		for ( auto curr = head; curr.Flink != &peb->Ldr->InMemoryOrderModuleList; curr = *curr.Flink ) {
			chloe::nt::ldr_data_table_entry* mod = reinterpret_cast< chloe::nt::ldr_data_table_entry* >( contains_record( curr.Flink, chloe::nt::ldr_data_table_entry, InMemoryOrderLinks ) );

			if ( mod->BaseDllName.Buffer )
				if ( chloe::compare( mod->BaseDllName.Buffer, name ) )
					return chloe::module_t{ mod->DllBase, mod->BaseDllName.Buffer, mod->SizeOfImage };
		}

		return {};
	}

	inline const void* get_exported_function( const chloe::module_t module, const char* function ) {
		if ( !module.base )
			return {};

		const auto base = reinterpret_cast< std::uintptr_t >( module.base );
		const auto dos_header = reinterpret_cast< chloe::nt::image_dos_header* >( base );
		const auto nt_header = reinterpret_cast< chloe::nt::image_nt_headers* >( base + dos_header->e_lfanew );

		const auto data_directory = reinterpret_cast< const chloe::nt::image_data_directory* >( &nt_header->OptionalHeader.DataDirectory[ 0 ] );
		const auto image_export_directory = reinterpret_cast< chloe::nt::image_export_directory* >( base + data_directory->VirtualAddress );

		const auto rva_table = reinterpret_cast< const std::uintptr_t* >( base + image_export_directory->AddressOfFunctions );
		const auto ord_table = reinterpret_cast< const std::uint16_t* >( base + image_export_directory->AddressOfNameOrdinals );

		if ( image_export_directory ) {
			for ( std::uint32_t idx{ 0 }; idx < image_export_directory->NumberOfNames; idx++ ) {
				const auto fn_name = reinterpret_cast< const char* >( base + reinterpret_cast< const unsigned long* >( base + image_export_directory->AddressOfNames )[ idx ] );
				if ( chloe::compare( fn_name, function ) )
					return reinterpret_cast< void* >( base + ( ( std::uint32_t* ) ( base + image_export_directory->AddressOfFunctions ) )[ ord_table[ idx ] ] );
			}
		}

		return { };
	}
}
