/*
 * jmtpfs.cpp
 *
 *      Author: Jason Ferrara
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02111-1301, USA.
 * licensing@fsf.org
 */
#define FUSE_USE_VERSION 26
#include "ConnectedMtpDevices.h"
#include "mtpFilesystemErrors.h"
#include "Mutex.h"

#include <MtpRoot.h>
#include <iostream>
#include <fuse.h>
#include <fuse_opt.h>
#include <cstddef>
#include <errno.h>
#include <sstream>
#include <iomanip>
#include <assert.h>

#define JMTPFS_VERSION "0.1"

using namespace std;

/*
 * A global lock for filesystem operations. I really didn't want to do this, but trying to keep everything
 * in sync when different threads accessed the same nodes in the filesystem was getting way to complicated.
 * So for now just to get something working I've pulled out all the lower level synchronization code and
 * we'll just use the global lock here.
 */
RecursiveMutex	globalLock;

#define FUSE_ERROR_BLOCK_START \
	try \
	{ \
	LockMutex lock(globalLock);

#define FUSE_ERROR_BLOCK_END \
	} \
	catch(FileNotFound&) \
	{ \
		return -ENOENT; \
	} \
	catch(MtpDeviceDisconnected&) \
	{ \
		exit(-1); \
	} \
	catch(MtpFilesystemErrorWithErrorCode& e) \
	{ \
		return -(e.ErrorCode()); \
	} \
	catch(std::exception&) \
	{ \
		return -EIO; \
	}


MtpDevice* currentDevice;
MtpMetadataCache* metadataCache;

std::unique_ptr<MtpNode> getNode(const FilesystemPath& path)
{
	if (path.Head() != "/")
		throw FileNotFound(path.str());
	std::unique_ptr<MtpNode> root(new MtpRoot(*currentDevice, *metadataCache));
	FilesystemPath childPath = path.Body();
	if (childPath.Empty())
		return root;
	else
		return root->getNode(childPath);
}


extern "C" int jmtpfs_getattr(const char* pathStr, struct stat* info)
{
	FUSE_ERROR_BLOCK_START

		FilesystemPath path(pathStr);
		getNode(path)->getattr(*info);
		return 0;

	FUSE_ERROR_BLOCK_END

}

extern "C" int jmtpfs_readdir(const char* pathStr, void* buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi)
{
	FUSE_ERROR_BLOCK_START

		FilesystemPath path(pathStr);
		std::unique_ptr<MtpNode> n = getNode(path);
		std::vector<std::string> contents = n->readdir();
		for(std::vector<std::string>::iterator i = contents.begin(); i != contents.end(); i++)
		{
			if (filler(buf,i->c_str(),0, 0))
				return 0;
		}
		return 0;

	FUSE_ERROR_BLOCK_END
}


extern "C" int jmtpfs_open(const char *pathStr, struct fuse_file_info *)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	getNode(path)->Open();
	return 0;

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_release(const char *pathStr, struct fuse_file_info *)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	getNode(path)->Close();
	return 0;

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_read(const char *pathStr, char *buf, size_t  size, off_t offset, struct fuse_file_info *)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	return getNode(path)->Read(buf,size,offset);

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_mkdir(const char* pathStr, mode_t mode)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	getNode(path.AllButTail())->mkdir(path.Tail());
	return 0;

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_rmdir(const char* pathStr)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	getNode(path)->Remove();
	return 0;

	FUSE_ERROR_BLOCK_END
}


extern "C" int jmtpfs_create(const char* pathStr, mode_t mode, struct fuse_file_info *fileInfo)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	std::unique_ptr<MtpNode> n = getNode(path.AllButTail());
	n->CreateFile(path.Tail());
	n = getNode(path);
	n->Open();
	return 0;

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_write(const char *pathStr, const char *data, size_t size, off_t offset, struct fuse_file_info *)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	return getNode(path)->Write(data, size, offset);

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_truncate(const char *pathStr, off_t length)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	getNode(path)->Truncate(length);
	return 0;

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_unlink(const char *pathStr)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	getNode(path)->Remove();
	return 0;

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_flush(const char *pathStr, struct fuse_file_info *)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	getNode(path)->Close();
	return 0;

	FUSE_ERROR_BLOCK_END
}


extern "C" int jmtpfs_rename(const char *pathStr, const char *newPathStr)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	std::unique_ptr<MtpNode> n = getNode(path);
	FilesystemPath newPath(newPathStr);
	std::unique_ptr<MtpNode> newParent = getNode(newPath.AllButTail());
	n->Rename(*newParent, newPath.Tail());

	return 0;

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_statfs(const char *pathStr, struct statvfs *stat)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	std::unique_ptr<MtpNode> n = getNode(path);
	MtpStorageInfo storageInfo = n->GetStorageInfo();

	if (storageInfo.maxCapacity > 0)
	{
		stat->f_bsize = 512;  // We have to pick some block size, so why not 512?
		stat->f_blocks = storageInfo.maxCapacity / stat->f_bsize;
		stat->f_bfree = storageInfo.freeSpaceInBytes / stat->f_bsize;
		stat->f_bavail = stat->f_bfree;
		stat->f_namemax = 233;
	}
	else
		stat->f_flag = ST_RDONLY;

	return 0;

	FUSE_ERROR_BLOCK_END
}


struct jmtpfs_options
{
	jmtpfs_options() : listDevices(0), displayHelp(0),
			showVersion(0), device(0) {}

	int	listDevices;
	int displayHelp;
	int showVersion;
	char* device;
};

static struct fuse_opt jmtpfs_opts[] = {
		{"-l", offsetof(struct jmtpfs_options, listDevices), 1},
		{"-h", offsetof(struct jmtpfs_options, displayHelp), 1},
		{"-device=%s", offsetof(struct jmtpfs_options, device),0},
		{"-V", offsetof(struct jmtpfs_options, showVersion),1},
		{"--version", offsetof(struct jmtpfs_options, showVersion),1},
		FUSE_OPT_END
};


static struct fuse_operations jmtpfs_oper = {
		0,
};

int main(int argc, char *argv[])
{
	std::unique_ptr<MtpDevice> device;
	std::unique_ptr<MtpMetadataCache> cache;

	LIBMTP_Init();
	jmtpfs_oper.getattr = jmtpfs_getattr;
	jmtpfs_oper.readdir = jmtpfs_readdir;
	jmtpfs_oper.open = jmtpfs_open;
	jmtpfs_oper.release = jmtpfs_release;
	jmtpfs_oper.read = jmtpfs_read;
	jmtpfs_oper.mkdir = jmtpfs_mkdir;
	jmtpfs_oper.rmdir = jmtpfs_rmdir;
	jmtpfs_oper.create = jmtpfs_create;
	jmtpfs_oper.write = jmtpfs_write;
	jmtpfs_oper.truncate = jmtpfs_truncate;
	jmtpfs_oper.unlink = jmtpfs_unlink;
	jmtpfs_oper.flush = jmtpfs_flush;
	jmtpfs_oper.rename = jmtpfs_rename;
	jmtpfs_oper.statfs = jmtpfs_statfs;

	jmtpfs_options options;

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	if (fuse_opt_parse(&args, &options, jmtpfs_opts,0)==-1)
	{
		std::cerr << "Error parsing arguments" << std::endl;
		return -1;
	}

	if (options.listDevices)
	{
		ConnectedMtpDevices devices;
		std::vector<std::string> devListing;
		for(int i=0; i < devices.NumDevices(); i++)
		{
			ConnectedDeviceInfo devInfo = devices.GetDeviceInfo(i);
			std::ostringstream devText;
			devText << devInfo.bus_location << ", " << (uint) devInfo.devnum << ", ";
			devText << "0x" <<  hex << setfill('0') << setw(4) << devInfo.product_id << ", ";
			devText << "0x" << hex << setfill('0') << setw(4)<< devInfo.vendor_id << ", ";
			devText << devInfo.product << ", " << devInfo.vendor;
			devListing.push_back(devText.str());
		}
		std::cout << "Available devices (busLocation, devNum, productId, vendorId, product, vendor):" << std::endl;
		for(std::vector<std::string>::iterator i = devListing.begin(); i != devListing.end(); i++)
			std::cout << *i << std::endl;
		return 0;
	}

	int requestedBusLocation=-1;
	int requestedDevnum=-1;
	if (options.device)
	{
		std::string devstr(options.device);
		size_t p = devstr.find(",");
		if (p != std::string::npos)
		{
			std::istringstream busStr(devstr.substr(0,p));
			busStr >> requestedBusLocation;
			std::istringstream devnumStr(devstr.substr(p+1));
			devnumStr >> requestedDevnum;
		}
		if ((requestedBusLocation==-1) || (requestedDevnum==-1))
		{
			std::cerr << "Invalid device specification" << std::endl;
			options.displayHelp = 1;
		}
	}


	if (options.displayHelp)
	{
		fuse_opt_add_arg(&args, "-h");
	}
	else if (options.showVersion)
	{
		fuse_opt_add_arg(&args, "-V");
	}
	else
	{
		currentDevice = 0;
		ConnectedMtpDevices devices;
		if (devices.NumDevices()==0)
		{
			std::cerr << "No mtp devices found." << std::endl;
			return -1;
		}
		for(int i = 0; i<devices.NumDevices(); i++)
		{
			ConnectedDeviceInfo devInfo = devices.GetDeviceInfo(i);
			if (((devInfo.bus_location == requestedBusLocation) &&
				  (devInfo.devnum == requestedDevnum)) ||
				  (requestedBusLocation == -1) || (requestedDevnum == -1))
			{
				device = devices.GetDevice(i);
				currentDevice = device.get();
				break;
			}
		}
		if (currentDevice == 0)
		{
			std::cerr << "Requested device not found" << std::endl;
			return -1;
		}
		cache = std::unique_ptr<MtpMetadataCache>(new MtpMetadataCache);
		metadataCache = cache.get();
	}

	if (options.showVersion)
	{
		std::cout << "jmtpfs version: " << JMTPFS_VERSION << std::endl;
	}
	int result = fuse_main(args.argc, args.argv, &jmtpfs_oper, 0);

	if (options.displayHelp)
	{
		std::cout << std::endl << "jmtpfs options:" << std::endl;
		std::cout << "    -l                          list available mtp devices and then exit" << std::endl;
		std::cout << "    -device=<busnum>,<devnum>   Device to mount. It not specified the first device found is used"<< std::endl;

	}

	return result;
}
