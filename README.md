WingEngine is a small, simple to use, and efficient data-oriented 3d game engine for Wii, developed in C++. This wiki page presents its features and some internal technical details.
The engine is built using the Wii Homebrew development tools: devkitpro-ppc and libogc. WingEngine is a **work-in-progress**, some of its features are just a basic implementation meant to be expanded in next versions, as the engine gets incrementally constructed.


## Features

* **Core**: Foundation (bitsquid)
For the core functionalities, we used the _Bitsquid Foundation_ library, it provides basic data structures like array, queue/stack, hash, string stream, and basic memory allocation.

* **Mesh Loading**: _Assimp_ is used for model loading, so all formats supported by it can be loaded. The engine uses a custom mesh data format, mesh formats outputted from assimp can be converted using the command-line tool [MeshConv](https://github.com/rafarc/MeshConv). The mesh is a binary file, detailed in [this page](https://github.com/rafarc/WingEngine/wiki/File-Formats).


* **Material and Texture**: Currently, a simple material system is implemented. Materials can reference a diffuse texture, and contain sub-materials. The material system will be expanded to support more complex materials. The material is a binary file, detailed in the file formats [page](https://github.com/rafarc/WingEngine/wiki/File-Formats).


* **Particle System**: a simple implementation of particle system using billboards, bucket sort for depth sorting, and interpolators for color, position, size and rotation. This component will also be expanded in future versions, to provide a powerful flexible particle system.


* **Asset Management**: The asset management component ensures that assets are loaded into memory only once, even if multiple instances of them exist in the game.


* **Entity Management**: Similar to the asset management, this component manages the game entities, and make sure unique entities are loaded into memory only once. The entity manager is implemented using _packed arrays_.


* **Scene Graph**: The scene graph enables hierarchies to be formed between game entities. The transform of the children are pre multipled by the parent's. It is important to note that in WingEngine, the "Scene Graph" is just a concept, it is not a monolithic class coupled to the other engine components.


* **Console**: A simple console provides a basic yet essential debugging functionality. 


* **Text Rendering**: WingEngine uses the FreeTypeGX library to render text. 


* **Skybox**: Basic skybox implementation. This component will be extended in the future, to allow animated skyboxes, and other types like skydome and skyplane.


* **Collision Detection**: Routines for collision detection between AABBs, OOBBs and spheres.


## **Dependencies**: 
* [Bitsquid Foundation](https://bitbucket.org/bitsquid/foundation): basic data structures.
* [Devkitpro](http://devkitpro.org/): toolchains for homebrew development on Wii.
* [libogc](http://wiibrew.org/wiki/Libogc): provides access to the console hardware.
* [PNGU](http://wiibrew.org/wiki/PNGU): wrapper for libpng, facilitates handling of png files.
* [FreeTypeGX](http://wiibrew.org/wiki/FreeTypeGX): wrapper for freetype lib, for reading font files and rendering text
* [Assimp](http://assimp.sourceforge.net/): library for loading 3D meshes.

## Class Diagram

Currently these are the classes in the engine:
![](https://github.com/rafarc/WingEngine/blob/master/doc/classes.png)


## **Screenshots**: 
* Meshes

![](https://github.com/rafarc/WingEngine/blob/master/doc/screenshot_01.png)

* Particle system

![](https://github.com/rafarc/WingEngine/blob/master/doc/screenshot_02.png)
