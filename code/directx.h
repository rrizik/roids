#pragma once

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <d3d11.h>
#include <d3dcompiler.h>

global ID3D11Device*        d3d_device    = 0;
global ID3D11DeviceContext* d3d_context   = 0;
global IDXGISwapChain*      d3d_swapchain = 0;

global ID3D11RenderTargetView* d3d_framebuffer = 0;
global ID3D11DepthStencilView* d3d_depthbuffer = 0;

global ID3D11Buffer* vertex_buffer   = 0;
global ID3D11Buffer* index_buffer    = 0;
global ID3D11Buffer* constant_buffer = 0;

global ID3D11VertexShader* vertex_shader = 0;
global ID3D11PixelShader* pixel_shader   = 0;
global ID3D11InputLayout* input_layout   = 0;


static HRESULT
d3d_create_device_and_swapchain(HWND hwindow){
    // NOTE: (device) represents the logical graphics adapter. Can be backed by a physical hardware device or software emulation layer. The device allows you to create resourced as necessary, for example (buffers, shaders, textures, ...)
    // NOTE: (device context) is used to issue rendering commands and configuring the rendering pipeline
    // NOTE: (swapchain) is an aggregation of framenbuffers (usually 2, sometimes 3). It allows you to draw to a back buffer, and then display it to the front later when you are done drawing (prevents a lot of rendering issues)

    DXGI_SWAP_CHAIN_DESC sd = {0};
    sd.BufferDesc.Format  = DXGI_FORMAT_B8G8R8A8_UNORM; // NOTE: 8btis per color channel, 8 bits for alpha.
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.SampleDesc.Count = 1; // NOTE: Multisampling. Anti aliasing sample count (up to 4 guaranteed).
    sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT; // NOTE: How we want to use the back buffer. In this case we want to draw to it.
    sd.BufferCount  = 1; // NOTE: Number of back buffers to use in our swap chain. 1 for one frontbuffer one backbuffer
    sd.OutputWindow = hwindow; // NOTE: Which window to draw to.
    sd.Windowed     = true; // NOTE: Start windowed
    sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;

#if DEBUG
    u32 flags = D3D11_CREATE_DEVICE_DEBUG;
#else
    u32 flags = 0;
#endif

    HRESULT h_result = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, flags, 0, 0, D3D11_SDK_VERSION, &sd, &d3d_swapchain, &d3d_device, 0, &d3d_context);
    return(h_result);
}

static HRESULT
d3d_create_framebuffer(){
    HRESULT result = {0};

    // NOTE: Gain access to a texture subresource in the swapchain (the framebuffer address)
    ID3D11Texture2D* framebuffer_address;
    result = d3d_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&framebuffer_address);
    if(FAILED(result)){ return(result); }

    // NOTE: Use the texture resource to create a render target (framebuffer)
    result = d3d_device->CreateRenderTargetView(framebuffer_address, 0, &d3d_framebuffer);
    if(FAILED(result)){ return(result); }
    framebuffer_address->Release();

    return(result);
}

static void
d3d_init_viewport(){
    D3D11_VIEWPORT viewport = {0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f};
    d3d_context->RSSetViewports(1, &viewport); // NOTE: sets the viewport to use (count, viewports)
}

static HRESULT
d3d_create_depthbuffer(){
    HRESULT result = {0};

    D3D11_TEXTURE2D_DESC texture_desc = {0};
    texture_desc.Width = SCREEN_WIDTH;
    texture_desc.Height = SCREEN_HEIGHT;
    texture_desc.ArraySize = 1;
    texture_desc.MipLevels = 1;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.Format = DXGI_FORMAT_D32_FLOAT;
    texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthbuffer_texture;
    result = d3d_device->CreateTexture2D(&texture_desc, 0, &depthbuffer_texture);
    if(FAILED(result)) { return(result); }

    D3D11_DEPTH_STENCIL_VIEW_DESC depthbuffer_desc;
    memset(&depthbuffer_desc, 0, sizeof(depthbuffer_desc));
    depthbuffer_desc.Format = DXGI_FORMAT_D32_FLOAT;
    depthbuffer_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    result = d3d_device->CreateDepthStencilView(depthbuffer_texture, &depthbuffer_desc, &d3d_depthbuffer);
    depthbuffer_texture->Release();
    if(FAILED(result)) { return(result); }

    return(result);
}

static HRESULT
d3d_init_shaders(String8 path_src){
    HRESULT result = ZERO_INIT;

#if DEBUG
    u32 shader_compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    u32 shader_compile_flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    ScratchArena scratch = begin_scratch(0);
    String8 shader_file = str8_literal("simple_shader.hlsl");
    String8 utf8_shader_path = str8_path_append(scratch.arena, path_src, shader_file);
    String16 utf16_shader_path = os_utf8_utf16(scratch.arena, utf8_shader_path);
    ID3DBlob* vs_blob, *ps_blob, *error;
    //result = D3DCompileFromFile(L"code\\simple_shader.hlsl", 0, 0, "vs_main", "vs_5_0", shader_compile_flags, 0, &vs_blob, &error);
    result = D3DCompileFromFile((wchar*)utf16_shader_path.str, 0, 0, "vs_main", "vs_5_0", shader_compile_flags, 0, &vs_blob, &error);
    if(FAILED(result)) {
        print("Error: failed D3DCompileFromFile()\n");
        print("--- Message: %s\n", (char*)error->GetBufferPointer());
        return(result);
    }

    //result = D3DCompileFromFile(L"code\\simple_shader.hlsl", 0, 0, "ps_main", "ps_5_0", shader_compile_flags, 0, &ps_blob, &error);
    result = D3DCompileFromFile((wchar*)utf16_shader_path.str, 0, 0, "ps_main", "ps_5_0", shader_compile_flags, 0, &ps_blob, &error);
    if(FAILED(result)) {
        print("Error: failed D3DCompileFromFile()\n");
        print("--- Message: %s\n", (char*)error->GetBufferPointer());
        return(result);
    }
    end_scratch(scratch);

    result = d3d_device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), 0, &vertex_shader);
    if(FAILED(result)) { return(result); }

    result = d3d_device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), 0, &pixel_shader);
    if(FAILED(result)) { return(result); }

    D3D11_INPUT_ELEMENT_DESC input_element_desc[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      //{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      //{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(v3), D3D11_INPUT_PER_VERTEX_DATA, 0},
      //{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(v3), D3D11_INPUT_PER_VERTEX_DATA, 0},
      //{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(v3) + sizeof(v3), D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    result = d3d_device->CreateInputLayout(input_element_desc, array_count(input_element_desc), vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &input_layout);
    if(FAILED(result)) { return(result); }

    d3d_context->IASetInputLayout(input_layout);

    return(result);
}

static v2 verticies[] = {
    { 0.0f,  0.5f },
    { 0.5f, -0.5f },
    {-0.5f, -0.5f },
};
static u32 vertex_stride = sizeof(v2);
static u32 vertex_offset = 0;
static HRESULT
d3d_create_vertex_buffer(){
    u32 vertex_count = array_count(verticies);

    D3D11_BUFFER_DESC buffer_desc = {0};
    buffer_desc.StructureByteStride = sizeof(v2);
    buffer_desc.ByteWidth = sizeof(v2) * vertex_count;
    buffer_desc.Usage     = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_resource = {0};
    vertex_resource.pSysMem = verticies;
    HRESULT result = d3d_device->CreateBuffer(&buffer_desc, &vertex_resource, &vertex_buffer);
    return(result);
}




