# SynthToy
Portable modular synthesizer

SynthToy is set of modules; a library used to generate sounds from a modular synthesis graph, and accompanying graphical editor, written in C/GTK/Cairo.

It features:
- multi-threaded architecture (lock-free algorithms and synchronization)
- object-oriented design in C (using GObject-style classes)
- modular (separate components for GUI, synthesizer, several audio interfacing plugins)

Modules:
- Modular Synth Kit (MSK, libmsk) – a stand-alone library, capable of modeling and running a synthesis system; this library is controlled through an API, and its only requirement is the portability library Glib.
- Graphical MSK Editor (GMSK, libgmsk) – a library that adds a layer over libmsk, giving it a graphical editor; this editor can be integrated into any applications that have a render target supported by Cairo and Pango.
- mod-jack, mod-winmm etc – a set of libraries, each offering an interface to audio and MIDI devices.
- SynthToy – an application integrating all of the above, with the help of GTK+.

Screenshot:
<img src="http://i.imgur.com/MqMg0yD.png?1" />
