# Neko Engine

Neko Engine is a 3D game engine currently being developed by two students from CITM-UPC Terrassa, Sandra Alvarez and Guillem Costa.

- Sandra's GitHub account: [Sandruski](https://github.com/Sandruski)
- Guillem's GitHub account: [DatBeQuiet](https://github.com/DatBeQuiet)
- Our GitHub organization: [WickedNekomata](https://github.com/WickedNekomata)

## About the engine

The code is written in C++.

- GitHub repository: [NekoEngine](https://github.com/WickedNekomata/NekoEngine)

### Features
- Gameobjects' hierarchy can be modified using drag and drop at panel hierarchy. User can also create/delete gameobject using right button to any gameobject at hierarchy and
  delete them using delete button from keyboard.
- User can create/swap/reorder components for gameobjects at inspector using the button "Add component" and drag and drop.
- The entire assets folder is shown at assets panel. If any asset is removed/moved/renamed from windows (offline mode), assets panel would show everything correctly.
- If any assets is modified in mode offline (example using photoshop), the asset would be reimported.
- User can drag and drop any texture to a material component using drag and drop. For fbx, dae and obj, user can "open" them to see its' associated resources and drag and drop
  them to a mesh component.
- User can drag and drop any resource from library to an equivalent component.
- User can select any Texture or fbx (and equivalent) and see the current import settings at inspector, change them and reimport it.
- For every new fbx/dae/obj at assets, a new scene would be generated. Using drag and drop with the scene in assets panel to hierachy panel the scene would be loaded.
- User is able to save the current scene or load any new scene at Menu->file->Save Scene/Load Scene.
- User can add Camera component to any gameobject and set any camera as main camera. If more than one camera is set as main, user would not be able to enter play mode.
- At debug draw panel user can change the current debug options for quadtree and bounding boxes.
- At edit panel user is able to swap guizmo mode (shortcut: 1,2,3), enter playmode and/or change playmode's delta time.

If something is not working as expected, please, revise console panel to understand what is happening.
If, after that, you assume the engine is running an error, revise issues or set a new one (we would be very grateful).

New features will be added soon in future releases. :)

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
- Hold Ctrl:
	- I: open/close Inspector panel
	- S: open/close Settings panel
	- C: open/close Console panel
	- H: open/close Hierarchy panel
	- A: open/close Assets panel
	- D: open/close DebugDraw panel

#### Other
- Swap Imguizmo mode:
	- 1: Translate mode
	- 2: Rotate Mode
	- 3: Scale Mode
	- 4: World/local mode

## GAME EXE
Executing Game.exe you will execute a build generated with the engine. The current scene for the game.exe is stored at Settings.
Please, do not remove the scene's associated assets from assets folder or the scene from settings.

### Tools used to develop the engine

- IDE: Microsoft Visual Studio 2017
- External libraries: 
	- SDL 2.0.8
	- OpenGL 3.1
	- Glew 2.1
	- Assimp 4.1
	- DevIL 1.8
	- ImGui 1.65
	- ImGuizmo 1.0
	- MathGeoLib 1.5
	- PCG Random Number Generator 0.94
	- Parson
	- PhysFS 3.0.1
	- MMGR

### Innovations

- Uvs Matrix: You can modify uvs transform for each texture on a gameobject.
- Docking and Viewports: ImGui docking & viewports beta has been implemented to the engine.

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