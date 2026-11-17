# SciVis - Visual Studio 2026 on Windows

## Building

Within the folder "build" is a VS solution called "CS453_Project2.sln", which you should be able to open in Visual Studio. After doing so, the first task is to ensure all of the include paths are set by going to **Project** > **Properties** > **Configuration Properties** > **VC++ Directories**, and then including the paths to `freeglut-3.0.0/include` and `freeglut-3.0.0/lib` in the row that says **Include Directories**.

Within this same window, go back to the left-hand column to **Linker** > **Input**. In **Additional Dependencies**, insert the path to `freeglut-3.0.0/lib/freeglut.lib`. This file is needed for building, and the linker will say it can't find it if you don't explicitly include it like this.

Now go from **Input** to **System**. Change **SubSystem** to `Console (/SUBSYSTEM:CONSOLE)`. Close the Properties window by clicking **OK**.

Build by right-clicking the `CS453_Project2` project and clicking **Build** or by going to the top of the screen where it says **Build**, then clicking **Build Solution**.

## Running

You must first set the learnply project to be run. Do this by right-clicking the `CS453_Project2` project and clicking **Set as Startup Project**.

You must specify the path to the ply file in your Project Settings. Do this by right-clicking the `CS453_Project2` project, navigating to the **Project** > **Properties** > **Debug** tab, and in the **Command line arguments** field, enter the path to the ply file you want to render.

(Note: I had to copy the files `freeglut.dll` and `glew.dll` into the debug folder in order to run the project.)

You may now run the project by pressing **F5** or clicking the **Start** button.

**Note:** that if your file path has spaces in it, it must be surrounded by quotes.

## Keyboard

1: solid rendering
2: wireframe rendering
3: checkerboard rendering
4: vector field flow visualization
5: grayscale rendering
6: multicolor rendering
9: heatmap rendering
h: apply height to scalar field
r: resets camera viewpoint
c: prompts for a scalar value to draw a contour
n: propmts for a number of evenly spaced contours
p: toggles between single and bicolor contours
