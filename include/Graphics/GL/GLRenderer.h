#pragma once
#if COMPILE_OPEN_GL

#include "../Renderer.h"

struct GameContext;

class GLRenderer : public Renderer
{
public:
	GLRenderer(GameContext& gameContext);
	virtual ~GLRenderer();

	virtual glm::uint Initialize(const GameContext& gameContext, const RenderObjectCreateInfo* createInfo) override;

	virtual void PostInitialize() override;

	virtual void Update(const GameContext& gameContext) override;
	virtual void Draw(const GameContext& gameContext) override;
	virtual void ReloadShaders(GameContext& gameContext) override;

	virtual void SetTopologyMode(glm::uint renderID, TopologyMode topology) override;
	virtual void SetClearColor(float r, float g, float b) override;

	virtual void OnWindowSize(int width, int height) override;

	virtual void SetVSyncEnabled(bool enableVSync) override;
	virtual void Clear(int flags, const GameContext& gameContext) override;
	virtual void SwapBuffers(const GameContext& gameContext) override;

	virtual void UpdateTransformMatrix(const GameContext& gameContext, glm::uint renderID, const glm::mat4& model) override;

	virtual int GetShaderUniformLocation(glm::uint program, const std::string uniformName) override;
	virtual void SetUniform1f(glm::uint location, float val) override;

	virtual glm::uint GetProgram(glm::uint renderID) override;
	virtual void DescribeShaderVariable(glm::uint renderID, glm::uint program, const std::string& variableName, int size,
		Renderer::Type renderType, bool normalized, int stride, void* pointer) override;

	virtual void Destroy(glm::uint renderID) override;

private:
	static glm::uint BufferTargetToGLTarget(BufferTarget bufferTarget);
	static glm::uint TypeToGLType(Type type);
	static glm::uint UsageFlagToGLUsageFlag(UsageFlag usage);
	static glm::uint TopologyModeToGLMode(TopologyMode topology);

	struct RenderObject
	{
		glm::uint renderID;

		glm::uint VAO;
		glm::uint VBO;
		glm::uint IBO;

		GLenum topology = GL_TRIANGLES;

		glm::uint vertexBuffer;
		VertexBufferData* vertexBufferData = nullptr;

		bool indexed;
		glm::uint indexBuffer;
		std::vector<glm::uint>* indices = nullptr;

		// Uniform IDs
		int projection;
		int view;
		int viewInverse;
		int viewProjection;
		int model;
		int modelInvTranspose;
		int modelViewProjection;
		int camPos;
		int viewDir;
		int lightDir;
		int ambientColor;
		int specularColor;
		int useDiffuseTexture;
		int useNormalTexture;
		int useSpecularTexture;

		glm::uint shaderIndex;

		std::string diffuseMapPath;
		glm::uint diffuseMapID;
		std::string specularMapPath;
		glm::uint specularMapID;
		std::string normalMapPath;
		glm::uint normalMapID;
	};

	typedef std::vector<RenderObject*>::iterator RenderObjectIter;

	RenderObject* GetRenderObject(glm::uint renderID);
	RenderObjectIter Destroy(RenderObjectIter iter);
	void UnloadShaders();
	void LoadShaders();

	// TODO: use sorted data type (map)
	std::vector<RenderObject*> m_RenderObjects;

	bool m_VSyncEnabled;

	struct Shader
	{
		glm::uint program;
		glm::uint vertexShader;
		glm::uint fragmentShader;

		Uniform::Type constantBufferUniforms;
		Uniform::Type dynamicBufferUniforms;
	};

	std::vector<Shader> m_LoadedShaders;

	// Scene info variable locations
	//int m_LightDirID;
	//int m_AmbientColorID;
	//int m_SpecularColorID;

	GLRenderer(const GLRenderer&) = delete;
	GLRenderer& operator=(const GLRenderer&) = delete;
};

#endif // COMPILE_OPEN_GL