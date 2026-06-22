<div align="center">
  <h1>NavyEngine: Recorrido Virtual Interactivo</h1>
  <p><strong>Un motor gráfico C++ personalizado construido desde cero con OpenGL para simular paisajes hiperrealistas.</strong></p>
  <p><a href="README_EN.md">Read this in English</a></p>
  <br>
  <p><strong><a href="https://youtu.be/yxzT-5iLvpk">Ver Video DEMO del Proyecto en YouTube</a></strong></p>
</div>

---

## Sobre el Proyecto
**NavyEngine** es una aplicación gráfica avanzada desarrollada en C++ que proporciona un entorno inmersivo en primera persona. El usuario puede caminar y explorar la costa de una isla detallada mientras observa un océano simulado físicamente. El objetivo principal de este proyecto es demostrar la capacidad de integración de matemáticas complejas (FFT), físicas de renderizado (PBR) y gestión de recursos multimedia en OpenGL puro, sin depender de motores comerciales como Unity o Unreal.

## Características Principales

* **Simulación Realista de Océano (FFT):** El agua no es una textura plana; se genera en tiempo real evaluando el Espectro de JONSWAP y la Transformada Rápida de Fourier mediante **Compute Shaders** de OpenGL 4.3+, simulando físicas de olas auténticas.
* **Renderizado Basado en Físicas (PBR & IBL):** Modelos 3D de alta calidad iluminados mediante un mapa ambiental (Skybox) e Image-Based Lighting, asegurando que materiales como la arena y la piedra reaccionen a la luz de forma fotorrealista.
* **Cámara "Walking Simulator":** Sistema de control inmersivo en primera persona (FPS) restringido al plano horizontal y contenido por un sistema de *Bounding Box* (Caja de colisión) invisible para evitar salir de los límites de la isla.
* **Audio Ambiental Inmersivo:** Motor de sonido multihilo integrado nativamente que reproduce bucles de olas y brisa para maximizar la inmersión.
* **Interfaz de Usuario (UI) Nativa:** Menú principal e interactivo renderizado totalmente mediante quads texturizados sobre el framebuffer principal.

## Tecnologías y Librerías Utilizadas
* **C++17** - Lenguaje principal.
* **OpenGL 4.3+** - API gráfica (esencial para los Compute Shaders del océano).
* **GLFW / GLEW** - Gestión de ventanas, eventos de hardware y extensiones GL.
* **GLM (OpenGL Mathematics)** - Cálculos de álgebra lineal, matrices de cámaras y transformaciones.
* **Assimp** - Importación de modelos complejos (`.glb`, `.obj`).
* **WinMM** - Sistema nativo de Windows utilizado para el motor de audio.

---

## Cómo Jugar (Usuario Final)
Si solo quieres probar el entorno virtual sin tocar código:
1. Ve a la sección de **Releases** a la derecha de este repositorio.
2. Descarga el archivo **`RecorridoVirtual_Windows_x64.zip`**.
3. Extrae el archivo en cualquier carpeta de tu computadora.
4. Haz doble clic en `NavyEngine.exe` y disfruta del recorrido usando **W, A, S, D** para moverte y el ratón para mirar.

---

## Compilación Local (Para Desarrolladores)

Si deseas descargar el código fuente, modificar los *shaders* o compilarlo por ti mismo:

### Importante: Modelos 3D y Assets Pesados
Para mantener el repositorio limpio y rápido, los modelos 3D pesados (`.glb`, `.obj`) no están incluidos en el código fuente de GitHub.
Para poder compilar y ejecutar el proyecto localmente sin que falle al cargar los modelos, debes:
1. Ir a la pestaña **Releases** de este repositorio.
2. Descargar el archivo de recursos (por ejemplo, llamado **`Assets_Modelos.zip`**).
3. Descomprimir los archivos de los modelos directamente dentro de la carpeta `NavyEngine/NavyEngine/Modelos3D/` de tu proyecto clonado.

### Requisitos Previos
* Windows 10/11.
* **Visual Studio 2022** con el paquete *"Desarrollo para el escritorio con C++"* instalado.
* Tarjeta gráfica compatible con OpenGL 4.3 o superior.

### Pasos para Compilar
1. Clona este repositorio:
   ```bash
   git clone https://github.com/TU_USUARIO/Recorrido_Virtual.git
   ```
2. Asegúrate de haber descargado los modelos 3D desde los Releases como se indicó arriba.
3. Abre la carpeta del proyecto y haz doble clic en el archivo **`NavyEngine.sln`** para abrirlo en Visual Studio.
4. En la barra superior de Visual Studio, asegúrate de que el objetivo de compilación esté en:
   * **Configuración:** `Release` (o `Debug` si deseas ver logs de errores).
   * **Plataforma:** `x64`.
5. Ve al menú **Compilar (Build)** y selecciona **Recompilar Solución (Rebuild Solution)**.
6. Presiona **F5** (o el botón *Local Windows Debugger*) para lanzar la aplicación.

### Empaquetado Automático
Dentro de la raíz del repositorio, encontrarás un script llamado **`create_release.ps1`**. Si has realizado cambios y compilado en Release, simplemente abre PowerShell en esa carpeta y ejecuta el script. Este programa se encargará de copiar el `.exe`, las `.dll` y todas las carpetas de *assets* para generar un `.zip` limpio y listo para distribuir.

---

## Controles
* **Movimiento:** W (Adelante), S (Atrás), A (Izquierda), D (Derecha).
* **Vista:** Mueve el ratón libremente.
* **Salir:** Presiona la tecla `ESC`.

## Créditos
Desarrollado como proyecto de integración gráfica avanzada.
* *Implementación de Océano (FFT):* Compute Shaders adaptados al entorno PBR.
* *Integración y UI:* Arquitectura de cámara, Menús, Audio y Assets.
