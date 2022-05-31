#include "render.hpp"
#include "tarkov.hpp"
// i got tha devil on ma phone he tell me keep doin this shit.
extern "C" void _fltused( ) { __ud2( ); }
extern "C" void __chkstk( ) { }
extern "C" void __CxxFrameHandler4( ) { __ud2( ); }

#pragma intrinsic(memset) // weird fix to compile bug in release
extern void* __cdecl memset( void*, int, size_t );

#pragma function(memset)
void* memset( void* dest, int c, size_t count ) {
	char* b = ( char* ) dest;
	while ( count-- )
		*b++ = ( char ) c;

	return dest;
}

auto hk_hit_camera_frame( void* _rcx [[ maybe_unused ]] ) {

	if ( !r::setup( ) )
		return;

	r::text( 10, 10, L"666 line internal.", { 255, 0, 0, 255 } );

	auto world = eft::world::find( );
	auto cam = eft::camera::find( );

	if ( !world || !cam )
		return;

	auto players{ world->players( ) };
	if ( !players )
		return;

	auto ct{ players->get_count( ) };
	if ( ct <= 0 )
		return;

	for ( auto idx{ 0 }; idx < ct; idx++ ) {
		auto player{ players->get( idx ) };
		if ( !player )
			continue;

		auto head{ player->bone_transform( 133 )->position( ) };
		auto base{ player->bone_transform( 0 )->position( ) };

		auto w2s_head{ cam->world_to_screen_point( head ) };
		auto w2s_half{ cam->world_to_screen_point( base + eft::vec3_t{ 0, 1.9f, 0 } ) };
		auto w2s_base{ cam->world_to_screen_point( base ) };

		if ( w2s_head.empty( ) || w2s_base.empty( ) || w2s_half.empty( ) )
			return;

		const float box_height{ ( w2s_base.y - w2s_head.y ) * 1.20f };
		const float box_width{ ( w2s_base.y - w2s_half.y ) * 0.45f };

		const float box_x{ w2s_base.x - box_width * 0.50f };
		const float box_y{ w2s_base.y - box_height };

		const auto name = player->name( );

		r::rect( box_x, box_y, box_width, box_height, { 255, 0, 0, 255 } );
		r::text( box_x + box_width + 2.f, box_y + 3.f, name->buffer, { 255, 255, 255, 255 } );
	}
}

auto DllMain( void* module_instance [[ maybe_unused ]], std::uint32_t call_reason, void* reserved [[ maybe_unused ]] ) -> bool {
	if ( call_reason != 1 )
		return false;

	mono::init( );

	auto place_hk = [ ]( void* tar, void* ours ) {
		auto _ptr = reinterpret_cast< unsigned char* >( tar );

		_ptr[ 0 ] = 0x48;
		_ptr[ 1 ] = 0xB8;
		*reinterpret_cast< void** >( _ptr + 2 ) = ours;
		_ptr[ 10 ] = 0xFF;
		_ptr[ 11 ] = 0xE0;

		return true;
	};

	mono_method( dont_destroy_on_load,  void( * )( std::uintptr_t ), "UnityEngine.CoreModule", "UnityEngine", "Object", "DontDestroyOnLoad", 1 );
	mono_method( create_game_object,    void( * )( std::uintptr_t, eft::string ), "UnityEngine.CoreModule", "UnityEngine", "GameObject", "Internal_CreateGameObject", 2 );
	mono_method( add_component,		    void( * )( std::uintptr_t, std::uintptr_t ), "UnityEngine.CoreModule", "UnityEngine", "GameObject", "AddComponent", 1 );
	auto game_object{ mono::object_new( mono::get_root_domain( ), mono::find_class( "UnityEngine.CoreModule", "UnityEngine", "GameObject" ) ) };
	
	create_game_object( game_object, L"" );
	add_component( game_object, mono::type_object( "Assembly-CSharp", "", "HitCameraShakerTest" ) );
	dont_destroy_on_load( game_object );

	mono_method( hit_cam_ongui, void*, "Assembly-CSharp", "", "HitCameraShakerTest", "OnGUI", 0 );
	place_hk( hit_cam_ongui, &hk_hit_camera_frame );

	return true;
}