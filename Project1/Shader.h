#pragma once
/*
The shader type
SHINY will give the object the fragment shader: fragmentshader_shiny.frag
MATTE will give the object the fragment shader: fragmentshader_matte.frag
*/
enum class Shader {
	SHINY, MATTE
};