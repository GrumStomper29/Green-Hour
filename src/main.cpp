
#include "shaders/shared.inl"

#include <SDL3/SDL.h>
//#include <SDL3/SDL_syswm.h>

#include <entt/entity/registry.hpp>

#include <vulkan/vulkan.h>
#include <daxa/daxa.hpp>

#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

#include <glm/glm.hpp>

#include <array>
#include <iostream>
#include <memory>
#include <span>

#include <unordered_map>
#include <string>
//#include <string_view>

#ifdef _WIN32
#include "windows.h"
#endif

struct Transform
{
	glm::vec3 position{};
	glm::vec3 rotation{};
	glm::vec3 scale   {};
};

struct Scene
{
	using EntityName = std::string;

	entt::registry registry{};

	entt::entity createEntity(std::string name)
	{
		entt::entity entity{ registry.create() };

		registry.emplace<EntityName>(entity, name);

		return entity;
	}
};




struct Renderer
{
	daxa::NativeWindowHandle nativeWinHandle{};
	daxa::Instance instance                 {};
	daxa::Device device                     {};
	daxa::Swapchain swapchain               {};

	daxa::PipelineManager pipelineManager{};
};


daxa::NativeWindowPlatform getNativeWindowPlatform()
{
#ifdef _WIN32

	return daxa::NativeWindowPlatform::WIN32_API;

#elifdef __linux__

	const char* videoDriver{ SDL_GetCurrentVideoDriver() };

	if (videoDriver == "x11")
	{
		return daxa::NativeWindowPlatform::XLIB_API;
	}
	else if (videoDriver == "wayland")
	{
		return daxa::NativeWindowPlatform::WAYLAND_API
	}

#endif

	return daxa::NativeWindowPlatform::UNKNOWN;
}

daxa::NativeWindowHandle getNativeWindowHandle(SDL_Window* const window)
{
	auto windowProperties{ SDL_GetWindowProperties(window) };

#ifdef _WIN32

	return (HWND)SDL_GetPointerProperty(windowProperties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);

#elifdef __linux__

	switch (getNativeWindowPlatform())
	{ // todo: verify the SDL enums and are any casts required?
	case daxa::NativeWindowPlatform::WAYLAND_API: 
		return SDL_GetPointerProperty(windowProperties, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
	case daxa::NativeWindowPlatform::XLIB_API:
	default:
		return SDL_GetPointerProperty(windowProperties, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, nullptr);
	}

#endif
}


void uploadVertexDataTask(daxa::TaskGraph& tg, daxa::TaskBufferView vertices)
{
	tg.add_task({ 
		.attachments{ daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_WRITE, vertices) },
		.task{ [=](daxa::TaskInterface ti) {
				auto data = std::array{
					Vertex{.pos = {-0.5f, +0.5f, 0.0f}, .color = {1.0f, 0.0f, 0.0f}},
					Vertex{.pos = {+0.5f, +0.5f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},
					Vertex{.pos = {+0.0f, -0.5f, 0.0f}, .color = {0.0f, 0.0f, 1.0f}},
				};
				auto stagingBufferId{ ti.device.create_buffer({
					.size{ sizeof(Vertex) * 3 },
					.allocate_info{ daxa::MemoryFlagBits::HOST_ACCESS_RANDOM },
					.name{ "my staging buffer" },
				}) };
				ti.recorder.destroy_buffer_deferred(stagingBufferId);

				auto* bufferPtr{ ti.device.buffer_host_address_as<std::array<Vertex, 3>>(stagingBufferId).value() };
				*bufferPtr = data;
				ti.recorder.copy_buffer_to_buffer({
					.src_buffer{ stagingBufferId },
					.dst_buffer{ ti.get(vertices).ids[0] },
					.size{ 3 * sizeof(Vertex) },
					});
			}},
		});
}

void drawVerticesTask(daxa::TaskGraph& tg, std::shared_ptr<daxa::RasterPipeline> pipeline, daxa::TaskBufferView vertices, daxa::TaskImageView renderTarget)
{
	tg.add_task({
		.attachments{
			daxa::inl_attachment(daxa::TaskBufferAccess::VERTEX_SHADER_READ, vertices),
			daxa::inl_attachment(daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageViewType::REGULAR_2D, renderTarget),
		},
		.task{ [=](daxa::TaskInterface ti) 
		{
			const auto size = ti.device.info(ti.get(renderTarget).ids[0]).value().size;

			daxa::RenderCommandRecorder renderRecorder{ std::move(ti.recorder).begin_renderpass({
				.color_attachments{ std::array{
					daxa::RenderAttachmentInfo{
						.image_view{ ti.get(renderTarget).view_ids[0] },
						.load_op{ daxa::AttachmentLoadOp::CLEAR },
						.clear_value{ std::array<daxa::f32, 4>{ 0.0f, 0.0f, 0.0f, 1.0f } },
					},
				} },
				.render_area{ .width{ size.x }, .height{ size.y } },
			}) };

			renderRecorder.set_pipeline(*pipeline);
			renderRecorder.push_constant(PushConstant{
				.vertexPtr{ ti.device.device_address(ti.get(vertices).ids[0]).value() },
			});
			renderRecorder.draw({ .vertex_count{ 3 } });
			ti.recorder = std::move(renderRecorder).end_renderpass();
		} },
		.name{ "draw vertices" },
	});
}

int main()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		std::cerr << "SDL couldn't initialize video.\n";
	}

	SDL_WindowFlags windowFlags{ SDL_WINDOW_VULKAN };
	SDL_Window* window = SDL_CreateWindow("Green Hour", 640, 480, windowFlags);

	//auto windowProperties{ SDL_GetWindowProperties(window) };

	Renderer renderer{};
	//renderer.nativeWinHandle = (HWND)SDL_GetPointerProperty(windowProperties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
	renderer.nativeWinHandle = getNativeWindowHandle(window);
	renderer.instance = daxa::create_instance({});
	renderer.device = renderer.instance.create_device_2(renderer.instance.choose_device({}, {}));

	renderer.swapchain = renderer.device.create_swapchain({
		.native_window{ renderer.nativeWinHandle },
		.native_window_platform{ daxa::NativeWindowPlatform::WIN32_API },
		.surface_format_selector{ [](daxa::Format format)
		{
			switch (format)
			{
				case daxa::Format::R8G8B8A8_UINT: return 100;
				default: return daxa::default_format_score(format);
			}
		} },
		.present_mode{ daxa::PresentMode::MAILBOX },
		.image_usage{ daxa::ImageUsageFlagBits::TRANSFER_DST },
		.name{ "swapchain" },
	});

	renderer.pipelineManager = daxa::PipelineManager({
		.device = renderer.device,
		.root_paths = {
			DAXA_SHADER_INCLUDE_DIR,
			"./../src/shaders",
		},
		.default_language{ daxa::ShaderLanguage::GLSL },
		.default_enable_debug_info{ true },
		.name = "my pipeline manager",
	});

	std::shared_ptr<daxa::RasterPipeline> pipeline{};
	{
		auto result{ renderer.pipelineManager.add_raster_pipeline2({
				.vertex_shader_info{ daxa::ShaderCompileInfo2{.source{ daxa::ShaderFile{ "main.glsl" } } } },
				.fragment_shader_info{ daxa::ShaderCompileInfo2{.source{ daxa::ShaderFile{ "main.glsl" } } } },
				.color_attachments{ {.format{ renderer.swapchain.get_format() } } },
				.raster{},
				.push_constant_size{ sizeof(PushConstant) },
				.name{ "my pipeline" },
			}) };

		if (result.is_err())
		{
			std::cerr << result.message() << std::endl;
			return -1;
		}

		pipeline = result.value();
	}

	auto bufferId{ renderer.device.create_buffer({
		.size{ 3 * sizeof(Vertex) },
		.name{ "my vertex data" },
		}) };

	auto taskSwapchainImage{ daxa::TaskImage{ {.swapchain_image{ true }, .name{ "swapchain image" } } } };

	daxa::TaskBuffer taskVertexBuffer({
		.initial_buffers{.buffers{ std::span{ &bufferId, 1 } } },
		.name{ "task vertex buffer" },
		});

	auto loopTaskGraph{ daxa::TaskGraph({
		.device{ renderer.device },
		.swapchain{ renderer.swapchain },
		.name{ "loop" },
	}) };

	loopTaskGraph.use_persistent_buffer(taskVertexBuffer);
	loopTaskGraph.use_persistent_image(taskSwapchainImage);

	drawVerticesTask(loopTaskGraph, pipeline, taskVertexBuffer, taskSwapchainImage);

	loopTaskGraph.submit({});
	loopTaskGraph.present({});
	loopTaskGraph.complete({});

	Scene scene{};
	scene.createEntity("Frog");

	{
		daxa::TaskGraph uploadTaskGraph{ {.device{ renderer.device }, .name{ "upload" } } };

		uploadTaskGraph.use_persistent_buffer(taskVertexBuffer);

		uploadVertexDataTask(uploadTaskGraph, taskVertexBuffer);

		uploadTaskGraph.submit({});
		uploadTaskGraph.complete({});
		uploadTaskGraph.execute({});
	}

	bool windowShouldClose{ false };

	while (!windowShouldClose)
	{
		SDL_Event e{};
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_EVENT_QUIT)
			{
				windowShouldClose = true;
			}
		}

		auto swapchainImage{ renderer.swapchain.acquire_next_image() };
		if (swapchainImage.is_empty())
		{
			continue;
		}

		taskSwapchainImage.set_images({ .images{ std::span{ &swapchainImage, 1 } } });

		loopTaskGraph.execute({});

		renderer.device.collect_garbage();
	}

	renderer.device.destroy_buffer(bufferId);

	renderer.device.wait_idle();
	renderer.device.collect_garbage();

	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
