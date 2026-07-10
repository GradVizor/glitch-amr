# Persistent USB Port Setup: ESP32 & RPLIDAR (CP210x)

This configuration resolves the issue where the RPLIDAR and ESP32 randomly swap between `/dev/ttyUSB0` and `/dev/ttyUSB1` on reboot, by mapping each device to a stable, predictable path.

## Issue History

The original fix assumed the RPLIDAR could be pinned to a persistent symlink using a udev rule matched on its USB Vendor ID and Product ID alone (`idVendor`/`idProduct`).

This approach failed once the ESP32 was added to the same host: both the ESP32 and the RPLIDAR A2M8 use the same Silicon Labs **CP210x** USB-to-UART bridge, so both devices enumerate with the **identical `VID:PID` of `10c4:ea60`**, and additionally report the **same default factory serial number (`0001`)**. With no software-level identifier left to distinguish them, a rule matched only on VID/PID/serial binds the `/dev/rplidar` symlink to whichever device happens to enumerate first — non-deterministically assigning either device to the symlink across reboots.

**Root cause:** VID:PID-based udev matching breaks down when multiple devices on the same host share identical USB descriptors. The fix is to key off physical USB topology (which port the device is plugged into) instead, since that stays fixed across reboots even when descriptor values collide.

## Fix: Physical Port-Based Mapping

### Option 1 — Direct `/dev/serial/by-path/` Mapping (easiest)

Linux already creates persistent symlinks based on the physical USB socket a device is connected to — no udev rule required. Reference these paths directly in launch files or source code:

- **ESP32:** `/dev/serial/by-path/platform-xhci-hcd.2.auto-usb-0:1.2:1.0-port0`
- **RPLIDAR:** `/dev/serial/by-path/platform-xhci-hcd.2.auto-usb-0:1.3:1.0-port0`

*Note: each device must stay plugged into its designated physical socket for these paths to remain valid.*

### Option 2 — Friendly Symlinks via udev Rules

Use this if the application needs clean names like `/dev/esp32` and `/dev/rplidar` rather than the verbose `by-path` strings.

**1. Create/edit the rule file:**
```bash
sudo nano /etc/udev/rules.d/99-rplidar-esp32.rules
```

**2. Add port-specific rules.** The key addition versus the original rule is `KERNELS`, which pins the match to a specific physical hub port (`1-1.2` / `1-1.3` here) rather than relying on VID/PID alone:
```text
KERNEL=="ttyUSB*", KERNELS=="1-1.2", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", MODE="0666", SYMLINK+="esp32"
KERNEL=="ttyUSB*", KERNELS=="1-1.3", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", MODE="0666", SYMLINK+="rplidar"
```

**3. Reload and apply:**
```bash
sudo udevadm control --reload-rules && sudo udevadm trigger
```

**4. Verify:** unplug and reconnect both devices, then run:
```bash
ls -l /dev/esp32 /dev/rplidar
```
Each should resolve to the correct active `ttyUSB*` device, regardless of which one grabbed `ttyUSB0` vs `ttyUSB1` on this boot.

### Finding Your Own `KERNELS` Values

The `1-1.2` / `1-1.3` port paths above are specific to this machine's USB hub topology and will differ on other hardware. To find the correct value for a given device:

```bash
udevadm info -a -n /dev/ttyUSB0 | grep KERNELS
```

Use the **first** `KERNELS=="X-X.X"` entry listed (this corresponds to the immediate USB device, not an upstream root hub).
