#ifndef D3D11_INIT_H
#define D3D11_INIT_H

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "dxgi.lib")

#if COMPILER_CL
#pragma warning(push, 0)
#endif
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <DirectXMath.h> // TODO: Get rid of this later when you can replace it with your own code
#if COMPILER_CL
#pragma warning(pop)
#endif

//#include <d3d11.h>
//#include <dxgidebug.h>
//#pragma comment(lib, "dxguid.lib")


namespace dx = DirectX;
using namespace dx;

global ID3D11Device1*        d3d_device;
global ID3D11DeviceContext1* d3d_context;
global IDXGISwapChain1*      d3d_swapchain;
global D3D11_VIEWPORT d3d_viewport;

global ID3D11Texture2D*        d3d_framebuffer;
global ID3D11RenderTargetView* d3d_framebuffer_view;

global ID3D11Texture2D*        d3d_depthbuffer;
global ID3D11DepthStencilView* d3d_depthbuffer_view;


global ID3D11DepthStencilState* d3d_depthstencil_state;
global ID3D11RasterizerState1*  d3d_rasterizer_state;
global ID3D11SamplerState*      d3d_sampler_state;
global ID3D11BlendState*        d3d_blend_state; // note: maybe use BlendState1 later


global ID3D11VertexShader* d3d_3d_vertex_shader;
global ID3D11PixelShader*  d3d_3d_pixel_shader;
global ID3D11InputLayout*  d3d_3d_input_layout;
//global ID3D11ShaderResourceView* d3d_shader_resource;
//global ID3D11Texture2D* d3d_texture;

global ID3D11VertexShader* d3d_2d_quad_vertex_shader;
global ID3D11PixelShader*  d3d_2d_quad_pixel_shader;
global ID3D11InputLayout*  d3d_2d_quad_input_layout;

global ID3D11VertexShader* d3d_2d_textured_vertex_shader;
global ID3D11PixelShader*  d3d_2d_textured_pixel_shader;
global ID3D11InputLayout*  d3d_2d_textured_input_layout;

global ID3D11VertexShader* d3d_d7_sprite_vertex_shader;
global ID3D11PixelShader*  d3d_d7_sprite_pixel_shader;

global ID3D11Buffer* d3d_vertex_buffer_8mb;
global ID3D11Buffer* d3d_vertex_buffer;
global ID3D11Buffer* d3d_index_buffer;
global ID3D11Buffer* d3d_instance_buffer;
global ID3D11Buffer* d3d_constant_buffer;

//global ID3D11Texture2D* image_texture;
//global ID3D11Texture2D* ship_texture;
//global ID3D11Texture2D* tree_texture;
//global ID3D11Texture2D* circle_texture;
//global ID3D11Texture2D* bullet_texture;
//global ID3D11Texture2D* test_texture;

global ID3D11ShaderResourceView* image_shader_resource;
global ID3D11ShaderResourceView* ship_shader_resource;
global ID3D11ShaderResourceView* tree_shader_resource;
global ID3D11ShaderResourceView* circle_shader_resource;
global ID3D11ShaderResourceView* bullet_shader_resource;
global ID3D11ShaderResourceView* test_shader_resource;

global ID3D11Texture2D* white_texture;
global ID3D11ShaderResourceView* white_shader_resource;

global D3D11_INPUT_ELEMENT_DESC input_layout_3d[] = {
        // vertex data
        {"POS",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COL",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEX",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        // instance data
        {"TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,                            D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16,                           D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32,                           D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48,                           D3D11_INPUT_PER_INSTANCE_DATA, 1},
};

global D3D11_INPUT_ELEMENT_DESC input_layout_2dui_textured[] = {
        // vertex data
        {"POS",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COL",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEX",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

global D3D11_INPUT_ELEMENT_DESC layout_2d_quad[] = {
        // vertex data
        {"POS",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COL",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

typedef struct ConstantBuffer{
    XMMATRIX view;
    XMMATRIX projection;
} ConstantBuffer;

typedef struct ConstantBuffer2D{
    v2s32 screen_res;
} ConstantBuffer2D;

typedef struct InstanceData {
    XMMATRIX transform;
} InstanceData;
global u32 instance_count = 3;
global InstanceData cube_instances[3];

typedef struct Transform2D{
    DirectX::XMFLOAT2 translate;
    float rotate;
    DirectX::XMFLOAT2 scale;
} Transform2D;

typedef struct Texture2D{
    ID3D11ShaderResourceView* view;
    ID3D11Texture2D* tex;
} Texture2D;

typedef struct Vertex3{
    v3 position;
    RGBA color;
    v2 uv;
} Vertex3;

typedef struct Vertex2{
    v2 position;
    RGBA color;
} Vertex2;

static RGBA CLEAR =   {1.0f, 1.0f, 1.0f,  0.0f};
static RGBA RED =     {1.0f, 0.0f, 0.0f,  1.0f};
static RGBA GREEN =   {0.0f, 1.0f, 0.0f,  1.0f};
static RGBA BLUE =    {0.0f, 0.0f, 1.0f,  1.0f};
static RGBA MAGENTA = {1.0f, 0.0f, 1.0f,  1.0f};
static RGBA TEAL =    {0.0f, 1.0f, 1.0f,  1.0f};
static RGBA PINK =    {0.92f, 0.62f, 0.96f, 1.0f};
static RGBA YELLOW =  {0.9f, 0.9f, 0.0f,  1.0f};
static RGBA ORANGE =  {1.0f, 0.5f, 0.15f,  1.0f};
static RGBA DARK_GRAY =   {0.5f, 0.5f, 0.5f,  1.0f};
static RGBA LIGHT_GRAY =   {0.8f, 0.8f, 0.8f,  1.0f};
static RGBA WHITE =   {1.0f, 1.0f, 1.0f,  1.0f};
static RGBA BLACK =   {0.0f, 0.0f, 0.0f,  1.0f};
static RGBA ARMY_GREEN =   {0.25f, 0.25f, 0.23f,  1.0f};
static RGBA BACKGROUND_COLOR = {0.2f, 0.29f, 0.29f, 1.0f};

static RGBA CONSOLE_INPUT_BACKGROUND_COLOR = {20/255.0f, 20/255.0f, 20/255.0f, 1.0f};
static RGBA CONSOLE_OUTPUT_BACKGROUND_COLOR = {29/255.0f, 29/255.0f, 29/255.0f, 1.0f};
static RGBA CONSOLE_TEXT_INPUT_COLOR = {215/255.0f, 175/255.0f, 135/255.0f, 1.0f};
static RGBA CONSOLE_TEXT_OUTPUT_COLOR = {215/255.0f, 175/255.0f, 135/255.0f, 1.0f};
static RGBA CONSOLE_CURSOR_COLOR = {226/255.0f, 226/255.0f, 226/255.0f, 1.0f};

static void d3d_init_debug_stuff();
static void d3d_load_shader(String8 shader_path, D3D11_INPUT_ELEMENT_DESC* input_layout, u32 layout_count, ID3D11VertexShader** d3d_vertex_shader, ID3D11PixelShader** d3d_pixel_shader, ID3D11InputLayout** d3d_input_layout);
static void d3d_load_shader2(String8 shader_path, ID3D11VertexShader** d3d_vertex_shader, ID3D11PixelShader** d3d_pixel_shader);
static void d3d_init(Window window);
static void init_texture_resource(Bitmap* bitmap, ID3D11ShaderResourceView** shader_resource);
static void d3d_release();

#endif
