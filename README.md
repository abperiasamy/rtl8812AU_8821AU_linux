rtl8812AU_8821AU_linux
======================

rtl8812AU_8821AU linux kernel driver for AC1200 (801.11ac) Wireless Dual-Band USB Adapter

##Autocompiling with DKMS

```
sudo cp -R . /usr/src/rtl8812AU_8821AU_linux-1.0
sudo dkms add -m rtl8812AU_8821AU_linux -v 1.0
sudo dkms build -m rtl8812AU_8821AU_linux -v 1.0
sudo dkms install -m rtl8812AU_8821AU_linux -v 1.0
```

