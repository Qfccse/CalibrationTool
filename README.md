# CalibrationTool

qt creator为5.14.2版本，安装教程https://blog.csdn.net/u014779536/article/details/109160027

opencv为4.5.1版本

# readme

 In this folder, you will find the project files, PowerPoint presentation, and demo video for the Camera Calibration Tool final project. 

## Code

The code files are divided into two parts. The first part is the code for the win10 platform, available in `CalibrationTool.rar`. The second part is the code for the Linux platform, available in `CalibrationTool_linux.rar`. The source code is the same for both, but the win10 platform is compiled using vs2019 (including `.sln `files), while the Linux platform requires QtCreator for running and thus needs a .pro file. Before running, make sure to install `Qt 5.14.2` and `OpenCV 4.5.1`. For the win10 code, you also need to install the following Qt dependencies.

```
3D
3D Quick
Charts
Core
Gui
Widgets
```



## demo

 In the `demo_linux.mp4 `, we mainly demonstrate the calibration process of uploading images and calibrating a standard camera. 

In the `demo_win.mp4`, we mainly demonstrate the calibration process of taking photos and calibrating a fisheye camera.



