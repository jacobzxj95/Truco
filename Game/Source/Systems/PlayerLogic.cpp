#include "PlayerLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Visuals.h"
#include "../Components/Gameplay.h"
#include "../Entities/Prefabs.h"
#include "../Events/Playevents.h"


using namespace AVT; // Not the Example Space Game
using namespace GW::INPUT; // input libs
using namespace GW::AUDIO; // audio libs

// Connects logic to traverse any players and allow a controller to manipulate them
bool AVT::PlayerLogic::Init(	std::shared_ptr<flecs::world> _game, 
							std::weak_ptr<const GameConfig> _gameConfig, 
							GW::INPUT::GInput _immediateInput, 
							GW::INPUT::GBufferedInput _bufferedInput, 
							GW::INPUT::GController _controllerInput,
							GW::AUDIO::GAudio _audioEngine,
							GW::CORE::GEventGenerator _eventPusher,
							Level_Data &_data,
							DevVar& _dev)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	immediateInput = _immediateInput;
	bufferedInput = _bufferedInput;
	controllerInput =	_controllerInput;
	audioEngine = _audioEngine;
	dev = &_dev;
	dead = false;
	// Init any helper systems required for this task
	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
	int width = (*readCfg).at("Window").at("width").as<int>();
	float speed = (*readCfg).at("Player1").at("speed").as<float>();
	chargeTime = (*readCfg).at("Player1").at("chargeTime").as<float>();
	// scale = (*readCfg).at("Player1").at("scale").as<float>();
	
	data = &_data;

	// add logic for updating players
	playerSystem = game->system<Player, Position, Orientation, Mesh, ControllerID, Collidable>("Player System")
		.iter([this, speed](flecs::iter it, Player*, Position* p, Orientation* o, Mesh* me, ControllerID* c, Collidable* col) {
		for (auto i : it) {
			
			// left-right movement

			if (!once)
			{
				for (int i = 0; i < data->levelModels.size(); i++)
				{
					if (data->levelModels[i].filename == bulletName)
					{
						bulletIndex = i;
						break;
					}
				}
				for (int i = 0; i < data->levelModels.size(); i++)
				{
					if (data->levelModels[i].filename == playerName)
					{
						playerIndex = i;
						break;
					}
				}
				once = true;
			}

			float input = 0, xaxis = 0, zaxis = 0, udRotation = 0, lrRotation = 0, scale = 1, pSpeed = 0, pScale = 0, firing = 0;
			// Use the controller/keyboard to move the player around the screen			
			if (c[i].index == 0) { // enable keyboard controls for player 1
				immediateInput.GetState(G_KEY_A, input); xaxis -= input;
				immediateInput.GetState(G_KEY_D, input); xaxis += input;

				immediateInput.GetState(G_KEY_W, input); zaxis += input;
				immediateInput.GetState(G_KEY_S, input); zaxis -= input; input = 0;

				immediateInput.GetState(G_KEY_UP, input); udRotation += input;
				immediateInput.GetState(G_KEY_DOWN, input); udRotation -= input;

				immediateInput.GetState(G_KEY_RIGHT, input); lrRotation -= input;
				immediateInput.GetState(G_KEY_LEFT, input); lrRotation += input; input = 0;

				immediateInput.GetState(G_KEY_E, input); pScale += input;
				immediateInput.GetState(G_KEY_Q, input); pScale -= input; input = 0;

				immediateInput.GetState(G_KEY_LEFTSHIFT, input); pSpeed += input; input = 0;

				immediateInput.GetState(G_KEY_V, firing);

				immediateInput.GetState(G_KEY_F11, input); if (input != 0 && !dev->devActive) { dev->devActive = true; }
				immediateInput.GetState(G_KEY_ESCAPE, input); if (input != 0 && dev->devActive) { dev->devActive = false; dev->doOnce = true; }
			}

			if (firing != 0.0f)
				int nDebug = 0;


			// grab left-thumb stick
			controllerInput.GetState(c[i].index, G_LX_AXIS, input); xaxis += input;
			controllerInput.GetState(c[i].index, G_DPAD_LEFT_BTN, input); xaxis -= input;
			controllerInput.GetState(c[i].index, G_DPAD_RIGHT_BTN, input); xaxis += input; input = 0;
			xaxis = G_LARGER(xaxis, -1);// cap right motion
			xaxis = G_SMALLER(xaxis, 1);// cap left motion
			
			// apply movement
			
			if (pScale)
				scale += 0.05 * pScale;

			/*// apply movement - horizontal
			p[i].value.x += xaxis * it.delta_time() * speed * (pSpeed + 0.5);
			// limit the player to stay within -1 to +1 NDC
			p[i].value.x = G_LARGER(p[i].value.x, -0.8f);
			p[i].value.x = G_SMALLER(p[i].value.x, +0.8f);

			// apply movement - vertical
			p[i].value.y += yaxis * it.delta_time() * speed * (pSpeed + 0.5);
			// limit the player to stay within -1 to +1 NDC
			p[i].value.y = G_LARGER(p[i].value.y, -0.8f);
			p[i].value.y = G_SMALLER(p[i].value.y, +0.8f);*/

			
			
			//apply rotation
			float theta = 0.0f;
			if (lrRotation == 1)
			{
				if (udRotation == 1)
					theta = 3.0f;
				else if (udRotation == -1)
					theta = 1.0f;
				else
					theta = 2.0f;

			}
			else if (lrRotation == -1)
			{
				if (udRotation == 1)
					theta = 5.0f;
				else if (udRotation == -1)
					theta = 7.0f;
				else
					theta = 6.0f;
			}
			else
			{
				if (udRotation == 1)
					theta = 4.0f;
				else if(udRotation == -1)
					theta = 0.0f;
			}
			//std::cout << "Theta: " << theta  << '\t' << "Theta in Radians: " << (theta * 3.141593) / 4 << '\t' << "Direction: " << lrRotation << '\t' << udRotation << '\n';
			float thetaRads = -(theta * 3.1415926535797) / 4; //Convert into Radians!!!
			
			//GW::MATH2D::GMATRIX2F currRotation = { cos(thetaRads), -sin(thetaRads), sin(thetaRads), cos(thetaRads) };
			GW::MATH::GMATRIXF currRotation3 = { cos(thetaRads), 0, sin(thetaRads), 0,
												 0, 1, 0, 0,
												 -sin(thetaRads), 0, cos(thetaRads), 0,
												 0, 0, 0, 1 };
			//proxyM.RotateYLocalF(currRotation3, thetaRads, currRotation3);
			

			GW::MATH::GMATRIXF mat = data->levelTransforms[data->levelInstances[playerIndex].transformStart]; //copy matrix
			//GW::MATH::GMATRIXF identityMatrix = GW::MATH::GIdentityMatrixF;

			GW::MATH::GMATRIXF original = mat; //track original pos to stop movement
			
			GW::MATH::GVECTORF scaleVec = { scale, scale, scale, scale }; //read in the proper scaling
			//proxyM.ScaleLocalF(resultMatrix, scaleVec, resultMatrix); //apply scale to the matrix
			proxyM.ScaleLocalF(mat, scaleVec, mat); //apply scale to the matrix
			
			//currRotation3.row4 = { mat.row4.x, mat.row4.y, mat.row4.z, mat.row4.w }; //apply the transform to the already existing rotation matrix (currRotation3)
			//proxyM.RotateYLocalF(resultMatrix, 0.1f * 3.14f / 180.0f, resultMatrix);
			//proxyM.RotateYLocalF(mat, 0.1f * 3.14f / 180.0f, mat);
			//proxyM.MultiplyMatrixF(mat, currRotation3, mat);
			
			GW::MATH::GVECTORF vec = { xaxis * it.delta_time() * speed * (pSpeed + 0.5), 0.0f, zaxis * it.delta_time() * speed * (pSpeed + 0.5), 1.0f }; //get the velocity vector

			//proxyM.MultiplyMatrixF(currRotation3, resultMatrix, resultMatrix);

			//proxyM.TranslateLocalF(resultMatrix, vec, resultMatrix);
			proxyM.TranslateGlobalF(mat, vec, mat);
			
			mat.row1.x = currRotation3.row1.x;
			mat.row1.z = currRotation3.row1.z;
			mat.row3.x = currRotation3.row3.x;
			mat.row3.z = currRotation3.row3.z;

			//proxyM.MultiplyMatrixF(resultMatrix, mat, resultMatrix);
			//data->levelTransforms[data->levelInstances[playerIndex].transformStart].row4;

			//data->levelTransforms[data->levelInstances[playerIndex].transformStart] = resultMatrix;
			
			// affect movement from collision
			if (it.entity(i).has<CollidedWith>(flecs::Wildcard))
			{
				it.entity(i).remove<CollidedWith>(flecs::Wildcard);
				mat.row4.x = original.row4.x - xaxis * 0.1f;
				mat.row4.z = original.row4.z - zaxis * 0.1f;
				//std::cout << "wall hit"  << std::endl;
			}

			data->levelTransforms[data->levelInstances[playerIndex].transformStart] = mat;
			//o[i].value = mat;
			me[i].t = data->levelTransforms[data->levelInstances[playerIndex].transformStart];
			

			GW::MATH2D::GVECTOR2F direction = { lrRotation, udRotation };
			// fire weapon if they are in a firing state
			if (firing && perFrame) {
				perFrame = 0;
				Position offset = p[i];
				Orientation rotation = o[i];
				Velocity speed;
				speed.value = direction;
				offset.value.y += 0.05f;
				FireLasers(it.world(), offset, rotation, speed);
				it.entity(i).remove<Firing>();
			}
			else if (!firing ){
				perFrame = 1;
			}

			for (int i = 0; i <= 49; i++)
			{
				if(bulletsOut[i + 1])
					proxyM.TranslateLocalF(data->levelTransforms[data->levelInstances[bulletIndex].transformStart + i], stdVelocity, data->levelTransforms[data->levelInstances[bulletIndex].transformStart + i]);
			}
		}
		// process any cached button events after the loop (happens multiple times per frame)
		ProcessInputEvents(it.world());
	});

	game->system<Player>("Player Death System")
	.each([&](flecs::entity e, Player)
	{
		e.each<CollidedWith>([&](flecs::entity hit)
		{
			if (hit.has<Enemy>())
			{
				float scale = 99;
				GW::MATH::GVECTORF scaleVec = { scale, scale, scale, scale };
				GW::MATH::GMATRIXF mat = data->levelTransforms[data->levelInstances[playerIndex].transformStart];
				proxyM.ScaleLocalF(mat, scaleVec, mat);
				data->levelTransforms[data->levelInstances[playerIndex].transformStart] = mat;
				//playerSystem.disable();
				dead = true;
				//std::cout << "ded" << std::endl;
			}
		});
	});
	
	// Create an event cache for when the spacebar/'A' button is pressed
	pressEvents.Create(Max_Frame_Events); // even 32 is probably overkill for one frame
		
	// register for keyboard and controller events
	bufferedInput.Register(pressEvents);
	controllerInput.Register(pressEvents);

	// create the on explode handler
	onExplode.Create([this](const GW::GEvent& e) {
		AVT::PLAY_EVENT event; AVT::PLAY_EVENT_DATA eventData;
		if (+e.Read(event, eventData)) {
			// only in here if event matches
			std::cout << "Lmao got em\n";
		}
	});
	_eventPusher.Register(onExplode);

	return true;
}

// Free any resources used to run this system
bool AVT::PlayerLogic::Shutdown()
{
	playerSystem.destruct();
	game.reset();
	gameConfig.reset();

	return true;
}

// Toggle if a system's Logic is actively running
bool AVT::PlayerLogic::Activate(bool runSystem)
{
	if (playerSystem.is_alive()) {
		(runSystem) ? 
			playerSystem.enable() 
			: playerSystem.disable();
		return true;
	}
	return false;
}

bool AVT::PlayerLogic::ProcessInputEvents(flecs::world& stage)
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
bool AVT::PlayerLogic::FireLasers(flecs::world& stage, Position origin, Orientation direction, Velocity speed)
{
	// Grab the prefab for a laser round
	flecs::entity bullet;
	RetreivePrefab("Lazer Bullet", bullet);

	float scaleX = 0.02, scaleY = 0.1;

	//data->levelTransforms[data->levelInstances[bulletIndex].transformStart];	Bullet Index

	/*GW::MATH2D::GMATRIX2F rotatedBullet;
	GW::MATH2D::GMatrix2D::Scale2F(GW::MATH2D::GIdentityMatrix2F, GW::MATH2D::GVECTOR2F{ scaleX , scaleY }, rotatedBullet);
	GW::MATH2D::GMatrix2D::Multiply2F(rotatedBullet, direction.value, direction.value);*/
	
	//origin.value.x -= 0.05f;
	/*auto laser = stage.entity().is_a(bullet)
		.set<Position>(origin)
		.set<Orientation>(direction)
		.set<Velocity>(speed);
	

	// if this shot is charged
	if (chargeEnd - chargeStart >= chargeTime) {
		chargeEnd = chargeStart;
		laser.set<ChargedShot>({ 2 })
			.set<Material>({1,0,0});
	}*/

	// Create Iterator
	/*std::vector<GW::MATH::GMATRIXF>::iterator iter = data->levelTransforms.begin();*/
	// Calculate Distance using obj in models
	/*int dist = bulletIndex + data->levelInstances[bulletIndex].transformCount;*/
	//Insert Matrix into Matrix Array
	/*for (int i = 0; i < dist; i++)
	{
		iter++;
	}*/
	//Add to Array Counts
	/*data->levelTransforms.insert(iter, data->levelTransforms[data->levelInstances[bulletIndex].transformStart]);
	data->levelInstances[bulletIndex].transformCount++;

	data->levelTransforms[data->levelInstances[bulletIndex].transformStart].row4 = data->levelTransforms[data->levelInstances[playerIndex - 1].transformStart].row4;
	data->levelTransforms[data->levelInstances[bulletIndex].transformStart].row4.y += 3;*/
	

	//Actual past here
	GW::MATH::GMATRIXF bulletCopy = data->levelTransforms[data->levelInstances[playerIndex].transformStart];
	
 	data->levelTransforms[data->levelInstances[bulletIndex].transformStart + currentBullet] = bulletCopy;
	
	if (currentBullet > 50)
		currentBullet = 0;
	else 
		currentBullet++;
	bulletsOut[currentBullet] = 1;
	
	// play the sound of the Lazer prefab
	GW::AUDIO::GSound shoot = *bullet.get<GW::AUDIO::GSound>();
	shoot.Play();

	return true;
}