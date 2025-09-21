#pragma once

#include <daxa/daxa.inl>

struct Vertex
{
	daxa_f32vec3 pos;
	daxa_f32vec3 color;
};

DAXA_DECL_BUFFER_PTR(Vertex);

struct PushConstant
{
	daxa_BufferPtr(Vertex) vertexPtr;
};