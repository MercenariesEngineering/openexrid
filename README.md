## Synopsis

This project introduces an open solution to provide dynamic mask images of a 3d rendered image in any compositing software.

Using the openidmask node, the user of the compositing software will select a set of rendered objects (by picking them, typing their names or using a regular expression to match their names).
The node will render a perfect antialized mask for the object selection.

## Motivation

During animation and VFX productions, the compositing team needs the rendering team to render some id masks images to perform compositing operations
on parts of the images. The needed mask images may change from a shot to another one. A lot of mask images may be sometimes requiered.
Sometime masks are missing, the shot goes back to the rendering departement.

We think having a dynamic mask solution may smooth this workflow.

## Architecture

The dynamic mask id image is a deep EXR image containing the coverage information of every object in every pixel.

The renderer uses the libopenidmask library to build the mask id image during the rendering.

The OpenFX plug-in reads and generates the mask in your prefered compositing tool.

## Tests

Compile and run the /test application.

## Contributors

Mercenaries Engineering, the guys behind Guerilla Render.
