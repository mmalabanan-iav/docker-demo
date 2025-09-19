# Docker Demo Application Example

## Prerequesites

| Tools | Description | Environment |
| --- | --- | --- |
| **[USB/IP](https://github.com/dorssel/usbipd-win/releases/latest)** (usbipd) | This connects USB devices into WSL (Win11). Latest version is good to use. | host |
| **usbutils** | provides commands such as `lsusb` (WSL). | host (WSL) |
| **openocd** | debugging client | host (WSL) |
| **gdb-multiarch** | debugging server | host (WSL) |
| **docker** | Install docker desktop | host |
| **minicom** | serial communication via UART | host (WSL) |

## Guides

### Building Docker image

At the root of the project folder:

```bash
docker build -t docker-demo .
```

### Running Docker image

Again, at the root of the project folder:

```bash
docker run -it --rm --privileged -v "$PWD":/workspace docker-demo
```

> **NOTE**: You may add `--hostname <name you want>` to distinguish container vs. host.

### Getting out of the docker container

To exit out of the container, `Ctrl+D`.

### Building the application

1) Spin up the docker-demo container.
2) To have CMake generate the build files, use the following command:

    ```bash
    cmake -B build -G Ninja
    ```

    > **NOTE**: If the build directory exists prior, delete the whole folder.

3) To build the application, use the following command:

    ```bash
    cmake --build build
    ```

4) Now that the application has been built, you may exit out of the container by pressing `Ctrl + D`.
5) To flash the application, use the following command:

    ```bash
    make flash
    ```

### Controlling the LED

1) Open minicom via terminal

    ```bash
    minicom -b 115200 -D /dev/<device ID>
    ```

2) Select from the following:

| Command | Description |
| --- | --- |
| on | Turns on LED |
| off | Turns off LED |
| blink | Blinks LED at 200ms rate |
| stop | Stops blinking or turns off LED |

### Connecting microcontroller into WSL

> The following is required when host is Windows 11.

1) Install USB/IP (`usbipd`), may require admins rights.
2) Once installed, run `usbipd list` in PowerShell.

    ![usbipd list](docs/img/usbipd-list.png)

3) Run `usbipd bind --busid <BUS ID>` to allow the device to be shared with WSL, may require admin rights.

    For example, using the ID in the previous image:

    ```powershell
    usbipd bind --busid 1-3
    ```

    Then executing `usbipd list`,

    ![usbipd bind](docs/img/usbipd-bind.png)

4) Run `usbipd attach --wsl --busid <BUS ID>` to attach the USB device to WSL.

    ![usbipd attach](docs/img/usbipd-attach.png)

    **NOTE**: Sometimes the binding fails when attaching the USB device, if this happends, `usbipd bind --force --busid <BUS ID>` must be used.

5) In WSL, run `lsusb` to find the USB device.

    ![lsusb](docs/img/lsusb.png)

### Using OpenOCD without `sudo`

In WSL or in Linux environment, sometimes `sudo` is required to execute openocd since it will be dealing with USB.

Execute `./scripts/setup-stlink.sh`, so using openOCD with the USB will not require `sudo`.
