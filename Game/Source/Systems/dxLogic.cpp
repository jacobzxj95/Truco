#include "dxLogic.h"

//Bryan's Code
using namespace AVT; // Example Space Game

bool AVT::dxLogic::Init(
	std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::GRAPHICS::GDirectX11Surface _d3d11,
	GW::INPUT::GInput _immediateInput,
	GW::SYSTEM::GWindow _window,
	Level_Data& _dataOrientedLoader,
	DevVar& _dev)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	d3d11 = _d3d11;
	input = _immediateInput;
	window = _window;
	dataOrientedLoader = &_dataOrientedLoader;
	dev = &_dev;
	//zFar = 100.0f;
	//zNear = 0.1f;
	
	_d3d11.GetAspectRatio(aspectRatio);
	_window.GetHeight(H);
	_window.GetWidth(W);
	_window.GetClientHeight(height);
	_window.GetClientWidth(width);
	FOV = G_DEGREE_TO_RADIAN_F(65);
	enemyDeadCount = 0;

	proxy.Create();
	log.Create("../LevelLoaderLog.txt");
	log.EnableConsoleLogging(true); // mirror output to the console
	log.Log("Start Program.");

	dataOrientedLoader->LoadLevel("../Obj/Obj/Level/GameLevel.txt", "../Obj/Obj/UMS_Props", log);
	//dataOrientedLoader->LoadLevel("../Obj/Obj/Level/TestLevel.txt", "../Obj/Obj/UMS_Props", log);
	std::cout << std::endl;

//	devActive = (*readCfg).at("Player1").at("devActive").as<bool>();

	for (int i = 0; i < dataOrientedLoader->levelModels.size(); i++)
	{
		if (dataOrientedLoader->levelModels[i].filename == playerName)
		{
			playerIndex = i;
			break;
		}
	}
	GW::MATH::GMATRIXF temp = GW::MATH::GIdentityMatrixF;
	proxy.InverseF(view, temp);
	temp.row4 = dataOrientedLoader->levelTransforms[dataOrientedLoader->levelInstances[playerIndex].transformStart].row4;
	proxy.TranslateGlobalF(temp, eye, temp);
	proxy.RotateXLocalF(temp, 90.0f * 3.14f / 180.0f, temp);
	proxy.InverseF(temp, temp);
	view = temp;
	projection = GW::MATH::GIdentityMatrixF;
	//proxy.ProjectionDirectXLHF(G_DEGREE_TO_RADIAN_F(65), aspectRatio, zNear, zFar, projection);
	projection.row1.x = 2 / (right - left);
	projection.row2.y = 2 / (top - bot);
	projection.row3.z = 2 / (distant - close);
	projection.row1.w = -((right + left) / (right - left));
	projection.row2.w = -((top + bot) / (top - bot));
	projection.row3.w = -((-distant + -close) / (-distant - -close));



	// Setup all vulkan resources
	if (LoadShaders() == false)
		return false;
	if (LoadBuffers() == false)
		return false;
	if (SetupPipeline() == false)
		return false;
	// Setup drawing engine
	if (SetupDrawcalls() == false)
		return false;

	return true;
}

bool AVT::dxLogic::Activate(bool runSystem)
{
	if (startDraw.is_alive() &&
		updateDraw.is_alive() &&
		completeDraw.is_alive()) {
		if (runSystem) {
			startDraw.enable();
			updateDraw.enable();
			completeDraw.enable();
		}
		else {
			startDraw.disable();
			updateDraw.disable();
			completeDraw.disable();
		}
		return true;
	}
	return false;
}

bool AVT::dxLogic::Shutdown()
{
	startDraw.destruct();
	updateDraw.destruct();
	completeDraw.destruct();
	return true; // vulkan resource shutdown handled via GEvent in Init()
}

std::string AVT::dxLogic::ShaderAsString(const char* shaderFilePath)
{
	std::string output;
	unsigned int stringLength = 0;
	GW::SYSTEM::GFile file; file.Create();
	file.GetFileSize(shaderFilePath, stringLength);
	if (stringLength && +file.OpenBinaryRead(shaderFilePath)) {
		output.resize(stringLength);
		file.Read(&output[0], stringLength);
	}
	else
		std::cout << "ERROR: Shader Source File \"" << shaderFilePath << "\" Not Found!" << std::endl;
	return output;
}

bool AVT::dxLogic::LoadShaders()
{
	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
	std::string vertexShaderSource = (*readCfg).at("Shaders").at("vertex").as<std::string>();
	std::string pixelShaderSource = (*readCfg).at("Shaders").at("pixel").as<std::string>();
	
	if (vertexShaderSource.empty() || pixelShaderSource.empty())
		return false;

	vertexShaderSource = ShaderAsString(vertexShaderSource.c_str());
	pixelShaderSource = ShaderAsString(pixelShaderSource.c_str());

	if (vertexShaderSource.empty() || pixelShaderSource.empty())
		return false;
	
	ID3D11Device* device = nullptr;
	d3d11.GetDevice((void**)&device);
	// Create Vertex Shader
	//shaderc_compilation_result_t result = shaderc_compile_into_spv( // compile
	//	compiler, vertexShaderSource.c_str(), vertexShaderSource.length(),
	//	shaderc_vertex_shader, "main.vert", "main", options);
	//if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
	//	std::cout << "Vertex Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
	//GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
	//	(char*)shaderc_result_get_bytes(result), &vertexShader);
	//shaderc_result_release(result); // done
	
	// Create Pixel Shader
	//result = shaderc_compile_into_spv( // compile
	//	compiler, pixelShaderSource.c_str(), pixelShaderSource.length(),
	//	shaderc_fragment_shader, "main.frag", "main", options);
	//if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
	//	std::cout << "Pixel Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
	//GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
	//	(char*)shaderc_result_get_bytes(result), &pixelShader);
	//shaderc_result_release(result);

	InitializePipeline(device);

	device->Release();
	return true;
}

bool AVT::dxLogic::LoadBuffers()
{
	ID3D11Device* creator = nullptr;
	d3d11.GetDevice((void**)&creator);

	InitializeVertexBuffer(creator);
	IntializeGraphics();
	ChangeWire(creator);
	//for (int i = 0; i < max_frames; ++i) {
	//
	//	if (VK_SUCCESS != GvkHelper::create_buffer(physicalDevice, device,
	//		sizeof(INSTANCE_UNIFORMS), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	//		&uniformHandle[i], &uniformData[i]))
	//		return false;
	//		
	//	if (VK_SUCCESS != GvkHelper::write_to_buffer( device, uniformData[i], 
	//		&instanceData, sizeof(INSTANCE_UNIFORMS)))
	//		return false; 
	//}
	creator->Release();
	return true;
}

bool AVT::dxLogic::SetupPipeline()
{
	PipelineHandles cur = GetCurrentPipelineHandles();
	//ID3D11RenderTargetView* renderPass = nullptr;
	//d3d11.GetRenderTargetView((void**)&renderPass);
	// Assembly State
	// Vertex Input State
	// Viewport State (we still need to set this up even though we will overwrite the values)
	//VkViewport viewport = {
 //       0, 0, static_cast<float>(1920), static_cast<float>(1080), 0, 1
 //   };
 //   VkRect2D scissor = { {0, 0}, {1920, 1080} }; // we will overwrite this in Draw
	//VkPipelineViewportStateCreateInfo viewport_create_info = {};
	//viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	//viewport_create_info.viewportCount = 1;
	//viewport_create_info.pViewports = &viewport;
	//viewport_create_info.scissorCount = 1;
	//viewport_create_info.pScissors = &scissor;
	// Rasterizer State
	//VkPipelineRasterizationStateCreateInfo rasterization_create_info = {};
	//rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	//rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
	//rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
	//rasterization_create_info.lineWidth = 1.0f;
	//rasterization_create_info.cullMode = VK_CULL_MODE_NONE; // disable culling
	//rasterization_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	//rasterization_create_info.depthClampEnable = VK_FALSE;
	//rasterization_create_info.depthBiasEnable = VK_FALSE;
	//rasterization_create_info.depthBiasClamp = 0.0f;
	//rasterization_create_info.depthBiasConstantFactor = 0.0f;
	//rasterization_create_info.depthBiasSlopeFactor = 0.0f;
	// Multisampling State
	//RenderTargetCreation
	//VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
	//multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	//multisample_create_info.sampleShadingEnable = VK_FALSE;
	//multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	//multisample_create_info.minSampleShading = 1.0f;
	//multisample_create_info.pSampleMask = VK_NULL_HANDLE;
	//multisample_create_info.alphaToCoverageEnable = VK_FALSE;
	//multisample_create_info.alphaToOneEnable = VK_FALSE;
	// Depth-Stencil State
	//Bullet hole thing
	//VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
	//depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	//depth_stencil_create_info.depthTestEnable = VK_TRUE;
	//depth_stencil_create_info.depthWriteEnable = VK_TRUE;
	//depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
	//depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
	//depth_stencil_create_info.minDepthBounds = 0.0f;
	//depth_stencil_create_info.maxDepthBounds = 1.0f;
	//depth_stencil_create_info.stencilTestEnable = VK_FALSE;
	// Color Blending Attachment & State
	//How
	//VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
	//color_blend_attachment_state.colorWriteMask = 0xF;
	//color_blend_attachment_state.blendEnable = VK_FALSE;
	//color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
	//color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
	//color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
	//color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	//color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
	//color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
	//Who
	//VkPipelineColorBlendStateCreateInfo color_blend_create_info = {};
	//color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	//color_blend_create_info.logicOpEnable = VK_FALSE;
	//color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
	//color_blend_create_info.attachmentCount = 1;
	//color_blend_create_info.pAttachments = &color_blend_attachment_state;
	//color_blend_create_info.blendConstants[0] = 0.0f;
	//color_blend_create_info.blendConstants[1] = 0.0f;
	//color_blend_create_info.blendConstants[2] = 0.0f;
	//color_blend_create_info.blendConstants[3] = 0.0f;
	// Dynamic State
	// Descriptor Setup
	// In this scenario we have the same descriptorSetLayout for both shaders...
	// However, many times you would want seperate layouts for each since they tend to have different needs
	// Descriptor layout
	// Create a descriptor pool!
	// Create a descriptorSet for each uniform buffer!
	// link descriptor sets to uniform buffers (one for each bufferimage)
	// Descriptor pipeline layout
	// Pipeline State... (FINALLY)

	SetUpPipeline(cur);
	ReleasePipelineHandles(cur);
	return true;
}

bool AVT::dxLogic::SetupDrawcalls() // I SCREWED THIS UP MAKES SO MUCH SENSE NOW
{
	// create a unique entity for the renderer (just a Tag)
	// this only exists to ensure we can create systems that will run only once per frame. 
	struct RenderingSystem {}; // local definition so we control iteration counts
	game->entity("Rendering System").add<RenderingSystem>();
	// an instanced renderer is complex and needs to run additional system code once per frame
	// to do this I create 3 systems:
	// A pre-update system, that runs only once (using our Tag above)
	// An update system that iterates over all renderable components (may run multiple times)
	// A post-update system that also runs only once rendering all collected data

	// only happens once per frame
	startDraw = game->system<RenderingSystem>().kind(flecs::PreUpdate)
		.each([this](flecs::entity e, RenderingSystem& s) {
		// reset the draw counter only once per frame
		draw_counter = 0; 
	});
	// may run multiple times per frame, will run after startDraw
	updateDraw = game->system<Position, Orientation, Material>().kind(flecs::OnUpdate)
		.each([this](flecs::entity e, Position& p, Orientation& o, Material& m) {
		// copy all data to our instancing array
		int i = draw_counter; 
		instanceData.instance_transforms[i] = GW::MATH::GIdentityMatrixF;
		instanceData.instance_transforms[i].row4.x = p.value.x;
		instanceData.instance_transforms[i].row4.y = p.value.y;
		// transfer 2D orientation to 4x4
		instanceData.instance_transforms[i].row1.x = o.value.row1.x;
		instanceData.instance_transforms[i].row1.y = o.value.row1.y;
		instanceData.instance_transforms[i].row2.x = o.value.row2.x;
		instanceData.instance_transforms[i].row2.y = o.value.row2.y;
		// set color
		instanceData.instance_colors[i].x = m.diffuse.value.x;
		instanceData.instance_colors[i].y = m.diffuse.value.y;
		instanceData.instance_colors[i].z = m.diffuse.value.z;
		instanceData.instance_colors[i].w = 1; // opaque
		// increment the shared draw counter but don't go over (branchless) 
		int v = static_cast<int>(Instance_Max) - static_cast<int>(draw_counter + 2);
		// if v < 0 then 0, else 1, https://graphics.stanford.edu/~seander/bithacks.html
		int sign = 1 ^ ((unsigned int)v >> (sizeof(int) * CHAR_BIT - 1)); 
		draw_counter += sign;
	});
	// runs once per frame after updateDraw
	completeDraw = game->system<RenderingSystem>().kind(flecs::PostUpdate)
		.each([this](flecs::entity e, RenderingSystem& s) {
		// run the rendering code just once!
		// Copy data to this frame's buffer
		// grab the current Vulkan commandBuffer
		// what is the current client area dimensions?
		unsigned int width, height;
		window.GetClientWidth(width);
		window.GetClientHeight(height);
		// setup the pipeline's dynamic settings
		//VkViewport viewport = {
		//	0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1
		//};
		//ScissorRect
		//VkRect2D scissor = { {0, 0}, {width, height} };
		//vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		//vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		// Set the descriptorSet that contains the uniform buffer allocated for this framebuffer 
		// now we can draw

		//vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexHandle, offsets);

		//vkCmdDraw(commandBuffer, 4, draw_counter, 0, 0); // draw'em all!
	});
	// NOTE: I went with multi-system approach for the ease of passing lambdas with "this"
	// There is a built-in solution for this problem referred to as a "custom runner":
	// https://github.com/SanderMertens/flecs/blob/master/examples/cpp/systems/custom_runner/src/main.cpp
	// The negative is that it requires the use of a C callback which is less flexibe vs the lambda
	// you could embed what you need in the ecs and use a lookup to get it but I think that is less clean
	
	// all drawing operations have been setup
	return true;
}

void AVT::dxLogic::IntializeGraphics()
{
	ID3D11Device* creator;
	d3d11.GetDevice((void**)&creator);
	InitializeVertexBuffer(creator);
	InitializePipeline(creator);

	InitializeUI(creator);

	creator->Release();
}

void AVT::dxLogic::Intializegraphics()
{
	ID3D11Device* creator;
	d3d11.GetDevice((void**)&creator);
	InitializeVertexBuffer(creator);
	InitializePipeline(creator);

	enemyDeadCount = 0;

	creator->Release();
}

void AVT::dxLogic::InitializeVertexBuffer(ID3D11Device* creator)
{
	CreateVertexBuffer(creator, dataOrientedLoader->levelVertices.data(), sizeof(H2B::VERTEX)
		* dataOrientedLoader->levelVertices.size());
	CreateIndexBuffer(creator, dataOrientedLoader->levelIndices.data(), sizeof(UINT) * dataOrientedLoader->levelIndices.size());
	CreateConstantBuffer(creator, nullptr, sizeof(ShadeVar));
	CreateStructuredBuffer(creator, dataOrientedLoader->levelTransforms.data(),
		sizeof(GW::MATH::GMATRIXF) * renderMax);
	renderCount = dataOrientedLoader->levelTransforms.size();
}

void AVT::dxLogic::CreateVertexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
{
	D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
	CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_VERTEX_BUFFER);
	creator->CreateBuffer(&bDesc, &bData, vertexBuffer.ReleaseAndGetAddressOf());
}

void AVT::dxLogic::CreateConstantBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
{
	D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
	CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	creator->CreateBuffer(&bDesc, nullptr, constantBuffer.ReleaseAndGetAddressOf());
}

void AVT::dxLogic::CreateStructuredBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
{
	D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
	CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE,
		D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, sizeof(GW::MATH::GIdentityMatrixF));
	HRESULT hr = creator->CreateBuffer(&bDesc, &bData, structuredBuffer.ReleaseAndGetAddressOf());

	D3D11_SHADER_RESOURCE_VIEW_DESC pDesc;
	ZeroMemory(&pDesc, sizeof(pDesc));
	pDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_BUFFER;
	/*pDesc.Buffer.NumElements = dataOrientedLoader.levelTransforms.size();*/
	pDesc.Buffer.NumElements = sizeInBytes / sizeof(GW::MATH::GMATRIXF);
	creator->CreateShaderResourceView(structuredBuffer.Get(), &pDesc, tSRV.ReleaseAndGetAddressOf());
}

void AVT::dxLogic::CreateIndexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
{
	D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
	CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_INDEX_BUFFER);
	creator->CreateBuffer(&bDesc, &bData, indexBuffer.ReleaseAndGetAddressOf());
}

void AVT::dxLogic::InitializePipeline(ID3D11Device* creator)
{
	UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
	compilerFlags |= D3DCOMPILE_DEBUG;
#endif
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = CompileVertexShader(creator, compilerFlags);
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob = CompilePixelShader(creator, compilerFlags);

	CreateVertexInputLayout(creator, vsBlob);
}

Microsoft::WRL::ComPtr<ID3DBlob> AVT::dxLogic::CompileVertexShader(ID3D11Device* creator, UINT compilerFlags)
{
	std::string vertexShaderSource = ReadFileIntoString("../Shaders/Color2DInstancedVS.hlsl");

	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;

	if (SUCCEEDED(D3DCompile(vertexShaderSource.c_str(), vertexShaderSource.length(),
		nullptr, nullptr, nullptr, "main", "vs_4_0", compilerFlags, 0,
		vsBlob.ReleaseAndGetAddressOf(), errors.ReleaseAndGetAddressOf())))
	{
		creator->CreateVertexShader(vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(), nullptr, vertexShader.ReleaseAndGetAddressOf());
	}
	else
		std::cout << (char*)errors->GetBufferPointer() << std::endl;

	return vsBlob;
}

Microsoft::WRL::ComPtr<ID3DBlob> AVT::dxLogic::CompilePixelShader(ID3D11Device* creator, UINT compilerFlags)
{
	std::string pixelShaderSource = ReadFileIntoString("../Shaders/Color2DInstancedPS.hlsl");

	Microsoft::WRL::ComPtr<ID3DBlob> psBlob, errors;

	if (SUCCEEDED(D3DCompile(pixelShaderSource.c_str(), pixelShaderSource.length(),
		nullptr, nullptr, nullptr, "main", "ps_4_0", compilerFlags, 0,
		psBlob.ReleaseAndGetAddressOf(), errors.ReleaseAndGetAddressOf())))
	{
		creator->CreatePixelShader(psBlob->GetBufferPointer(),
			psBlob->GetBufferSize(), nullptr, pixelShader.ReleaseAndGetAddressOf());
	}
	else
		std::cout << (char*)errors->GetBufferPointer() << std::endl;

	return psBlob;
}

void AVT::dxLogic::CreateVertexInputLayout(ID3D11Device* creator, Microsoft::WRL::ComPtr<ID3DBlob>& vsBlob)
{
	D3D11_INPUT_ELEMENT_DESC format[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NRM", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	creator->CreateInputLayout(format, ARRAYSIZE(format),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		vertexFormat.ReleaseAndGetAddressOf());
}

PipelineHandles AVT::dxLogic::GetCurrentPipelineHandles()
{
	PipelineHandles retval;
	auto r = d3d11.GetImmediateContext((void**)&retval.context);
	d3d11.GetRenderTargetView((void**)&retval.targetView);
	d3d11.GetDepthStencilView((void**)&retval.depthStencil);
	return retval;
}

void AVT::dxLogic::SetUpPipeline(PipelineHandles handles)
{
	SetRenderTargets(handles);
	SetVertexBuffers(handles);
	SetRaster(handles);
	SetShaders(handles);

	handles.context->IASetInputLayout(vertexFormat.Get());
	handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void AVT::dxLogic::SetRenderTargets(PipelineHandles handles)
{
	ID3D11RenderTargetView* const views[] = { handles.targetView };
	handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
}

void AVT::dxLogic::SetVertexBuffers(PipelineHandles handles)
{
	const UINT strides[] = { sizeof(H2B::VERTEX) };
	const UINT offsets[] = { 0 };
	ID3D11Buffer* const buffs[] = { vertexBuffer.Get() };
	handles.context->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
	handles.context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	//handles.context->RSSetState(mWireframeRS);
	handles.context->VSSetShaderResources(0, 1, tSRV.GetAddressOf());
	handles.context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
	handles.context->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
}

void AVT::dxLogic::SetRaster(PipelineHandles handles)
{
	handles.context->RSSetState(wireframe);
}

void AVT::dxLogic::SetShaders(PipelineHandles handles)
{
	handles.context->VSSetShader(vertexShader.Get(), nullptr, 0);
	handles.context->PSSetShader(pixelShader.Get(), nullptr, 0);
}

void AVT::dxLogic::ReleasePipelineHandles(PipelineHandles toRelease)
{
	toRelease.depthStencil->Release();
	toRelease.targetView->Release();
	toRelease.context->Release();
}

void AVT::dxLogic::DrawScene(PipelineHandles handles, GW::MATH::GMATRIXF camera) {
	D3D11_MAPPED_SUBRESOURCE gpuBuffer;
	D3D11_MAPPED_SUBRESOURCE gpuFontBuffer;

	if (!dev->devActive)
	{
		GW::MATH::GMATRIXF temp;
		proxy.InverseF(camera, temp);
		temp.row4.x = game->entity("Player One").get<Mesh>()->t.row4.x;
		temp.row4.z = game->entity("Player One").get<Mesh>()->t.row4.z;
		proxy.InverseF(temp, camera);
	}
	shadeVar.view = camera;
	shadeVar.projection = projection;
	shadeVar.lColor = lColor;
	shadeVar.lDir = lDir;
	shadeVar.ambTerm = ambTerm;
	shadeVar.mats = mats;
	shadeVar.camPos = camPos;
	HRESULT hr = handles.context->Map(structuredBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
	memcpy(gpuBuffer.pData, dataOrientedLoader->levelTransforms.data(), sizeof(GW::MATH::GMATRIXF) * dataOrientedLoader->levelTransforms.size());
	handles.context->Unmap(structuredBuffer.Get(), 0);
	for (size_t j = 0; j < dataOrientedLoader->levelModels.size(); j++)
	{
		const Level_Data::LEVEL_MODEL model = dataOrientedLoader->levelModels[j];
		const Level_Data::MODEL_INSTANCES instance = dataOrientedLoader->levelInstances[j];
		for (size_t i = 0; i < model.meshCount; i++)
		{
			const H2B::MESH& cur = dataOrientedLoader->levelMeshes[model.batchStart + i];
			shadeVar.transformStart.x = instance.transformStart;
			shadeVar.mats = dataOrientedLoader->levelMaterials[model.materialStart + cur.materialIndex].attrib;
			handles.context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			memcpy(gpuBuffer.pData, &shadeVar, sizeof(ShadeVar));
			handles.context->Unmap(constantBuffer.Get(), 0);
			handles.context->DrawIndexedInstanced(cur.drawInfo.indexCount,
				instance.transformCount,
				model.indexStart + cur.drawInfo.indexOffset,
				model.vertexStart, instance.transformStart);
		}
	}
}

void AVT::dxLogic::Render()
{
	PipelineHandles curHandles = GetCurrentPipelineHandles();
	SetUpPipeline(curHandles);
	shadeVar.view = view;
	DrawScene(curHandles, view);
	//myDur = std::chrono::duration_cast<std::chrono::microseconds>(
	//	std::chrono::steady_clock::now() - start).count() / 1000000.0f;
	ReleasePipelineHandles(curHandles);
}

std::vector<Sprite>	AVT::dxLogic::LoadHudFromXML(std::string filepath)
{
	std::vector<Sprite> result;

	tinyxml2::XMLDocument document;
	tinyxml2::XMLError error_message = document.LoadFile(filepath.c_str());
	if (error_message != tinyxml2::XML_SUCCESS)
	{
		std::cout << "XML file [" + filepath + "] did not load properly." << std::endl;
		return std::vector<Sprite>();
	}

	std::string name = document.FirstChildElement("hud")->FindAttribute("name")->Value();
	GW::MATH2D::GVECTOR2F screen_size;
	screen_size.x = atof(document.FirstChildElement("hud")->FindAttribute("width")->Value());
	screen_size.y = atof(document.FirstChildElement("hud")->FindAttribute("height")->Value());

	tinyxml2::XMLElement* current = document.FirstChildElement("hud")->FirstChildElement("element");
	while (current)
	{
		Sprite s = Sprite();
		name = current->FindAttribute("name")->Value();
		FLOAT x = atof(current->FindAttribute("pos_x")->Value());
		FLOAT y = atof(current->FindAttribute("pos_y")->Value());
		FLOAT sx = atof(current->FindAttribute("scale_x")->Value());
		FLOAT sy = atof(current->FindAttribute("scale_y")->Value());
		FLOAT r = atof(current->FindAttribute("rotation")->Value());
		FLOAT d = atof(current->FindAttribute("depth")->Value());
		GW::MATH2D::GVECTOR2F s_min, s_max;
		s_min.x = atof(current->FindAttribute("sr_x")->Value());
		s_min.y = atof(current->FindAttribute("sr_y")->Value());
		s_max.x = atof(current->FindAttribute("sr_w")->Value());
		s_max.y = atof(current->FindAttribute("sr_h")->Value());
		UINT tid = atoi(current->FindAttribute("textureID")->Value());

		s.SetName(name);
		s.SetScale(sx, sy);
		s.SetPosition(x, y);
		s.SetRotation(r);
		s.SetDepth(d);
		s.SetScissorRect({ s_min, s_max });
		s.SetTextureIndex(tid);

		result.push_back(s);

		current = current->NextSiblingElement();
	}
	return result;
}

SPRITE_DATA AVT::dxLogic::UpdateSpriteConstantBufferData(const Sprite& s)
{
	SPRITE_DATA temp = { 0 };
	temp.pos_scale.x = s.GetPosition().x;
	temp.pos_scale.y = s.GetPosition().y;
	temp.pos_scale.z = s.GetScale().x;
	temp.pos_scale.w = s.GetScale().y;
	temp.rotation_depth.x = s.GetRotation();
	temp.rotation_depth.y = s.GetDepth();
	return temp;
}

SPRITE_DATA AVT::dxLogic::UpdateTextConstantBufferData(const Text& s)
{
	SPRITE_DATA temp = { 0 };
	temp.pos_scale.x = s.GetPosition().x;
	temp.pos_scale.y = s.GetPosition().y;
	temp.pos_scale.z = s.GetScale().x;
	temp.pos_scale.w = s.GetScale().y;
	temp.rotation_depth.x = s.GetRotation();
	temp.rotation_depth.y = s.GetDepth();
	return temp;
}

void AVT::dxLogic::CreateVertexFontBuffer(ID3D11Device* creator)
{
	float verts[] =
	{
		-1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, 1.0f, 1.0f
	};

	D3D11_SUBRESOURCE_DATA vbData = { verts, 0, 0 };
	CD3D11_BUFFER_DESC vbDesc(sizeof(verts), D3D11_BIND_VERTEX_BUFFER);
	hr = creator->CreateBuffer(&vbDesc, &vbData, vertexUIBuffer.ReleaseAndGetAddressOf());
}

void AVT::dxLogic::CreateIndexFontBuffer(ID3D11Device* creator)
{
	unsigned int indices[] =
	{
		0, 1, 2,
		1, 3, 2
	};

	D3D11_SUBRESOURCE_DATA ibData = { indices, 0, 0 };
	CD3D11_BUFFER_DESC ibDesc(sizeof(indices), D3D11_BIND_INDEX_BUFFER);
	hr = creator->CreateBuffer(&ibDesc, &ibData, indexUIBuffer.ReleaseAndGetAddressOf());
}

void AVT::dxLogic::InitializeUIBuffers(ID3D11Device* creator)
{
	CreateVertexFontBuffer(creator);
	CreateIndexFontBuffer(creator);
	CreateConstantFontBuffer(creator);
}

void AVT::dxLogic::CreateConstantFontBuffer(ID3D11Device* creator)
{
	D3D11_SUBRESOURCE_DATA cbData = { &constantBufferData, 0, 0 };
	CD3D11_BUFFER_DESC cbDesc(sizeof(constantBufferData), D3D11_BIND_CONSTANT_BUFFER);
	hr = creator->CreateBuffer(&cbDesc, &cbData, constantUIBuffer.ReleaseAndGetAddressOf());
}

void AVT::dxLogic::InitializeFontPipeline(ID3D11Device* creator)
{
	UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
	compilerFlags |= D3DCOMPILE_DEBUG;
#endif
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = CompileVertexFontShader(creator, compilerFlags);
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob = CompilePixelFontShader(creator, compilerFlags);

	CreateVertexFontInputLayout(creator, vsBlob);
}

Microsoft::WRL::ComPtr<ID3DBlob> AVT::dxLogic::CompileVertexFontShader(ID3D11Device* creator, UINT compilerFlags)
{
	std::string vertexShaderSource = ReadFileIntoString("../Shaders/VertexShaderFont.hlsl");

	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;

	if (SUCCEEDED(D3DCompile(vertexShaderSource.c_str(), vertexShaderSource.length(),
		nullptr, nullptr, nullptr, "main", "vs_4_0", compilerFlags, 0,
		vsBlob.ReleaseAndGetAddressOf(), errors.ReleaseAndGetAddressOf())))
	{
		creator->CreateVertexShader(vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(), nullptr, vertexShaderFont.ReleaseAndGetAddressOf());
	}
	else
		std::cout << (char*)errors->GetBufferPointer() << std::endl;

	return vsBlob;
}

Microsoft::WRL::ComPtr<ID3DBlob> AVT::dxLogic::CompilePixelFontShader(ID3D11Device* creator, UINT compilerFlags)
{
	std::string pixelShaderSource = ReadFileIntoString("../Shaders/PixelShaderFont.hlsl");

	Microsoft::WRL::ComPtr<ID3DBlob> errors;
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob; errors.Reset();
	if (SUCCEEDED(D3DCompile(pixelShaderSource.c_str(), pixelShaderSource.length(),
		nullptr, nullptr, nullptr, "main", "ps_4_0", compilerFlags, 0,
		psBlob.GetAddressOf(), errors.GetAddressOf())))
	{
		hr = creator->CreatePixelShader(psBlob->GetBufferPointer(),
			psBlob->GetBufferSize(), nullptr, pixelShaderFont.ReleaseAndGetAddressOf());
	}
	else
		std::cout << (char*)errors->GetBufferPointer() << std::endl;

	return psBlob;
}

void AVT::dxLogic::CreateVertexFontInputLayout(ID3D11Device* creator, Microsoft::WRL::ComPtr<ID3DBlob>& vsBlob)
{
	D3D11_INPUT_ELEMENT_DESC format[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	hr = creator->CreateInputLayout(format, ARRAYSIZE(format),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		vertexFontFormat.ReleaseAndGetAddressOf());
}

void AVT::dxLogic::CreateStates(ID3D11Device* creator)
{
	CD3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = creator->CreateBlendState(&blendDesc, blendState.ReleaseAndGetAddressOf());

	CD3D11_DEPTH_STENCIL_DESC depthStencilDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = creator->CreateDepthStencilState(&depthStencilDesc, depthStencilState.ReleaseAndGetAddressOf());

	CD3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	rasterizerDesc.ScissorEnable = true;
	hr = creator->CreateRasterizerState(&rasterizerDesc, rasterizerState.ReleaseAndGetAddressOf());
}

void AVT::dxLogic::CreateDDSTexture(ID3D11Device* creator)
{
	std::wstring texture_names[] =
	{
		//L"greendragon.dds",
		//L"heart.dds",
		/*L"HUD_Sharp_backplate.dds",
		L"Health_left.dds",
		L"Health_right.dds",
		L"Mana_left.dds",
		L"Mana_right.dds",
		L"Stamina_backplate.dds",
		L"Stamina.dds",*/
		L"Center_top.dds",
		L"heart.dds",
		L"font_consolas_32.dds"
	};

	for (size_t i = 0; i < ARRAYSIZE(texture_names); i++)
	{
		std::wstring texturePath = LTEXTURES_PATH;
		texturePath += texture_names[i];
		hr = DirectX::CreateDDSTextureFromFile(creator, texturePath.c_str(), nullptr, shaderResourceView[i].ReleaseAndGetAddressOf());
	}

	CD3D11_SAMPLER_DESC samp_desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
	hr = creator->CreateSamplerState(&samp_desc, samplerState.ReleaseAndGetAddressOf());
}

void AVT::dxLogic::InitializeSprite() 
{
	//heart = Sprite();
	//heart.SetName("heart");
	//heart.SetPosition(0.0f, 0.0f);
	//heart.SetScale(0.1f, 0.1f);
	//heart.SetDepth(0.1f);
	//heart.SetTextureIndex(TEXTURE_ID::heart);
	//heart.SetScissorRect({ 0, 0, (float)width, (float)height });

	//hud.push_back(heart);

	std::string filepath = XML_PATH;
	filepath += "hud.xml";
	HUD xml_items = LoadHudFromXML(filepath);
	hud.insert(hud.end(), xml_items.begin(), xml_items.end());

	auto sortfunc = [=](const Sprite& a, const Sprite& b)
	{
		return a.GetDepth() > b.GetDepth();
	};
	std::sort(hud.begin(), hud.end(), sortfunc);
}

void AVT::dxLogic::InitializeHud(ID3D11Device* creator)
{
	// https://evanw.github.io/font-texture-generator/
	std::string filepath = XML_PATH;
	filepath += "font_consolas_32.xml";
	bool success = consolas32.LoadFromXML(filepath);

	staticText = Text();
	staticText.SetText("Score: ");
	staticText.SetFont(&consolas32);
	staticText.SetPosition(-0.75f, 0.85f);
	staticText.SetScale(0.75f, 0.75f);
	staticText.SetRotation(0.0f);
	staticText.SetDepth(0.01f);
	staticText.Update(width, height);

	// vertex buffer creation for the staticText
	const auto& staticVerts = staticText.GetVertices();
	D3D11_SUBRESOURCE_DATA svbData = { staticVerts.data(), 0, 0 };
	CD3D11_BUFFER_DESC svbDesc(sizeof(TextVertex) * staticVerts.size(), D3D11_BIND_VERTEX_BUFFER);
	hr = creator->CreateBuffer(&svbDesc, &svbData, vertexBufferStaticText.ReleaseAndGetAddressOf());

	staticText1 = Text();
	staticText1.SetText("Collected: ");
	staticText1.SetFont(&consolas32);
	staticText1.SetPosition(0.65f, 0.85f);
	staticText1.SetScale(0.75f, 0.75f);
	staticText1.SetRotation(0.0f);
	staticText1.SetDepth(0.01f);
	staticText1.Update(width, height);

	// vertex buffer creation for the staticText
	const auto& staticVerts1 = staticText1.GetVertices();
	D3D11_SUBRESOURCE_DATA svbData1 = { staticVerts1.data(), 0, 0 };
	CD3D11_BUFFER_DESC svbDesc1(sizeof(TextVertex) * staticVerts1.size(), D3D11_BIND_VERTEX_BUFFER);
	hr = creator->CreateBuffer(&svbDesc1, &svbData1, vertexBufferStaticText1.ReleaseAndGetAddressOf());

	staticText2 = Text();
	staticText2.SetText("Collect!!!");
	staticText2.SetFont(&consolas32);
	staticText2.SetPosition(0.0f, 0.0f);
	staticText2.SetScale(3.0f, 3.0f);
	staticText2.SetRotation(0.0f);
	staticText2.SetDepth(0.01f);
	staticText2.Update(width, height);

	// vertex buffer creation for the staticText
	const auto& staticVerts2 = staticText2.GetVertices();
	D3D11_SUBRESOURCE_DATA svbData2= { staticVerts2.data(), 0, 0 };
	CD3D11_BUFFER_DESC svbDesc2(sizeof(TextVertex) * staticVerts2.size(), D3D11_BIND_VERTEX_BUFFER);
	hr = creator->CreateBuffer(&svbDesc2, &svbData2, vertexBufferStaticText2.ReleaseAndGetAddressOf());

	dynamicText = Text();
	dynamicText.SetFont(&consolas32);
	dynamicText.SetPosition(-0.60f, 0.85f);
	dynamicText.SetScale(0.75f, 0.75f);
	dynamicText.SetRotation(0.0f);
	dynamicText.SetDepth(0.01f);

	CD3D11_BUFFER_DESC dvbDesc(sizeof(TextVertex) * 6 * 5000, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	hr = creator->CreateBuffer(&dvbDesc, nullptr, vertexBufferDynamicText.ReleaseAndGetAddressOf());

	dynamicText1 = Text();
	dynamicText1.SetFont(&consolas32);
	dynamicText1.SetPosition(0.90f, 0.85f);
	dynamicText1.SetScale(0.75f, 0.75f);
	dynamicText1.SetRotation(0.0f);
	dynamicText1.SetDepth(0.01f);

	CD3D11_BUFFER_DESC dvbDesc1(sizeof(TextVertex) * 6 * 5000, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	hr = creator->CreateBuffer(&dvbDesc1, nullptr, vertexBufferDynamicText1.ReleaseAndGetAddressOf());
}

void AVT::dxLogic::UpdateUI()
{
	ID3D11DeviceContext* con = nullptr;
	d3d11.GetImmediateContext((void**)&con);
	window.GetClientWidth(width);
	window.GetClientHeight(height);

	D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	con->RSSetViewports(1, &viewport);
	static auto start = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

	//float states[256] = { 0 };
	//for (size_t i = 0; i < ARRAYSIZE(states); i++)
	//{
	//	inputProxy.GetState(G_KEY_UNKNOWN + i, states[i]);
	//}
	float speed = 0.5f;
	//auto scissor_rect = hud[TEXTURE_ID::HUD_HP_LEFT].GetScissorRect();
	auto scissor_rect = hud[TEXTURE_ID::heart1].GetScissorRect();

	//if (states[G_KEY_LEFT]) { if (scissor_rect.min.x > ((width / 2) - (width / 4) - 15)) scissor_rect.min.x -= speed; }
	//if (states[G_KEY_RIGHT]) { if (scissor_rect.min.x < (width / 2))	scissor_rect.min.x += speed; }
	//hud[TEXTURE_ID::HUD_HP_LEFT].SetScissorRect(scissor_rect);
	hud[TEXTURE_ID::heart1].SetScissorRect(scissor_rect);

	float right = (width / 2);
	float left = (width / 2) - (width / 4) - 15;
	float current = scissor_rect.min.x;
	float ratio = fabs(right - current) / fabs(right - left);
	dynamicText.SetText(std::to_string((int)round(100 * enemyDeadCount)));
	dynamicText.Update(width, height);
	dynamicText1.SetText(std::to_string((int)round(ratio * 0)));
	dynamicText1.Update(width, height);
	const auto& verts = dynamicText.GetVertices();
	const auto& verts1 = dynamicText1.GetVertices();
	D3D11_MAPPED_SUBRESOURCE msr = { 0 };
	D3D11_MAPPED_SUBRESOURCE msr1 = { 0 };
	con->Map(vertexBufferDynamicText.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	memcpy(msr.pData, verts.data(), sizeof(TextVertex) * verts.size());
	con->Unmap(vertexBufferDynamicText.Get(), 0);
	con->Map(vertexBufferDynamicText1.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr1);
	memcpy(msr1.pData, verts1.data(), sizeof(TextVertex) * verts1.size());
	con->Unmap(vertexBufferDynamicText1.Get(), 0);

	con->Release();
}

void AVT::dxLogic::SetUpandDraw(ID3D11Device* creator)
{
	static auto start = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(
		std::chrono::steady_clock::now() - start).count();

	ID3D11DeviceContext* con;
	ID3D11RenderTargetView* view;
	ID3D11DepthStencilView* depth;
	d3d11.GetImmediateContext((void**)&con);
	d3d11.GetRenderTargetView((void**)&view);
	d3d11.GetDepthStencilView((void**)&depth);

	ID3D11RenderTargetView* const views[] = { view };
	con->OMSetRenderTargets(ARRAYSIZE(views), views, depth);
	con->OMSetBlendState(blendState.Get(), NULL, 0xFFFFFFFF);
	con->OMSetDepthStencilState(depthStencilState.Get(), 0xFFFFFFFF);
	con->RSSetState(rasterizerState.Get());

	const UINT strides[] = { sizeof(float) * 4 };
	const UINT offsets[] = { 0 };
	ID3D11Buffer* const buffs[] = { vertexUIBuffer.Get() };

	con->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
	con->IASetIndexBuffer(indexUIBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	con->VSSetShader(vertexShaderFont.Get(), nullptr, 0);
	con->PSSetShader(pixelShaderFont.Get(), nullptr, 0);
	con->IASetInputLayout(vertexFontFormat.Get());
	con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	con->VSSetConstantBuffers(0, 1, constantUIBuffer.GetAddressOf());

	for (size_t i = 0; i < hud.size(); i++)
	{
		const Sprite& current = hud[i];
		constantBufferData = UpdateSpriteConstantBufferData(current);
		auto scissor = current.GetScissorRect();
		D3D11_RECT rect = { (LONG)scissor.min.x, (LONG)scissor.min.y, (LONG)scissor.max.x, (LONG)scissor.max.y };
		con->RSSetScissorRects(1, &rect);
		con->UpdateSubresource(constantUIBuffer.Get(), 0, nullptr, &constantBufferData, 0, 0);
		con->PSSetShaderResources(1, 1, shaderResourceView[i].GetAddressOf());
		con->PSSetSamplers(0, 1, samplerState.GetAddressOf());
		con->DrawIndexed(6, 0, 0);
	}

	D3D11_RECT rect = { 0, 0, width, height };
	con->RSSetScissorRects(1, &rect);

	con->IASetVertexBuffers(0, 1, vertexBufferStaticText.GetAddressOf(), strides, offsets);
	con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	constantBufferData = UpdateTextConstantBufferData(staticText);
	con->PSSetShaderResources(1, 1, shaderResourceView[TEXTURE_ID::FONT_CONSOLAS].GetAddressOf());
	con->UpdateSubresource(constantUIBuffer.Get(), 0, nullptr, &constantBufferData, 0, 0);
	con->Draw(staticText.GetVertices().size(), 0);
	
	con->IASetVertexBuffers(0, 1, vertexBufferStaticText1.GetAddressOf(), strides, offsets);
	con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	constantBufferData = UpdateTextConstantBufferData(staticText1);
	con->PSSetShaderResources(1, 1, shaderResourceView[TEXTURE_ID::FONT_CONSOLAS].GetAddressOf());
	con->UpdateSubresource(constantUIBuffer.Get(), 0, nullptr, &constantBufferData, 0, 0);
	con->Draw(staticText1.GetVertices().size(), 0);

	con->IASetVertexBuffers(0, 1, vertexBufferStaticText2.GetAddressOf(), strides, offsets);
	con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	constantBufferData = UpdateTextConstantBufferData(staticText2);
	con->PSSetShaderResources(1, 1, shaderResourceView[TEXTURE_ID::FONT_CONSOLAS].GetAddressOf());
	con->UpdateSubresource(constantUIBuffer.Get(), 0, nullptr, &constantBufferData, 0, 0);
	if (elapsed <= 2.5f)
	con->Draw(staticText2.GetVertices().size(), 0);

	con->IASetVertexBuffers(0, 1, vertexBufferDynamicText.GetAddressOf(), strides, offsets);
	con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	constantBufferData = UpdateTextConstantBufferData(dynamicText);
	con->PSSetShaderResources(1, 1, shaderResourceView[TEXTURE_ID::FONT_CONSOLAS].GetAddressOf());
	con->UpdateSubresource(constantUIBuffer.Get(), 0, nullptr, &constantBufferData, 0, 0);
	con->Draw(dynamicText.GetVertices().size(), 0);

	con->IASetVertexBuffers(0, 1, vertexBufferDynamicText1.GetAddressOf(), strides, offsets);
	con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	constantBufferData = UpdateTextConstantBufferData(dynamicText1);
	con->PSSetShaderResources(1, 1, shaderResourceView[TEXTURE_ID::FONT_CONSOLAS].GetAddressOf());
	con->UpdateSubresource(constantUIBuffer.Get(), 0, nullptr, &constantBufferData, 0, 0);
	con->Draw(dynamicText1.GetVertices().size(), 0);

	depth->Release();
	view->Release();
	con->Release();
}

void AVT::dxLogic::InitializeUI(ID3D11Device* creator)
{
	d3d11.GetDevice((void**)&creator);
	InitializeUIBuffers(creator);
	InitializeFontPipeline(creator);
	CreateStates(creator);
	CreateDDSTexture(creator);
	CreateConstantFontBuffer(creator);
	InitializeSprite();
	InitializeHud(creator);
}

void AVT::dxLogic::RenderFont() 
{
	ID3D11Device* creator = nullptr;
	d3d11.GetDevice((void**)&creator);
	SetUpandDraw(creator);
	creator->Release();
}

void AVT::dxLogic::DevCam()
{
	GW::MATH::GMATRIXF temp;
	proxy.InverseF(view, temp);

	auto now = std::chrono::steady_clock::now();
	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - lUpdate).count() / 1000000.0f;
	lUpdate = now;

	float w;float s;float a;float d;
	float shift;float space;
	float p, o, l, k = 0;
	window.GetClientHeight(H);
	window.GetClientWidth(W);
	input.GetState(G_KEY_W, w);input.GetState(G_KEY_S, s);
	input.GetState(G_KEY_A, a);input.GetState(G_KEY_D, d);
	input.GetState(G_KEY_P, p);input.GetState(G_KEY_O, o);
	input.GetState(G_KEY_L, l);input.GetState(G_KEY_K, k);
	input.GetState(G_KEY_SPACE, space);
	input.GetState(G_KEY_LEFTSHIFT, shift);
	#pragma region Basic_Movement
	if (w != 0)
	{
		GW::MATH::GVECTORF vec;
		vec.x = 0; vec.y = 0; vec.z = move;
		proxy.TranslateLocalF(temp, vec, temp);
	}
	if (s != 0)
	{
		GW::MATH::GVECTORF vec;
		vec.x = 0; vec.y = 0; vec.z = -move;
		proxy.TranslateLocalF(temp, vec, temp);
	}
	if (a != 0)
	{
		GW::MATH::GVECTORF vec;
		vec.x = -move; vec.y = 0; vec.z = 0;
		proxy.TranslateLocalF(temp, vec, temp);
	}
	if (d != 0)
	{
		GW::MATH::GVECTORF vec;
		vec.x = move; vec.y = 0; vec.z = 0;
		proxy.TranslateLocalF(temp, vec, temp);
	}

	if (shift != 0)
	{
		GW::MATH::GVECTORF vec;
		vec.x = 0; vec.y = -move; vec.z = 0;
		proxy.TranslateGlobalF(temp, vec, temp);
	}
	if (space != 0)
	{
		GW::MATH::GVECTORF vec;
		vec.x = 0; vec.y = move; vec.z = 0;

		proxy.TranslateGlobalF(temp, vec, temp);
	}
	#pragma endregion
	if (p != 0)
	{
		dev->persActive = true;
		ChangePers();
	}
	if (o != 0)
	{
		dev->persActive = false;
		ChangePers();
	}
	if (l != 0)
	{
		dev->wireActive = true;
		Creator();
	}
	if (k != 0)
	{
		dev->wireActive = false;
		Creator();
	}
	GW::GReturn result = input.GetMouseDelta(mouseXCh, mouseYCh);
	if (G_PASS(result) && result != GW::GReturn::REDUNDANT)
	{
		GW::MATH::GMATRIXF pitchM; proxy.IdentityF(pitchM);
		GW::MATH::GMATRIXF yawM; proxy.IdentityF(yawM);


		float thumbSpeed = 3.14 * deltaTime;
		float TotalPitch = FOV * mouseYCh / H * (- thumbSpeed - 10);
		proxy.RotateXLocalF(pitchM, -TotalPitch, pitchM);
		proxy.MultiplyMatrixF(pitchM, temp, temp);

		float TotalYaw = FOV * mouseXCh / W * (thumbSpeed + 10);
		proxy.RotateYLocalF(yawM, TotalYaw, yawM);
		GW::MATH::GVECTORF row = temp.row4;
		proxy.MultiplyMatrixF(temp, yawM, temp);
		temp.row4 = row;
	}
	proxy.InverseF(temp, view);
}
void AVT::dxLogic::ResetCam()
{
	view = GW::MATH::GIdentityMatrixF;
	GW::MATH::GMATRIXF temp;
	proxy.InverseF(view, temp);
	temp.row4 = dataOrientedLoader->levelTransforms[dataOrientedLoader->levelInstances[playerIndex].transformStart].row4;
	proxy.TranslateGlobalF(temp, eye, temp);
	proxy.RotateXLocalF(temp, 90.0f * 3.14f / 180.0f, temp);
	proxy.InverseF(temp, view);
	dev->doOnce = false;
}
void AVT::dxLogic::ChangePers()
{
	projection = GW::MATH::GIdentityMatrixF;
	if (dev->persActive)
	{
		proxy.ProjectionDirectXLHF(G_DEGREE_TO_RADIAN_F(65), aspectRatio, zNear, zFar, projection);
	}
	if (!dev->persActive)
	{
		projection.row1.x = 2 / (right - left);
		projection.row2.y = 2 / (top - bot);
		projection.row3.z = 2 / (distant - close);
		projection.row1.w = -((right + left) / (right - left));
		projection.row2.w = -((top + bot) / (top - bot));
		projection.row3.w = -((-distant + -close) / (-distant - -close));
	}
}
void AVT::dxLogic::Creator()
{
	ID3D11Device* creator = nullptr;
	d3d11.GetDevice((void**)&creator);
	ChangeWire(creator);
}
void AVT::dxLogic::ChangeWire(ID3D11Device* creator)
{
	D3D11_RASTERIZER_DESC wireDesc;
	ZeroMemory(&wireDesc, sizeof(D3D11_RASTERIZER_DESC));
	if (dev->wireActive)
	{
		wireDesc.CullMode = D3D11_CULL_NONE;
		wireDesc.DepthClipEnable = true;
		wireDesc.FillMode = D3D11_FILL_WIREFRAME;
	}
	if (!dev->wireActive)
	{
		wireDesc.CullMode = D3D11_CULL_NONE;
		wireDesc.DepthClipEnable = true;
		wireDesc.FillMode = D3D11_FILL_SOLID;
	}
	creator->CreateRasterizerState(&wireDesc, &wireframe);
}

void AVT::dxLogic::SpawnObj(GW::MATH::GMATRIXF loc, int obj)
{
	if (renderCount < renderMax)
	{
		// Create Iterator
		std::vector<GW::MATH::GMATRIXF>::iterator iter = dataOrientedLoader->levelTransforms.begin();
		// Calculate Distance using obj in models
		int dist = obj + dataOrientedLoader->levelInstances[obj].transformCount;
		//Insert Matrix into Matrix Array
		for (int i = 0; i < dist; i++)
		{
			iter++;
		}
		//Add to Array Counts
		dataOrientedLoader->levelTransforms.insert(iter, loc);
		dataOrientedLoader->levelInstances[obj].transformCount++;
		renderCount++;
	}
}

void AVT::dxLogic::SpawnObj(GW::MATH::GMATRIXF loc, const char* objectInfoPath, const char* h2bFolderPath, std::string obj)
{
	if (renderCount < renderMax)
	{
		if (!dataOrientedLoader->ImportObj(loc, objectInfoPath, h2bFolderPath, log))
		{
			log.LogCategorized("ERROR", "Fatal Error loading object");
		}
	}
}

void AVT::dxLogic::RestartLevel()
{
		dataOrientedLoader->UnloadLevel();
		dataOrientedLoader->LoadLevel("../Obj/Obj/Level/GameLevel.txt","../Obj/Obj/UMS_Props", log);
		Intializegraphics();
}
//GUI code from bryan here
void AVT::dxLogic::SetEnemies(UINT enemiesKilled)
{
	enemyDeadCount = enemiesKilled;
} 