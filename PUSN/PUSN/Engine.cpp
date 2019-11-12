#include "Engine.h"

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	timer.Start();
	this->windowWidth = width;
	this->windowHeight = height;
	this->guiData = shared_ptr<GuiData>(new GuiData());

	char cCurrentPath[FILENAME_MAX];
	_getcwd(cCurrentPath, sizeof(cCurrentPath));
	this->path = std::string(cCurrentPath) + "\\Paths\\";

	if (!this->InitializeWindowAndMessageHandling(hInstance, window_title, window_class, width, height))
		return false;

	if (!this->InitializeGraphics())
		return false;

	return true;
}

void Engine::Update()
{
	float dt = timer.GetMilisecondsElapsed();
	if (!guiData->paused)
		this->millingMachine->Update(dt, this->millingMaterial.get());
	timer.Restart();

	while (!keyboard.CharBufferIsEmpty())
	{
		unsigned char ch = keyboard.ReadChar();
	}

	while (!keyboard.KeyBufferIsEmpty())
	{
		KeyboardEvent kbe = keyboard.ReadKey();
		unsigned char keycode = kbe.GetKeyCode();
	}

	while (!mouse.EventBufferIsEmpty())
	{
		MouseEvent me = mouse.ReadEvent();
		if (mouse.IsRightDown())
		{
			if (me.GetType() == MouseEvent::EventType::RAW_MOVE)
			{
				this->Camera3D.AdjustRotation(-(float)me.GetPosY() * 0.01f, 0, -(float)me.GetPosX() * 0.01f);
			}
		}
	}

	float Camera3DSpeed = 0.06f;

	if (keyboard.KeyIsPressed(VK_SHIFT))
	{
		Camera3DSpeed = 1;
	}

	if (keyboard.KeyIsPressed('W'))
	{
		this->Camera3D.AdjustPosition(this->Camera3D.GetForwardVector(true) * Camera3DSpeed * dt);
	}
	if (keyboard.KeyIsPressed('S'))
	{
		this->Camera3D.AdjustPosition(this->Camera3D.GetBackwardVector(true) * Camera3DSpeed * dt);
	}
	if (keyboard.KeyIsPressed('A'))
	{
		this->Camera3D.AdjustPosition(this->Camera3D.GetLeftVector(true) * Camera3DSpeed * dt);
	}
	if (keyboard.KeyIsPressed('D'))
	{
		this->Camera3D.AdjustPosition(this->Camera3D.GetRightVector(true) * Camera3DSpeed * dt);
	}
	if (keyboard.KeyIsPressed('Q'))
	{
		this->Camera3D.AdjustPosition(0.0f, 0.0f, Camera3DSpeed * dt);
	}
	if (keyboard.KeyIsPressed('E'))
	{
		this->Camera3D.AdjustPosition(0.0f, 0.0f, -Camera3DSpeed * dt);
	}

	if (keyboard.KeyIsPressed('C'))
	{
		XMVECTOR lightPosition = this->Camera3D.GetPositionVector();
		lightPosition += this->Camera3D.GetForwardVector();
		this->light.SetPosition(lightPosition);
		this->light.SetRotation(this->Camera3D.GetRotationFloat3());
	}

}

void Engine::RenderFrame()
{
	cb_ps_light.data.dynamicLightColor = light.lightColor;
	cb_ps_light.data.dynamicLightStrength = light.lightStrength;
	cb_ps_light.data.dynamicLightPosition = light.GetPositionFloat3();
	cb_ps_light.data.dynamicLightAttenuation_a = light.attenuation_a;
	cb_ps_light.data.dynamicLightAttenuation_b = light.attenuation_b;
	cb_ps_light.data.dynamicLightAttenuation_c = light.attenuation_c;
	cb_ps_light.ApplyChanges();
	deviceContext->PSSetConstantBuffers(0, 1, cb_ps_light.GetAddressOf());

	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	deviceContext->ClearRenderTargetView(defaultRenderTargetView.Get(), bgcolor);
	deviceContext->ClearDepthStencilView(defaultDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->RSSetState(defaultRasterizerState.Get());
	deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	deviceContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());


	////sprite mask
	//deviceContext->OMSetDepthStencilState(depthStencilState_drawMask.Get(), 0);
	//deviceContext->IASetInputLayout(vertexshader_2d.GetInputLayout());
	//deviceContext->PSSetShader(pixelshader_2d_discard.GetShader(), NULL, 0);
	//deviceContext->VSSetShader(vertexshader_2d.GetShader(), NULL, 0);
	//sprite.Draw(camera2D.GetWorldMatrix() * camera2D.GetOrthoMatrix());


	deviceContext->VSSetShader(vertexshader.GetShader(), NULL, 0);
	deviceContext->PSSetShader(pixelshader.GetShader(), NULL, 0);
	deviceContext->IASetInputLayout(vertexshader.GetInputLayout());
	//deviceContext->OMSetDepthStencilState(depthStencilState_applyMask.Get(), 0);
	deviceContext->OMSetDepthStencilState(defaultDepthStencilState.Get(), 0);

	{
		//gameObject.Draw(Camera3D.GetViewMatrix() * Camera3D.GetProjectionMatrix());
	}
	{
		//deviceContext->PSSetShader(pixelshader_nolight.GetShader(), NULL, 0);
		//light.Draw(Camera3D.GetViewMatrix() * Camera3D.GetProjectionMatrix());
	}
	//-------

	//deviceContext->IASetInputLayout(my_vs.GetInputLayout());
	//deviceContext->OMSetDepthStencilState(depthStencilState.Get(), 0);

	//deviceContext->RSSetState(rasterizerState.Get());
	//deviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
	//deviceContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());

	//deviceContext->PSSetShaderResources(0, 1, material_srv.GetAddressOf());*/

	//millingCutterMesh->Draw();
	//millingMaterial->Randomize();
	//millingMaterial->UpdateVertexBuffer();
	RenderMilling();
	RenderGui();
	RenderFPS();

	this->swapchain->Present(0, NULL);
}

void Engine::RenderFPS() {
	//Draw Text
	static int fpsCounter = 0;
	static std::string fpsString = "FPS: 0";
	fpsCounter += 1;
	if (fpsTimer.GetMilisecondsElapsed() > 1000.0)
	{
		fpsString = "FPS: " + std::to_string(fpsCounter);
		fpsCounter = 0;
		fpsTimer.Restart();
	}
	spriteBatch->Begin();
	spriteFont->DrawString(spriteBatch.get(), StringHelper::StringToWide(fpsString).c_str(), DirectX::XMFLOAT2(0, 0), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
	spriteBatch->End();
}

void Engine::RenderMilling()
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->VSSetShader(my_vs.GetShader(), NULL, 0);
	deviceContext->PSSetShader(my_ps.GetShader(), NULL, 0);

	deviceContext->PSSetConstantBuffers(0, 1, cb_ps_color.GetAddressOf());
	deviceContext->VSSetConstantBuffers(0, 1, cb_vs_vertexshader.GetAddressOf());


	cb_vs_vertexshader.data.worldMatrix = XMMatrixIdentity();
	cb_vs_vertexshader.data.wvpMatrix = Camera3D.GetViewMatrix() * Camera3D.GetProjectionMatrix();
	cb_vs_vertexshader.ApplyChanges();

	cb_ps_color.data.color = { 0.8f, 0.7f, 0.2f };
	cb_ps_color.ApplyChanges();

	if (guiData->wireframe)
		deviceContext->RSSetState(rasterizerStateWireFrame.Get());
	else
		deviceContext->RSSetState(nullptr);

	if (guiData->flatShading)
		deviceContext->GSSetShader(my_gs.GetShader(), NULL, 0);

	millingMaterial->Draw();
	deviceContext->RSSetState(nullptr);

	deviceContext->GSSetShader(NULL, NULL, 0);


	cb_ps_color.data.color = { 0.7f, 0.7f, 0.7f };
	cb_ps_color.ApplyChanges();

	cb_vs_vertexshader.data.worldMatrix = millingMachine->millingCutterMesh->transformMatrix;
	cb_vs_vertexshader.data.wvpMatrix = millingMachine->millingCutterMesh->transformMatrix * Camera3D.GetViewMatrix() * Camera3D.GetProjectionMatrix();
	cb_vs_vertexshader.ApplyChanges();
	millingMachine->millingCutterMesh->Draw();

	if (guiData->showPath) {
		cb_ps_color.data.color = { 1, 0.2f, 0.2f };
		cb_ps_color.ApplyChanges();

		cb_vs_vertexshader.data.worldMatrix = millingMachine->pathMesh->transformMatrix;
		cb_vs_vertexshader.data.wvpMatrix = millingMachine->pathMesh->transformMatrix * Camera3D.GetViewMatrix() * Camera3D.GetProjectionMatrix();
		cb_vs_vertexshader.ApplyChanges();
		millingMachine->pathMesh->Draw();
	}
}

void Engine::RenderGui() {

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::SetNextWindowSize(ImVec2(500, 700), ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(10, 50), ImGuiCond_Once);

	if (!ImGui::Begin("Main Panel"))
	{
		ImGui::End();
		return;
	}

	const int buffSize = 256;
	//static char buf[buffSize] = "t1.k16";
	static char buf[buffSize] = "elephant\\test1.k16";
	ImGui::InputText("path", buf, buffSize);
	if (ImGui::Button("Load configuration")) {
		millingMachine->LoadDataFromFile(path + std::string(buf));
		guiData->toolRadius = millingMachine->cutRadius;
		guiData->flat = millingMachine->flatCut;
		millingMachine->materialDepthViolated = false;
		millingMachine->toolDepthViolated = false;
		millingMachine->Reset();
		guiData->paused = true;
	}

	ImGui::Separator();

	if (ImGui::Button("Reset material")) {
		millingMaterial->Reset();
		guiData->paused = true;
	}

	ImGui::SameLine();

	if (ImGui::Button("To end"))
		while (!millingMachine->finished)
			millingMachine->Update(100000, millingMaterial.get());

	ImGui::SameLine();

	if (guiData->paused) {
		if (ImGui::Button("Start"))
			guiData->paused = false;
	}
	else {
		if (ImGui::Button("Pause"))
			guiData->paused = true;
	}

	ImGui::Separator();

	ImGui::SliderFloat("size x", &guiData->size.x, 50, 300);
	ImGui::SliderFloat("size y", &guiData->size.y, 50, 300);
	ImGui::SliderFloat("size z", &guiData->size.z, 20, 100);
	ImGui::SliderInt("grid x", &guiData->gridX, 50, 500);
	ImGui::SliderInt("grid y", &guiData->gridY, 50, 500);
	ImGui::SliderFloat("radius", &guiData->toolRadius, 5, 20);
	ImGui::Checkbox("flat cut", &guiData->flat);
	if (ImGui::Button("Apply")) {
		millingMaterial->Initialize(guiData->size, guiData->gridX, guiData->gridY);
		millingMachine->SetMillingCutterMesh(guiData->toolRadius, guiData->flat);
		millingMachine->Reset();
		guiData->paused = true;
	}


	if (ImGui::Button("GeneratePaths")) {
		pathGenerator->GeneratePaths();
	}

	ImGui::Separator();

	ImGui::SliderFloat("speed", &millingMachine->speed, 0.1, 3);
	ImGui::SliderFloat("step size", &millingMachine->stepSize, 0.1, 5);
	ImGui::SliderFloat("max material depth", &millingMachine->materialDepth, 5, 100);
	ImGui::SliderFloat("max tool depth", &millingMachine->toolDepth, 5, 100);
	ImGui::Checkbox("wirerfme only", &guiData->wireframe);
	ImGui::Checkbox("flat shading", &guiData->flatShading);
	ImGui::Checkbox("show path", &guiData->showPath);

	ImGui::Separator();

	if (millingMachine->toolDepthViolated)
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Tool depth violated!");

	if (millingMachine->materialDepthViolated)
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Material depth violated!");

	if (millingMachine->millingViolated)
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Milling direction violated!");

	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

bool Engine::InitializeGraphics()
{
	this->fpsTimer.Start();

	if (!InitializeDirectX())
		return false;

	if (!InitializeShaders())
		return false;

	if (!InitializeScene())
		return false;

	InitGui();
	InitMilling();

	return true;
}

bool Engine::InitializeDirectX()
{
	try
	{
		HWND hwnd = this->GetHWND();
		std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

		if (adapters.size() < 1)
		{
			ErrorLogger::Log("No IDXGI Adapters found.");
			return false;
		}

		DXGI_SWAP_CHAIN_DESC scd = { 0 };

		scd.BufferDesc.Width = this->windowWidth;
		scd.BufferDesc.Height = this->windowHeight;
		scd.BufferDesc.RefreshRate.Numerator = 60;
		scd.BufferDesc.RefreshRate.Denominator = 1;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;

		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.BufferCount = 1;
		scd.OutputWindow = hwnd;
		scd.Windowed = TRUE;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		HRESULT hr;
		hr = D3D11CreateDeviceAndSwapChain(adapters[0].pAdapter, //IDXGI Adapter
			D3D_DRIVER_TYPE_UNKNOWN,
			NULL, //FOR SOFTWARE DRIVER TYPE
			NULL, //FLAGS FOR RUNTIME LAYERS
			NULL, //FEATURE LEVELS ARRAY
			0, //# OF FEATURE LEVELS IN ARRAY
			D3D11_SDK_VERSION,
			&scd, //Swapchain description
			this->swapchain.GetAddressOf(), //Swapchain Address
			this->device.GetAddressOf(), //Device Address
			NULL, //Supported feature level
			this->deviceContext.GetAddressOf()); //Device Context Address

		COM_ERROR_IF_FAILED(hr, "Failed to create device and swapchain.");

		//Create & set the Viewport
		CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(this->windowWidth), static_cast<float>(this->windowHeight));;
		this->deviceContext->RSSetViewports(1, &viewport);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		hr = this->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		COM_ERROR_IF_FAILED(hr, "GetBuffer Failed.");

		hr = this->device->CreateRenderTargetView(backBuffer.Get(), NULL, this->defaultRenderTargetView.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create render target view.");

		//Describe our Depth/Stencil Buffer
		CD3D11_TEXTURE2D_DESC depthStencilTextureDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, this->windowWidth, this->windowHeight);
		depthStencilTextureDesc.MipLevels = 1;
		depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		hr = this->device->CreateTexture2D(&depthStencilTextureDesc, NULL, this->defaultDepthStencilBuffer.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil buffer.");

		hr = this->device->CreateDepthStencilView(this->defaultDepthStencilBuffer.Get(), NULL, this->defaultDepthStencilView.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil view.");

		this->deviceContext->OMSetRenderTargets(1, this->defaultRenderTargetView.GetAddressOf(), this->defaultDepthStencilView.Get());

		//Create depth stencil state
		CD3D11_DEPTH_STENCIL_DESC depthstencildesc(D3D11_DEFAULT);
		depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

		hr = this->device->CreateDepthStencilState(&depthstencildesc, this->defaultDepthStencilState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil state.");

		//CD3D11_DEPTH_STENCIL_DESC depthstencildesc_drawMask(D3D11_DEFAULT);
		//depthstencildesc_drawMask.DepthEnable = FALSE;
		//depthstencildesc_drawMask.StencilEnable = TRUE;

		//depthstencildesc_drawMask.BackFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
		//depthstencildesc_drawMask.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		//depthstencildesc_drawMask.BackFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		//depthstencildesc_drawMask.BackFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;

		//depthstencildesc_drawMask.FrontFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
		//depthstencildesc_drawMask.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		//depthstencildesc_drawMask.FrontFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		//depthstencildesc_drawMask.FrontFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_INCR_SAT;

		//hr = this->device->CreateDepthStencilState(&depthstencildesc_drawMask, this->depthStencilState_drawMask.GetAddressOf());
		//COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil state for drawing mask.");

		//CD3D11_DEPTH_STENCIL_DESC depthstencildesc_applyMask(D3D11_DEFAULT);
		//depthstencildesc_applyMask.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
		//depthstencildesc_applyMask.StencilEnable = TRUE;

		//depthstencildesc_applyMask.BackFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
		//depthstencildesc_applyMask.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		//depthstencildesc_applyMask.BackFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		//depthstencildesc_applyMask.BackFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;

		//depthstencildesc_applyMask.FrontFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
		//depthstencildesc_applyMask.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		//depthstencildesc_applyMask.FrontFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		//depthstencildesc_applyMask.FrontFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;

		//hr = this->device->CreateDepthStencilState(&depthstencildesc_applyMask, this->depthStencilState_applyMask.GetAddressOf());
		//COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil state for applying mask.");



		////Create Rasterizer State
		//CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
		//hr = this->device->CreateRasterizerState(&rasterizerDesc, this->rasterizerState.GetAddressOf());
		//COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state.");

		////Create Rasterizer State for culling front
		//CD3D11_RASTERIZER_DESC rasterizerDesc_CullFront(D3D11_DEFAULT);
		//rasterizerDesc_CullFront.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
		//hr = this->device->CreateRasterizerState(&rasterizerDesc_CullFront, this->rasterizerState_CullFront.GetAddressOf());
		//COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state.");

		////Create Blend State
		//D3D11_RENDER_TARGET_BLEND_DESC rtbd = { 0 };
		//rtbd.BlendEnable = true;
		//rtbd.SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
		//rtbd.DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
		//rtbd.BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		//rtbd.SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
		//rtbd.DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ZERO;
		//rtbd.BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		//rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;

		//D3D11_BLEND_DESC blendDesc = { 0 };
		//blendDesc.RenderTarget[0] = rtbd;

		//hr = this->device->CreateBlendState(&blendDesc, this->blendState.GetAddressOf());
		//COM_ERROR_IF_FAILED(hr, "Failed to create blend state.");

		spriteBatch = std::make_unique<DirectX::SpriteBatch>(this->deviceContext.Get());
		spriteFont = std::make_unique<DirectX::SpriteFont>(this->device.Get(), L"Data\\Fonts\\comic_sans_ms_16.spritefont");

		//Create sampler description for sampler state
		CD3D11_SAMPLER_DESC sampDesc(D3D11_DEFAULT);
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		hr = this->device->CreateSamplerState(&sampDesc, this->samplerState.GetAddressOf()); //Create sampler state
		COM_ERROR_IF_FAILED(hr, "Failed to create sampler state.");
	}
	catch (COMException & exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}
	return true;
}

bool Engine::InitializeShaders()
{
	//2d shaders
	D3D11_INPUT_ELEMENT_DESC layout2D[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
	};

	UINT numElements2D = ARRAYSIZE(layout2D);

	if (!vertexshader_2d.Initialize(this->device, L"vertexshader_2d.cso", layout2D, numElements2D))
		return false;

	if (!pixelshader_2d.Initialize(this->device, L"pixelshader_2d.cso"))
		return false;

	if (!pixelshader_2d_discard.Initialize(this->device, L"pixelshader_2d_discard.cso"))
		return false;

	//3d shaders
	D3D11_INPUT_ELEMENT_DESC layout3D[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"NORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
	};

	UINT numElements3D = ARRAYSIZE(layout3D);

	if (!vertexshader.Initialize(this->device, L"vertexshader.cso", layout3D, numElements3D))
		return false;

	if (!pixelshader.Initialize(this->device, L"pixelshader.cso"))
		return false;

	if (!pixelshader_nolight.Initialize(this->device, L"pixelshader_nolight.cso"))
		return false;

	//-------
	if (!my_vs.Initialize(this->device, L"my_vs.cso", layout3D, numElements3D))
		return false;

	if (!my_ps.Initialize(this->device, L"my_ps.cso"))
		return false;

	if (!my_gs.Initialize(this->device, L"my_gs.cso"))
		return false;

	return true;
}

bool Engine::InitializeScene()
{
	try
	{
		//Load Texture
		HRESULT hr = DirectX::CreateWICTextureFromFile(this->device.Get(), L"Data\\Textures\\seamless_grass.jpg", nullptr, grassTexture.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create wic texture from file.");

		hr = DirectX::CreateWICTextureFromFile(this->device.Get(), L"Data\\Textures\\pinksquare.jpg", nullptr, pinkTexture.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create wic texture from file.");

		hr = DirectX::CreateWICTextureFromFile(this->device.Get(), L"Data\\Textures\\seamless_pavement.jpg", nullptr, pavementTexture.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create wic texture from file.");

		//Initialize Constant Buffer(s)
		hr = this->cb_vs_vertexshader_2d.Initialize(this->device.Get(), this->deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize 2d constant buffer.");

		hr = this->cb_vs_vertexshader.Initialize(this->device.Get(), this->deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

		hr = this->cb_ps_color.Initialize(this->device.Get(), this->deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

		hr = this->cb_ps_light.Initialize(this->device.Get(), this->deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

		this->cb_ps_light.data.ambientLightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		this->cb_ps_light.data.ambientLightStrength = 1.0f;


		if (!light.Initialize(this->device.Get(), this->deviceContext.Get(), this->cb_vs_vertexshader))
			return false;

		Camera3D.SetPosition(0.0f, -100.0f, 200.0f);
		//Camera3D.SetRotation(1.2f, 0.0f, 0.0f);
		Camera3D.SetProjectionValues(90.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.1f, 3000.0f);


		//---------------------------------------------
		/*int N = 10;

		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = N;
		desc.Height = N;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;

		hr = device->CreateTexture2D(&desc, nullptr, material_texture.GetAddressOf());

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		deviceContext->Map(material_texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		unsigned int* data = ((unsigned int*)mappedResource.pData);

		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				int a = 255, r = 255, g = 255, b = 255;
				data[(j + i * N)] = (a << 24) + (r << 16) + (g << 8) + b;
			}
		}

		deviceContext->Unmap(material_texture.Get(), 0);

		hr = device->CreateShaderResourceView(material_texture.Get(), nullptr, &material_srv);*/

		//Create Rasterizer State
		CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
		rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
		hr = this->device->CreateRasterizerState(&rasterizerDesc, this->rasterizerStateWireFrame.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state.");
	}
	catch (COMException & exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}
	return true;
}

void Engine::InitGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(this->GetHWND());
	ImGui_ImplDX11_Init(this->device.Get(), this->deviceContext.Get());
	ImGui::StyleColorsDark();
}

void Engine::InitMilling()
{
	millingMaterial = std::shared_ptr<MillingMaterial>(new MillingMaterial(device.Get(), deviceContext.Get()));
	millingMaterial->Initialize(guiData->size, guiData->gridX, guiData->gridY);
	millingMachine = std::shared_ptr<MillingMachine>(new MillingMachine(device.Get(), deviceContext.Get()));
	//millingMachine->LoadDataFromFile(path + "\\t1.k16");
	millingMachine->LoadDataFromFile(path + "elephant\\test1.k16");

	guiData->toolRadius = millingMachine->cutRadius;
	guiData->flat = millingMachine->flatCut;
	millingMachine->Reset();

	pathGenerator = std::shared_ptr<PathGenerator>(new PathGenerator(millingMaterial.get()));
}
