
# BmpView
A simple UEFI image viewer for the bmp file format.

![Hello world](https://github.com/ArcherPergande/BmpView/blob/main/Images/HelloWorld.bmp?raw=true)

Installation (SUPPORTS X64 ONLY)
-----------------------------------------------------
A. Download the pre-built binary from a release.

B. Copy source to edk2 root and execute “build -p BmpView/BmpView.dsc”

Usage
-----------------------------------------------------
In the UEFI shell run “BmpView FILE_NAME.bmp” to open an image. Once an image is open ESC terminates the program while the left, right, up, and down arrow keys are used for navigation.

[![UEFI shell example](https://img.youtube.com/vi/txKM7doz0LU/0.jpg)](https://www.youtube.com/watch?v=txKM7doz0LU)

Dependencies
-----------------------------------------------------
1. EDK2 (Building)
2. UEFI Shell Environment (Runtime)
