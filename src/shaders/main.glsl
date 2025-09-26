// Includes the daxa shader API
#include <daxa/daxa.inl>

// Enabled the extension GL_EXT_debug_printf
#extension GL_EXT_debug_printf : enable

// Includes our shared types we created earlier
#include <shared.inl>

// Enabled the push constant MyPushConstant we specified in shared.inl
DAXA_DECL_PUSH_CONSTANT(PushConstant, push)

// We can define the vertex & fragment shader in one single file
#if DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_VERTEX

layout(location = 0) out daxa_f32vec3 v_col;
void main()
{
    Vertex vert = deref(push.vertexPtr[gl_VertexIndex]);
    gl_Position = daxa_f32vec4(vert.pos, 1);
    v_col = vert.color;
}

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_FRAGMENT

layout(location = 0) in daxa_f32vec3 v_col;
layout(location = 0) out daxa_f32vec4 color;
void main()
{
    color = daxa_f32vec4(v_col, 1);

    uint packed = packUnorm4x8(color);

    // simulate rgba6
    packed &= ~0x03030303;

    color = unpackUnorm4x8(packed);
}

#endif