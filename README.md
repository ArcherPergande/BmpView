
# BmpView
A simple UEFI image viewer for the bmp file format. Supports uncompressed 1, 4, 8, 16, and 24 bit depths.

![Hello world](https://github.com/ArcherPergande/BmpView/blob/main/Images/HelloWorld.bmp?raw=true)

Installation (SUPPORTS X64 ONLY)
-----------------------------------------------------
A. Download the pre-built binary from a release.

B. Copy source to edk2 root and execute “build -p BmpView/BmpView.dsc”

Usage
-----------------------------------------------------
In the UEFI shell run “BmpView FILE_NAME.bmp” to open an image. Once an image is open ESC terminates the program while the left, right, up, and down arrow keys are used for navigation.

https://drive.google.com/file/d/1ywbdd5GmYZIbKE3x6x_QpI2ebfBSbYoU/view?usp=sharing

Dependencies
-----------------------------------------------------
1. EDK2 (Building)
2. UEFI Shell Environment (Runtime)
