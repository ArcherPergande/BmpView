
# BmpView
A simple UEFI image viewer for the bmp file format.

Installation (SUPPORTS X64 ONLY)
-----------------------------------------------------
A. Download the pre-built binary from a release.

B. Copy source to edk2 root and execute “build -p BmpView/BmpView.dsc”

Usage
-----------------------------------------------------
In the UEFI shell run “BmpView FILE_NAME.bmp” to open an image. Once an image is open ESC terminates the program while the left, right, up, and down arrow keys are used for navigation.

Dependencies
-----------------------------------------------------
1. EDK2 (Building)
2. UEFI Shell Environment (Runtime)
