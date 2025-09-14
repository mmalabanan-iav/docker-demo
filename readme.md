# Docker Demo application Example

Docker Demo application Example


## Prerequesites

| ** Tools ** | ** Description ** |
| --- | --- |
| **[USB/IP](https://github.com/dorssel/usbipd-win/releases/latest)** (usbipd) | This connects USB devices into WSL (Win11). Latest version is good to use. |
| **usbutils** | provides commands such as `lsusb` (WSL). |
| **openocd** | debugging client |
| **gdb-multiarch** | debugging server |
| ** docker ** | Install docker desktop |


## Guides

### Getting the microcontroller to connect into WSL

1) Install USB/IP (usbipd), may require admins rights.
2) Once installed, run `usbipd list` in PowerShell.
![usbipd list](docs\img\usbipd-list.png)
3) Run `usbipd bind --busid <BUS ID>` to allow the device to be shared with WSL, may require admin rights.

    For example, using the ID in the previous image:
    > usbipd bind --busid 1-3

    Then executing `usbipd list`,
    ![usbipd list](docs\img\usbipd-bind.png)

4) Run `usbipd attach --wsl --busid <BUS ID>` to attach the USB device to WSL.

    ![usbipd list](docs\img\usbipd-attach.png)

    **NOTE**: Sometimes the binding fails when attaching the USB device, if this happends, `usbipd bind --force --busid <BUS ID>` must be used.

5) In WSL, run `lsusb` to find the USB device.


    ![usbipd list](docs\img\lsusb.png)