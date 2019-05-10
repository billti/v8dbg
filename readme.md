# DbgExt
This project shows a basic example of a debugger extension. It uses the DataModel
as much as possible (see [DataModel Overview] and the [DataModel Manager]).

## Building

Open a `Native x64 Developer Tools` command prompt installed by VS 2019.
Create a .\x64 directory under the project and CD into it
To create the project files run: `cmake -G Ninja ../`
To build, in the same directory run: `ninja`

The resulting dbgext.dll and pdb file should be generated in the directory.

### Release builds

The above will create a debug build by default. To build a release build, create
a release directory to use and change the CMake command to:

`cmake -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo ../`

## Debugging the extension

To debug the extension, launch a WinDbgx instance to debug with an active target, e.g.

`windbgx notepad.exe`

The WinDbgx process itself does not host the extensions, but a helper process.
Attach another instance of WinDbgx to the enghost.exe helper process, e.g.

`windbgx -pn enghost.exe`

Set a breakpoint for when the extension initializes, e.g.

`bm dbgext!DebugExtensionInitialize`

Load the extension in the target debugger, which should trigger the breakpoint.

`.load "C:\\src\\github\\dbgext\\x64\\dbgext.dll"`

[DataModel Overview]: https://github.com/Microsoft/WinDbg-Libraries/tree/master/DbgModelCppLib
[DataModel Manager]: https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/data-model-cpp-objects#-the-data-model-manager
