#pragma once
#include "sapphire.hpp"
#define create_type( name, args ) using mono_##name = args; inline mono_##name name;
#define assign_type( name ) name = sapphire_type( L"mono-2.0-bdwgc.dll", mono_##name );
#define mono_method( fn, args, ... ) using t_##fn = args; static t_##fn fn{ 0 }; if(!fn) fn = reinterpret_cast< t_##fn >( mono::method( ##__VA_ARGS__ ) )
#define mono_method_a( fn, args, ... ) using t_##fn = args; static t_##fn fn{ 0 }; if(!fn) fn = reinterpret_cast< t_##fn >( mono::arg_method( ##__VA_ARGS__ ) )
#define mono_field( fn, ... ) std::uintptr_t fn{ 0 }; if(!fn) fn = mono::field( ##__VA_ARGS__ );

namespace mono
{
	create_type( get_root_domain, std::uintptr_t( * )( ) );
	create_type( assembly_get_image, std::uintptr_t( * )( std::uintptr_t ) );
	create_type( class_get_methods, std::uintptr_t( * )( std::uintptr_t, std::uintptr_t* ) );
	create_type( method_signature, std::uintptr_t( * )( std::uintptr_t ) );
	create_type( signature_get_param_count, int( * )( std::uintptr_t ) );
	create_type( method_get_param_names, void( * )( std::uintptr_t, const char** ) );
	create_type( object_new, std::uintptr_t( * )( std::uintptr_t, std::uintptr_t ) );
	create_type( class_from_name, std::uintptr_t( * )( std::uintptr_t, const char*, const char* ) );
	create_type( field_get_offset, std::uintptr_t( * )( std::uintptr_t ) );
	create_type( class_get_type, std::uintptr_t( * )( std::uintptr_t ) );
	create_type( type_get_object, std::uintptr_t( * )( std::uintptr_t, std::uintptr_t ) );
	create_type( compile_method, std::uintptr_t( * )( std::uintptr_t ) );
	create_type( domain_assembly_open, std::uintptr_t( * )( std::uintptr_t, const char* ) );
	create_type( class_get_method_from_name, std::uintptr_t( * )( std::uintptr_t, const char* name, int ) );
	create_type( class_get_field_from_name, std::uintptr_t( * )( std::uintptr_t, const char* name ) );
	create_type( thread_attach, void( * )( std::uintptr_t ) );

	inline void init( ) {
		assign_type( get_root_domain );
		assign_type( assembly_get_image );
		assign_type( class_get_methods );
		assign_type( method_signature );
		assign_type( signature_get_param_count );
		assign_type( method_get_param_names );
		assign_type( object_new );
		assign_type( class_from_name );
		assign_type( field_get_offset );
		assign_type( class_get_type );
		assign_type( type_get_object );
		assign_type( compile_method );
		assign_type( class_get_method_from_name );
		assign_type( class_get_field_from_name );
		assign_type( thread_attach );

		thread_attach( get_root_domain( ) );
	}

	inline auto find_class( const char* assembly_name, const char* name_space, const char* klass_name ) -> const std::uintptr_t {
		const auto domain = get_root_domain( );

		const auto assembly = domain_assembly_open( domain, assembly_name );
		if ( !assembly )
			return { };

		const auto img = assembly_get_image( std::uintptr_t( assembly ) );
		if ( !img )
			return { };

		const auto klass = class_from_name( img, name_space, klass_name );
		if ( !klass )
			return { };

		return klass;
	}

	inline auto type_object( const char* assembly_name, const char* name_space, const char* name ) -> const std::uintptr_t {
		auto klass = find_class( assembly_name, name_space, name );
		return type_get_object( get_root_domain( ), class_get_type( klass ) );
	}

	inline auto field( const char* assembly_name, const char* ns, const char* klass_name, const char* name ) -> const std::uintptr_t {
		const auto klass = find_class( assembly_name, ns, klass_name );
		if ( !klass )
			return{ };

		const auto field = class_get_field_from_name( klass, name );
		if ( !field )
			return { };

		return field_get_offset( field );
	}

	inline auto arg_method( const char* assembly_name, const char* ns, const char* klass_name, const char* name, int argument_number, const char* arg_name ) -> const std::uintptr_t {

		std:uintptr_t iter{};
		const auto klass = find_class( assembly_name, ns, klass_name );
		if ( !klass )
			return{ };

		while ( const auto method = class_get_methods( klass, &iter ) ) {
			const auto signature = method_signature( method );
			if ( !signature )
				continue;

			auto param_count = signature_get_param_count( signature );
			if ( !param_count || param_count < argument_number || param_count > argument_number )
				continue;

			const char* _names[ 128 ] = { 0 };
			method_get_param_names( method, _names );

			for ( int i = 0; i < param_count; i++ ) {
				if ( _names[ i ] ) {
					if ( chloe::compare( _names[ i ], arg_name ) )
						return compile_method( method );
				}
			}
		}

		return { };
	}

	inline auto method( const char* assembly_name, const char* ns, const char* klass_name, const char* name, int argument_number ) -> const std::uintptr_t {
		const auto klass = find_class( assembly_name, ns, klass_name );
		if ( !klass )
			return{ };

		const auto method = class_get_method_from_name( klass, name, argument_number );
		if ( !method )
			return { };

		return compile_method( method );
	}
}