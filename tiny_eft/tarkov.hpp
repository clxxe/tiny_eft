#pragma once
#include "mono.hpp"

#define clomp_assert( x ) if( !x ) return nullptr; // lol really saving lines out here.
#define clomp_min( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )

namespace eft {
	inline void* unity_base;

	const struct vec3_t {
		float x, y, z;
		vec3_t( ) : x( 0 ), y( 0 ), z( 0 ) { }
		vec3_t( float x, float y, float z ) : x( x ), y( y ), z( z ) { }
		vec3_t operator+( const vec3_t& vector ) const { return vec3_t( x + vector.x, y + vector.y, z + vector.z ); }
		auto empty( ) -> bool { return x == 0 && y == 0 && z == 0; }
	};

	template<typename type_t>
	class list {
	private:
		char pad_0x10[ 0x10 ];
		std::uintptr_t list_base;
		std::uint32_t count;
	public:
		int get_count( ) {
			if ( !this || !list_base )
				return 0;

			return int( this->count );
		}

		type_t get( std::uint32_t index ) {
			if ( !this || !list_base )
				return type_t( );
			return *reinterpret_cast< type_t* >( this->list_base + 0x20 + index * 0x8 );
		}
	};

	const class string
	{
	public:
		char zpad[ 0x10 ]{ };
		int size{ };
		wchar_t buffer[ 128 + 1 ];
	public:
		string( const wchar_t* st )
		{
			size = clomp_min( chloe::wcslen( st ), 128 );
			for ( size_t idx{ 0 }; idx < size; idx++ )
				buffer[ idx ] = st[ idx ];

			buffer[ size ] = 0;
		}
	};

	const class world {
	public:
		static auto find( ) -> world* {
			mono_method( find_go, std::uintptr_t( * )( eft::string ), "UnityEngine.CoreModule", "UnityEngine", "GameObject", "Find", 1 );
			const auto gw = find_go( L"GameWorld" );
			clomp_assert( gw );
			const auto w = *reinterpret_cast< std::uintptr_t* >( gw + 0x10 );
			clomp_assert( w );
			const auto lgw = *reinterpret_cast< std::uintptr_t* >( w + 0x30 );
			clomp_assert( lgw );
			const auto lgw2 = *reinterpret_cast< std::uintptr_t* >( lgw + 0x18 );
			clomp_assert( lgw2 );

			return *reinterpret_cast< world** >( lgw2 + 0x28 );
		}

		auto players( ) -> eft::list< class player* >* {
			mono_field( o_registered_players, "Assembly-CSharp", "EFT", "GameWorld", "RegisteredPlayers" );
			return *reinterpret_cast< eft::list< class player* >** >( this + o_registered_players );
		}
	};

	const class player {
	public:
		auto bone_transform( std::uint32_t idx ) -> class transform* {
			mono_field( o_player_body, "Assembly-CSharp", "EFT", "Player", "_playerBody" );
			mono_field( o_root_joint, "Assembly-CSharp", "EFT", "PlayerBody", "SkeletonRootJoint" );
			mono_field( o_values, "Assembly-CSharp", "Diz.Skinning", "Skeleton", "_values" );

			const auto body{ *reinterpret_cast< std::uintptr_t* >( this + o_player_body ) };
			clomp_assert( body );
			const auto skele{ *reinterpret_cast< std::uintptr_t* >( body + o_root_joint ) };
			clomp_assert( skele );
			const auto values{ *reinterpret_cast< eft::list< class transform* >** >( skele + o_values ) };
			clomp_assert( values );

			return values->get( idx );
		}

		auto name( ) -> eft::string* {
			mono_method( get_profile, std::uintptr_t( * )( void* ), "Assembly-CSharp", "EFT", "Player", "get_Profile", 0 );
			const auto profile = get_profile( this );
			clomp_assert( profile );
			const auto info{ *reinterpret_cast< std::uintptr_t* >( profile + 0x28 ) };
			clomp_assert( info );
			const auto name{ *reinterpret_cast< std::uintptr_t* >( info + 0x10 ) };
			clomp_assert( name );

			return reinterpret_cast< eft::string* >( name );
		}
	};

	const class camera {
	public:
		static auto find( ) -> camera*
		{
			if ( !unity_base )
				unity_base = chloe::get_sapphire_module( L"UnityPlayer.dll" ).base;
			const auto get_main_camera{ reinterpret_cast< camera * ( __fastcall* )( ) >( std::uintptr_t( unity_base ) + 0x92EF60 ) };
			return get_main_camera( );
		}

		auto world_to_screen_point( eft::vec3_t position ) -> eft::vec3_t {
			eft::vec3_t out;
			if ( !unity_base )
				unity_base = chloe::get_sapphire_module( L"UnityPlayer.dll" ).base;
			
			const auto world_to_screen_point{ reinterpret_cast< void( __fastcall* )( void*, eft::vec3_t, int, eft::vec3_t& ) >( std::uintptr_t( unity_base ) + 0x92C5A0 ) };
			world_to_screen_point( this, position, 2, out );
			mono_method( get_height, int ( * )( ), "UnityEngine.CoreModule", "UnityEngine", "Screen", "get_height", 0 );

			out.y = get_height( ) - out.y;
			if ( out.z < 0.01f )
				return { -100 , -100 , -100 };

			return out;
		}
	};

	const class transform {
	public:
		auto position( ) -> eft::vec3_t {
			eft::vec3_t out;
			if ( !unity_base )
				unity_base = chloe::get_sapphire_module( L"UnityPlayer.dll" ).base;
			const auto get_position{ reinterpret_cast< void( __fastcall* )( void*, eft::vec3_t& ) >( std::uintptr_t( unity_base ) + 0x998DA0 ) };
			get_position( this, out );
			return out;
		}
	};
}