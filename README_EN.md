<div align="center">
  <h1>🏝️ NavyEngine: Interactive Virtual Tour</h1>
  <p><strong>A custom C++ graphics engine built from scratch with OpenGL to simulate hyper-realistic landscapes.</strong></p>
  <p>🌎 <a href="README.md">Leer en Español</a></p>
  <br>
  <p>🎥 <strong><a href="https://youtu.be/yxzT-5iLvpk">Watch the Project DEMO Video on YouTube</a></strong></p>
</div>

---

## 📖 About the Project
**NavyEngine** is an advanced graphics application developed in C++ that provides an immersive first-person environment. The user can walk and explore the coastline of a detailed island while observing a physically simulated ocean. The main goal of this project is to demonstrate the integration capabilities of complex mathematics (FFT), rendering physics (PBR), and multimedia resource management in pure OpenGL, without relying on commercial engines like Unity or Unreal.

## 🌟 Key Features

* 🌊 **Realistic Ocean Simulation (FFT):** The water is not a flat texture; it is generated in real-time by evaluating the JONSWAP Spectrum and the Fast Fourier Transform via **Compute Shaders** in OpenGL 4.3+, simulating authentic wave physics.
* 🏝️ **Physically Based Rendering (PBR & IBL):** High-quality 3D models illuminated using an environmental map (Skybox) and Image-Based Lighting, ensuring materials like sand and stone react to light in a photorealistic manner.
* 🚶 **"Walking Simulator" Camera:** Immersive first-person (FPS) control system restricted to the horizontal plane and contained by an invisible *Bounding Box* system to prevent moving out of the island boundaries.
* 🎵 **Immersive Environmental Audio:** Natively integrated multi-threaded sound engine that plays looping ocean waves and breeze to maximize immersion.
* 🖥️ **Native User Interface (UI):** Main and interactive menu rendered entirely using textured quads over the main framebuffer.

## 🛠️ Technologies and Libraries Used
* **C++17** - Main language.
* **OpenGL 4.3+** - Graphics API (essential for ocean Compute Shaders).
* **GLFW / GLEW** - Window management, hardware events, and GL extensions.
* **GLM (OpenGL Mathematics)** - Linear algebra calculations, camera matrices, and transformations.
* **Assimp** - Complex model importation (`.glb`, `.obj`).
* **WinMM** - Native Windows system used for the audio engine.

---

## 🚀 How to Play (End User)
If you just want to try the virtual environment without touching any code:
1. Go to the **Releases** section on the right side of this repository.
2. Download the file **`RecorridoVirtual_Windows_x64.zip`**.
3. Extract the file to any folder on your computer.
4. Double-click `NavyEngine.exe` and enjoy the tour using **W, A, S, D** to move and the mouse to look around.

---

## 💻 Local Compilation (For Developers)

If you want to download the source code, modify the *shaders*, or compile it yourself:

### ⚠️ Important: 3D Models and Heavy Assets
To keep the repository clean and fast, heavy 3D models (`.glb`, `.obj`) are not included in the GitHub source code.
To compile and run the project locally without it crashing when loading models, you must:
1. Go to the **Releases** tab of this repository.
2. Download the resource file (e.g., named **`Assets_Models.zip`**).
3. Extract the model files directly into the `NavyEngine/NavyEngine/Modelos3D/` folder of your cloned project.

### Prerequisites
* Windows 10/11.
* **Visual Studio 2022** with the *"Desktop development with C++"* workload installed.
* Graphics card compatible with OpenGL 4.3 or higher.

### Compilation Steps
1. Clone this repository:
   ```bash
   git clone https://github.com/YOUR_USERNAME/Recorrido_Virtual.git
   ```
2. Make sure you have downloaded the 3D models from the Releases as indicated above.
3. Open the project folder and double-click the **`NavyEngine.sln`** file to open it in Visual Studio.
4. In the top bar of Visual Studio, ensure the build target is set to:
   * **Configuration:** `Release` (or `Debug` if you want to see error logs).
   * **Platform:** `x64`.
5. Go to the **Build** menu and select **Rebuild Solution**.
6. Press **F5** (or the *Local Windows Debugger* button) to launch the application.

### Automated Packaging
Inside the root of the repository, you will find a script named **`create_release.ps1`**. If you have made changes and compiled in Release, simply open PowerShell in that folder and run the script. This program will handle copying the `.exe`, the `.dll`s, and all the *assets* folders to generate a clean `.zip` ready for distribution.

---

## 🕹️ Controls
* **Movement:** W (Forward), S (Backward), A (Left), D (Right).
* **View:** Move the mouse freely.
* **Exit:** Press the `ESC` key.

## 🤝 Credits
Developed as an advanced graphical integration project.
* *Ocean Implementation (FFT):* Compute Shaders adapted to the PBR environment.
* *Integration and UI:* Camera architecture, Menus, Audio, and Assets.
