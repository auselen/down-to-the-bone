#BeagleBone Black bare metal example 01
## Building
Use `CROSS_COMPILE` to specify toolchain. If not specified it should try to use `arm-linux-gnueabihf-`.

For example:

```shell
$ CROSS_COMPILE=~/bin/gcc-linaro-arm-linux-gnueabihf-4.8-2013.09_linux/bin/arm-linux-gnueabihf- make
```

Load generated startup.o via your prefered method. See parent directory for USB booting.
