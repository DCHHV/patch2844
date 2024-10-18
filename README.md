LP-2844 Offline Firmware Update Tool
===================================

**NOTE!** This has only been tested in one specific instance and really only exists as a proof of concept! You have been warned.


A simple tool to update UPS 2844 thermal printers to stock firmware. It could also be potentially used to recover a botched firmware update or fix other scenarios where the printer is nonfunctional/nonresponsive.

The specific situation this was created for was to update from firmware UPS V4.51 to stock V4.70.1A

This repo includes original dumped firmware blobs (which have the serial number set to all 0s), as well as the unmodified firmware update file from Zebra directly.

Building this project results in a single binary with a specific purpose. It takes in two binary blobs, one from each of the two flash ICs on the LP-2844 mainboard, and outputs a combined binary blob of both the two flash ICs, a copy of that blob with the firmware update applied, and a second set of binary blobs meant to be written back to each flash IC:

```
./patchzebra even_byte.bin odd_byte.bin update-file.prg output
```

For more information on this project, see [https://dchhv.org/project/2019/01/27/ups2844convert.html](https://dchhv.org/project/2019/01/27/ups2844convert.html)
