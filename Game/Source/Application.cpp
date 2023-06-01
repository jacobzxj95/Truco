#include "Application.h"
// open some Gateware namespaces for conveinence 
// NEVER do this in a header file!
using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;

bool Application::Init()
{
	eventPusher.Create();

	// load all game settigns
	gameConfig = std::make_shared<GameConfig>();
	// create the ECS system
	game = std::make_shared<flecs::world>();
	// init all other systems
	if (InitWindow() == false)
		return false;
	if (InitInput() == false)
		return false;
	if (InitAudio() == false)
		return false;
	if (InitGraphics() == false)
		return false;
	if (InitEntities() == false)
		return false;
	if (InitSystems() == false)
		return false;
	return true;
}

bool Application::Run()
{
	// grab vsync selection
	bool vsync = gameConfig->at("Window").at("vsync").as<bool>();
	// set background color from settings
	const char* channels[] = { "red", "green", "blue" };

	bool winClosed = false;
	GW::CORE::GEventResponder winHandler;
	winHandler.Create([&winClosed](GW::GEvent e) {
		GW::SYSTEM::GWindow::Events ev;
		if (+e.Read(ev) && ev == GW::SYSTEM::GWindow::Events::DESTROY)
			winClosed = true;
		});
	window.Register(winHandler);
	float clr[] = { 0.12f, 0.15f, 0.16f, 0 };
	while (+window.ProcessWindowEvents())
	{
		IDXGISwapChain* swap;
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		ID3D11DepthStencilView* depth;
		if (winClosed == true)
			return true;
		if (+d3d11.GetImmediateContext((void**)&con) &&
			+d3d11.GetRenderTargetView((void**)&view) &&
			+d3d11.GetDepthStencilView((void**)&depth) &&
			+d3d11.GetSwapchain((void**)&swap)) {
			con->ClearRenderTargetView(view, clr);
			con->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH, 1, 0);
			dxRenderingSystem.UpdateUI();
			dxRenderingSystem.Render();
			dxRenderingSystem.RenderFont();
			dxRenderingSystem.SetEnemies(enemySystem.killedEnemies);

			if (playerSystem.dead)
			{
				dxRenderingSystem.RestartLevel();
				playerSystem.dead = false;
			}
			
			//dxLogic.Render();
			if (dev->devActive)
			{
				dxRenderingSystem.DevCam();
			}
			if (dev->doOnce && !dev->devActive)
			{
				dev->persActive = false;
				dev->wireActive = false;
				dxRenderingSystem.ResetCam();
				dxRenderingSystem.ChangePers();
				dxRenderingSystem.Creator();
			}

			if (GameLoop() == false) {
				swap->Present(1, 0);
				swap->Release();
				view->Release();
				depth->Release();
				con->Release();
				return false;
			}
			if (swap->Present(1, 0)) {
				swap->Release();
				view->Release();
				depth->Release();
				con->Release();
				// failing EndFrame is not always a critical error, see the GW docs for specifics
			}
		}
		else
			return false;
	}
	return true;
}

bool Application::Shutdown()
{
	// disconnect systems from global ECS
	if (playerSystem.Shutdown() == false)
		return false;
	if (dxRenderingSystem.Shutdown() == false)
		return false;
	if (levelSystem.Shutdown() == false)
		return false;
	if (physicsSystem.Shutdown() == false)
		return false;
	if (bulletSystem.Shutdown() == false)
		return false;
	if (enemySystem.Shutdown() == false)
		return false;

	return true;
}

bool Application::InitWindow()
{
	// grab settings
	int width = gameConfig->at("Window").at("width").as<int>();
	int height = gameConfig->at("Window").at("height").as<int>();
	int xstart = gameConfig->at("Window").at("xstart").as<int>();
	int ystart = gameConfig->at("Window").at("ystart").as<int>();
	std::string title = gameConfig->at("Window").at("title").as<std::string>();
	// open window
	if (+window.Create(xstart, ystart, width, height, GWindowStyle::WINDOWEDLOCKED) &&
		+window.SetWindowName(title.c_str())) {
		return true;
	}
	return false;
}

bool Application::InitInput()
{
	if (-gamePads.Create())
		return false;
	if (-immediateInput.Create(window))
		return false;
	if (-bufferedInput.Create(window))
		return false;
	return true;
}

bool Application::InitAudio()
{
	if (-audioEngine.Create())
		return false;
	return true;
}

bool Application::InitGraphics()
{
#ifndef NDEBUG
	const char* debugLayers[] = {
		"VK_LAYER_KHRONOS_validation", // standard validation layer
		//"VK_LAYER_RENDERDOC_Capture" // add this if you have installed RenderDoc
	};
	if (+d3d11.Create(window, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
		return true;
#else
	if (+d3d11.Create(window, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
		return true;
#endif
	return false;
}

bool Application::InitEntities()
{
	// Load bullet prefabs
	if (weapons.Load(game, gameConfig, audioEngine) == false)
		return false;
	// Load the player entities
	if (players.Load(game, gameConfig) == false)
		return false;
	// Load the enemy entities
	if (enemies.Load(game, gameConfig, audioEngine) == false)
		return false;
	// Load the obstacle entities
	if (obstacles.Load(game) == false)
		return false;

	return true;
}

bool Application::InitSystems()
{
	// connect systems to global ECS
	if (dxRenderingSystem.Init(game, gameConfig, d3d11, immediateInput, window, dataOrientedLoader, *dev) == false)
		return false;
	if (playerSystem.Init(game, gameConfig, immediateInput, bufferedInput,
		gamePads, audioEngine, eventPusher, dataOrientedLoader, *dev) == false)
		return false;
	if (levelSystem.Init(game, gameConfig, audioEngine, dataOrientedLoader) == false)
		return false;
	if (physicsSystem.Init(game, gameConfig) == false)
		return false;
	if (bulletSystem.Init(game, gameConfig,dataOrientedLoader) == false)
		return false;
	if (enemySystem.Init(game, gameConfig, eventPusher, dataOrientedLoader) == false)
		return false;
	if (obstacleSystem.Init(game, dataOrientedLoader) == false)
		return false;

	return true;
}

bool Application::GameLoop()
{
	// compute delta time and pass to the ECS system
	static auto start = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(
		std::chrono::steady_clock::now() - start).count();
	start = std::chrono::steady_clock::now();
	// let the ECS system run
	return game->progress(static_cast<float>(elapsed));
}
