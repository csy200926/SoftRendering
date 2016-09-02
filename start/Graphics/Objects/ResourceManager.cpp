#include "ResourceManager.h"
#include "Texture.h"
#include "Material.h"

#include "../../Utilities/Dirent.h"

#include <vector>

namespace SYSR
{
	ResourceManager *ResourceManager::instance = nullptr;
	ResourceManager::ResourceManager()
	{

	}
	ResourceManager::~ResourceManager()
	{

	}

	
	bool ResourceManager::LoadTexturesFromPath(const char *i_path)
	{
		using namespace std;

		vector<string> texturesStr;

		DIR *dir;
		struct dirent *ent;
		
		if ((dir = opendir(i_path)) != NULL) {
			/* print all the files and directories within directory */
			while ((ent = readdir(dir)) != NULL) {
				texturesStr.push_back(ent->d_name);//printf("%s\n", ent->d_name);
			}
			closedir(dir);
		}
		else {
			/* could not open directory */
			perror("");
			return false;
		}

		string directoryStr(i_path);
		for (int i = 2; i < texturesStr.size(); i++)
		{
			string path = (directoryStr + texturesStr[i]);
			Texture *pTexture = new Texture(path.c_str());
			string textureNameStr = texturesStr[i];
			textureNameStr = textureNameStr.substr(0, textureNameStr.size() - 4);

			textures.insert(make_pair(textureNameStr, pTexture));
		}


	}
	bool ResourceManager::LoadMaterialsFromPath(const char *i_path)
	{
		using namespace std;

		vector<string> materialsStr;

		DIR *dir;
		struct dirent *ent;

		if ((dir = opendir(i_path)) != NULL) {
			/* print all the files and directories within directory */
			while ((ent = readdir(dir)) != NULL) {
				materialsStr.push_back(ent->d_name);//printf("%s\n", ent->d_name);
			}
			closedir(dir);
		}
		else {
			/* could not open directory */
			perror("");
			return false;
		}

		string directoryStr(i_path);
		for (int i = 2; i < materialsStr.size(); i++)
		{
			string path = (directoryStr + materialsStr[i]);
			LoadFromMtlFile(path.c_str());

		}
	}

	bool ResourceManager::LoadFromMtlFile(const char *i_path)
	{
		using namespace std;

		FILE * file = fopen(i_path, "r");
		if (file == NULL){
			printf("Impossible to open the file !\n");
			return false;

		}

		Material *pCurMaterial = nullptr;
		while (1){
			char lineHeader[128];
			// read the first word of the line
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
				break; // EOF = End Of File. Quit the loop.

			if (strcmp(lineHeader, "newmtl") == 0)
			{
				char materialName[128];
				fscanf(file, "%s\n", materialName);
				pCurMaterial = new Material();
				materials.insert(make_pair(materialName, pCurMaterial));
			}
			else if (strcmp(lineHeader, "map_Kd") == 0)
			{
				char textureName[128];
				fscanf(file, "%s\n", textureName);
				string textureNameStr(textureName);
				textureNameStr = textureNameStr.substr(0, textureNameStr.size() - 4);
				Texture *pTexture = nullptr;
				map<string, Texture*>::iterator it = textures.find(textureNameStr);
				if (it != textures.end())
				{
					pCurMaterial->SetTexture(it->second);
				}

				
			}
		}



		return true;
	}
	/*		using namespace std;

		FILE * file = fopen(i_path, "r");
		if (file == NULL){
			printf("Impossible to open the file !\n");
			return;

		}


		while (1){
			char lineHeader[128];
			// read the first word of the line
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
				break; // EOF = End Of File. Quit the loop.
		
			if (strcmp(lineHeader, "v") == 0){
				
			}

		
		
		}*/


}