#pragma once

#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler" )

#include <d3d11.h>       // D3D interface
#include <d3dcompiler.h> // shader compiler
#include <DirectXMath.h> // TODO: fucking ANNIHILATE this. Its fucking trash. And I dont want garbage imports

// NOTE: DX11 global inits
global ID3D11Device*           d3d_device         = 0;
global ID3D11DeviceContext*    d3d_device_context = 0;
global IDXGISwapChain*         swap_chain         = 0;
global ID3D11RenderTargetView* backbuffer         = 0;
global ID3D11DepthStencilView* depthbuffer        = 0;

global ID3D11VertexShader* vertex_shader          = 0;
global ID3D11PixelShader* pixel_shader            = 0;
global ID3D11InputLayout* input_layout            = 0;

global ID3D11Buffer* vertex_buffer                = 0;
global ID3D11Buffer* index_buffer                 = 0;
global ID3D11Buffer* constant_buffer              = 0;
global ID3D11ShaderResourceView* texture_view     = 0;

global ID3D11RasterizerState* rasterizer_state    = 0;
global ID3D11SamplerState* sampler_state          = 0;
global ID3D11BlendState* blend_state              = 0;


static HRESULT
d3d_create_device_and_swap_chain(Window window){
    HRESULT h_result = ZERO_INIT;

    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {0};
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // NOTE: 8btis per color channel, 8 bits for alpha.
    swap_chain_desc.BufferDesc.Width  = window.width;
    swap_chain_desc.BufferDesc.Height = window.height;
    swap_chain_desc.SampleDesc.Count  = 1; // NOTE: Multisampling. Anti aliasing sample count (up to 4 guaranteed).
    swap_chain_desc.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT; // NOTE: How we want to use the back buffer. In this case we want to draw to it.
    swap_chain_desc.BufferCount       = 1; // NOTE: Number of back buffers to use in our swap chain.
    swap_chain_desc.OutputWindow      = window.handle; // NOTE: Which window to draw to.
    swap_chain_desc.Windowed          = true;
    swap_chain_desc.Flags             = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

#if DEBUG
    u32 flags = D3D11_CREATE_DEVICE_DEBUG;
#else
    u32 flags = 0;
#endif
    h_result = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, flags, 0, 0, D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, &d3d_device, 0, &d3d_device_context);
    return(h_result);
}

static HRESULT
d3d_create_depthbuffer(){
    HRESULT result = ZERO_INIT;

    D3D11_TEXTURE2D_DESC texture_desc = {0};
    texture_desc.Width     = SCREEN_WIDTH;
    texture_desc.Height    = SCREEN_HEIGHT;
    texture_desc.ArraySize = 1;
    texture_desc.MipLevels = 1;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.Format    = DXGI_FORMAT_D32_FLOAT;
    texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthbuffer_texture;
    result = d3d_device->CreateTexture2D(&texture_desc, 0, &depthbuffer_texture);
    if(FAILED(result)) { return(result); }

    D3D11_DEPTH_STENCIL_VIEW_DESC depthbuffer_desc;
    memset(&depthbuffer_desc, 0, sizeof(depthbuffer_desc));
    depthbuffer_desc.Format = DXGI_FORMAT_D32_FLOAT;
    depthbuffer_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthbuffer_desc.Flags = 0;
    result = d3d_device->CreateDepthStencilView(depthbuffer_texture, &depthbuffer_desc, &depthbuffer);
    depthbuffer_texture->Release();
    if(FAILED(result)) { return(result); }

    return(result);
}

static HRESULT
d3d_create_render_target(){
    HRESULT result = ZERO_INIT;

    ID3D11Texture2D* backbuffer_address;
    result = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer_address); // NOTE: Get the backbuffer address from the swap_chain
    if(FAILED(result)){ return(result); }

    result = d3d_device->CreateRenderTargetView(backbuffer_address, 0, &backbuffer); // NOTE: Create the backbuffer
    if(FAILED(result)){ return(result); }

    backbuffer_address->Release();
    d3d_device_context->OMSetRenderTargets(1, &backbuffer, depthbuffer); // NOTE: Sets the render target (count, targets, advanced)
    return(result);
}

static void
d3d_init_viewport(Window window){
    D3D11_VIEWPORT viewport = {
        .TopLeftX = 0.0f,
        .TopLeftY = 0.0f,
        .Width = (f32)window.width,
        .Height = (f32)window.height,
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f,
    };
    d3d_device_context->RSSetViewports(1, &viewport); // NOTE: sets the viewport to use (count, viewports)
}

static HRESULT
d3d_init_shaders(){
    HRESULT result = ZERO_INIT;

#if DEBUG
    u32 shader_compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    u32 shader_compile_flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
    ID3DBlob* vs_blob, * ps_blob, * error = 0;
    result = D3DCompileFromFile(L"code\\shaders.hlsl", 0, 0, "vs_main", "vs_5_0", shader_compile_flags, 0, &vs_blob, &error);
    if(FAILED(result)) { return(result); }

    result = D3DCompileFromFile(L"code\\shaders.hlsl", 0, 0, "ps_main", "ps_5_0", shader_compile_flags, 0, &ps_blob, &error);
    if(FAILED(result)) { return(result); }

    result = d3d_device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), 0, &vertex_shader);
    if(FAILED(result)) { return(result); }

    result = d3d_device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), 0, &pixel_shader);
    if(FAILED(result)) { return(result); }

    u32 rgba = sizeof(RGBA);
    u32 v = sizeof(v3);
    D3D11_INPUT_ELEMENT_DESC input_element_desc[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      //{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(v3), D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(v3), D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(v3) + sizeof(v3), D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    result = d3d_device->CreateInputLayout(input_element_desc, ArrayCount(input_element_desc), vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &input_layout);
    if(FAILED(result)) { return(result); }

    d3d_device_context->IASetInputLayout(input_layout);

    return(result);
}

static void
d3d_clean(){
    backbuffer->Release();
    depthbuffer->Release();
    swap_chain->Release();
    swap_chain->SetFullscreenState(false, 0);
    d3d_device->Release();
    d3d_device_context->Release();
    vertex_shader->Release();
    pixel_shader->Release();
    vertex_buffer->Release();
}


