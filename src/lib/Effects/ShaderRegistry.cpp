#include "ShaderRegistry.h"
#include <utility>

std::unordered_map<std::string, CF_Shader> ShaderRegistry::s_shaders;

void ShaderRegistry::registerAndLoad(const std::string& name, const char* filename)
{
	// Compile/load draw shader now and store handle.
	CF_Shader shader = cf_make_draw_shader(filename);
	if (shader.id == 0) {
		printf("ShaderRegistry: ERROR: Failed to load shader '%s' from '%s'\n", name.c_str(), filename);
	} else {
		printf("ShaderRegistry: Loaded shader '%s' from '%s'\n", name.c_str(), filename);
	}
	s_shaders[name] = shader;
}

CF_Shader ShaderRegistry::get(const std::string& name)
{
	auto it = s_shaders.find(name);
	if (it == s_shaders.end()) return CF_Shader{0};
	return it->second;
}

void ShaderRegistry::clear()
{
	s_shaders.clear();
}

void ShaderRegistry::registerAndLoadAll()
{
	registerAndLoad("red", "red_flash.shd");
	registerAndLoad("green", "green_flash.shd");
}


