#pragma once

namespace SYSR
{
	class PixelShader;
	class VertexShader;
	class Texture;

	class Material
	{
	public:
		PixelShader* m_PixelShader;
		VertexShader* m_VertexShader;
		Texture *m_pTexture;

		Material();
		~Material();

		inline void SetTexture(Texture* i_texture){ m_pTexture = i_texture; }

	private:
		
	};
}