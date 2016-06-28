ANGLE -- ms-holographic-experimental
=====

This experimental branch has changes that allow you to run an ANGLE app on HoloLens, or anything
that supports Windows Holographic - such as the Microsoft HoloLens Emulator.

In this branch, ANGLE has been updated to recognize a holographic space and a spatial coordinate 
system. These are passed in by the app when ANGLE is initialized; the holographic space replaces
the native CoreWindow in the surface creation properties, and the spatial coordinate system is
also provided as a surface creation property. This coordinate system will be used to create stereo
view and projection matrices that are provided to the shader pipeline by ANGLE.

Support for one holographic camera is included. The ANGLE app can draw in stereo using the 
optimized instanced rendering technique that is supported by Microsoft HoloLens. The additional 
optimization to set the render target array index without using a geometry shader is detected and
enabled when running on applicable hardware.

To see the changes you will need to make to your app, take a look at the updated app template. 
Instructions for installing it are provided later in this document.


## Remarks

This branch contains code that is a work-in-progress; it is not fully tested. You may encounter 
issues while using it.

**Note** This branch requires Visual Studio 2015 Update 2 to build, and a Windows Holographic 
device to execute. Windows Holographic devices include the Microsoft HoloLens and the Microsoft 
HoloLens Emulator.

To obtain information about Windows 10 development, go to the [Windows Dev Center]
(http://go.microsoft.com/fwlink/?LinkID=532421).

To obtain information about the tools used for Windows Holographic development, including
Microsoft Visual Studio 2015 Update 2 and the Microsoft HoloLens Emulator, go to
[Install the tools](https://developer.microsoft.com/windows/holographic/install_the_tools).

## System requirements

**For holographic apps:** Windows 10 Holographic

## Build ANGLE with Windows Holographic experimental support included

1. Clone the experimental branch: [https://github.com/Microsoft/angle/tree/ms-holographic-experimental]
   (https://github.com/Microsoft/angle/tree/ms-holographic-experimental).
2. Start Microsoft Visual Studio 2015 Update 2 and select **File** \> **Open** \> **Project/Solution**.
3. Starting in the folder where you cloned the branch, go to winrt\10\src\ and open angle.sln.
4. Set your target platform to Win32, and set Debug or Release as desired.
5. Right-click on the libAngle project and select **Build**.

## Build the ANGLE UWP app template with Windows Holographic experimental support included

1. Open an Explorer window and go to the folder where you cloned the experimental branch.
2. Navigate to templates\ and run install.bat.
3. Start Microsoft Visual Studio 2015 Update 2 and select **File** \> **New ** \> **Project**.
4. Under **Templates** \> **Visual C\+\+** \> **Windows** \> **Universal**, select 
   **App for OpenGL ES (Windows Universal)**.
5. Name your app, select a folder, and click **OK**.
6. Set your target platform to x86, and set Debug or Release as desired.
7. Right-click on your app project and select **Build**.

**Note:** You may now begin development, or proceed to build and run the sample content.

## Run the sample

The next steps depend on whether you just want to deploy the app, or you want to both deploy and
run it.

### Deploying to the Microsoft HoloLens emulator

- Click the debug target drop-down, and select **Microsoft HoloLens Emulator**.
- Select **Build** \> **Deploy** Solution.

### Deploying to a Microsoft HoloLens

- Developer unlock your Microsoft HoloLens. For instructions, go to [Enable your device for development]
  (https://msdn.microsoft.com/windows/uwp/get-started/enable-your-device-for-development#enable-your-windows-10-devices).
- Find the IP address of your Microsoft HoloLens. The IP address can be found in **Settings**
  \> **Network & Internet** \> **Wi-Fi** \> **Advanced options**. Or, you can ask Cortana for this
  information by saying: "Hey Cortana, what's my IP address?"
- Right-click on your project in Visual Studio, and then select **Properties**.
- In the Debugging pane, click the drop-down and select **Remote Machine**.
- Enter the IP address of your Microsoft HoloLens into the field labelled **Machine Name**.
- Click **OK**.
- Select **Build** \> **Deploy** Solution.

### Pairing your developer-unlocked Microsoft HoloLens with Visual Studio

The first time you deploy from your development PC to your developer-unlocked Microsoft HoloLens,
you will need to use a PIN to pair your PC with the Microsoft HoloLens.
- When you select **Build** \> **Deploy Solution**, a dialog box will appear for Visual Studio to
  accept the PIN.
- On your Microsoft HoloLens, go to **Settings** \> **Update** \> **For developers**, and click on
  **Pair**.
- Type the PIN displayed by your Microsoft HoloLens into the Visual Studio dialog box and click
  **OK**.
- On your Microsoft HoloLens, select **Done** to accept the pairing.
- The solution will then start to deploy.

### Deploying and running the sample

- To debug the sample and then run it, follow the steps listed above to connect your
  developer-unlocked Microsoft HoloLens, then press F5 or select **Debug** \> **Start Debugging**.
  To run the sample without debugging, press Ctrl+F5 or select **Debug** \> **Start Without Debugging**.
