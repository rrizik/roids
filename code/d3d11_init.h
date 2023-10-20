#pragma once

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h> // TODO: Get rid of this later when you can replace it with your own code


namespace dx = DirectX;
using namespace dx;

global ID3D11Device1*        d3d_device;
global ID3D11DeviceContext1* d3d_context;
global IDXGISwapChain1*      d3d_swapchain;

global ID3D11Texture2D*        d3d_framebuffer;
global ID3D11RenderTargetView* d3d_framebuffer_view;

global ID3D11Texture2D*        d3d_depthbuffer;
global ID3D11DepthStencilView* d3d_depthbuffer_view;

global ID3D11ShaderResourceView* d3d_shader_resource;

global ID3D11DepthStencilState* d3d_depthstencil_state;
global ID3D11RasterizerState1*  d3d_rasterizer_state;
global ID3D11SamplerState*      d3d_sampler_state;
global ID3D11BlendState*        d3d_blend_state; // note: maybe use BlendState1 later

global ID3D11Texture2D* d3d_texture;

// TODO: not sure I'm still using these
global ID3D11Buffer* d3d_vertex_buffer;
global ID3D11Buffer* d3d_index_buffer;
global ID3D11Buffer* d3d_constant_buffer;
global ID3D11Buffer* d3d_instance_buffer;

global ID3D11VertexShader* vertex_shader;
global ID3D11PixelShader*  pixel_shader;
global ID3D11InputLayout*  d3d_input_layout;
global D3D11_VIEWPORT d3d_viewport;

static void
d3d_init_debug_stuff(){
    ID3D11InfoQueue* info;
    d3d_device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&info);
    info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
    info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
    info->Release();
}

static void
d3d_init_device_and_context(){
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
d3d_init_swapchain(HWND window){
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
d3d_init_framebuffer(){
    HRESULT hr;

    hr = d3d_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d_framebuffer);
    assert_hr(hr);

    hr = d3d_device->CreateRenderTargetView(d3d_framebuffer, 0, &d3d_framebuffer_view);
    assert_hr(hr);
}

static void
d3d_init_depthbuffer(){
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
d3d_init_depthstencil(){
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
d3d_init_rasterizer_state(){
    HRESULT hr;

    D3D11_RASTERIZER_DESC1 rasterizer_desc = {};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_BACK;

    hr = d3d_device->CreateRasterizerState1(&rasterizer_desc, &d3d_rasterizer_state);
    assert_hr(hr);

    d3d_context->RSSetState(d3d_rasterizer_state);
}

static void
d3d_init_sampler_state(){
    HRESULT hr;

    D3D11_SAMPLER_DESC sampler_desc = {
        .Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
        .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
        .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
        .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
        .MipLODBias = 2.0f,
        .MinLOD = 0.0f,
        .MaxLOD = FLT_MAX,
    };
    hr = d3d_device->CreateSamplerState(&sampler_desc, &d3d_sampler_state);
    assert_hr(hr);

    d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);
}

static void
d3d_init_blend_state(){
    HRESULT hr;

    D3D11_BLEND_DESC blend_desc = {
        .AlphaToCoverageEnable = true,
        .IndependentBlendEnable = false,
        .RenderTarget[0].BlendEnable = true,
        .RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD,
        .RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA,
        .RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA,
        .RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD,
        .RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE,
        .RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO,
        .RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL,
    };
    hr = d3d_device->CreateBlendState(&blend_desc, &d3d_blend_state);
    assert_hr(hr);
    d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);
}

static void
d3d_set_viewport(f32 top_left_x, f32 top_left_y, f32 width, f32 height, f32 min_depth, f32 max_depth){
    // NOTE: can be used to create split screens for 2+ players
    d3d_viewport = {
        .TopLeftX = top_left_x,
        .TopLeftY = top_left_y,
        .Width = width,
        .Height = height,
        .MinDepth = min_depth,
        .MaxDepth = max_depth
    };
    d3d_context->RSSetViewports(1, &d3d_viewport);
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
        {"POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,                            D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16,                           D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32,                           D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48,                           D3D11_INPUT_PER_INSTANCE_DATA, 1},
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

typedef struct Vertex{
    v3 position;
    v2 uv;
} Vertex;

typedef struct Mesh{
    ID3D11Buffer* vertex_buffer;
    ID3D11Buffer* index_buffer;
    u32 vertex_offset;
    u32 vertex_stride;
    u32 vertex_count;
    u32 index_stride;
    u32 index_count;
} Mesh;

//static Vertex cube[] = {
//    { make_v3(-1.0f, -1.0f, -1.0f), make_v2(0.0f, 0.0f) },
//    { make_v3( 1.0f, -1.0f, -1.0f), make_v2(1.0f, 0.0f) },
//    { make_v3(-1.0f,  1.0f, -1.0f), make_v2(0.0f, 1.0f) },
//    { make_v3( 1.0f,  1.0f, -1.0f), make_v2(1.0f, 1.0f) },
//    { make_v3(-1.0f, -1.0f,  1.0f), make_v2(1.0f, 0.0f) },
//    { make_v3( 1.0f, -1.0f,  1.0f), make_v2(0.0f, 1.0f) },
//    { make_v3(-1.0f,  1.0f,  1.0f), make_v2(1.0f, 1.0f) },
//    { make_v3( 1.0f,  1.0f,  1.0f), make_v2(1.0f, 1.0f) },
//};

static Vertex cube[] = {
    { make_v3(-20.0f, -20.0f,  20.0f), {0.0f, 0.0f}},
    { make_v3( 20.0f, -20.0f,  20.0f), {0.0f, 1.0f}},
    { make_v3(-20.0f,  20.0f,  20.0f), {1.0f, 0.0f}},
    { make_v3( 20.0f,  20.0f,  20.0f), {1.0f, 1.0f}},

    { make_v3(-20.0f, -20.0f, -20.0f), {0.0f, 0.0f}},
    { make_v3(-20.0f,  20.0f, -20.0f), {0.0f, 1.0f}},
    { make_v3( 20.0f, -20.0f, -20.0f), {1.0f, 0.0f}},
    { make_v3( 20.0f,  20.0f, -20.0f), {1.0f, 1.0f}},

    { make_v3(-20.0f,  20.0f, -20.0f), {0.0f, 0.0f}},
    { make_v3(-20.0f,  20.0f,  20.0f), {0.0f, 1.0f}},
    { make_v3( 20.0f,  20.0f, -20.0f), {1.0f, 0.0f}},
    { make_v3( 20.0f,  20.0f,  20.0f), {1.0f, 1.0f}},

    { make_v3(-20.0f, -20.0f, -20.0f), {0.0f, 0.0f}},
    { make_v3( 20.0f, -20.0f, -20.0f), {0.0f, 1.0f}},
    { make_v3(-20.0f, -20.0f,  20.0f), {1.0f, 0.0f}},
    { make_v3( 20.0f, -20.0f,  20.0f), {1.0f, 1.0f}},

    { make_v3( 20.0f, -20.0f, -20.0f), {0.0f, 0.0f}},
    { make_v3( 20.0f,  20.0f, -20.0f), {0.0f, 1.0f}},
    { make_v3( 20.0f, -20.0f,  20.0f), {1.0f, 0.0f}},
    { make_v3( 20.0f,  20.0f,  20.0f), {1.0f, 1.0f}},

    { make_v3(-20.0f, -20.0f, -20.0f), {0.0f, 0.0f}},
    { make_v3(-20.0f, -20.0f,  20.0f), {0.0f, 1.0f}},
    { make_v3(-20.0f,  20.0f, -20.0f), {1.0f, 0.0f}},
    { make_v3(-20.0f,  20.0f,  20.0f), {1.0f, 1.0f}},
};


static u32 cube_indicies[] = {
    0, 1, 2,    // side 1
    2, 1, 3,
    4, 5, 6,    // side 2
    6, 5, 7,
    8, 9, 10,    // side 3
    10, 9, 11,
    12, 13, 14,    // side 4
    14, 13, 15,
    16, 17, 18,    // side 5
    18, 17, 19,
    20, 21, 22,    // side 6
    22, 21, 23,
};

static void
d3d_init_vertex_buffers(Mesh* mesh, Vertex* verticies){
    HRESULT hr;

    D3D11_BUFFER_DESC buffer_desc = {0};
    buffer_desc.StructureByteStride = mesh->vertex_stride;
    buffer_desc.ByteWidth = mesh->vertex_stride * mesh->vertex_count;
    buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_resource = {0};
    vertex_resource.pSysMem = verticies;
    hr = d3d_device->CreateBuffer(&buffer_desc, &vertex_resource, &mesh->vertex_buffer);
    assert_hr(hr);
}

typedef struct InstanceData {
    XMMATRIX transform;
} InstanceData;
global u32 instance_count = 2;
global InstanceData instances[2];
//global ID3D11Buffer* buffers[] = { vertex_buffer, d3d_instance_buffer };
//global u32 buffers_stride[] = { sizeof(Vertex), sizeof(InstanceData) };
//global u32 buffers_offset[] = { 0, 0 };

static void
d3d_set_vertex_buffer(Mesh* mesh, Vertex* verticies){
    HRESULT hr;

    D3D11_BUFFER_DESC buffer_desc = {0};
    buffer_desc.StructureByteStride = mesh->vertex_stride;
    buffer_desc.ByteWidth = mesh->vertex_stride * mesh->vertex_count;
    buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_resource = {0};
    vertex_resource.pSysMem = verticies;
    hr = d3d_device->CreateBuffer(&buffer_desc, &vertex_resource, &mesh->vertex_buffer);
    assert_hr(hr);

    ID3D11Buffer* buffers[] = {mesh->vertex_buffer, d3d_instance_buffer};
    u32 strides[] = {sizeof(Vertex), sizeof(InstanceData)};
    u32 offset[] = {0, 0};

    d3d_context->IASetVertexBuffers(0, 2, buffers, strides, offset);
}

static void
d3d_init_index_buffer(Mesh* mesh, u32* indicies){
    HRESULT hr;

    D3D11_BUFFER_DESC buffer_desc = {0};
    buffer_desc.StructureByteStride = mesh->index_stride;
    buffer_desc.ByteWidth = mesh->index_stride * mesh->index_count;
    buffer_desc.Usage     = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA index_resource  = {0};
    index_resource.pSysMem                 = indicies;
    hr = d3d_device->CreateBuffer(&buffer_desc, &index_resource, &mesh->index_buffer);
    assert_hr(hr);
}

static void
d3d_set_index_buffer(Mesh* mesh, u32* indicies){
    HRESULT hr;

    D3D11_BUFFER_DESC buffer_desc = {0};
    buffer_desc.StructureByteStride = mesh->index_stride;
    buffer_desc.ByteWidth = mesh->index_stride * mesh->index_count;
    buffer_desc.Usage     = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA index_resource  = {0};
    index_resource.pSysMem                 = indicies;
    hr = d3d_device->CreateBuffer(&buffer_desc, &index_resource, &mesh->index_buffer);
    assert_hr(hr);

    d3d_context->IASetIndexBuffer(mesh->index_buffer, DXGI_FORMAT_R32_UINT, 0);
}

typedef struct Constants{
    XMMATRIX transform;
} Constants;

//typedef struct ConstantBuffer2{
//    struct{
//        float r;
//        float g;
//        float b;
//        float a;
//    } face_colors[6];
//} ConstandBuffer2;
//
//static ConstantBuffer2 ps_cb = {
//    {
//        {1.0f, 0.0f, 1.0f, 1.0f},
//        {1.0f, 0.0f, 0.0f, 1.0f},
//        {0.0f, 1.0f, 0.0f, 1.0f},
//        {0.0f, 0.0f, 1.0f, 1.0f},
//        {1.0f, 1.0f, 0.0f, 1.0f},
//        {0.0f, 1.0f, 1.0f, 1.0f},
//    }
//};

static void
d3d_init_constant_buffer(){
    HRESULT hr;

    D3D11_BUFFER_DESC buffer_desc = {0};
    buffer_desc.ByteWidth = sizeof(Constants);
    buffer_desc.Usage     = D3D11_USAGE_DYNAMIC;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = d3d_device->CreateBuffer(&buffer_desc, 0, &d3d_constant_buffer);
    assert_hr(hr);

    //D3D11_SUBRESOURCE_DATA constant_resource = {};
    //buffer_desc.ByteWidth = sizeof(ConstantBuffer2);
}

static void
d3d_set_constant_buffer(v3 pos, v3 angle, v3 scale){
    f32 aspect_ratio = (f32)SCREEN_HEIGHT / (f32)SCREEN_WIDTH;
    D3D11_MAPPED_SUBRESOURCE mapped_subresource;
    d3d_context->Map(d3d_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);

    Constants* constants = (Constants*)mapped_subresource.pData;
    constants->transform = XMMatrixTranspose(
        XMMatrixRotationX(angle.x) *
        XMMatrixRotationY(angle.y) *
        XMMatrixRotationZ(angle.z) *
        XMMatrixScaling(scale.x, scale.y, scale.z) *
        XMMatrixTranslation(pos.x, pos.y, pos.z) *
        XMMatrixPerspectiveLH(1.0f, aspect_ratio, 1.0f, 1000.0f)
    );
    d3d_context->Unmap(d3d_constant_buffer, 0);

    d3d_context->VSSetConstantBuffers(0, 1, &d3d_constant_buffer);
}

static void
d3d_set_texture(Bitmap image){
    HRESULT hr;

    D3D11_TEXTURE2D_DESC texture_desc = {
        .Width = (u32)image.width,
        .Height = (u32)image.height,
        .MipLevels = 1, // mip levels to use. Set to 0 for mips
        .ArraySize = 1,
        .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
        .SampleDesc = {1, 0},
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    };

    D3D11_SUBRESOURCE_DATA data = {
        .pSysMem = image.base,
        .SysMemPitch = (u32)image.stride,
    };

    hr = d3d_device->CreateTexture2D(&texture_desc, &data, &d3d_texture);
    assert_hr(hr);

    hr = d3d_device->CreateShaderResourceView(d3d_texture, 0, &d3d_shader_resource);
    assert_hr(hr);

    ///d3d_context->GenerateMips(texture_view);
    d3d_context->PSSetShaderResources(0, 1, &d3d_shader_resource);
}


static void
d3d_set_instance_buffer(InstanceData* instances){
    D3D11_BUFFER_DESC instance_buffer_desc = {0};
    instance_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
    instance_buffer_desc.ByteWidth = sizeof(InstanceData) * instance_count;
    instance_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA instance_data = {};
    instance_data.pSysMem = { instances };

    d3d_device->CreateBuffer(&instance_buffer_desc, &instance_data, &d3d_instance_buffer);
    //d3d_context->UpdateSubresource(d3d_instance_buffer, 0, 0, instances, 0, 0);
}

static void
d3d_present(){
    d3d_swapchain->Present(1, 0);
}
