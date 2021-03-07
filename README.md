#### QFlexProp

QFlexProp is a GUI tool to compile source files for the [Parallax Inc.](https://parallax.com) Propeller 2.
The source can be one of various languages supported by the [spin2cpp](https://github.com/totalspectrum/spin2cpp/) tools by totalspectrum.

It integrates an interface to the `flexspin` binary from the `spin2cpp` package, and a (VT220) serial terminal emulator which can be configured to talk to a Propeller 2 board attached to a serial port of your PC.

It aims to be an alternative for [flexprop](https://github.com/totalspectrum/flexprop/) using Qt5 and its platform independent capabilities for the GUI elements and QSerialPort abstraction.

The current state of this project is: working to some extent.

There are, and probably always will be, lots of things left to do or to improve.
As of now QFlexProp has been built and tested only on [Void Linux](https://voidlinux.org) while in theory it should work on Windows and MacOS as well.
