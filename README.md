## Synopsis

We present a new storage scheme for computer graphic images based on OpenEXR 2.

Using such EXR/Id files, the compositing artist can isolate an object selection (by picking them or using a regular expression to match their names) and color correct them with no edge artefact, which was not possible to achieve without rendering the object selection on its own layer.

The technique is demonstrated in an open source software suite, including a library to read and write the EXR/Id files, Nuke deep plug-ins and an OpenFX plug-in which generate the images in any compositing software.

## Motivation

During animation and VFX productions, the compositing department needs the rendering team to render some id masks images or to separate the rendering in layers to perform compositing operations
on parts of the images. The needed mask or layer may change from a shot to another one. A lot of images may be sometimes requiered.
Sometime masks are missing, the shot goes back to the rendering departement.

We think having a file format able to isolate any object may smooth this workflow.

## Download

Download the latest binary plug-ins from the [Release section](https://github.com/MercenariesEngineering/openexrid/releases)

## Video tutorials

[![Open in Youtube](http://img.youtube.com/vi/Ucn3KE3JDFA/0.jpg)](http://www.youtube.com/watch?v=Ucn3KE3JDFA&t=3m58s "Open in Youtube")

[![Open in Youtube](https://img.youtube.com/vi/W_ltvSMrwnQ/0.jpg)](http://www.youtube.com/watch?v=W_ltvSMrwnQ "Open in Youtube")

## Architecture

The dynamic image is a standard Deep OpenEXR image containing the coverage information of every object in every pixel.

The renderer writes such openexrid image during the rendering.

The Nuke plug-ins can isolate any object from the deep image flow.

The OpenFX plug-in isolates any object in your prefered compositing tool from an openexrid image.

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
