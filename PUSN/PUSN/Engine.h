#pragma once
#include "WindowContainer.h"
#include "Timer.h"
#include "GuiData.h"
#include "Graphics\Camera2D.h"
#include "Graphics\Camera2D.h"
#include "Graphics\Sprite.h"
#include "Graphics\RenderableGameObject.h"
#include "Graphics\Light.h"
#include "Milling\MillingMachine.h"
#include "Milling\MillingMaterial.h"
#include "Graphics\ImGui\imgui.h"
#include "Graphics\ImGui\imgui_impl_dx11.h"
#include "Graphics\ImGui\imgui_impl_win32.h"
#include <memory>

class Engine : public WindowContainer
{
public:
	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	void Update();
	void RenderFrame();
private:

	Camera3D Camera3D;
	Camera2D camera2D;
	Sprite sprite;
	RenderableGameObject gameObject;
	Light light;

	std::shared_ptr<MillingMaterial> millingMaterial;
	std::shared_ptr<MillingMachine> millingMachine;
	Timer timer;
	shared_ptr<GuiData> guiData;

	bool InitializeGraphics();
	bool InitializeDirectX(HWND hwnd);
	bool InitializeShaders();
	bool InitializeScene();
	void InitGui(HWND hwnd);

	void RenderGui();
	void RenderFPS();

	VertexShader my_vs;
	PixelShader my_ps;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerStateWireFrame;

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> defaultRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> defaultDepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> defaultDepthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> defaultDepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> defaultRasterizerState;

	VertexShader vertexshader_2d;
	VertexShader vertexshader;
	PixelShader pixelshader_2d;
	PixelShader pixelshader_2d_discard;

	PixelShader pixelshader;
	PixelShader pixelshader_nolight;
	ConstantBuffer<CB_VS_vertexshader_2d> cb_vs_vertexshader_2d;
	ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;
	ConstantBuffer<CB_PS_light> cb_ps_light;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState_drawMask;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState_applyMask;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_CullFront;

	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> spriteFont;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pinkTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> grassTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pavementTexture;

	int windowWidth = 0;
	int windowHeight = 0;
	Timer fpsTimer;
};