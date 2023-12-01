# The Pixel Garage(WIP!)

A simple experimental Toy 3D Graphics engine with an idea to have multiple backend (API) / platform support initially starting with Vulka, D3D12 for Windows using C/C++. 
WIP for the initial planned features :
*D3D12 & Vulkan backend which is the key to its design is the integration of multiple backend APIs
*IMGUI for user interface rendering, ensuring a flexible and easy-to-use interface for developers
*Basic Ray Tracing Techniques: Implementation of ray tracing for shadows, ambient occlusion (AO), and reflections/lighting effects, contributing to advanced visual fidelity.
*Deferred rendering.
*Basic Bling Phong lighting
*GLTF model loading

#Roadmap features
* Scene/Framegraph : An organized structure for managing scenes and rendering sequences / passes
* Memory Allocators: Integration of Vulkan Memory Allocator (VMA) and Direct3D 12 Memory Allocator (D3D12MA) for optimized memory management.
* Material Pipeline: A robust pipeline for material processing and management.
* GPU Controlled Rendering: employ GPU-driven techniques to manage rendering tasks, allowing for more efficient resource utilization and real-time rendering adjustments such as early HiZ / Frustrum based culling techniques.
* Volumetric Rendering: Advanced rendering techniques to create realistic three-dimensional effects like fog, smoke, or clouds.
* Mesh Shading support: Introduces a more efficient approach to geometry processing, enhancing performance and scalability in rendering complex scenes.

