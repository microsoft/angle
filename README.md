# ANGLE -- Microsoft repository

### In short
Going forward, please use Google's master ANGLE repository. https://opensource.google.com/projects/angle.

Please follow the [Dev Setup instructions](https://chromium.googlesource.com/angle/angle/+/master/doc/DevSetup.md) there to build ANGLE. Note that you'll want to set `target_os = "winuwp"` in your GN args if you want to build for UWP.

### What is ANGLE?
ANGLE allows Windows users to seamlessly run OpenGL ES content by efficiently translating OpenGL ES API into DirectX 11 API calls.

### What is this repository?
This repository was maintained by Microsoft while we made changes to ANGLE. All of our changes, including Windows Store support and performance improvements for all D3D11 platforms, have now been submitted upstream to the master ANGLE repository, so this repository has mostly been deprecated.

### When should I use this repository?
We strongly recommend that you use the master ANGLE repository going forward. There are a couple of exceptions though:

1) [This branch](https://github.com/microsoft/angle/tree/2017-lkg) includes support for Windows 8.1 or Windows Phone 8.1 support, which is not available in Google's repository.

2) [This branch](https://github.com/Microsoft/angle/tree/ms-holographic-experimental) includes an experimental version of ANGLE with HoloLens support, created by the HoloLens team.

### Where can I get help?
In general we recommend asking your questions on the master ANGLE mailing list. The mailing list includes regular ANGLE contributors and several Microsoft employees.

Otherwise, there are still some resources available in this repository:

1) In the past Microsoft maintained UWP Visual Studio templates for ANGLE. These templates are deprecated, and they don't work with Visual Studio 2017 or later. However, they may be useful if you are trying to understand how to use ANGLE in a UWP, so they are still available [here](https://github.com/microsoft/angle/tree/2017-lkg/templates).

2) In the past Microsoft wrote detailed Wiki pages on a variety of topics relating to ANGLE. These pages may be outdated now, but they are still available [here](https://github.com/microsoft/angle/wiki) in case they are useful to you.

### What is the NuGet package?
In short: we recommend building ANGLE from the master Google repository instead of using the ANGLE NuGet package.

The ANGLE NuGet package is an easy way to use a prebuilt version of ANGLE in your UWP apps. It is available [here](https://www.nuget.org/packages/ANGLE.WindowsStore/). The package hasn't been updated since late 2016 though, so it is missing a lot of features compared to recent builds from Google's master repository.

### Useful Links
- [Deprecated ANGLE Windows 8.0 branch](https://github.com/Microsoft/angle-win8.0)
- [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). 
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) 
or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional 
questions or comments.
