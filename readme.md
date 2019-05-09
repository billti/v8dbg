Open a `Native x64 Developer Tools` command prompt
Create a .\x64 directory under the project and CD into it
To create the project files: `cmake -G Ninja ../`
To build, in the same folder: `ninja`

The resulting dbgext.dll and pdb should be generated in the folder.
