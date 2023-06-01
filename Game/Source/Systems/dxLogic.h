// The rendering system is responsible for drawing all objects
#ifndef dxLogic_H
#define dxLogic_H

// Contains our global game settings
#include "../GameConfig.h"
#include "../Components/Identification.h"
#include "../Components/Visuals.h"
#include "../Components/Physics.h"
#include <chrono>
#include <windows.h>
#include <shobjidl.h> 
#include <DDSTextureLoader.h>
#include "h2bParser.h"
#include "FileIntoString.h"
#include "load_data_oriented.h"
#include "sprite.h"
#include "Font.h"
#include <chrono>


// example space game (avoid name collisions)
namespace AVT
{
	struct DevVar
	{
		bool devActive = false;
		bool doOnce = false;
		bool persActive = false;
		bool wireActive = false;
	};
	struct ShadeVar {
		GW::MATH::GMATRIXF world = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF view = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF projection = GW::MATH::GIdentityMatrixF;
		GW::MATH::GVECTORF lDir = { -1, -1, 2, 1 };
		GW::MATH::GVECTORF lColor = { 0.9, 0.9, 1.0, 1.0 };
		GW::MATH::GVECTORF camPos = { 0.75, 0.25, -1.5, 1 };
		GW::MATH::GVECTORF ambTerm = { 0.25, 0.25, 0.35, 1.0 };
		H2B::ATTRIBUTES mats;
		GW::MATH::GVECTORF transformStart = { 0, 0, 0, 0 };
	};

	struct PipelineHandles
	{
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* targetView;
		ID3D11DepthStencilView* depthStencil;
	};

	struct SPRITE_DATA
	{
		GW::MATH::GVECTORF pos_scale;
		GW::MATH::GVECTORF rotation_depth;
	};

	//Change to fit custom
	//enum TEXTURE_ID { heart, HUD_BACKPLATE, HUD_HP_LEFT, HUD_HP_RIGHT, HUD_MP_LEFT, HUD_MP_RIGHT, HUD_STAM_BACKPLATE, HUD_STAM, HUD_CENTER, heart1, FONT_CONSOLAS, COUNT };
	enum TEXTURE_ID { HUD_CENTER, heart1, FONT_CONSOLAS, COUNT };

	using HUD = std::vector<Sprite>;

	class dxLogic
	{

#pragma region MustHave

		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// handle to our running ECS systems
		flecs::system startDraw;
		flecs::system updateDraw;
		flecs::system completeDraw;
		// Used to query screen dimensions
		GW::SYSTEM::GWindow window;
		// Vulkan resources used for rendering
		GW::GRAPHICS::GDirectX11Surface d3d11;
		GW::INPUT::GInput input;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		structuredBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	tSRV;
		ID3D11RasterizerState* wireframe;

		ShadeVar shadeVar;

		float aspectRatio;
		unsigned int H;
		unsigned int W;
		float FOV;

		float x = 20;
		float y = 20;
		float z = 20;

		float zFar = 1000.0f;
		float zNear = 0.1f;
		
		//Positive Values
		float distant = z;
		float right = x;
		float top = y;
		//Negative values
		float close = -z;
		float left = -x;
		float bot = -y;

		DevVar* dev;
		float move = 1.0f;
		float mouseXCh;
		float mouseYCh;

		float deltaTime;
		std::chrono::steady_clock::time_point lUpdate;

		GW::MATH::GMatrix proxy;
		std::vector<GW::MATH::GMATRIXF> world;
		GW::MATH::GVECTORF eye = { 0.0f, 10.0f, 0.0f, 1.0f };
		GW::MATH::GVECTORF at = { 0, 0, 0, 1.0f };
		GW::MATH::GVECTORF up = { 0, 0, 1 };
		GW::MATH::GMATRIXF view = GW::MATH::GIdentityMatrixF;
		
		GW::MATH::GMATRIXF projection = GW::MATH::GIdentityMatrixF;
		GW::MATH::GVECTORF lDir = { -3, -3, 3, 1 };
		GW::MATH::GVECTORF lColor = { 0.9, 0.9, 1.0, 1.0 };
		GW::MATH::GVECTORF camPos = { 0.75, 0.25, -1.5, 1 };
		GW::MATH::GVECTORF ambTerm = { 0.25, 0.25, 0.35, 1.0 };
		H2B::ATTRIBUTES mats;
		
		Level_Data* dataOrientedLoader;

		int renderCount = 0;
		int renderMax = 2000;
		int playerIndex = 0;
		const char* playerName = "Leela.h2b";

		bool devActive;
		std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();

		GW::SYSTEM::GLog log;

		int enemyDeadCount;
#pragma endregion

#pragma region UI

		Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexUIBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				indexUIBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				constantUIBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBufferStaticText;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBufferStaticText1;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBufferStaticText2;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBufferDynamicText;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBufferDynamicText1;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexShaderFont;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelShaderFont;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>			vertexFontFormat;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView[TEXTURE_ID::COUNT];
		Microsoft::WRL::ComPtr<ID3D11SamplerState>			samplerState;
		Microsoft::WRL::ComPtr<ID3D11BlendState>			blendState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		depthStencilState;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>		rasterizerState;

		//UIinfo
		UINT												width = 0, height = 0;
		//Sprite												heart;
		HUD													hud;
		Font												consolas32;
		Text												staticText;
		Text												staticText1;
		Text												staticText2;
		Text												dynamicText;
		Text												dynamicText1;
		SPRITE_DATA											constantBufferData = { 0 };

		HRESULT hr = E_NOTIMPL;
#pragma endregion

	public:
		// attach the required logic to the ECS 
		bool Init(
					std::shared_ptr<flecs::world> _game,
					std::weak_ptr<const GameConfig> _gameConfig,
					GW::GRAPHICS::GDirectX11Surface _d3d11,
					GW::INPUT::GInput _immediateInput,
					GW::SYSTEM::GWindow _window,
					Level_Data& _dataOrientedLoader,
					DevVar& _dev);

		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();
		void SpawnObj(GW::MATH::GMATRIXF loc, int obj);
		void SpawnObj(GW::MATH::GMATRIXF loc, const char* objectInfoPath, const char* h2bFolderPath, std::string obj);
		
	private:
		// Loading funcs
		bool LoadShaders();
		bool LoadBuffers();
		bool SetupPipeline();
		bool SetupDrawcalls();
		// Utility funcs
		std::string ShaderAsString(const char* shaderFilePath);
		void IntializeGraphics();
		void Intializegraphics();
		void InitializeVertexBuffer(ID3D11Device* creator);
		void CreateVertexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes);
		void CreateConstantBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes);
		void CreateStructuredBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes);
		void CreateIndexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes);
		void InitializePipeline(ID3D11Device* creator);
		Microsoft::WRL::ComPtr<ID3DBlob> CompileVertexShader(ID3D11Device* creator, UINT compilerFlags);
		Microsoft::WRL::ComPtr<ID3DBlob> CompilePixelShader(ID3D11Device* creator, UINT compilerFlags);
		void CreateVertexInputLayout(ID3D11Device* creator, Microsoft::WRL::ComPtr<ID3DBlob>& vsBlob);
		PipelineHandles GetCurrentPipelineHandles();
		void SetUpPipeline(PipelineHandles handles);
		void SetRenderTargets(PipelineHandles handles);
		void SetVertexBuffers(PipelineHandles handles);
		void SetRaster(PipelineHandles handles);
		void SetShaders(PipelineHandles handles);
		void ReleasePipelineHandles(PipelineHandles toRelease);
		void DrawScene(PipelineHandles handles, GW::MATH::GMATRIXF camera);
		std::vector<Sprite>	LoadHudFromXML(std::string filepath);
		SPRITE_DATA UpdateSpriteConstantBufferData(const Sprite& s);
		SPRITE_DATA UpdateTextConstantBufferData(const Text& s);
		void CreateVertexFontBuffer(ID3D11Device* creator);
		void CreateIndexFontBuffer(ID3D11Device* creator);
		void InitializeFontPipeline(ID3D11Device* creator);
		Microsoft::WRL::ComPtr<ID3DBlob> CompileVertexFontShader(ID3D11Device* creator, UINT compilerFlags);
		Microsoft::WRL::ComPtr<ID3DBlob> CompilePixelFontShader(ID3D11Device* creator, UINT compilerFlags);
		void CreateVertexFontInputLayout(ID3D11Device* creator, Microsoft::WRL::ComPtr<ID3DBlob>& vsBlob);
		void CreateStates(ID3D11Device* creator);
		void CreateDDSTexture(ID3D11Device* creator);
		void CreateConstantFontBuffer(ID3D11Device* creator);
		void InitializeSprite();
		void InitializeHud(ID3D11Device* creator);
		void SetUpandDraw(ID3D11Device* creator);
		void InitializeUIBuffers(ID3D11Device* creator);
		void InitializeUI(ID3D11Device* creator);

	public:
		void Render();
		void RenderFont();
		void UpdateUI();
		void RestartLevel();
		void DevCam();
		void ResetCam();
		void ChangePers();
		void Creator();
		void ChangeWire(ID3D11Device* creator);
		void SetEnemies(UINT enemiesKilled);
	private:
		// Uniform Data Definitions
		static constexpr unsigned int Instance_Max = 240;
		struct INSTANCE_UNIFORMS
		{
			GW::MATH::GMATRIXF instance_transforms[Instance_Max];
			GW::MATH::GVECTORF instance_colors[Instance_Max];
		}instanceData;
		// how many instances will be drawn this frame
		int draw_counter = 0;
	};
};

#endif