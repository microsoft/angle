ANGLE -- future-dev branch
=====

This is an active branch with Windows Store versions of ANGLE. It contains
full support for Windows 8.1 and Windows Phone 8.1, and other features such as
performance improvements and memory savings.

If you are developing for Windows 8.0 or Windows Phone 8.0 then please see the
version of ANGLE in the 'winrt' branch.

About ANGLE
=====
ANGLE allows Windows users to seamlessly run OpenGL ES content, by translating 
OpenGL ES API into DirectX 11 API calls.

More information about the ANGLE project is available here: 
https://code.google.com/p/angleproject/

Requirements
=====
* Visual Studio Community 2013 (Update 4), or higher/later.
* Python 2.7 installed and added to the system PATH
* Windows 8.1 for local Windows development.

Getting Started
=====
The /templates/ directory contains easy-to-use Visual Studio templates for
OpenGL ES/ANGLE. View the README in that directory for installation 
instructions.

The Windows Store solutions for ANGLE are located here:

* /winrt/8.1/windows/src/angle.sln
* /winrt/8.1/windowsphone/src/angle.sln

The ANGLE solution for Windows desktop applications is located here:

* /src/angle.sln

Features (available now)
=====
+ Full Windows Store support for both ICoreWindow and SwapChainPanel for XAML 
applications.
+ This branch uses approximately half the memory compared to older versions of
ANGLE for Windows Store.
+ GPU performance gains, particularly on mobile devices.
+ D3D Feature Level 9.3 support, for running on mobile devices.
+ Runtime shader compilation on both Windows 8.1 and Windows Phone 8.1

Work-in-progress features
=====
+ Improved D3D Feature Level 9.3 support.
+ Improved OpenGL ES 3.0 compliance.
+ Compressed texture support.
+ Many other features.

Feedback
=====
If you have feedback about this branch then we would love to hear it. Please 
log an issue on this GitHub page, or contact the Microsoft Open Tech Inc. 
contributors directly (listed below). 

Microsoft Open Tech Inc. Contributors
=====
* Cooper Partin (coopp-at-microsoft-dot-com)
* Austin Kinross (aukinros-at-microsoft-dot-com)