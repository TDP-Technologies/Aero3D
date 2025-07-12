# Aero3D


# Build Instructions

This guide describes the steps to clone and build the project with Vulkan SDK support.

## Step 1. Clone the repository

```bash
git clone --recurse-submodules <repository-URL>
```

> If you already cloned the repository without recursive submodules, update them with:

```bash
git submodule update --init --recursive
```

## Step 2. Install Vulkan SDK

Make sure you have Vulkan SDK installed. You can download it from the official site:  
https://vulkan.lunarg.com/sdk/home

For Linux (e.g., Ubuntu) you can install via package manager or manually.

## Step 3. Sync shader dependencies

Open terminal and navigate to the folder:

```bash
cd vendor/shaderc/utils
```

Execute python file 

```bash
python git-sync-deps.py
```

## Step 4. Build the project

Go back to the root directory and run:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja -S . -B build && cmake --build build
```

This will configure a Debug build using the Ninja generator and then build the project.

---

If you have any questions or issues, feel free to ask! This guide may wont work on specific platform, so if you cant build the project write on my email
or open issue and I will 100% help you.