# Computer Graphics Final Assignment

This is an assignment made for the final project of Computer Graphics. It's a OpenGL application that shows a simple scene.

## Controls
WASD to move, mouse move/IJKL to pan, space to jump, v to switch into drone mode, ] to show debug information (if available), Shift+A to pause/resume animations.

## Requirements

- OpenGL
- FreeGLUT
- GLFW
- Glew
- GLM

## Steps to build
For Linux or Windows users with Cmake and make:
```console
$ git clone git@github.com:Stroopwafe1/CG-Final.git
$ cd CG-Final
$ cmake -S . -B build
$ cd build && make
$ ./build/CG_Final
```
For Windows, the solution file is added, open that and make sure you have the requirements installed. I suggest `vcpkg` for this.