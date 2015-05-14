ANGLE -- ms-master branch
=====
ANGLE allows Windows users to seamlessly run OpenGL ES content by efficiently translating 
OpenGL ES API into DirectX 11 API calls. This repository contains a version of ANGLE focused 
on helping you build great Windows apps.

ANGLE fully supports these C++ app types:
- Universal Windows apps (Windows 10)
- Windows 8.1 and Windows Phone 8.1 apps
- Windows desktop applications

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
10_0<br>
</td>
<td>Modern Desktop PCs<br>Surface Pros</td>
<td>OpenGL ES 2.0<br> Parts of OpenGL ES 3.0</td>
</tr>
<tr>
<td>
9_3
</td>
<td>Windows Phones</td>
<td>OpenGL ES 2.0 (except <a href=https://github.com/MSOpenTech/angle/wiki/Known-Issues>minor corner cases</a>)</td>
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

More Info
=====

For detailed information about ANGLE, please visit our wiki (found [here](https://github.com/MSOpenTech/angle/wiki)). Our wiki 
contains lots of useful information about ANGLE, including:

- Guides to help you get started with ANGLE in Windows apps
- Tips and tricks to get good performance out of ANGLE
- Sample code and documentation
- And more!

For a broad overview of ANGLE and how it works, please take a look at [//BUILD/ 2015 presentation](http://channel9.msdn.com/Events/Build/2015/3-686).

Requirements
=====

Windows 10 Development:
* [Visual Studio 2015 Community RC or higher](https://www.visualstudio.com/downloads/visual-studio-2015-downloads-vs.aspx)
* Windows 10 for local Windows development

Windows 8.1 and/or Windows Phone 8.1 Development:
* Visual Studio Community 2013 (Update 4), or higher/later.
* Windows 8.1 for local Windows development.

Quick Start
=====
1. Clone or download ANGLE from our GitHub repository
2. Install our easy-to-use Visual Studio templates by running install.bat in the /templates/ directory of your copy of ANGLE, or follow [these manual steps](https://github.com/MSOpenTech/angle/wiki/Installing-Templates).
3. Open the appropriate ANGLE Visual Studio solution for your project, and build all flavors of it
4. In Visual Studio go "File -> New -> Project", create a new ANGLE application, and hit F5 to run it!

The Windows 10 Visual Studio solution for ANGLE is located here:
* /winrt/10/windows/src/angle.sln

The Windows 8.1 Visual Studio solutions for ANGLE are located here:

* /winrt/8.1/windows/src/angle.sln
* /winrt/8.1/windowsphone/src/angle.sln

The Visual Studio solution for Windows desktop applications is located here:

* /src/angle.sln

Useful Links
=====
- [Recent breaking changes](https://github.com/MSOpenTech/angle/wiki/breaking-changes)
- [Known issues with ANGLE](https://github.com/MSOpenTech/angle/wiki/known-issues)
- [Master ANGLE project info](https://code.google.com/p/angleproject/)
- [Deprecated ANGLE Windows 8.0 branch](https://github.com/MSOpenTech/angle-win8.0)

Feedback
=====
If you have feedback about this branch then we would love to hear it. Please 
create an issue on this GitHub page, or contact the Microsoft contributors directly.

Microsoft Contributors
=====
- Cooper Partin (coopp-at-microsoft-dot-com)
- Austin Kinross (aukinros-at-microsoft-dot-com)
