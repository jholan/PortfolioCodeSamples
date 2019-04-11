#pragma once

#include <vector>

#include "Engine/Core/Types.hpp"
#include "Engine/Rendering/Utility/D3D11PreDefines.hpp"

#include "Engine/Core/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"

class Window;
class RHIInstance;
class RHIOutput;

class GPUBuffer;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;

class ShaderProgram;
class ComputeShaderProgram;

class FF_RasterizerState;
class FF_DepthStencilTestState;
class FF_BlendState;

class VertexLayout;

class ShaderResourceView;
class RenderTargetView;
class DepthStencilView;
class UnorderedAccessView;

class Sampler;

class FrameBuffer;



class InputLayoutPair
{
public:
	InputLayoutPair() {};
	InputLayoutPair(const VertexLayout* layout, ID3D11InputLayout* dxLayout) : m_layout(layout), m_dxInputLayout(dxLayout) {};

	const VertexLayout*	m_layout = nullptr;
	ID3D11InputLayout*	m_dxInputLayout = nullptr;
};


class RHIDevice
{
public:
	// Creation
	RHIDevice() {};
	~RHIDevice() {};

	void					Initialize(RHIInstance* rhiInstance, ID3D11Device* device, ID3D11DeviceContext* deviceContext, IDXGIAdapter* gpu, uint gpuIndex);
	void					Destroy();

	RHIOutput*				CreateOutputForWindow(Window* window);


	// IA
	void					SetPrimitiveTopology(RENDER_CONSTANT topology);
	
	void					BindVertexBuffer(const VertexBuffer& vertexBuffer);
	void					BindIndexBuffer(const IndexBuffer& indexBuffer);
	
	void					BindInputLayout(const VertexLayout* layout);


	// Programmable
	void					BindConstantBuffer(RENDER_CONSTANT constantBufferBindPoint, ConstantBuffer& constantBuffer);

	void					BindShaderProgram(const ShaderProgram& shaderProgram);
	void					UnbindShaderProgram();

	void					BindShaderResourceView(RENDER_CONSTANT bindPoint, const ShaderResourceView& view);
	void					UnbindShaderResourceView(RENDER_CONSTANT bindPoint);
	void					UnbindShaderResourceViews(RENDER_CONSTANT bindPoint, uint count);
	void					BindSampler(RENDER_CONSTANT bindPoint, const Sampler& sampler);
	
	
	// RS
	void					BindRasterizerState(const FF_RasterizerState& rasterizerState);
	
	void					SetViewport(const Vector2& lowerLeft, const Vector2& upperRight, float windowHeight);


	// OM
	void					BindDepthStencilTestState(const FF_DepthStencilTestState& depthState);
	void					BindBlendState(const FF_BlendState& blendState);

	void					BindFramebuffer(const FrameBuffer& framebuffer);
	
	void					ClearRenderTargetView(const RenderTargetView& rtv, const RGBA& color);
	void					ClearDepthStencilView(const DepthStencilView& dsv, float depth = 1.0f);
	void					ClearStencil(const DepthStencilView& dsv, uint8 stencil = 0);


	// Invocation
	void					Draw(uint vertexCount, uint offset);
	void					DrawIndexed(uint indexCount, uint offset);


	// Compute
	void					BindComputeShader(const ComputeShaderProgram& computeShaderProgram);

	void					BindUnorderedAccessView(uint bindPoint, const UnorderedAccessView& view);
	void					UnbindUnorderedAccesView(uint bindPoint);
	void					UnbindUnorderedAccesViews(RENDER_CONSTANT lowBindPoint, uint highBindPoint);

	void					DispatchCompute(uint x, uint y, uint z);


	ID3D11Device*			GetDevice() const;
	ID3D11DeviceContext*	GetContext() const;

	const std::vector<InputLayoutPair>& GetValidLayouts();


	// Debug
	// Annotation
	void					StartAnnotation(const std::string& title) const;
	void					EndAnnotation() const;


private:
	RHIInstance*			m_instance = nullptr;
	ID3D11Device*			m_device = nullptr;			// A virtual GPU, it is used to make resources
	ID3D11DeviceContext*	m_deviceContext = nullptr;	// Generates rendering commands
	IDXGIAdapter*			m_gpu = nullptr;
	uint					m_gpuIndex;

	std::vector<InputLayoutPair> m_validVertexInputLayouts;

	// Debug
	ID3DUserDefinedAnnotation* m_annotator;
};