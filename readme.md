## Building

Open a `Native x64 Developer Tools` command prompt
Create a .\x64 directory under the project and CD into it
To create the project files: `cmake -G Ninja ../`
To build, in the same folder: `ninja`

The resulting dbgext.dll and pdb should be generated in the folder.

### Release builds

To build a release build, create a release directory to use and change the CMake command to:

`cmake -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo ../`

## Debugging the extension

To debug, launch a WinDbgx instance to debug with an active target, e.g.

`windbgx notepad.exe`

The WinDbgx process itself does not host the extensions, but a helper process.
Attach another instance of WinDbgx to the dbgeng.exe host of the first, e.g.

`windbgx -pn enghost.exe`

Set a breakpoint in the new instance for when the extension initializes, e.g.

`bm dbgext!DebugExtensionInitialize`

Load the extension in the target debugger, which should trigger the breakpoint.

`.load "C:\\src\\github\\dbgext\\x64\\dbgext.dll"`
