# Neko Engine

Neko Engine is a 3D game engine currently being developed by two students from CITM-UPC Terrassa, Sandra Alvarez and Guillem Costa.

- Sandra's GitHub account: [Sandruski](https://github.com/Sandruski)
- Guillem's GitHub account: [DatBeQuiet](https://github.com/DatBeQuiet)
- Our GitHub organization: [WickedNekomata](https://github.com/WickedNekomata)

## About the engine

The code is written in C++.

- GitHub repository: [NekoEngine](https://github.com/WickedNekomata/NekoEngine)

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
- Hold Ctrl:
	- I: open/close Inspector panel
	- S: open/close Settings panel
	- C: open/close Console panel

### Tools used to develop the engine

- IDE: Microsoft Visual Studio 2017
- External libraries: 
	- SDL 2.0.8
	- OpenGL 3.1
	- Glew 2.1
	- Assimp 4.1
	- DevIL 1.8
	- ImGui 1.65
	- MathGeoLib 1.5
	- PCG Random Number Generator 0.94
	- Parson
	- PhysFS 3.0.1
	- MMGR

### Innovations

- File System (PHYSFS): used with the file 'config.json', to read and write.
- Multitexturing: you can add multiple textures to a single mesh by enabling the
multitexturing option in the Material section of the Inspector panel. To add a new
texture to a texture unit, first select the texture unit in which you want to add the 
texture and then drag and drop the texture. All the textures used by the texture
units will be blended together. (Use the example texture from the Textures folder
too see how a texture with alpha is blended. Be creative!)

## Assets

Both the Scarfy model and the tree model belong to the 3D artist [Mateo Costa](https://www.artstation.com/mateocosta).

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