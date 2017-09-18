# TX81Z Programmer

https://mgregory22.me/tx81z/programmer.html

This is an MS-Windows software editor for the 1987 Yamaha TX81Z synthesizer module.

The TX81Z Programmer was originally released on May 25, 2005.  I originally released it as $20 shareware, but now I've decided to release it as open source under the GNU Public License version 2.

This code was originally written on Visual C++ 6.  I now have it configured to compile on VS2015 Community Edition.

## Software Dependencies

* [Visual Studio Community Edition 2017](https://www.visualstudio.com/thank-you-downloading-visual-studio/?sku=Community)
* [Microsoft Visual Studio 2017 Installer Projects](https://marketplace.visualstudio.com/items?itemName=VisualStudioProductTeam.MicrosoftVisualStudio2017InstallerProjects) to build the installer project
* [Microsoft HTML Help Workshop 1.3](https://msdn.microsoft.com/en-us/library/windows/desktop/ms669985.aspx) to build the HTML Help file

## File Layout

* Directories:
    * doc/ - docs and HTML Help file project
    * setup/ - installer project
    * src/ - executable project
    * src/wnd/ - application windows created using CreateWindow
    * src/dlg/ - application windows created using CreateDialog (ie they have a dialog resource, which is usually made with the dialog editor)
    * src/ctrl/ - custom dialog controls
    * src/msg/ - general code library (linked list, files, paths, error handling, random numbers, string processing, etc.)
    * src/gui/ - general gui library
    * src/midi/ - low-level MIDI library (send sysex message, etc.)
* Files:
    * src/prog.h - global constants
    * src/prog.c - global procedures and WinMain(), which creates the main window
    * src/wnd/mainWnd.c - main window
    * src/tx81z.c - high-level TX81Z-specific MIDI library (send voice, etc.)
    * src/freqratios.c - tables of oscillator frequencies
    * src/tx81z\_init.c - tables of init data
    * src/tx81z\_meta.c - structures for representing the TX81Z's internal state

## Conventions

* Each .c file contains 8 sections. The first 6 sections declare what's in the file:
    * Global Constants
    * Global Procedures
    * Global Variables
    * Unit (file scope) Constants
    * Unit Procedures
    * Unit Variables
* The next 2 sections are:
    * Global Procedure definitions
    * Unit Procedure definitions
* The global declaration sections are duplicated from the header files.  I did this to make it easy to browse the contents of the file and jump from declaration to definition in Vim.  It's probably not that useful in Visual Studio.
* A global identifier is composed of the name of the module, an underscore, and the procedure name.
* A unit identifier is similar but begins with a two letter lowercase abbreviation.
* The files freqratios.c, tx81z\_inits.c, and tx81z\_meta.c are #included by other .c files where they belong (just to get them out of my face), so they are not included in the project.

