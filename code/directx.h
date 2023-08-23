#pragma once

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h> // TODO: Get rid of this later when you can replace it with your own code


namespace dx = DirectX;
using namespace dx;

global ID3D11Device1*        d3d_device    = 0;
global ID3D11DeviceContext1* d3d_context   = 0;
global IDXGISwapChain1*      d3d_swapchain = 0;

global ID3D11Texture2D* d3d_framebuffer              = 0;
global ID3D11RenderTargetView* d3d_framebuffer_view  = 0;

global ID3D11Texture2D* d3d_depthbuffer              = 0;
global ID3D11DepthStencilView* d3d_depthbuffer_view  = 0;

global ID3D11DepthStencilState* d3d_depthstencil_state = 0;
global ID3D11RasterizerState1* d3d_rasterizer_state    = 0;
global ID3D11SamplerState* d3d_sampler_state           = 0;

global ID3D11Buffer* vertex_buffer   = 0;
global ID3D11Buffer* index_buffer    = 0;
global ID3D11Buffer* vs_constant_buffer  = 0;
global ID3D11Buffer* vs_constant_buffer2 = 0;
global ID3D11Buffer* ps_constant_buffer  = 0;

global ID3D11VertexShader* vertex_shader = 0;
global ID3D11PixelShader* pixel_shader   = 0;
global ID3D11InputLayout* d3d_input_layout   = 0;

static void
d3d_create_device_and_context(){
    HRESULT hr;

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    u32 flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ID3D11Device* baseDevice;
    ID3D11DeviceContext* baseDeviceContext;
    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &baseDevice, nullptr, &baseDeviceContext);
    assert_hr(hr);

    hr = baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)(&d3d_device));
    assert_hr(hr);

    hr = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)(&d3d_context));
    assert_hr(hr);
}

static void
d3d_create_swapchain(HWND window){
    HRESULT hr;

    IDXGIDevice1* dxgiDevice;
    hr = d3d_device->QueryInterface(__uuidof(IDXGIDevice1), (void**)(&dxgiDevice));
    assert_hr(hr);

    IDXGIAdapter* dxgiAdapter;
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    assert_hr(hr);

    IDXGIFactory2* dxgiFactory;
    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)(&dxgiFactory));
    assert_hr(hr);

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
    swapChainDesc.Width              = 0; // use window width
    swapChainDesc.Height             = 0; // use window height
    swapChainDesc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    swapChainDesc.Stereo             = FALSE;
    swapChainDesc.SampleDesc.Count   = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount        = 2;
    swapChainDesc.Scaling            = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD; // prefer DXGI_SWAP_EFFECT_FLIP_DISCARD, see Minimal D3D11 pt2
    swapChainDesc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags              = 0;

    dxgiFactory->CreateSwapChainForHwnd(d3d_device, window, &swapChainDesc, 0, 0, &d3d_swapchain);
}

static void
d3d_create_framebuffer(){
    HRESULT hr;

    hr = d3d_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d_framebuffer);
    assert_hr(hr);

    hr = d3d_device->CreateRenderTargetView(d3d_framebuffer, 0, &d3d_framebuffer_view);
    assert_hr(hr);
}

static void
d3d_create_depthbuffer(){
    HRESULT hr;

    D3D11_TEXTURE2D_DESC depthbuffer_desc = {0};
    d3d_framebuffer->GetDesc(&depthbuffer_desc); // copy from framebuffer properties
    depthbuffer_desc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthbuffer_desc.Format    = DXGI_FORMAT_D32_FLOAT;
    depthbuffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    hr = d3d_device->CreateTexture2D(&depthbuffer_desc, 0, &d3d_depthbuffer);
    assert_hr(hr);

    hr = d3d_device->CreateDepthStencilView(d3d_depthbuffer, 0, &d3d_depthbuffer_view);
    assert_hr(hr);
}

static void
d3d_create_depthstencil(){
    HRESULT hr;

    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
    depth_stencil_desc.DepthEnable = true;
    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc      = D3D11_COMPARISON_LESS;

    hr = d3d_device->CreateDepthStencilState(&depth_stencil_desc, &d3d_depthstencil_state);
    assert_hr(hr);

    d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);
}

static void
d3d_create_rasterizer_state(){
    HRESULT hr;

    D3D11_RASTERIZER_DESC1 rasterizer_desc = {};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_BACK;

    hr = d3d_device->CreateRasterizerState1(&rasterizer_desc, &d3d_rasterizer_state);
    assert_hr(hr);

    d3d_context->RSSetState(d3d_rasterizer_state);
}

static void
d3d_create_sampler_state(){
    HRESULT hr;

    D3D11_SAMPLER_DESC sampler_desc = {};
    sampler_desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampler_desc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    hr = d3d_device->CreateSamplerState(&sampler_desc, &d3d_sampler_state);
    assert_hr(hr);

    d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);
}

static void
d3d_set_viewport(){
    // NOTE: can be used to create split screens for 2+ players
    D3D11_VIEWPORT viewport = {
        .TopLeftX = 0.0f,
        .TopLeftY = 0.0f,
        .Width = SCREEN_WIDTH,
        .Height = SCREEN_HEIGHT,
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f
    };
    d3d_context->RSSetViewports(1, &viewport);
}

static void
d3d_load_vertex_shader(String8 path_shader){
    HRESULT hr;

#if DEBUG
    u32 shader_compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    u32 shader_compile_flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    ScratchArena scratch = begin_scratch(0);
    String8 shader_file = str8_literal("simple_shader.hlsl");
    String8 utf8_shader_path = str8_path_append(scratch.arena, path_shader, shader_file);
    String16 utf16_shader_path = os_utf8_utf16(scratch.arena, utf8_shader_path);

    ID3DBlob* vs_blob, *ps_blob, *error;
    hr = D3DCompileFromFile((wchar*)utf16_shader_path.str, 0, 0, "vs_main", "vs_5_0", shader_compile_flags, 0, &vs_blob, &error);
    if(FAILED(hr)) {
        print("Error: failed D3DCompileFromFile()\n");
        print("--- Message: %s\n", (char*)error->GetBufferPointer());
        assert_hr(hr);
    }

    D3D11_INPUT_ELEMENT_DESC input_element_desc[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      //{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
      //{"COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, sizeof(v2), D3D11_INPUT_PER_VERTEX_DATA, 0},
      //{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      //{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(v3), D3D11_INPUT_PER_VERTEX_DATA, 0},
      //{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(v3) + sizeof(v3), D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    hr = d3d_device->CreateInputLayout(input_element_desc, array_count(input_element_desc), vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &d3d_input_layout);
    assert_hr(hr);

    d3d_context->IASetInputLayout(d3d_input_layout);
    end_scratch(scratch);

    hr = d3d_device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), 0, &vertex_shader);
    assert_hr(hr);
}

static void
d3d_load_pixel_shader(String8 path){
    HRESULT hr;

#if DEBUG
    u32 shader_compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    u32 shader_compile_flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    ScratchArena scratch = begin_scratch(0);
    String8 shader_file = str8_literal("simple_shader.hlsl");
    String8 utf8_shader_path = str8_path_append(scratch.arena, path, shader_file);
    String16 utf16_shader_path = os_utf8_utf16(scratch.arena, utf8_shader_path);

    ID3DBlob* vs_blob, *ps_blob, *error;
    hr = D3DCompileFromFile((wchar*)utf16_shader_path.str, 0, 0, "ps_main", "ps_5_0", shader_compile_flags, 0, &ps_blob, &error);
    if(FAILED(hr)) {
        print("Error: failed D3DCompileFromFile()\n");
        print("--- Message: %s\n", (char*)error->GetBufferPointer());
        assert_hr(hr);
    }
    end_scratch(scratch);

    hr = d3d_device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), 0, &pixel_shader);
    assert_hr(hr);
}

typedef struct Mesh{
    ID3D11Buffer* vertex_buffer;
    ID3D11Buffer* index_buffer;
    u32 vertex_stride;
    u32 index_count;
} Mesh;

typedef struct Vertex{
    v3 position;
} Vertex;

static Vertex cube[] = {
    { make_v3(-1.0f, -1.0f, -1.0f) },
    { make_v3( 1.0f, -1.0f, -1.0f) },
    { make_v3(-1.0f,  1.0f, -1.0f) },
    { make_v3( 1.0f,  1.0f, -1.0f) },
    { make_v3(-1.0f, -1.0f,  1.0f) },
    { make_v3( 1.0f, -1.0f,  1.0f) },
    { make_v3(-1.0f,  1.0f,  1.0f) },
    { make_v3( 1.0f,  1.0f,  1.0f) },
};


static u32 vertex_count = array_count(cube);
static u32 vertex_stride = sizeof(Vertex);
static u32 vertex_offset = 0;

static u32 cube_indicies[] = {
    0,2,1, 2,3,1,
    1,3,5, 3,7,5,
    2,6,3, 3,6,7,
    4,5,7, 4,7,6,
    0,4,2, 2,4,6,
    0,1,4, 1,5,4,
};
static u32 index_stride = sizeof(u32);
static u32 index_count = array_count(cube_indicies);

static void
d3d_create_vertex_buffer(){
    D3D11_BUFFER_DESC buffer_desc = {0};
    buffer_desc.StructureByteStride = vertex_stride;
    buffer_desc.ByteWidth = vertex_stride * vertex_count;
    buffer_desc.Usage     = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_resource = {0};
    vertex_resource.pSysMem = cube;
    HRESULT result = d3d_device->CreateBuffer(&buffer_desc, &vertex_resource, &vertex_buffer);
    assert_hr(result);
}

static void
d3d_set_vertex_buffer(Mesh* mesh, Vertex* verticies, u32 count){
    mesh->vertex_stride = sizeof(Vertex);

    D3D11_BUFFER_DESC buffer_desc = {0};
    buffer_desc.StructureByteStride = mesh->vertex_stride;
    buffer_desc.ByteWidth = mesh->vertex_stride * count;
    buffer_desc.Usage     = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_resource = {0};
    vertex_resource.pSysMem = verticies;
    HRESULT result = d3d_device->CreateBuffer(&buffer_desc, &vertex_resource, &mesh->vertex_buffer);

    d3d_context->IASetVertexBuffers(0, 1, &mesh->vertex_buffer, &mesh->vertex_stride, &vertex_offset);
    assert_hr(result);
}

static void
d3d_create_index_buffer(){
    D3D11_BUFFER_DESC buffer_desc = {0};
    buffer_desc.StructureByteStride = index_stride;
    buffer_desc.ByteWidth = index_stride * index_count;
    buffer_desc.Usage     = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA index_resource  = {0};
    index_resource.pSysMem                 = cube_indicies;
    HRESULT result = d3d_device->CreateBuffer(&buffer_desc, &index_resource, &index_buffer);
    assert_hr(result);
}

static void
d3d_set_index_buffer(Mesh* mesh, u32* indicies, u32 count){
    mesh->index_count = count;

    u32 stride = sizeof(u32);
    D3D11_BUFFER_DESC buffer_desc = {0};
    buffer_desc.StructureByteStride = stride;
    buffer_desc.ByteWidth = stride * mesh->index_count;
    buffer_desc.Usage     = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA index_resource  = {0};
    index_resource.pSysMem                 = indicies;
    HRESULT result = d3d_device->CreateBuffer(&buffer_desc, &index_resource, &mesh->index_buffer);
    d3d_context->IASetIndexBuffer(mesh->index_buffer, DXGI_FORMAT_R32_UINT, 0);
    assert_hr(result);
}

typedef struct ConstantBuffer{
    XMMATRIX transform;
} ConstantBuffer;

typedef struct ConstantBuffer2{
    struct{
        float r;
        float g;
        float b;
        float a;
    } face_colors[6];
} ConstandBuffer2;

static ConstantBuffer vs_cb = {};
static ConstantBuffer vs_cb2 = {};
static ConstantBuffer2 ps_cb = {
    {
        {1.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 1.0f, 1.0f},
    }
};
static void
d3d_create_constant_buffer(){
    D3D11_BUFFER_DESC buffer_desc = {0};
    buffer_desc.ByteWidth = sizeof(ConstantBuffer);
    buffer_desc.Usage     = D3D11_USAGE_DYNAMIC;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA constant_resource = {};
    constant_resource.pSysMem = &vs_cb;
    HRESULT result = d3d_device->CreateBuffer(&buffer_desc, &constant_resource, &vs_constant_buffer);
    constant_resource.pSysMem = &vs_cb2;
    result = d3d_device->CreateBuffer(&buffer_desc, &constant_resource, &vs_constant_buffer2);
    assert_hr(result);

    buffer_desc.ByteWidth = sizeof(ConstantBuffer2);
    constant_resource.pSysMem = &ps_cb;
    result = d3d_device->CreateBuffer(&buffer_desc, &constant_resource, &ps_constant_buffer);
    assert_hr(result);
}

static void
d3d_set_constant_buffer(ConstantBuffer* cb){
    D3D11_BUFFER_DESC buffer_desc = {0};
    buffer_desc.ByteWidth = sizeof(ConstantBuffer);
    buffer_desc.Usage     = D3D11_USAGE_DYNAMIC;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA constant_resource = {};
    constant_resource.pSysMem = &cb;
    HRESULT result = d3d_device->CreateBuffer(&buffer_desc, &constant_resource, &vs_constant_buffer);
    assert_hr(result);
    d3d_context->VSSetConstantBuffers(0, 1, &vs_constant_buffer);
}

static void
d3d_init_constant_buffer(ConstantBuffer* cb, ID3D11Buffer* buffer){
    D3D11_BUFFER_DESC buffer_desc = {0};
    buffer_desc.ByteWidth = sizeof(ConstantBuffer);
    buffer_desc.Usage     = D3D11_USAGE_DYNAMIC;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA constant_resource = {};
    constant_resource.pSysMem = &cb;
    HRESULT result = d3d_device->CreateBuffer(&buffer_desc, &constant_resource, &buffer);
    assert_hr(result);
}

