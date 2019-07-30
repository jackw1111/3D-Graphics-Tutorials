# Simple OpenGL collisions

This is a demo of how Kasper Fauerby's [Improved Collision detection and Response](www.peroxide.dk/papers/collision/collision.pdf) can be implemented in OpenGL 3.3+ using GLM.

The paper outlines a method to transform 3D space to ellipsoid space (e space) for detecting whether the radius of the ellipsoid collides with any triangle in the scene, and realistic sliding the ellipsoid off the triangle plane it collides with. It has a number of optimizations to allow the algorithm to exit early and save CPU cycles, and so the technique is fairly optimized and thus is a commonly used collision algorithm. I highly recommend going through the paper once as an overview, and at least once more to understand the details.

The implementation is demonstrated on a [Model loading demo](https://learnopengl.com/Model-Loading/Model) from the learnOpenGL tutorials and provides a starting basis for building an OpenGL game engine.

The C++ and OpenGL knowledge in this tutorial is assumed to be beginner-intermediate level, but if you have any issues with it you can always start from the start of the learnOpenGL series, and this will be a smooth transition once you have learnt up to loading meshes and models :)

# Requirements
- Same requirements as [learnOpenGL series](https://learnopengl.com/Getting-started/Creating-a-window).

# Building
- A Code::Blocks project is provided and it should be as easy as building and running.
- Also, a Makefile will be provided as well if you don't use Code::Blocks. (ie. `make` and `./bin/Release/learnOpenGL` to run)

# To Do:
- fix gravity

# References:
- [learnOpenGL - Joey de Vries ](www.learnopengl.com)
- [Peroxide - Kasper Fauerby] (http://www.peroxide.dk/)
- [Exengine, a C 3D game engine - exezin](https://github.com/exezin/exengine)
