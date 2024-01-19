# The Pixel Garage(WIP!)

An experimental Toy 3D Graphics framework (C/C++) with an idea to have Hybrid rendering capabilities along with multiple backend (API) / platform support, initially starting with Vulkan, D3D12 for Windows 10/11. 

#WIP for the initial planned features :
* D3D12 & Vulkan backend which is the key to its design is the integration of multiple backend APIs
* Memory Allocators: Integration of Vulkan Memory Allocator (VMA) and Direct3D 12 Memory Allocator (D3D12MA) for optimized memory management.
* IMGUI for user interface rendering, ensuring a flexible and easy-to-use interface for developers
* Basic Ray Tracing Techniques: Implementation of ray tracing for shadows, ambient occlusion (AO), and reflections/lighting effects, contributing to advanced visual fidelity.
* Deferred rendering (G-buffer with Rasterization).
* GLTF model loading

#Roadmap features
* Scene/Framegraph : An organized structure for managing scenes and rendering sequences / passes
* Material Pipeline: A robust pipeline for material processing and management.
* GPU Controlled Rendering: employ GPU-driven techniques to manage rendering tasks, allowing for more efficient resource utilization and real-time rendering adjustments such as early HiZ / Frustrum based culling techniques.


