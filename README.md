# Rythm
### Goal
Rythm is intended to be my extensible framework for creating interactive 3D and 2D applications, known as games. Rythm does not aim to be a generalist "Engine" but instead aims to be a framework built specifically for my collection of games.

### Build & Run

#### Requirements
This project utilizes [VulkanSDK](https://www.lunarg.com/products/vulkan-sdk/) which should be installed outside of this project in your local environment.

#### Linux
```
git submodule init --recursive
./scripts/build-dependencies.sh
./scripts/linux-debug.sh
```
