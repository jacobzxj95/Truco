#include "PlayerLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Visuals.h"
#include "../Components/Gameplay.h"
#include "../Entities/Prefabs.h"
#include "../Events/Playevents.h"

using namespace ESG; // Example Space Game
using namespace GW::INPUT; // input libs
using namespace GW::AUDIO; // audio libs

// Connects logic to traverse any players and allow a controller to manipulate them
bool ESG::PlayerLogic::Init(	std::shared_ptr<flecs::world> _game, 
							std::weak_ptr<const GameConfig> _gameConfig, 
							GW::INPUT::GInput _immediateInput, 
							GW::INPUT::GBufferedInput _bufferedInput, 
							GW::INPUT::GController _controllerInput,
							GW::AUDIO::GAudio _audioEngine,
							GW::CORE::GEventGenerator _eventPusher)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	immediateInput = _immediateInput;
	bufferedInput = _bufferedInput;
	controllerInput =	_controllerInput;
	audioEngine = _audioEngine;
	// Init any helper systems required for this task
	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
	int width = (*readCfg).at("Window").at("width").as<int>();
	float speed = (*readCfg).at("Player1").at("speed").as<float>();
	chargeTime = (*readCfg).at("Player1").at("chargeTime").as<float>();
	// add logic for updating players
	playerSystem = game->system<Player, Position, Orientation, ControllerID>("Player System")
		.iter([this, speed](flecs::iter it, Player*, Position* p, Orientation* o, ControllerID* c) {
		for (auto i : it) {
			// left-right movement
			
			float input = 0, xaxis = 0, yaxis = 0, udRotation = 0, lrRotation = 0;
			// Use the controller/keyboard to move the player around the screen			
			if (c[i].index == 0) { // enable keyboard controls for player 1
				immediateInput.GetState(G_KEY_A, input); xaxis -= input;
				immediateInput.GetState(G_KEY_D, input); xaxis += input;

				immediateInput.GetState(G_KEY_W, input); yaxis += input;
				immediateInput.GetState(G_KEY_S, input); yaxis -= input; input = 0;

				immediateInput.GetState(G_KEY_UP, input); udRotation += input;
				immediateInput.GetState(G_KEY_DOWN, input); udRotation -= input;

				immediateInput.GetState(G_KEY_RIGHT, input); lrRotation -= input;
				immediateInput.GetState(G_KEY_LEFT, input); lrRotation += input; input = 0;
			}

			// grab left-thumb stick
			controllerInput.GetState(c[i].index, G_LX_AXIS, input); xaxis += input;
			controllerInput.GetState(c[i].index, G_DPAD_LEFT_BTN, input); xaxis -= input;
			controllerInput.GetState(c[i].index, G_DPAD_RIGHT_BTN, input); xaxis += input; input = 0;
			xaxis = G_LARGER(xaxis, -1);// cap right motion
			xaxis = G_SMALLER(xaxis, 1);// cap left motion

			
			// apply movement
			// affect movement from collision
			if (it.entity(i).has<CollidedWith>(flecs::Wildcard))
			{
				it.entity(i).remove<CollidedWith>(flecs::Wildcard);
				xaxis = 0;
				yaxis = 0;
			}
			
			// apply movement - horizontal
			p[i].value.x += xaxis * it.delta_time() * speed;
			// limit the player to stay within -1 to +1 NDC
			p[i].value.x = G_LARGER(p[i].value.x, -0.8f);
			p[i].value.x = G_SMALLER(p[i].value.x, +0.8f);

			// apply movement - vertical
			p[i].value.y += yaxis * it.delta_time() * speed;
			// limit the player to stay within -1 to +1 NDC
			p[i].value.y = G_LARGER(p[i].value.y, -0.8f);
			p[i].value.y = G_SMALLER(p[i].value.y, +0.8f);
			
			//apply rotation
			float theta = 0.0f;
			if (lrRotation == 1)
			{
				if (udRotation == 1)
					theta = 7.0f;
				else if (udRotation == -1)
					theta = 5.0f;
				else
					theta = 6.0f;

			}
			else if (lrRotation == -1)
			{
				if (udRotation == 1)
					theta = 1.0f;
				else if (udRotation == -1)
					theta = 3.0f;
				else
					theta = 2.0f;
			}
			else
			{
				if (udRotation == 1)
					theta = 0.0f;
				else if(udRotation == -1)
					theta = 4.0f;
			}
			//std::cout << "Theta: " << theta  << '\t' << "Theta in Radians: " << (theta * 3.141593) / 4 << '\t' << "Direction: " << lrRotation << '\t' << udRotation << '\n';
			float thetaRads = (theta * 3.1415926535797) / 4; //Convert into Radians!!!
			
			GW::MATH2D::GMATRIX2F currRotation = {cos(thetaRads), -sin(thetaRads), sin(thetaRads), cos(thetaRads)};
			o[i].value = currRotation;
			GW::MATH2D::GMatrix2D::Scale2F(o[i].value, GW::MATH2D::GVECTOR2F{0.15, 0.15}, o[i].value);



			// fire weapon if they are in a firing state
			if (it.entity(i).has<Firing>()) {
				Position offset = p[i];
				offset.value.y += 0.05f;
				FireLasers(it.world(), offset);
				it.entity(i).remove<Firing>();
			}
		}
		// process any cached button events after the loop (happens multiple times per frame)
		ProcessInputEvents(it.world());
	});

	// Create an event cache for when the spacebar/'A' button is pressed
	pressEvents.Create(Max_Frame_Events); // even 32 is probably overkill for one frame
		
	// register for keyboard and controller events
	bufferedInput.Register(pressEvents);
	controllerInput.Register(pressEvents);

	// create the on explode handler
	onExplode.Create([this](const GW::GEvent& e) {
		ESG::PLAY_EVENT event; ESG::PLAY_EVENT_DATA eventData;
		if (+e.Read(event, eventData)) {
			// only in here if event matches
			std::cout << "Enemy Was Destroyed!\n";
		}
	});
	_eventPusher.Register(onExplode);

	return true;
}

// Free any resources used to run this system
bool ESG::PlayerLogic::Shutdown()
{
	playerSystem.destruct();
	game.reset();
	gameConfig.reset();

	return true;
}

// Toggle if a system's Logic is actively running
bool ESG::PlayerLogic::Activate(bool runSystem)
{
	if (playerSystem.is_alive()) {
		(runSystem) ? 
			playerSystem.enable() 
			: playerSystem.disable();
		return true;
	}
	return false;
}

bool ESG::PlayerLogic::ProcessInputEvents(flecs::world& stage)
{
	// pull any waiting events from the event cache and process them
	GW::GEvent event;
	while (+pressEvents.Pop(event)) {
		bool fire = false;
		GController::Events controller;
		GController::EVENT_DATA c_data;
		GBufferedInput::Events keyboard;
		GBufferedInput::EVENT_DATA k_data;
		// these will only happen when needed
		if (+event.Read(keyboard, k_data)) {
			if (keyboard == GBufferedInput::Events::KEYPRESSED) {
				if (k_data.data == G_KEY_SPACE) {
					fire = true;
					chargeStart = stage.time();
				}
			}
			if (keyboard == GBufferedInput::Events::KEYRELEASED) {
				if (k_data.data == G_KEY_SPACE) {
					chargeEnd = stage.time();
					if (chargeEnd - chargeStart >= chargeTime) {
						fire = true;
					}
				}
			}
		}
		else if (+event.Read(controller, c_data)) {
			if (controller == GController::Events::CONTROLLERBUTTONVALUECHANGED) {
				if (c_data.inputValue > 0 && c_data.inputCode == G_SOUTH_BTN)
					fire = true;
			}
		}
		if (fire) {
			// grab player one and set them to a firing state
			stage.entity("Player One").add<Firing>();
		}
	}
	return true;
}

// play sound and launch two laser rounds
bool ESG::PlayerLogic::FireLasers(flecs::world& stage, Position origin)
{
	// Grab the prefab for a laser round
	flecs::entity bullet;
	RetreivePrefab("Lazer Bullet", bullet);

	origin.value.x -= 0.05f;
	auto laserLeft = stage.entity().is_a(bullet)
		.set<Position>(origin);
	origin.value.x += 0.1f;
	auto laserRight = stage.entity().is_a(bullet)
		.set<Position>(origin);
	// if this shot is charged
	if (chargeEnd - chargeStart >= chargeTime) {
		chargeEnd = chargeStart;
		laserLeft.set<ChargedShot>({ 2 })
			.set<Material>({1,0,0});
		laserRight.set<ChargedShot>({ 2 })
			.set<Material>({ 1,0,0 });
	}

	// play the sound of the Lazer prefab
	GW::AUDIO::GSound shoot = *bullet.get<GW::AUDIO::GSound>();
	shoot.Play();

	return true;
}