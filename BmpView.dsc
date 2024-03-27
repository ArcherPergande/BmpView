[Defines]
  PLATFORM_NAME                  = BmpView
  PLATFORM_GUID                  = 8f6f7a56-5e51-4a07-a4a4-44a6c1c8f814
  PLATFORM_VERSION               = 1.0
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build
  SUPPORTED_ARCHITECTURES        = X64
  BUILD_TARGETS                  = DEBUG | RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]

    #
    # Entry point
    #
    UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf

    #
    # UefiLib
    #
    PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
    MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
    DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
    BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
    BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
    UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
    DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
    UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
    PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
    RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
    UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
    
    #
    # ShellLib
    #
    FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
    HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
    SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
    UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
    ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf


[Components]
    BmpView/BmpView.inf