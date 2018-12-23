# Neko Engine

Neko Engine is a 3D game engine currently being developed by two students from CITM-UPC Terrassa, Sandra Alvarez and Guillem Costa. The entire development has been supervised by the lecturer Ricard Pillosu.

The code is written in C++.

- GitHub repository: [NekoEngine](https://github.com/WickedNekomata/NekoEngine)


## Team

![](team_photo.JPG)

- Sandra's GitHub account: [Sandruski](https://github.com/Sandruski)
- Guillem's GitHub account: [DatBeQuiet](https://github.com/DatBeQuiet)
- Our GitHub organization: [WickedNekomata](https://github.com/WickedNekomata)

## Main Core Subsystems

For the team, it was super important to keep every subsystem as simple as possible but always with the maximum efficiency that we could obtain.

### GameObjects (Entity-Component-Pattern)
In order to implement GameObjects, we followed the Entity-Component-Pattern for its modularity.
The components that can be attached are: Transform Component, Mesh Component, Material Component and Camera Component.

For a fast understanding, here you have a simple sheme with a short explanation.

![](GameObjectsSheme.png)

As you can see in the image above, there is a main class called Gameobject that have instances for every component attached.
Those instances are virtual classes that inherits from a base class called Component.

If we implement a n-Child tree of gameobjects using this component pattern we finally get a Unity-like gameobjects system. 

### Resource Manager (Reference Counting, Library and Metas)
Since the very firsts engine-system scratches, we wanted to achive an easy but efficient way to manage vram. We both approved that the best way to did it was using a Reference Counting system.

Basically, we have a Resources Manager that keeps track of every Resource (fbx, dds, shader, etc) and, thanks to a variable stored on them, we can count every reference to them. When a Resource goes from 0 to 1 references we load it on memory and the same goes when the references down from 1 to 0 but, in this case, unloading them from memory.

Assimp is a very nice library to import models but, because of the quantity of data, some formats are very low to read. In order to fix this issue, we stored all meshes in a very simple format on the Library folder. The same concept was applied for every resource so, at the end of the day, the time required for loading scenes was reduced by far.

We also wanted to allow the user to manage his assets in a unity-style method. In order to approach it, .metas joined the party. A .meta is a text file that stores the propierties of a resource and its uuid (unique identifier) and works as a link between the asset and the resource. 

### GUI (Using Imgui)
In any game engine an easy-to-use UI is a must and, thanks to Imgui library, this was super fast an simple to do.
The way we did it was having a GUI module that works as a manager for all imgui panels. To approach the inspector panel, we implemented an OnEditor method at each component so, when a gameobject is selected, we draw its components on the inspector.

## Shaders Subsystem

![](driver_graphics_pipeline.jpg)

[Image Source](https://developer.apple.com/library/archive/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_shaders/opengl_shaders.html)

## Video

## Source code and latest release

- Source: [NekoEngine](https://github.com/WickedNekomata/NekoEngine)

- Latest Release: [NekoEngine](https://github.com/WickedNekomata/NekoEngine/Releases)

### Tools used to develop the engine

- IDE: Microsoft Visual Studio 2017
- External libraries: 
	- SDL 2.0.8
	- OpenGL 3.1
	- Glew 2.1
	- Assimp 4.1
	- DevIL 1.8
	- ImGui 1.66 WIP
	- ImGuizmo 1.0
	- MathGeoLib 1.5
	- PCG Random Number Generator 0.94
	- Parson
	- PhysFS 3.0.1
	- MMGR
	- ImGuiColorTextEdit

## License

```
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
```
