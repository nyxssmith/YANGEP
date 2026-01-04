#ifndef SHADER_REGISTRY_H
#define SHADER_REGISTRY_H

#include <cute.h>
#include <unordered_map>
#include <string>

using namespace Cute;

class ShaderRegistry
{
public:
	static void registerAndLoad(const std::string& name, const char* filename);
	static CF_Shader get(const std::string& name);
	static void clear();
	static void registerAndLoadAll();

private:
	static std::unordered_map<std::string, CF_Shader> s_shaders;
};

#endif // SHADER_REGISTRY_H


