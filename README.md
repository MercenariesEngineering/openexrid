## Synopsis

We present a new storage scheme for computer graphic images based on OpenEXR 2.

Using such EXR/Id files, the compositing artist can isolate an object selection (by picking them or using a regular expression to match their names) and color correct them with no edge artefact, which was not possible to achieve without rendering the object selection on its own layer.

The technique is demonstrated in an open source software suite, including a library to read and write the EXR/Id files and an OpenFX plug-in which generates the images in any compositing software.

## Motivation

During animation and VFX productions, the compositing department needs the rendering team to render some id masks images or to separate the rendering in layers to perform compositing operations
on parts of the images. The needed mask or layer may change from a shot to another one. A lot of images may be sometimes requiered.
Sometime masks are missing, the shot goes back to the rendering departement.

We think having a file format able to isolate any object may smooth this workflow.

## Architecture

The dynamic image is an OpenEXR 2 image containing the coverage information of every object in every pixel.

The renderer uses the openexrid library to build the mask id image during the rendering.

The OpenFX plug-in reads and generates the mask in your prefered compositing tool.

## Compilation

See the INSTALL file

## Tests

Compile and run the /test application.

## Data

You can download those two sample files :

http://mercenariesengineering.github.io/openexrid/forest.exrid <br />
http://mercenariesengineering.github.io/openexrid/openexrid.exrid

## Contributors

Mercenaries Engineering, the guys behind Guerilla Render.
