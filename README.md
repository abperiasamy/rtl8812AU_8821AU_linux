rtl8812AU_8821AU_linux
======================

rtl8812AU_8821AU linux kernel driver for AC1200 (801.11ac) Wireless Dual-Band USB Adapter

## Autocompiling with DKMS

```
sudo cp -R . /usr/src/rtl8812AU_8821AU_linux-1.0
sudo dkms add -m rtl8812AU_8821AU_linux -v 1.0
sudo dkms build -m rtl8812AU_8821AU_linux -v 1.0
sudo dkms install -m rtl8812AU_8821AU_linux -v 1.0
```

## To compile on the raspberry pi using raspian
#### You first need kernel headers (and the corresponding kernel binary) to build your module. 
```
sudo apt-get install linux-image-rpi-rpfv linux-headers-rpi-rpfv dkms build-essential bc
```
#### Then, tell raspbian to boot your newly installed kernel.  Append this at end of /boot/config.txt and reboot your Pi:
```
kernel=vmlinuz-3.10-3-rpi
initramfs initrd.img-3.10-3-rpi followkernel
```
#### Cd to the directory where your source is and compile
```
cd /usr/src/rtl8812AU_8821AU_linux
sudo make clean 
sudo make 
sudo make install
sudo modprobe -a 8812au
```


