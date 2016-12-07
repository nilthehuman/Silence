# __Silence__, an experimental 3D renderer

Status: pre-alpha (as in "doesn't work yet")

### What it is

__Silence__ is a proof-of-concept rendering program written in C++11 that uses
something called zones instead of rays to calculate light transport efficiently.
It also serves as the bulk of my bachelor's thesis.

__Silence__ is the sister project of another small rendering program called
__Retra__. __Retra__'s main purpose is to serve as a reference and benchmark
for __Silence__. They share the same featureset but __Retra__ is a vanilla path
tracer while __Silence__ uses a different, experimental algorithm.

### What it is not

__Silence__ is not an industry grade rendering tool. It is a concise prototype
built to explore the possibilities of a new algorithm for global illumination.

__Silence__ is not a valid solution for your GI assignment. As always you are
welcome to read the source, make changes, experiment and ask questions but you
probably got that assignment in the first place because you want to learn about
computer graphics or prove that you can build a renderer on your own.

### I want __Retra__ instead

Head on over to <https://github.com/nilthehuman/Retra> where __Retra__ and its
full source code is available under the same license terms as __Silence__.

## Installation

### Building __Silence__ from source

The best way to install __Silence__ is to build it from source on your machine.
The following instructions are for Unix-like operating systems but __Silence__
should be easy to build on Windows machines too with [MinGW][1]. You may have
to change the line endings from LF to CRLF on Windows.
The source is available on GitHub or as a tarball from [__TBA__]. Choose
whatever is convenient for you.

#### A. / Cloning from GitHub

Go to the directory you want to download to and copy the git repo like so:
```bash
git clone git://github.com/nilthehuman/Silence
```

#### B. / Downloading the tarball

Go to the directory you want to download to, get the tarball and extract it:
```bash
wget [...]
gunzip [file]
tar -xvf [file]
```

#### Compiling

A Makefile is provided with the source so you don't have to build the program
by hand. (GNU `make` is of course available in the official package repositories
of all common GNU/Linux distributions or in the [MinGW suite][1] for Windows.)

To compile __Silence__ with no graphical interface issue the following command in
the folder you downloaded it to:
```bash
make cli
```

Or, to compile _with_ graphical interface:
```bash
make gui
```

(Simply typing `make` builds both versions.)

If you see `==== Silence built successfully ====` you're golden. If the compiler
(or linker) complains about undefined stuff please refer to the "Dependencies"
section below.

The Makefile specifies g++ as the compiler. If you want to use a different
compiler just change the first two lines of the Makefile accordingly.
The following compilers have been tested and should work fine:
  * g++

### Pre-built packages

There are no pre-built binary packages available at this time.

## Dependencies

__Silence__ relies on...

  * [the C standard libraries][2] for math, string functions and assert
  * [the C++ Standard Library][3] for stream I/O, timing, containers and
  iterators
  * [OpenMP][4] for parallel processing

That's it, that's all you need to render to file. Since GCC comes with all of
these built-in you've probably already got them.

The graphical interface has the following additional dependencies:

  * OpenGL or an equivalent replacement (like [Mesa][5])
  * The OpenGL Utility Toolkit or an equivalent replacement (like [freeglut][6])

Both should be available in your favourite distro's official repositories.

## Usage

### Basic

Invoking __Silence__ as follows will output an image to `image.ppm`:

```bash
./silence scene.json
```

It is recommended to use the verbose flag to help with troubleshooting and show
you an estimate of the remaining time:

```bash
./silence scene.json -v
```

### Options

__Silence__ is only a proof of concept and doesn't have too many knobs on it.
Basically you can set the upper limit on __recursion depth__ and the __gamma
correction__ exponent.
Type `./silence -h` for a full list of command line options.

Example to render with a zone tree no deeper than 5 and no gamma correction:

```bash
./silence scene.json -v -d 5 -g 1
```

## Features

### The common featureset of __Retra__ and __Silence__

Below is a list of "external" features (stuff the user cares about) provided by
both __Retra__ and __Silence__.

  * Global illumination (obviously)
  * Three kinds of unscaled surface primitives:
    - Spheres
    - Planes
    - Triangles
  * Four kinds of basic BRDF's that can be combined to form more complicated
materials:
    - Diffuse
    - Metallic (Fresnel)
    - Reflecting
    - Refractive (dielectric)
  * Point lights
  * Area lights
  * Soft shadows
  * Gamma correction
  * A simple scene description format based on JSON
  * Image output in trivial image file format PPM
  * Graphical interface to display results on-the-fly
  * Some basic predefined object motions to demonstrate dynamic scenes
  * Verbose mode for troubleshooting and displaying an estimate of the remaining
rendering time

### "Internal" features of __Silence__

These are mainly implementation details users need not concern themselves with.

  * Background objects vs foreground objects
  * Back-face culling
  * Multi-threading (OpenMP)

### Optional later additions

These additional features might be added to the common featureset above (and
therefore to _both_ programs) at some later point, or they might not. Right now
they are all unsupported.

  * Ambient light
  * Octree / BSP tree

### Unfeatures

__Silence__ purposely does not and will not come with the following advanced
features.
This is for the sake of simplicity and keeping both the codebase and the set of
features small. (See the introductory section "What it is".)

  * Bidirectional path tracing
  * Metropolis light transport
  * Photon mapping
  * Spectral rendering
  * Blurred reflections
  * Depth of field effect
  * Object transformations
  * Anti-aliasing
  * Texture mapping
  * Bump mapping

## Known bugs and shortcomings

__Silence__ aims to be correct and unbiased, but it is not perfect and has some
bugs. Please do NOT rely on __Silence__ to produce ground truth renders.

  * No soft shadows yet
  * Indirect illumination is unreliable
  * __Silence__ assumes that all light zones are spawned in vacuum so if your
light source is inside a glass object it will look like the object is vacuum
and it's _the world_ that's made of glass

## Source tree

The __Silence__ source code and some marginal assets are organized as follows.

  * `src/core/`    -- Here be classes. All the actual image synthesis happens
  here. If you wish to use __Silence__'s engine in your own program, this is
  the part you need.
  * `src/gui/`     -- An interactive window where the image is rendered
  on-the-fly.
  * `src/parser/`  -- A non-strict good-enough parser for scene files.
  * `src/main.cpp` -- Provides a simple command line interface to __Silence__ or
  activates the GUI if required.
  See `./silence -h` for options.
  * `scenes/`      -- Some sample scene description files to get you started.
  A Cornell box and a few similar closed scenes with spheres.

Useful rules of thumb for keeping the interdependencies straight:

  * Source files in `src/core/` may not depend on files outside of `src/core/`.
  * Source files in `src/gui/` may not depend on files outside of `src/core/`
  and `src/gui/`.
  * Source files in `src/parser/` may not depend on files outside of `src/core/`
  and `src/parser/`. (An exception is `src/parser/parsemotion.cpp` depending on
  `src/gui/motion.h`.)

## Forking and Contributing

Fork away to your heart's content. [The License][7] not only allows but
encourages it. Do something amazing!

Pull requests are welcome too as long as they don't inflate the codebase, but I
may not always be quick to pull and merge your changes.
If you want to add features listed under "Unfeatures" please fork the project
instead.

## License

__Silence__ is free software released under the [GNU General Public License
version 3][7].
Please refer to the file named COPYING for more information.

Copyright 2016 Dániel Arató

## Frequently Asked Questions

#### __Q.__: How do I contact the maintainer?
__A.:__ You can reach me either in email or on GitHub.

Email: nil.the.human at gmail dot com

GitHub: https://github.com/nilthehuman

#### __Q.__: __Silence__ doesn't parse my scene file right.
__A.:__ You're probably missing a comma (`,`) between entries or a closing brace
(`}`) somewhere. Also please note that JSON uses brackets (`[`...`]`) instead of
parentheses (`(`...`)`) for tuples and lists. Try opening the file in a
syntax-aware editor.
Then again, the parser isn't perfect so if you've checked everything and you
still get an error please drop me a mail or a message.

#### __Q.__: Performance _sucks_.
__A.:__ __Silence__ is not an industry grade renderer and features no
optimizations to the basic zone tracing algorithm. The engine source is some
??? actual lines of code.
This is in keeping with the project's aim to be a compact demonstration of the
method I call zone tracing. If you're trying to use a small pure-CPU toy to
render blockbuster animations that's your problem anyway.

#### __Q.__: Why did you even bother specifying a license?
__A.:__ Good question! I understand you're tired of legal overhead and all the
free/non-free drama, but if you make programs or other digital content you
_should_ license them. In most jurisdictions any written work (programs
included) is [automatically copyrighted][8] once it is produced unless the
author expressly tells you how you can use it. Basically if you fail to license
your program and someone else downloads it and starts playing with the source
code they are already _commiting a crime_!
_Any_ free license is better than not having a license at all. If you can't be
bothered to care either way, just [slap a license on your work][9]. Pick GPL.
Or pick one at random. You'll be doing everyone a solid.
([Further reading.][10])

[1]:  http://www.mingw.org/wiki/HOWTO_Install_the_MinGW_GCC_Compiler_Suite
[2]:  http://www.gnu.org/software/libc/
[3]:  https://gcc.gnu.org/libstdc++/
[4]:  https://gcc.gnu.org/onlinedocs/libgomp/Enabling-OpenMP.html
[5]:  http://www.mesa3d.org/
[6]:  http://sourceforge.net/projects/freeglut/
[7]:  https://www.gnu.org/copyleft/gpl.html
[8]:  https://help.github.com/articles/open-source-licensing/#what-happens-if-i-dont-choose-a-license
[9]:  http://choosealicense.com/
[10]: http://www.softwareliberty.com/
