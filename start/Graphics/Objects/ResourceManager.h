#pragma once

#include <map>

namespace SYSR
{
	class Material;
	class Texture;
	class Mesh;

	class ResourceManager
	{
	public:
		static ResourceManager* GetInstance(){ return instance; }
		static ResourceManager* CreateInstance()
		{ 
			if (!instance)
			{
				instance = new ResourceManager();
			}
			return instance;
		}
		static bool DeleteInstance()
		{
			if (instance)
			{
				delete instance;
				return true;
			}
			return false;
		}

		bool LoadTexturesFromPath(const char *);
		bool LoadMaterialsFromPath(const char *);

		ResourceManager();
		~ResourceManager();

		Material* GetMaterialByName(const char* name)
		{
			std::map<std::string, Material*>::iterator it = materials.find(name);
			if (it != materials.end())
			{
				return it->second;
			}
			return nullptr;
		};

	private:

		std::map<std::string, Texture*> textures;
		std::map<std::string, Material*> materials;

		bool LoadFromMtlFile(const char *);

		static ResourceManager *instance;
	};
}