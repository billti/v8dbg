# DbgExt
This project shows a basic example of a debugger extension. It uses the DataModel
as much as possible (see [DataModel Manager]) via the native interfaces.

The source in the root directory is a generic starting point for implementation.
The custom code should implement the two methods declared near the top of
`dbgext.h` to initialize and clean-up, and make use of the two globals declared
there also.

## Building

Open a `Native x64 Developer Tools` command prompt installed by VS 2019.
Create a .\x64 directory under the project and CD into it
To create the build files run: `cmake -G Ninja ../`
To build, in the same directory run: `ninja`
(or, from the root directory, run `cmake --build ./x64`).

The resulting dbgext.dll and pdb file should be generated in the build directory.

### Release builds

The above will create a debug build by default. To build a release build, create
a release directory to use and change the CMake command to:

`cmake -G Ninja -D CMAKE_BUILD_TYPE=RelWithDebInfo ../`

## Debugging the extension

To debug the extension, launch a WinDbgx instance to debug with an active target, e.g.

`windbgx \src\github\v8\out\x64.debug\d8.exe -e "console.log('hello');"`

The WinDbgx process itself does not host the extensions, but a helper process.
Attach another instance of WinDbgx to the enghost.exe helper process, e.g.

`windbgx -pn enghost.exe`

Set a breakpoint in the session for when the extension initializes, e.g.

`bm dbgext!DebugExtensionInitialize`
`bm dbgext!V8ObjectContentsProperty::GetValue`

Load the extension in the target debugger (first session), which should trigger the breakpoint.

`.load "C:\\src\\github\\dbgext\\x64\\dbgext.dll"`

Note: For D8, `d8_exe!v8::Shell::RunMain` or `ExecuteString` is a good breakpoint to set.

`bp d8_exe!v8::Shell::ExecuteString`

Then trigger the `v8::internal::Object` model via something like:

`dx (d8_exe!v8::internal::Object*)source.val_`

[DataModel Manager]: https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/data-model-cpp-objects#-the-data-model-manager
