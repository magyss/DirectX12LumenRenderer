# DirectX12LumenRenderer
DirectX12LumenRenderer is a real-time rendering engine prototype built with C++ and DirectX 12, featuring an experimental Lumen-inspired global illumination system.

## ✨ Features

- Real-time ray tracing (DXR 1.1)
- Lumen-inspired global illumination
- Physically Based Rendering (PBR)
- Temporal Anti-Aliasing (TAA)
- HLSL shader pipeline
- Basic scene loading and model rendering

## Requirements

- Windows 10/11
- GPU with DX12 Ultimate support (for DXR)
- Visual Studio 2022
- CMake 3.20+

## Build Instructions

```bash
git clone https://github.com/magyss/DirectX12LumenRenderer.git
cd DirectX12LumenRenderer
mkdir build && cd build
cmake .. -G "Visual Studio Code"
cmake --build . --config Release
```

```markdown
## Project Structure

├── renderer/         # DirectX 12 rendering classes
├── scene/            # Meshes, camera, lighting
├── shaders/          # HLSL shaders
├── assets/           # Models, textures      
└── Main.cpp          # Entry point
```

## License

MIT — do what you want, just include this file.

## About

## 👤 About the Author

Hi! I'm a game developer and educator working at the intersection of technology, creativity, and teaching.  
I specialize in C++, graphics programming, and modern real-time rendering pipelines using DirectX 12, Vulkan, and Unreal Engine 5.

This project — **DirectX12LumenRenderer** — was created as part of my journey toward mastering high-performance graphics systems and exploring Lumen-style global illumination at a low level.

My current goals include:
- Developing real-time rendering tools from scratch using modern APIs
- Building advanced systems for lighting and ray tracing
- Teaching and documenting complex rendering concepts in a clear and approachable way

I’m also building a structured **6-month educational course** on graphics rendering with DirectX 12 and Unreal Engine 5 for beginners.

Feel free to check out my other work or reach out if you’re interested in graphics, game engines, or collaborative learning!

