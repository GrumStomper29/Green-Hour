
#include "shaders/shared.inl"

#include <SDL3/SDL.h>
//#include <SDL3/SDL_syswm.h>

// just to show that everything is working
//#define VMA_IMPLEMENTATION
//#include <vk_mem_alloc.h>

#include <entt/entity/registry.hpp>

#include <vulkan/vulkan.h>
#include <daxa/daxa.hpp>

#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

#include <array>
#include <iostream>
#include <memory>
#include <span>

#include "windows.h"

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
	auto window = SDL_CreateWindow("Green Hour", 640, 480, windowFlags);

	auto windowProperties{ SDL_GetWindowProperties(window) };
	
	daxa::NativeWindowHandle nativeWinHandle = (HWND)SDL_GetPointerProperty(windowProperties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);

	daxa::Instance instance{ daxa::create_instance({}) };
	daxa::Device   device  { instance.create_device_2(instance.choose_device({}, {})) };

	daxa::Swapchain swapchain{ device.create_swapchain({
		.native_window{ nativeWinHandle },
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
	}) };
	
	auto pipelineManager = daxa::PipelineManager({
		.device = device,
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
		auto result{ pipelineManager.add_raster_pipeline2({
				.vertex_shader_info{ daxa::ShaderCompileInfo2{.source{ daxa::ShaderFile{ "main.glsl" } } } },
				.fragment_shader_info{ daxa::ShaderCompileInfo2{.source{ daxa::ShaderFile{ "main.glsl" } } } },
				.color_attachments{ { .format{ swapchain.get_format() } } },
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

	auto bufferId{ device.create_buffer({
		.size{ 3 * sizeof(Vertex) },
		.name{ "my vertex data" },
		}) };

	auto taskSwapchainImage{ daxa::TaskImage{ { .swapchain_image{ true }, .name{ "swapchain image" } } } };

	daxa::TaskBuffer taskVertexBuffer({
		.initial_buffers{.buffers{ std::span{ &bufferId, 1 } } },
		.name{ "task vertex buffer" },
	});

	auto loopTaskGraph{ daxa::TaskGraph({
		.device{ device },
		.swapchain{ swapchain },
		.name{ "loop" },
	}) };

	loopTaskGraph.use_persistent_buffer(taskVertexBuffer);
	loopTaskGraph.use_persistent_image(taskSwapchainImage);

	drawVerticesTask(loopTaskGraph, pipeline, taskVertexBuffer, taskSwapchainImage);

	loopTaskGraph.submit({});
	loopTaskGraph.present({});
	loopTaskGraph.complete({});

	entt::registry registry{};


	struct Transform
	{
		float x{};
	};

	auto entity{ registry.create() };

	registry.emplace<Transform>(entity);


	{
		daxa::TaskGraph uploadTaskGraph{ {.device{ device }, .name{ "upload" } } };

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

		auto swapchainImage{ swapchain.acquire_next_image() };
		if (swapchainImage.is_empty())
		{
			continue;
		}
		
		taskSwapchainImage.set_images({ .images{ std::span{ &swapchainImage, 1 } } });

		loopTaskGraph.execute({});
		
		device.collect_garbage();
	}

	registry.destroy(entity);

	device.destroy_buffer(bufferId);

	device.wait_idle();
	device.collect_garbage();

	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
