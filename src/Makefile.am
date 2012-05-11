bin_PROGRAMS=jmtpfs
jmtpfs_SOURCES=jmtpfs.cpp MtpDevice.cpp ConnectedMtpDevices.cpp Mutex.cpp MtpFilesystemPath.cpp \
	MtpMetadataCache.cpp MtpNode.cpp MtpRoot.cpp MtpLibLock.cpp MtpStorage.cpp \
	MtpFolder.cpp MtpFile.cpp TemporaryFile.cpp MtpLocalFileCopy.cpp \
	MtpFuseContext.cpp
jmtpfs_CPPFLAGS = $(MTP_CFLAGS) $(FUSE_CFLAGS)
jmtpfs_LDADD = $(MTP_LIBS) $(FUSE_LIBS)
