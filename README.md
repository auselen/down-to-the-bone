#BeagleBone Black playground

## Document links
[BeagleBone Black System Reference Manual](https://github.com/CircuitCo/BeagleBone-Black/blob/master/BBB_SRM.pdf?raw=true)
[AM335x Technical Reference Manual](http://www.ti.com/product/am3359)

## Loading secondary program via usb 
Connect the micro usb cable to your BBB while S2 pressed down. Now ROM code on AM335x will try to load next stage from serial or over usb/rndis/bootp+tftp combination until it succeeds in one. After connecting usb with S2 pressed down, consecutive resets via reset button (S1) will trigger same way of booting for ROM code.

Text below tries to descibe how to work with usb/bootp+ftfp way of booting.

Easiest is to use a Linux computer with [Dnsmasq](http://www.thekelleys.org.uk/dnsmasq/doc.html). If you have an `apt` powered system you may find working with instructions below easier.

### Setting up Dnsmasq
Install dnsmasq. e.g.
```sh
sudo apt-get install dnsmasq
``` 
Append the configuration below
```
#disable DNS
port=0
interface=usb0
dhcp-range=192.168.0.50,192.168.0.150,12h
bootp-dynamic
enable-tftp
tftp-root=/tmp/tftp
dhcp-boot=beagle.img
```

Restart dnsmasq
``` sh
sudo /etc/init.d/dnsmasq restart
```

Resetting/plugging with S2 down, BBB should produce some logs by dnsmasq under syslog.
``` sh
$ sudo cat /var/log/syslog|grep dnsmasq
dnsmasq-dhcp[1620]: DHCP packet received on usb0 which has no address
dnsmasq-dhcp[1620]: BOOTP(usb0) 192.168.0.115 c8:a0:30:ae:2a:0a 
dnsmasq-tftp[1620]: sent /tmp/tftp/beagle.img to 192.168.0.115
```

On serial you may see `C` characters (xmodem).

##Examples
####bbb_baremetal_led
Simplest code to turn on led USR0. Suitable for peripheral booting - it doesn't use u-boot.
