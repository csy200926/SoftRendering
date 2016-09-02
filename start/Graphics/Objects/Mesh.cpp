#include "Mesh.h"

#include "../../Includes/glm/gtc/matrix_transform.hpp"

#include "../IndexBuffer.h"
#include "../VertexBuffer.h"
#include "../FakeShaders/PixelShader.h"
#include "../FakeShaders/VertexShader.h"
#include "../Graphic.h"
#include "../Objects/Material.h"
#include "../Camera.h"
#include "../../Utilities/Profiling.h"
#include "ResourceManager.h"

#include "glm.hpp"

#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <intrin.h>

namespace SYSR
{

	void InternalMesh::CalculateFaceData()
	{
		using namespace glm;

		int triangleCount = m_pIndexBuffer->GetCount();
		m_pFaceData = new FaceDataArray(triangleCount);


		Triangle_Index* m_pTriangles = m_pIndexBuffer->GetData();
		VSInput* m_vertices = m_pVertexBuffer->GetData();

		for (int i = 0; i < triangleCount; i++)
		{
			unsigned int indexA = m_pTriangles[i].indices[0];
			unsigned int indexB = m_pTriangles[i].indices[1];
			unsigned int indexC = m_pTriangles[i].indices[2];
			vec3 normal = -normalize(cross(m_vertices[indexA].position - m_vertices[indexB].position, m_vertices[indexC].position - m_vertices[indexB].position));

			m_pFaceData->pn[i] = dot(normal, m_vertices[indexB].position) * 100;
			m_pFaceData->x[i] = normal.x * 100;
			m_pFaceData->y[i] = normal.y * 100;
			m_pFaceData->z[i] = normal.z * 100;

		}

	}

	void InternalMesh::CalculateVisible(glm::vec3 i_position)
	{
		using namespace glm;
		using namespace std;

		// No rotations for now
		Camera::ModelToWorld_Matrix = glm::translate(glm::mat4(1.0f), i_position);

		mat4x4 worldToModel = Camera::ModelToWorld_Matrix;
		worldToModel = inverse(worldToModel);
		vec4 cameraPos(Camera::cameraPos,1);

		vec4 cameraPos_ModelSpaceV4(worldToModel * cameraPos);
		vec3 cameraPos_ModelSpace(cameraPos_ModelSpaceV4);

		Triangle_Index* m_pTriangles = m_pIndexBuffer->GetData();
		int m_triangleCount = m_pIndexBuffer->GetCount();

		//------------ Back culling ------------
		//	Foreach triangle
		//		Get normal 
		//		Get triangle position
		//		dot product of normal and viewVector
		//		if same dir     push into pool
		// Optimize:http://www.gamasutra.com/view/feature/131773/a_compact_method_for_backface_.php?print=1
		//vector<unsigned int>indicesPool;
		//vector<VSInput>verticesPool;

		// Do draw
		__m128i camPos_x_scalar = _mm_set1_epi32(cameraPos_ModelSpace.x * 100);
		__m128i camPos_y_scalar = _mm_set1_epi32(cameraPos_ModelSpace.y * 100);
		__m128i camPos_z_scalar = _mm_set1_epi32(cameraPos_ModelSpace.z * 100);

		__m128 scalar_10 = _mm_set1_ps(100);

		unsigned int count_TimesOfFour = (m_triangleCount / 4) * 4;
		for (unsigned int i = 0; i < count_TimesOfFour; i += 4)
		{

			__m128i normal_x_4x16 = _mm_loadl_epi64((const __m128i *)(&m_pFaceData->x[i]));
			__m128i normal_x_4x32 = _mm_unpacklo_epi16(normal_x_4x16, _mm_cmplt_epi16(normal_x_4x16, _mm_setzero_si128()));

			__m128i normal_y_4x16 = _mm_loadl_epi64((const __m128i *)(&m_pFaceData->y[i]));
			__m128i normal_y_4x32 = _mm_unpacklo_epi16(normal_y_4x16, _mm_cmplt_epi16(normal_y_4x16, _mm_setzero_si128()));

			__m128i normal_z_4x16 = _mm_loadl_epi64((const __m128i *)(&m_pFaceData->z[i]));
			__m128i normal_z_4x32 = _mm_unpacklo_epi16(normal_z_4x16, _mm_cmplt_epi16(normal_z_4x16, _mm_setzero_si128()));

			__m128i normal_pn_4x16 = _mm_loadl_epi64((const __m128i *)(&m_pFaceData->pn[i]));
			__m128i normal_pn_4x32 = _mm_unpacklo_epi16(normal_pn_4x16, _mm_cmplt_epi16(normal_pn_4x16, _mm_setzero_si128()));

			__m128i result_dot = _mm_mullo_epi32(normal_x_4x32, camPos_x_scalar);
			result_dot = _mm_add_epi32(result_dot, _mm_mullo_epi32(normal_y_4x32, camPos_y_scalar));
			result_dot = _mm_add_epi32(result_dot, _mm_mullo_epi32(normal_z_4x32, camPos_z_scalar));
			
			// divided by 1024
			__m128 result_dot_F = _mm_cvtepi32_ps(result_dot);
			result_dot_F = _mm_div_ps(result_dot_F, scalar_10);
			result_dot = _mm_cvtps_epi32(result_dot_F);

			result_dot = _mm_sub_epi32(result_dot, normal_pn_4x32);

			int *result = (int *)&result_dot;

			for (int j = 0; j < 4; j++)
			{
				Triangle_Index &triangle = m_pTriangles[i+j];
				triangle.isVisible = true;
				if (result[j] < 0)
					triangle.isVisible = false;
			}

		}
	}

	Mesh::Mesh()
	{ 
		using namespace glm;
		
		LoadFromFile("../Meshes/head.obj");
		for (int i = 0; i < InternalMeshes.size(); i++)
		{
			InternalMeshes[i].CalculateFaceData();
		}
	}
	Mesh::~Mesh()
	{
	}

	bool Mesh::LoadFromFile(const char *i_path)
	{
		using namespace glm;
		using namespace std;

		FILE * file = fopen(i_path, "r");
		if (file == NULL){
			     printf("Impossible to open the file !\n");
			     return false;
			
		}

		std::vector< unsigned int > uvIndices, normalIndices;
		std::vector< glm::vec3 > temp_vertices;
		std::vector< glm::vec2 > temp_uvs;
		std::vector< glm::vec3 > temp_normals;
		bool flushFace = false;
		Material* pCurMaterial = nullptr;

		//index from Obj   index for mesh
		map<unsigned int, unsigned int> pushedVerties;
		
		while (1){

			char lineHeader[128];
			// read the first word of the line
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
				break; // EOF = End Of File. Quit the loop.

			// else : parse lineHeader
			if (strcmp(lineHeader, "v") == 0){

				if (flushFace == true)
				{
					flushFace = false;

					InternalMesh internalMesh;
					internalMesh.m_pIndexBuffer = new IndexBuffer();
					internalMesh.m_pIndexBuffer->Set(m_pTriangles.size(), m_pTriangles.data());
					internalMesh.m_pVertexBuffer = new VertexBuffer();
					internalMesh.m_pVertexBuffer->Set(m_vertices.size(), m_vertices.data());
					internalMesh.m_pMaterial = pCurMaterial;

					m_pTriangles.clear();
					m_vertices.clear();
					pushedVerties.clear();

					InternalMeshes.push_back(internalMesh);
				}

				     glm::vec3 vertex;
				     fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				     temp_vertices.push_back(vertex);
			}
			else if (strcmp(lineHeader, "vt") == 0){
				     glm::vec2 uv;
				     fscanf(file, "%f %f\n", &uv.x, &uv.y);
				     temp_uvs.push_back(uv);

			}
			else if (strcmp(lineHeader, "vn") == 0){
				     glm::vec3 normal;
				     fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				     temp_normals.push_back(normal);

			}
			else if (strcmp(lineHeader, "usemtl") == 0)
			{
				char materialName[128];
				fscanf(file, "%s\n", materialName);
				pCurMaterial = ResourceManager::GetInstance()->GetMaterialByName(materialName);

			}
			else if (strcmp(lineHeader, "f") == 0){
				//std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				if (matches != 9){
					printf("File can't be read by our simple parser : ( Try exporting with other options\n");
					return false;

				}


				Triangle_Index triangle;

				for (int verIndex = 2; verIndex >= 0; verIndex--)
				{
					map<unsigned int, unsigned int>::iterator it = pushedVerties.find(vertexIndex[verIndex]);
					triangle.indices[verIndex] = m_vertices.size();
					if (it == pushedVerties.end())
					{
						VSInput vertex;
						vertex.position = temp_vertices[vertexIndex[verIndex] - 1];
						vertex.normal = temp_normals[normalIndex[verIndex] - 1];
						vertex.UV =  temp_uvs[uvIndex[verIndex] - 1];

						pushedVerties.insert(make_pair(vertexIndex[verIndex], triangle.indices[verIndex]));//cache it

						m_vertices.push_back(vertex);// "generate" the vertex
					}
					else
					{
						triangle.indices[verIndex] = it->second;
					}

				}

				m_pTriangles.push_back(triangle);
			}
			else if (strcmp(lineHeader, "g") == 0)
			{
				flushFace = true;

			}

		}


		if (flushFace == true)
		{
			flushFace = false;

			InternalMesh internalMesh;
			internalMesh.m_pIndexBuffer = new IndexBuffer();
			internalMesh.m_pIndexBuffer->Set(m_pTriangles.size(), m_pTriangles.data());
			internalMesh.m_pVertexBuffer = new VertexBuffer();
			internalMesh.m_pVertexBuffer->Set(m_vertices.size(), m_vertices.data());
			internalMesh.m_pMaterial = pCurMaterial;

			m_pTriangles.clear();
			m_vertices.clear();

			InternalMeshes.push_back(internalMesh);
		}

		return true;
	}

	void Mesh::Draw()
	{
			
	//	PROFILE_SCOPE_BEGIN("Draw")
		Material *pMaterial = nullptr;
		for (int i = 0; i < InternalMeshes.size(); i++)
		{
			// backcull has some accuracy bug
			InternalMeshes[i].CalculateVisible(m_position);
			pMaterial = InternalMeshes[i].m_pMaterial;
			if (pMaterial == nullptr)
				continue;//Currently just skip
			Graphics_Draw(InternalMeshes[i].m_pVertexBuffer, InternalMeshes[i].m_pIndexBuffer, pMaterial->m_VertexShader, pMaterial->m_PixelShader, pMaterial->m_pTexture);
		}
		//Graphics_Draw(m_pVertexBuffer, m_pIndexBuffer,m_pMaterial->m_VertexShader,m_pMaterial->m_PixelShader,m_pMaterial->m_pTexture);
	//	PROFILE_SCOPE_END
	}
}