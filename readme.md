Open a `Native x64 Developer Tools` command prompt
Create a .\x64 directory under the project and CD into it
To create the project files: `cmake -G Ninja ../`
To build, in the same folder: `ninja`

The resulting dbgext.dll and pdb should be generated in the folder.


To build a release build, create a release directory to use and change the CMake command to:

`cmake -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo ../`
