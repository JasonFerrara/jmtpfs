# jmtpfs

jmtpfs is a FUSE and libmtp based filesystem for accessing MTP (Media Transfer
Protocol) devices. It was specifically designed for exchaning files between 
Linux (and Mac OS X) systems and newer Android devices that support MTP but not USB Mass 
Storage.

The goal is to create a well behaved filesystem, allowing tools like find and
rsync to work as expected. MTP file types are set automatically based on file
type detection using libmagic. Setting the file appears to be necessary for 
some Android apps, like  Gallery, to be able to find and use the files.
  
Since it is meant as an Android file transfer utility, and I don't have
any non-Android MTP devices to test with, playlists and other non-file
based data are not supported.

## Building and installing:

See the INSTALL file.

## Usage:

Run jmtpfs with a directory as a parameter, and it will mount to that directory
the first MTP device it finds. You can then access the files on the device as
if it were a normal disk.
```
[jason@colossus ~]$ jmtpfs ~/mtp
Device 0 (VID=04e8 and PID=6860) is a Samsung GT-P7310/P7510/N7000/I9100/Galaxy Tab 7.7/10.1/S2/Nexus/Note.
Android device detected, assigning default bug flags
[jason@colossus ~]$ ls ~/mtp
Internal Storage
[jason@colossus ~]$ ls ~/mtp/Internal\ Storage/
Android  burstlyImageCache  DCIM  Music  Notifications  Pictures  testdir
[jason@colossus ~]$ df -h ~/mtp/Internal\ Storage
Filesystem      Size  Used Avail Use% Mounted on
jmtpfs           14G  3.1G   11G  23% /home/jason/mtp
[jason@colossus ~]$ cd ~/mtp/Internal\ Storage/
[jason@colossus Internal Storage]$ ls
Android  burstlyImageCache  DCIM  Music  Notifications  Pictures  testdir
[jason@colossus Internal Storage]$ cat > test.txt
Hello Android!
[jason@colossus Internal Storage]$ ls
Android            DCIM   Notifications  testdir
burstlyImageCache  Music  Pictures       test.txt
[jason@colossus Internal Storage]$ cat test.txt 
Hello Android!
[jason@colossus Internal Storage]$ rm test.txt 
[jason@colossus Internal Storage]$ 
```
Pass the -l option will list the attached MTP devices.
```
[jason@colossus ~]$ workspace/jmtpfs/src/jmtpfs -l
Device 0 (VID=04e8 and PID=6860) is a Samsung GT-P7310/P7510/N7000/I9100/Galaxy Tab 7.7/10.1/S2/Nexus/Note.
Available devices (busLocation, devNum, productId, vendorId, product, vendor):
2, 19, 0x6860, 0x04e8, GT-P7310/P7510/N7000/I9100/Galaxy Tab 7.7/10.1/S2/Nexus/Note, Samsung
```
You can choose which device to mount with the -device option.
```
[jason@colossus ~]$ workspace/jmtpfs/src/jmtpfs -device=2,19 ~/mtp
Device 0 (VID=04e8 and PID=6860) is a Samsung GT-P7310/P7510/N7000/I9100/Galaxy Tab 7.7/10.1/S2/Nexus/Note.
Android device detected, assigning default bug flags
[jason@colossus ~]$ ls ~/mtp
Internal Storage
```
Unmount with fusermount.
```
[jason@colossus ~]$ ls ~/mtp
Internal Storage
[jason@colossus ~]$ fusermount -u ~/mtp
[jason@colossus ~]$ ls ~/mtp
[jason@colossus ~]$
```

## Performance and implementation notes:

libmtp (and I assume the MTP protocol itself) doesn't support seeking within a 
file or partial file reads or writes. You have to fetch or send the entire 
file. To simluate normal random access files, when a file is opened the entire
file contents are copied from the device to a temporary file. Reads and writes
then operate on the temporary file. When the file is closed (or if a flush or
fsync occurs) then if a write has occurred since the file was last opened the
entire contents of the temporary file are sent back to the device. This means
repeatedly opening a file, making a small change, and closing it again will
be very slow.

Renaming or moving a file is implemented by copying the file from the device, 
writing it back to the device under the new name, and then deleting the 
original file. This makes renames, especially for large files, slow. This
has special significance when using rsync to copy files to the device. Rsync
copies to a temporary file, and then when the copy is complete it renames the
temporary file to the real filename. So when rsyncing to a jmtpfs filesystem, 
for each file, the data gets copied to the device, read back, and then copied
to the device again. There is a true rename (but not move) supported by libmtp,
but this appears to confuse some Android apps, so I don't use it. Image files,
for example, will disappear from the Gallery if they're renamed.
