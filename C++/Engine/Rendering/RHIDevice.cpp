#include "Engine/Rendering/RHIDevice.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11.h>
#include <d3d11_1.h>

#include "Engine/Core/EngineCommon.h"
#include "Engine/Rendering/Utility/RenderConstants.hpp"

#include "Engine/Core/Vertex.hpp"
#include "Engine/Core/Window.hpp"

#include "Engine/Rendering/RHIOutput.hpp"

#include "Engine/Rendering/GPUBuffer.hpp"

#include "Engine/Rendering/VertexLayout.hpp"
#include "Engine/Rendering/FF_RasterizerState.hpp"
#include "Engine/Rendering/FF_DepthStencilTestState.hpp"
#include "Engine/Rendering/FF_BlendState.hpp"
#include "Engine/Rendering/ShaderProgramStage.hpp"
#include "Engine/Rendering/ShaderProgram.h"
#include "Engine/Rendering/ComputeShaderProgram.hpp"

#include "Engine/Rendering/Sampler.hpp"
#include "Engine/Rendering/TextureView.hpp"
#include "Engine/Rendering/FrameBuffer.hpp"

extern Window* g_theWindow;



void RHIDevice::Initialize(RHIInstance* instance, ID3D11Device* device, ID3D11DeviceContext* deviceContext, IDXGIAdapter* gpu, uint gpuIndex)
{
	m_instance = instance;
	m_device = device;
	m_deviceContext = deviceContext;
	m_gpu = gpu;
	m_gpuIndex = gpuIndex;

	//// Input layouts
	ShaderProgramStage tmpVertP;
	tmpVertP.Initialize(this);
	tmpVertP.LoadFromFile("Data/HLSL/InputLayoutStub_PCU.vs");
	const VertexLayout* layout_Vert_P = Vertex_3DPCU::GetLayout();
	ID3D11InputLayout* dxInputLayout_Vert_P = CreateDx11InputLayoutFromLayout(this, tmpVertP.GetBytecode(), tmpVertP.GetBytecodeSizeBytes(), layout_Vert_P);
	tmpVertP.Destroy();
	
	ShaderProgramStage tmpVertPCUTBN;
	tmpVertPCUTBN.Initialize(this);
	tmpVertPCUTBN.LoadFromFile("Data/HLSL/InputLayoutStub_PCUTBN.vs");
	const VertexLayout* layout_Vert_PCUTBN = Vertex_3DPCUTBN::GetLayout();
	ID3D11InputLayout* dxInputLayout_Vert_PCUTBN = CreateDx11InputLayoutFromLayout(this, tmpVertPCUTBN.GetBytecode(), tmpVertPCUTBN.GetBytecodeSizeBytes(), layout_Vert_PCUTBN);
	tmpVertPCUTBN.Destroy();
	
	m_validVertexInputLayouts.push_back(InputLayoutPair(layout_Vert_P, dxInputLayout_Vert_P));
	m_validVertexInputLayouts.push_back(InputLayoutPair(layout_Vert_PCUTBN, dxInputLayout_Vert_PCUTBN));

	m_deviceContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void **)&m_annotator);
}


void RHIDevice::Destroy()
{
	for (int i = 0; i < (int)m_validVertexInputLayouts.size(); ++i)
	{
		InputLayoutPair& pair = m_validVertexInputLayouts[i];
		pair.m_dxInputLayout->Release();
	}
	m_validVertexInputLayouts.clear();

	m_instance = nullptr;

	m_device->Release();
	m_device = nullptr;

	m_deviceContext->Release();
	m_deviceContext = nullptr;

	m_gpu->Release();
	m_gpu = nullptr;

	m_gpuIndex = (uint)-1; // Intentional underflow
}


RHIOutput* RHIDevice::CreateOutputForWindow(Window* window)
{
	RHIOutput* output = new RHIOutput();
	output->Initialize(m_instance, this, window);

	return output;
}


void RHIDevice::SetViewport(const Vector2& lowerLeft, const Vector2& upperRight, float windowHeight)
{
	Vector2 upperLeft = Vector2();
	upperLeft.x = lowerLeft.x;
	upperLeft.y = windowHeight - upperRight.y;

	float width = upperRight.x - lowerLeft.x;
	float height = upperRight.y - lowerLeft.y;

	D3D11_VIEWPORT viewport;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = upperLeft.x;
	viewport.TopLeftY = upperLeft.y;

	// Create the viewport.
	m_deviceContext->RSSetViewports(1, &viewport);
}


void RHIDevice::ClearRenderTargetView(const RenderTargetView& rtv, const RGBA& color)
{
	float floatColor[4];
	color.GetAsFloats(floatColor[0], floatColor[1], floatColor[2], floatColor[3]);
	m_deviceContext->ClearRenderTargetView(rtv.GetHandle(), (float*)&floatColor ); 
}


void RHIDevice::ClearDepthStencilView(const DepthStencilView& dsv, float depth)
{
	m_deviceContext->ClearDepthStencilView(dsv.GetHandle(), D3D11_CLEAR_DEPTH, depth, 0);
}


void RHIDevice::ClearStencil(const DepthStencilView& dsv, uint8 stencil)
{
	m_deviceContext->ClearDepthStencilView(dsv.GetHandle(), D3D11_CLEAR_STENCIL, 0.0f, stencil);
}


void RHIDevice::BindFramebuffer(const FrameBuffer& frameBuffer)
{
	frameBuffer.GetDimensions(); // Errors on mismatched dimensions

	const std::vector<RenderTargetView*>& rtvs = frameBuffer.GetColorTargetArray();
	std::vector<ID3D11RenderTargetView*> dx11RTVs;
	dx11RTVs.reserve(rtvs.size());
	for (int i = 0; i < (int)rtvs.size(); ++i)
	{
		dx11RTVs.push_back(rtvs[i]->GetHandle());
	}

	DepthStencilView* dsv = frameBuffer.GetDepthStencilTarget();
	ID3D11DepthStencilView* dsvHandle = nullptr;
	if (dsv != nullptr)
	{
		dsvHandle = dsv->GetHandle();
	}
	m_deviceContext->OMSetRenderTargets((uint)rtvs.size(), dx11RTVs.data(), dsvHandle);
}


void RHIDevice::SetPrimitiveTopology(RENDER_CONSTANT topology)
{
	GUARANTEE_OR_DIE(IsRenderConstantAPrimitiveType(topology), "RHIDevice::SetPrimitiveTopology provided topology is invalid");

	m_deviceContext->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)topology);
}


void RHIDevice::BindInputLayout(const VertexLayout* layout)
{
	ID3D11InputLayout* inputLayout = nullptr;

	for (int i = 0; i < (int)m_validVertexInputLayouts.size(); ++i)
	{
		InputLayoutPair& pair = m_validVertexInputLayouts[i];
		if (pair.m_layout == layout)
		{
			inputLayout = pair.m_dxInputLayout;
			break;
		}
	}
	GUARANTEE_OR_DIE(inputLayout != nullptr, "Could not find input layout corresponding to the provided layout");

	m_deviceContext->IASetInputLayout(inputLayout);
}


void RHIDevice::BindShaderProgram(const ShaderProgram& shaderProgram)
{
	m_deviceContext->VSSetShader((ID3D11VertexShader*)shaderProgram.GetVertexShaderStage().GetHandle(), NULL, 0);
	m_deviceContext->PSSetShader((ID3D11PixelShader*)shaderProgram.GetFragmentShaderStage().GetHandle(), NULL, 0);
	m_deviceContext->HSSetShader((ID3D11HullShader*)shaderProgram.GetHullShaderStage().GetHandle(), NULL, 0);
	m_deviceContext->DSSetShader((ID3D11DomainShader*)shaderProgram.GetDomainShaderStage().GetHandle(), NULL, 0);
	
	m_deviceContext->CSSetShader(NULL, NULL, 0);
}


void RHIDevice::UnbindShaderProgram()
{
	m_deviceContext->VSSetShader(NULL, NULL, 0);
	m_deviceContext->PSSetShader(NULL, NULL, 0);
	m_deviceContext->HSSetShader(NULL, NULL, 0);
	m_deviceContext->DSSetShader(NULL, NULL, 0);
}


void RHIDevice::BindRasterizerState(const FF_RasterizerState& rasterizerState)
{
	m_deviceContext->RSSetState(rasterizerState.GetHandle());
}


void RHIDevice::BindDepthStencilTestState(const FF_DepthStencilTestState& depthState)
{
	m_deviceContext->OMSetDepthStencilState(depthState.GetHandle(), depthState.GetStencilReferenceValue());
}


void RHIDevice::BindBlendState(const FF_BlendState& blendState)
{
	m_deviceContext->OMSetBlendState(blendState.GetHandle(), nullptr, 0xffffffff);
}


void RHIDevice::BindShaderResourceView(RENDER_CONSTANT bindPoint, const ShaderResourceView& view)
{
	ID3D11ShaderResourceView* srvArray = view.GetHandle();
	m_deviceContext->PSSetShaderResources(bindPoint, 1, &srvArray);
	m_deviceContext->DSSetShaderResources(bindPoint, 1, &srvArray);
	m_deviceContext->CSSetShaderResources(bindPoint, 1, &srvArray);
}


void RHIDevice::UnbindShaderResourceView(RENDER_CONSTANT bindPoint)
{
	ID3D11ShaderResourceView* srvArray = nullptr;
	m_deviceContext->PSSetShaderResources(bindPoint, 1, &srvArray);
	m_deviceContext->DSSetShaderResources(bindPoint, 1, &srvArray);
	m_deviceContext->CSSetShaderResources(bindPoint, 1, &srvArray);
}


void RHIDevice::UnbindShaderResourceViews(RENDER_CONSTANT bindPoint, uint count)
{
	for (int i = 0; i < (int)count; ++i)
	{
		UnbindShaderResourceView(bindPoint + i);
	}
}


void RHIDevice::BindSampler(RENDER_CONSTANT bindPoint, const Sampler& sampler)
{
	ID3D11SamplerState* ssArray = sampler.GetHandle();
	m_deviceContext->PSSetSamplers(bindPoint, 1, &ssArray);
	m_deviceContext->DSSetSamplers(bindPoint, 1, &ssArray);
}


void RHIDevice::BindVertexBuffer(const VertexBuffer& vertexBuffer)
{
	const GPUBuffer* vb = vertexBuffer.GetGPUBuffer();

	ID3D11Buffer* vbArray = vb->GetHandle();				// Dx11 wants these values as arrays
	uint strideArray = vb->GetElementSizeBytes();
	uint offsetArray = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, &vbArray, &strideArray, &offsetArray);
}


void RHIDevice::BindIndexBuffer(const IndexBuffer& indexBuffer)
{
	const GPUBuffer* ib = indexBuffer.GetGPUBuffer();

	DXGI_FORMAT format = GetFormatForIndexBufferFromElementSizeBytes(ib->GetElementSizeBytes());
	m_deviceContext->IASetIndexBuffer(ib->GetHandle(), format, 0);
}


void RHIDevice::BindConstantBuffer(RENDER_CONSTANT constantBufferBindPoint, ConstantBuffer& constantBuffer)
{
	GUARANTEE_OR_DIE(IsRenderConstantAValidConstantBufferBindPoint(constantBufferBindPoint), "RHIDevice::BindConstantBuffer invalid bind point");

	constantBuffer.CopyToGPUBuffer();

	GPUBuffer* cb = constantBuffer.GetGPUBuffer();
	ID3D11Buffer* cbArray = cb->GetHandle();
	m_deviceContext->VSSetConstantBuffers(constantBufferBindPoint, 1, &cbArray);
	m_deviceContext->PSSetConstantBuffers(constantBufferBindPoint, 1, &cbArray);
	m_deviceContext->HSSetConstantBuffers(constantBufferBindPoint, 1, &cbArray);
	m_deviceContext->DSSetConstantBuffers(constantBufferBindPoint, 1, &cbArray);
	m_deviceContext->CSSetConstantBuffers(constantBufferBindPoint, 1, &cbArray);
}


void RHIDevice::Draw(uint vertexCount, uint offset)
{
	m_deviceContext->Draw(vertexCount, offset);
}


void RHIDevice::DrawIndexed(uint indexCount, uint offset)
{
	m_deviceContext->DrawIndexed(indexCount, offset, 0);
}


void RHIDevice::BindComputeShader(const ComputeShaderProgram& computeShaderProgram)
{
	m_deviceContext->VSSetShader(NULL, NULL, 0);
	m_deviceContext->PSSetShader(NULL, NULL, 0);
	m_deviceContext->HSSetShader(NULL, NULL, 0);
	m_deviceContext->DSSetShader(NULL, NULL, 0);

	m_deviceContext->CSSetShader((ID3D11ComputeShader*)computeShaderProgram.GetStage().GetHandle(), NULL, 0);
}


void RHIDevice::BindUnorderedAccessView(uint bindPoint, const UnorderedAccessView& view)
{
	ID3D11UnorderedAccessView* uavArray = view.GetHandle();
	m_deviceContext->CSSetUnorderedAccessViews(bindPoint, 1, &uavArray, NULL);
}


void RHIDevice::UnbindUnorderedAccesView(uint bindPoint)
{
	ID3D11UnorderedAccessView* uavArray = nullptr;
	m_deviceContext->CSSetUnorderedAccessViews(bindPoint, 1, &uavArray, NULL);
}


void RHIDevice::UnbindUnorderedAccesViews(RENDER_CONSTANT lowBindPoint, uint highBindPoint)
{
	for (int i = lowBindPoint; i <= (int)highBindPoint; ++i)
	{
		UnbindUnorderedAccesView(i);
	}
}


void RHIDevice::DispatchCompute(uint x, uint y, uint z)
{
	m_deviceContext->Dispatch(x, y, z);
}


ID3D11Device* RHIDevice::GetDevice() const
{
	return m_device;
}


ID3D11DeviceContext* RHIDevice::GetContext() const
{
	return m_deviceContext;
}


const std::vector<InputLayoutPair>& RHIDevice::GetValidLayouts()
{
	return m_validVertexInputLayouts;
}


void RHIDevice::StartAnnotation(const std::string& title) const
{
	std::wstring wideTitle = std::wstring(title.begin(), title.end());
	m_annotator->BeginEvent(wideTitle.c_str());
}


void RHIDevice::EndAnnotation() const
{
	m_annotator->EndEvent();
}