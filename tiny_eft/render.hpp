#pragma once
#include "tarkov.hpp"

namespace r {
	struct clr_t {
		float r, g, b, a;
		clr_t( float r, float g, float b, float a ) : r( r / 255 ), g( g / 255 ), b( b / 255 ), a( a / 255 ) { }
		clr_t( ) : r( 1 ), g( 1 ), b( 1 ), a( 1 ) { }
	};

	struct rect_t {
		float x, y, w, h;
		rect_t( float _x, float _y, float _w, float _h ) : x( _x ), y( _y ), w( _w ), h( _h ) { }
		rect_t( ) : x( 0 ), y( 0 ), w( 0 ), h( 0 ) { }
	};

	inline std::uintptr_t white, style;
	inline auto setup( ) -> bool {
		mono_method( get_white_texture, std::uintptr_t( * )( ), "UnityEngine.CoreModule", "UnityEngine", "Texture2D", "get_whiteTexture", 0 );
		mono_method( get_skin, std::uintptr_t( * )( ), "UnityEngine.IMGUIModule", "UnityEngine", "GUI", "get_skin", 0 );

		white = get_white_texture( );
		if ( !white )
			return false;

		auto gui_skin = get_skin( );
		if ( !gui_skin )
			return false;

		style = *reinterpret_cast< std::uintptr_t* >( gui_skin + 0x38 );
		if ( !style )
			return false;

		return true;
	}

	inline auto text( float x, float y, eft::string text, clr_t clr ) {
		if ( !style )
			return;

		mono_method( create_temp_content_fn, std::uintptr_t( * )( eft::string* ), "UnityEngine.IMGUIModule", "UnityEngine", "GUIContent", "Temp", 1 );
		mono_method( cl, void( * )( clr_t ), "UnityEngine.IMGUIModule", "UnityEngine", "GUI", "set_color", 1 );
		mono_method_a( label, void( * )( rect_t, std::uintptr_t, std::uintptr_t ), "UnityEngine.IMGUIModule", "UnityEngine", "GUI", "Label", 3, "content" );

		const auto content = create_temp_content_fn( &text );

		cl( clr );
		label( { x, y, 500, 25 }, content, style );
	}

	inline auto rect( float x, float y, float w, float h, clr_t clr ) {
		if ( !white )
			return;

		mono_method( cl, void( * )( clr_t ), "UnityEngine.IMGUIModule", "UnityEngine", "GUI", "set_color", 1 );
		mono_method( dt, void ( * )( rect_t, std::uintptr_t ), "UnityEngine.IMGUIModule", "UnityEngine", "GUI", "DrawTexture", 2 );

		cl( clr );
		dt( { x, y, 1, h }, white );
		dt( { x + w, y + 1, 1, h }, white );
		dt( { x, y, w + 1, 1 }, white );
		dt( { x, y + h, w, 1 }, white );
	}
}