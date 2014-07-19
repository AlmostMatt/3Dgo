3Dgo
====

General
_______

This directory contains a simple application that can be used as a starting 
point for the cs488 final project.

The environment is Simple Direct-Media Layer (SDL) with OpenGL for Linux.
It assumes that SDl and SDL-mixer libraries are installed.

For further information on SDL, you are encouraged to take a look on 
SDLr on-;line resources: http://www.libsdl.org

It also provides basic sound support.

The application renderes a simple OpenGL sphere and plays some sound when keys 'n', 'o' and 'p' are pressed or when the mouse butons are pressed.

Files
-----

SoundManager.h.cpp
	Sound support
SDL_GL.h.cpp 
	Environment setup: Create an OpenGL context using SDL
main.cpp
	Main file. Contains the rendering and the event handling


Makefile
--------

type make to compile the files
type make sample_tutorial to build the executable. The executable name is test

to run, type ./test

Sound Support
-------------

When built, the application creates an instance of SoundManager.
You can load wav file by simple call SM->LoadSound(char*). This function returns an integer which can be used to reference the sound:

SM->PlaySound(int )
SM->StopSound(int )
SM->PauseSound(int )
SM->ResumeSound(int )

