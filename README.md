# Stereo Demonstrator

The Stereo Demonstrator provides demo applications to experiment with
stereo rendering on stereo capable displays from [Schneider Digital](https://www.schneider-digital.com/en/)

![alt text](https://github.com/KDABLabs/stereo3ddemo/blob/master/screenshot.png)

This project builds a Qt3D based application and (optionally) a Vulkan based application.

Both applications allow loading 3D files *(.obj, .fbx, .gltf ...)* to view them in stereo.
A 3D cursor is drawn at the intersection between the mouse and the 3D model (fallsback to being place on focus plane if no intersection)

***Note:** that the Vulkan based application relies on KDAB's Vulkan engine which is not open sourced at this time.*

## Features

### Stereo Camera Controls:
- Projection Type (Asymmetric Frustums, Toe In)
- Focus Distance
- Eye Separation
- Flip Eyes

### Focus Controls
- AutoFocus + Focus Area
- Manual Focus Distance

### Field of View Controls:
- Manually Specified
- Deduced from Physical Dimensions

### Cursor Controls:
- Appearance
- Color
- Size

### Misc Controls:
- Wireframe mode
- Frustum Viewer

## Requirements
- OpenGL support (compatible GPU and drivers installed)
- GPU that supports quad buffers / multilayered swapchains (Nvidia Quadro & RTX, AMD Pro & FirePro)
  - **Note:** content will be displayed side by side on GPUs that do not support that feature
- Qt 6.8 with the Qt3D optional module

## Building

### Using VSCode (recommended)
- Copy CMakePresets.json.example to CMakePresets.json
- Edit CMAKE_PREFIX_PATH variable in CMakePresets.json to point to the Qt install path
- Build with VSCode
- Launch KDAB_Qt_Qt3D_OpenGL

### Using Command Line
- mkdir build; cd build
- cmake -DCMAKE_PREFIX_PATH=/path/to/Qt -DCMAKE_BUILD_TYPE=Release ..
- cmake --build .
- cmake --build . -t package
- ./KDAB_Qt_Qt3D_OpenGL


## Contact

- Visit us on GitHub: <https://github.com/KDAB/KDGpu>
- Email info@kdab.com for questions about copyright, licensing or commercial support.

Stay up-to-date with KDAB product announcements:

- [KDAB Newsletter](https://news.kdab.com)
- [KDAB Blogs](https://www.kdab.com/category/blogs)
- [KDAB on Twitter](https://twitter.com/KDABQt)

## Licensing

The Stereo Demonstrator is available under the terms of
the [GPL](https://github.com/KDABLabs/stereo3ddemo/blob/master/LICENSES/gpl-3.0.txt) license.

Contact KDAB at <info@kdab.com> if you need different licensing options.

## Get Involved

Please submit your contributions or issue reports from our GitHub space at <https://github.com/KDAB/KDGpu>.

Contact <info@kdab.com> for more information.

## About KDAB

The Stereo Demonstrator is supported and maintained by Klarälvdalens Datakonsult AB (KDAB).

The KDAB Group is the global No.1 software consultancy for Qt, C++ and
OpenGL applications across desktop, embedded and mobile platforms.

The KDAB Group provides consulting and mentoring for developing Qt applications
from scratch and in porting from all popular and legacy frameworks to Qt.
We continue to help develop parts of Qt and are one of the major contributors
to the Qt Project. We can give advanced or standard trainings anywhere
around the globe on Qt as well as C++, OpenGL, 3D and more.

Please visit <https://www.kdab.com> to meet the people who write code like this.
