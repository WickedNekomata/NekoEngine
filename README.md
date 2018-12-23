# Neko Engine

Neko Engine is a 3D game engine developed by two students from CITM-UPC Terrassa, Sandra Alvarez and Guillem Costa.

- Sandra's GitHub account: [Sandruski](https://github.com/Sandruski)
- Guillem's GitHub account: [DatBeQuiet](https://github.com/DatBeQuiet)
- Our GitHub organization: [WickedNekomata](https://github.com/WickedNekomata)

## About the engine

The code is written in C++.

- GitHub repository: [NekoEngine](https://github.com/WickedNekomata/NekoEngine)
- GitHub website: [NekoEngine](https://wickednekomata.github.io/NekoEngine/)

### Innovations

- Skybox
- Normal map (multitexturing with shaders). E.g.: the Brick Wall uses it
- Uniforms are shown in Inspector and can be modified in real-time from there
- Shader program binary

### Shaders sub-system

We have created 2 different waters using different shaders, a realistic ocean-like water and a cartoonish lake-like water.
We have also created a Brick Wall to show better how the Normal Map can be used in our 'normalMapShader'.

#### How to create a new shader?
1. Go to Edit -> Shader Editor.
2. Choose a name for your new shader program.
3. Add a new Vertex and Fragment.

4. Go to the Assets panel

If you do not have any shader objects created...
4.1 Mouse right click on a folder and Create Vertex Shader and Create Fragment Shader.
4.2 Choose a name for your new shader objects.

NOTE (Assets panel): if you click on a shader object, you can Edit Shader Object.

5. Drag and drop your Vertex Shader and your Fragment Shader from the Assets panel to the Shader Editor ('Empty Vertex' and 'Empty Fragment').

NOTE (Shader Editor): you can Edit a shader object.

6. Link and Save your new shader program.

#### How to modify an existing shader?
1. Go to the Assets panel.
2. If you click on a shader program, you can Edit Shader Program and Edit its shader objects.

NOTE: if you edit a shader object from a shader program that is being used by a game object, do not forget to:
- Compile and Save the shader object.
- Link and Save the shader program.
Any changes in the shader program will be automatically seen in the game object.
If the shader program that has been modified uses any Uniforms, those will be reset.

#### How to assign a shader to a material?
1. Click on the game object that you want to assign the shader to (it must have a Material Component).
2. Go to the Assets panel.
3. Drag and drop the shader program from the Assets panel to the Inspector ('Shader').
4. If the shader has uniforms, you can modify them in real-time from here.

NOTE: Default Shader uses Albedo and Specular textures. Use the 'normalMapShader' if you also want to use the Normal Map texture.

#### Other
Assets panel:
- Delete a shader object/program: mouse right click on a shader object/program and Delete Shader.
- Shaders are fully integrated with the Resource Manager, so you can also create, modify and delete them from outside the engine.
If you do it, do not forget to press the Refresh button.
- Copy Shaders Into Library: copies all the shaders found in the Assets folder to the Library folder, since shaders only live in the Assets folder.
Only useful when compiling for the Game.exe.

Code Editor:
- You can save shader objects that have not compiled (Compile and Save).

Shader Editor:
- Shader programs that have not linked will not be saved (Link and Save).

### Features

#### Game Objects
- Drag and drop game objects in the Hierarchy.
- Create/Delete game objects by right-clicking them at Hierarchy.
- Delete selected game object at Hierarchy by pressing the Delete key.

- Create/Delete components in Inspector.
- Swap/Reorder components in Inspector by drag and drop them through the 'Move' button.
- If more than one camera is set as main, play mode cannot be entered.

#### Resource Manager
Assets panel:
- The entire Assets folder is shown in real-time at the Assets panel.
- If any file (asset or meta) is removed/moved/renamed/changed from OS (offline mode), click the 'Refresh' button to update the affected resources.
- Dropped files will be imported.
- When importing a model, if it has a texture, the engine will search for it in the Assets folder. If it exists, it will be assigned to the model.

**IMPORTANT: after dropping any file into the engine or changing something in the Assets folder from outside the engine, push REFRESH button at Assets panel.
If the button is not clicked when one of this situations happens, the Assets panel will show the new files but resources will not be updated, so they will not work properly (invalid resource). 
Sorry for the inconvenience!**

- Drag and drop any texture to a material component.
- For models (fbx, dae and obj), open them to see its associated resources and drag and drop them to a mesh component.

- Select any texture or model and see its current import settings at Inspector. Change them and reimport the asset.

Library panel:
- Drag and drop any resource from the Library panel to an equivalent component.

- Select any resource and see its information at Inspector.

#### Scene Serialization
- Each new model in Assets generates a scene. To load the scene, drag and drop it from the Assets panel to the Hierarchy.
- Save the current scene or load a scene from Menu->File->Save Scene/Load Scene.

#### Options
- At Debug Draw panel, change the current debug draw options.
- At Edit panel, change between guizmo operations (shortcuts: W, E, R) and/or modes (shortcut: T).
- At Edit panel, enter Play mode. When entering Play mode, the scene is saved in memory.
- From Play mode, Pause or Tick the simulation.
- When leaving Play mode and entering Editor mode, the scene is loaded from memory.

#### Other
- At Settings panel, Scene section, check the quadtree structure. For debug purposes, random static game objects can be created (they will be automatically added to the quadtree).
- At Settings panel, Time Manager section, check the values of the game clock and the real time clock.


If something is not working as expected, please, revise console panel to understand what is happening.
If, after that, you assume the engine is running an error, revise issues or send a new one (we would be very grateful).

### Controls

#### Camera
- Mouse Hold Right:
	- WASD: move forward, left, backward and right
	- QE: move up and down
	- Look around
- F: look at target
- Hold Alt:
	- Mouse Hold Left: look around target
- Mouse Wheel: zoom in/out
- Mouse Hold Shift: movement speed x2, zoom speed x0.5

#### Windows
- Hold Alt:
	- I: open/close Inspector panel
	- S: open/close Settings panel
	- C: open/close Console panel
	- H: open/close Hierarchy panel
	- A: open/close Assets panel
	- D: open/close DebugDraw panel
	- E: open/close Edit panel

#### Other
- Change ImGuizmo operation/mode:
	- W: Translate operation
	- E: Rotate operation
	- R: Scale operation
	- T: World/Local mode

### Game exe

Executing Game.exe you will execute a build generated by the engine. 
The current scene for the Game.exe is stored at Settings. Please, do not remove it.

### Tools used to develop the engine

- IDE: Microsoft Visual Studio 2017
- External libraries: 
	- SDL 2.0.8
	- OpenGL 3.1
	- Glew 2.1
	- Assimp 4.1
	- DevIL 1.8
	- ImGui 1.66 WIP
	- ImGuiColorTextEdit 1.0
	- ImGuizmo 1.0
	- MathGeoLib 1.5
	- PCG Random Number Generator 0.94
	- Parson
	- PhysFS 3.0.1
	- MMGR

## License

MIT License

Copyright (c) 2018 WickedNekomata (Sandra Alvarez & Guillem Costa)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.