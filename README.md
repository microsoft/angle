ANGLE -- Microsoft Windows Store branch
=====
ANGLE allows Windows users to seamlessly run OpenGL ES content by efficiently translating 
OpenGL ES API into DirectX 11 API calls.

This repository is maintained by Microsoft to support the use of ANGLE by Windows Store app developers. It contains several branches:
 - [ms-master](https://github.com/Microsoft/angle/tree/ms-master) - for Universal Windows 10 apps (UWP) and desktop apps
 - [ms-win8.1](https://github.com/Microsoft/angle/tree/ms-win8.1) - for Windows 8.1 and Windows Phone 8.1 apps
 - [ms-holographic-experimental](https://github.com/Microsoft/angle/tree/ms-holographic-experimental) - for experimental HoloLens support

ms-master contains a copy of ANGLE that is regularly updated from the ANGLE [master branch](https://code.google.com/p/angleproject).
It also contains recent changes made by Microsoft that have not yet been merged back to ANGLE master 
_(our goal is to eventually merge everything, but if you want the latest and greatest 
  Windows Store features, you will find them here first)_
  
The repository also contains:
- [Documentation](https://github.com/microsoft/angle/wiki) and 
  [project templates](https://github.com/Microsoft/angle/tree/ms-master/templates) 
  focused on Windows Store app development
- Sample code and utilities such as 
  [DDS](https://github.com/Microsoft/angle/wiki/Loading-textures-from-dds-files) and 
  [WIC](https://github.com/Microsoft/angle/wiki/Loading-textures-from-image-files) 
  texture loaders

The ms-master branch and ms-win8.1 branch contain the source code used to build the ANGLE binaries that we publish on NuGet. 
The NuGet package with Windows 10 (UWP) binaries is [available here](https://www.nuget.org/packages/ANGLE.WindowsStore). 
The NuGet package with Win8.1/Phone8.1 binaries is [available here](https://www.nuget.org/packages/ANGLE.WindowsStore.win81).
  
Feature Support
=====
ANGLE supports different versions of OpenGL ES depending on the capabilities of the underlying hardware. 
In particular, the supported version depends on which 
[D3D Feature Levels](https://msdn.microsoft.com/en-us/library/windows/desktop/ff476876%28v=vs.85%29.aspx) 
the hardware supports:

<table>
<tr>
<th>Hardware<br>Feature Levels</th>
<th>Example devices</th>
<th>What does ANGLE support?</th>
</tr>
<tr>
<td>
11_1<br>
11_0<br>
10_1<br>
</td>
<td>Modern Desktop PCs<br>Surface Pros</td>
<td>OpenGL ES 2.0<br> OpenGL ES 3.0</td>
</tr>
<tr>
<td>
10_0<br>
</td>
<td></td>
<td>OpenGL ES 2.0</td>
</tr>
<tr>
<td>
9_3
</td>
<td>Windows Phones</td>
<td>OpenGL ES 2.0 (except <a href=https://github.com/Microsoft/angle/wiki/Known-Issues>minor corner cases</a>)</td>
</tr>
<tr>
<td>
9_2<br>
9_1
</td>
<td>Surface RT</td>
<td>OpenGL ES 2.0 (via software emulation)</td>
</tr>
<tr>
<td>
None
</td>
<td>Raspberry Pi 2</td>
<td>OpenGL ES 2.0 (via software emulation)</td>
</tr>
</table>

Getting ANGLE
=====

There are two ways to get ANGLE for Windows Store applications:
  1. Download compiled ANGLE binaries as a [NuGet package](http://github.com/Microsoft/angle/wiki/How-To-Use-the-ANGLE-NuGet-Package)
  2. Download and compile the ANGLE source code from this GitHub repository

Easy-to-use Visual Studio app templates are currently available for option 2 above. See the 'Quick Start' section below for more details.

Requirements
=====

Windows 10 Development:
* [Visual Studio 2015 Community or higher](https://www.visualstudio.com/downloads/visual-studio-2015-downloads-vs.aspx)
* Windows 10 for local Windows development

Clasic Windows (Desktop) Development:
* Visual Studio 2015 Community or higher.

For Windows 8.1 or Windows Phone 8.1 development, see the [ms-win8.1](https://github.com/Microsoft/angle/tree/ms-win8.1) branch.

More Info
=====

For detailed information about ANGLE, please visit our wiki (found [here](https://github.com/Microsoft/angle/wiki)). Our wiki 
contains lots of useful information about ANGLE, including:

- Guides to help you get started with ANGLE in Windows apps
- Tips and tricks to get good performance out of ANGLE
- Sample code and documentation
- And more!

For a broad overview of ANGLE and how it works, please take a look at our [//BUILD/ 2015 presentation](http://channel9.msdn.com/Events/Build/2015/3-686).

Quick Start (compiling from source)
=====
1. Clone or download ANGLE from our GitHub repository
2. Install our easy-to-use Visual Studio templates by running install.bat in the /templates/ directory of your copy of ANGLE, or follow [these manual steps](https://github.com/Microsoft/angle/wiki/Installing-Templates).
3. Open the appropriate ANGLE Visual Studio solution for your project, and build all flavors of it
4. In Visual Studio go "File -> New -> Project", create a new ANGLE application, and hit F5 to run it!

The Windows 10 Visual Studio solution for ANGLE is located here:
* /winrt/10/src/angle.sln

The Visual Studio solution for Windows desktop applications is located here:

* /src/angle.sln

For Windows 8.1 or Windows Phone 8.1 development, see the [ms-win8.1](https://github.com/Microsoft/angle/tree/ms-win8.1) branch.

Useful Links
=====
- [Recent breaking changes](https://github.com/Microsoft/angle/wiki/breaking-changes)
- [Known issues with ANGLE](https://github.com/Microsoft/angle/wiki/known-issues)
- [Master ANGLE project info](https://code.google.com/p/angleproject/)
- [Deprecated ANGLE Windows 8.0 branch](https://github.com/Microsoft/angle-win8.0)
- [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). 
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) 
or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional 
questions or comments.

Feedback
=====
If you have feedback about this branch then we would love to hear it. Please 
create an issue on this GitHub page, or contact the Microsoft contributors directly.

Microsoft Contributors
=====
- Cooper Partin (coopp-at-microsoft-dot-com)
- Austin Kinross (aukinros-at-microsoft-dot-com)
