#pragma once
#include <glm.hpp>
#include <vector>
#include <memory.h>
#include "../CommonStructs.h"
namespace SYSR
{
	class IndexBuffer;
	class VertexBuffer;
	class Material;

	struct FaceData
	{
		float pn;	// precalculated position*normal 
		signed short normal[3]; // normal of polygon times 0.0001f when used
	};

	struct FaceDataArray
	{
		int16_t *x,*y,*z;
		int16_t *pn;

		FaceDataArray()
		{
			x = nullptr;
			y = nullptr;
			z = nullptr;
			pn = nullptr;
		}

		FaceDataArray(unsigned int triangleCount)
		{
			x = (int16_t*)_aligned_malloc(triangleCount * sizeof(int16_t), 16);
			y = (int16_t*)_aligned_malloc(triangleCount * sizeof(int16_t), 16);
			z = (int16_t*)_aligned_malloc(triangleCount * sizeof(int16_t), 16);
			
			pn = (int16_t*)_aligned_malloc(triangleCount * sizeof(int16_t), 16);
		}
		~FaceDataArray()
		{
			//delete[]x;
			//delete[]y;
			//delete[]z;
			//delete[]pn;
		}
	};

	class InternalMesh
	{
	public:
		IndexBuffer *m_pIndexBuffer;
		VertexBuffer *m_pVertexBuffer;

		FaceDataArray *m_pFaceData;

		Material *m_pMaterial;

		InternalMesh()
		{
			m_pIndexBuffer = nullptr;
			m_pVertexBuffer = nullptr;

			m_pFaceData = nullptr;

			m_pMaterial = nullptr;
		}

		~InternalMesh()
		{ 
			//delete m_pVertexBuffer;
			//delete m_pIndexBuffer;
			//delete m_pFaceData; 
		};

		void CalculateFaceData();
		void CalculateVisible(glm::vec3 i_position);
		//Material *xxxx
	};

	class Mesh
	{
	public:
		Mesh();
		~Mesh();
		void Draw();

		bool LoadFromFile(const char *i_path);

		inline void SetMaterial(Material *i_pMaterial);
		inline void SetPosition(glm::vec3 &i_position);

		inline glm::vec3 GetPosition() const;
	private:
		std::vector<InternalMesh> InternalMeshes;

		glm::vec3 m_position;

		Material *m_pMaterial;

		std::vector<Triangle_Index> m_pTriangles;
		std::vector<VSInput>  m_vertices;

	};

	void Mesh::SetMaterial(Material *i_pMaterial) 
	{
		m_pMaterial = i_pMaterial;
	};

	void Mesh::SetPosition(glm::vec3 &i_position)
	{
		m_position = i_position;
	}
	glm::vec3 Mesh::GetPosition()const
	{
		return m_position;
	}
}