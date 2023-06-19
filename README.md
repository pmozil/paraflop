# Paraflop

![](assets/images/pic1.png)
![](assets/images/pic2.png)

This project contains chunks of code from
[the vulkan tutorial](https://vulkan-tutorial.com/) and
[Sacha Willems' vulkan samples](https://github.com/SaschaWillems/Vulkan). A big
thanks to the people who created those wondrous resources.

[**Also, very importantly, the vulkan specs**](https://registry.khronos.org/vulkan/site/guide/latest/vulkan_spec.html)

**Mind you this is an educational project. The author does not guarantee that the
code works, and you should double check any chunks of code you copy from here.**

The name was generated by
[this word does not exist](https://thisworddoesnotexist.com).

# Building

## Dependencies

You need Vulkan, GLM, GLFW, and the LunarG SDK. Here's the commands to instal
them on some systems.

## [There is no official glslc package for ubuntu, so just download this and copy it into `/usr/local/bin`](https://github.com/google/shaderc/blob/main/downloads.md)

## [Install the Khronos Texture Library library binaries for linking](https://github.com/KhronosGroup/KTX-Software)

## [Here's the `libktx` releases](https://github.com/KhronosGroup/KTX-Software/releases)

### Ubuntu

```
sudo apt install vulkan-tools \
libvulkan-dev \
vulkan-validationlayers-dev \
spirv-tools \
libglfw3-dev \
libglm-dev \
libxxf86vm-dev \
libxi-dev
```

### Fedora

```
sudo dnf install vulkan-tools \
vulkan-loader-devel \
mesa-vulkan-devel \
vulkan-validation-layers-devel \
glfw-devel \
glm-devel \
libXi-devel \
libXxf86vm-devel
```

### Arch

<b style="color: #ff0000">Use glfw-x11 is you are not on wayland</b>

```
sudo pacman -Syu vulkan-devel \
glfw-wayland \
glm \
libxi \
libxxf86vm
```

## Compiling

I mean, it's a CMake project. I'll still leave the commands here, just in case
you're lazy.

```bash
git clone --recursive https://github.com/pmozil/paraflop
cd paraflop

git submodule init
git submodule update
```

```bash
mkdir build
cd build
cmake ..
cmake --build ..
```

Now, to run this just do

```bash
./paraflop

```
