#pragma once

#include <cstdint>

class GLShader
{
private:
	uint32_t m_Program;
	uint32_t m_VertexShader;
	uint32_t m_GeometryShader;
	uint32_t m_FragmentShader;

	bool CompileShader(uint32_t type);
public:
	GLShader() : m_Program(0), m_VertexShader(0),
		m_GeometryShader(0), m_FragmentShader(0) {

	}
	~GLShader() {}

	inline uint32_t GetProgram() { return m_Program; }
	bool LoadVertexShader(const char* filename);
	bool LoadGeometryShader(const char* filename);
	bool LoadFragmentShader(const char* filename);
	bool Create();
	void Destroy();
};