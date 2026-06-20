# NavyEngine

**Simulation of a Virtual Marine Environment with OpenGL**

NavyEngine is an immersive 3D application that recreates a sea and an island using real-time computer graphics.

## Authors
* Carlos Alfaro
* Engel Gomez
* Jairo Flores

## Technologies Used
* C++
* OpenGL
* Blender

## Features

### Navigation Experience
* **Freedom of Movement:** Users can move in 360 degrees, allowing them to approach the shore of the island, climb to higher elevations, or move away towards the ocean horizon.
* **Intuitive Control:** Features a "flight" or "walk" control scheme, where the mouse directs the camera view and the keyboard controls movement.

### Implementation: Sea and Island
* **The Island:**
  * **Detailed Texturing:** The island utilizes rich textures to differentiate between sand, rock, and vegetation, rather than using solid colors.
  * **Sense of Scale:** Incorporates reference objects (such as rocks and vegetation) to give users a realistic perception of the island's size relative to their position.
* **The Sea:**
  * **Realistic Waves:** The sea features continuous ascending and descending motion, avoiding the appearance of a rigid plane.
  * **Sun Glare Effect:** Includes specular reflection, simulating the bright trail of light left by the sun on the water's surface.

### Atmosphere and Environment (Skybox)
* **Immersive Horizon:** Replaces plain black or gray backgrounds with a dynamic skybox featuring clouds and a clear horizon line where the sea meets the sky, eliminating any feeling of "emptiness" at the map's boundaries.
* **Global Illumination:** The entire scene is illuminated by a directional light (simulating the sun) that generates projected shadows from the island's objects onto the terrain, adding significant visual depth.
