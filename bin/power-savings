#!/bin/bash -ve

# For additional power savings, you can reduce the number of interrupts (with
# slower network speeds) by adding dwc_otg.speed=1 to your /boot/config.txt
# file. You can also add dwc_otg.fiq_fix_enable=1 to reduce the number of
# interrupts.

# Disable display
tvservice -o

# Turn off LEDs on Pi zero. From
# http://www.jeffgeerling.com/blogs/jeff-geerling/controlling-pwr-act-leds-raspberry-pi
echo none | sudo tee /sys/class/leds/led0/trigger
echo 1 | sudo tee /sys/class/leds/led0/brightness

# Remove some unneeded kernel modules
rmmod w1_gpio || true
rmmod wire || true
rmmod snd_bcm2835 snd_pcm snd_timer snd || true
rmmod bluetooth || true

# ... and services
systemctl stop avahi-daemon.service
systemctl disable avahi-daemon.service

systemctl stop triggerhappy.service
systemctl disable triggerhappy.service

# Enable powertop auto tuner
/usr/sbin/powertop --auto-tune

# Ensure system time is current
/usr/sbin/ntpdate-debian
