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
#include "FuseHeader.h"
#include "MtpFuseContext.h"
#include "MtpRoot.h"

#include <iostream>
#include <cstddef>
#include <errno.h>
#include <sstream>
#include <iomanip>
#include <assert.h>
#include <unistd.h>

#define JMTPFS_VERSION "0.5"

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
	LockMutex lock(globalLock); \
	    MtpFuseContext* context((MtpFuseContext*)(fuse_get_context()->private_data)); \

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



extern "C" int jmtpfs_getattr(const char* pathStr, struct stat* info)
{
	FUSE_ERROR_BLOCK_START

		FilesystemPath path(pathStr);
		context->getNode(path)->getattr(*info);
		info->st_uid = context->uid();
		info->st_gid = context->gid();
		return 0;

	FUSE_ERROR_BLOCK_END

}

extern "C" int jmtpfs_readdir(const char* pathStr, void* buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi)
{
	FUSE_ERROR_BLOCK_START

		FilesystemPath path(pathStr);
		std::unique_ptr<MtpNode> n = context->getNode(path);
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
	context->getNode(path)->Open();
	return 0;

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_release(const char *pathStr, struct fuse_file_info *)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	context->getNode(path)->Close();
	return 0;

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_read(const char *pathStr, char *buf, size_t  size, off_t offset, struct fuse_file_info *)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	return context->getNode(path)->Read(buf,size,offset);

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_mkdir(const char* pathStr, mode_t mode)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	context->getNode(path.AllButTail())->mkdir(path.Tail());
	return 0;

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_rmdir(const char* pathStr)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	context->getNode(path)->Remove();
	return 0;

	FUSE_ERROR_BLOCK_END
}


extern "C" int jmtpfs_create(const char* pathStr, mode_t mode, struct fuse_file_info *fileInfo)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	std::unique_ptr<MtpNode> n = context->getNode(path.AllButTail());
	n->CreateFile(path.Tail());
	n = context->getNode(path);
	n->Open();
	return 0;

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_write(const char *pathStr, const char *data, size_t size, off_t offset, struct fuse_file_info *)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	return context->getNode(path)->Write(data, size, offset);

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_truncate(const char *pathStr, off_t length)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	context->getNode(path)->Truncate(length);
	return 0;

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_unlink(const char *pathStr)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	context->getNode(path)->Remove();
	return 0;

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_flush(const char *pathStr, struct fuse_file_info *)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	context->getNode(path)->Close();
	return 0;

	FUSE_ERROR_BLOCK_END
}


extern "C" int jmtpfs_rename(const char *pathStr, const char *newPathStr)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	std::unique_ptr<MtpNode> n = context->getNode(path);
	FilesystemPath newPath(newPathStr);
	std::unique_ptr<MtpNode> newParent = context->getNode(newPath.AllButTail());
	n->Rename(*newParent, newPath.Tail());

	return 0;

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_statfs(const char *pathStr, struct statvfs *stat)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	std::unique_ptr<MtpNode> n = context->getNode(path);
	n->statfs(stat);
	return 0;

	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_chmod(const char* pathStr, mode_t mode)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	context->getNode(path);
	// a noop since mtp doesn't support permissions. But we need to pretend
    // to do it to make things like "cp -r" and the mac finder happy.

	return 0;
	FUSE_ERROR_BLOCK_END
}

extern "C" int jmtpfs_utime(const char* pathStr, struct utimbuf*)
{
	FUSE_ERROR_BLOCK_START

	FilesystemPath path(pathStr);
	context->getNode(path);
	// a noop since mtp doesn't support permissions. But we need to pretend
    // to do it to make things like "cp -r" and the mac finder happy.

	return 0;
	FUSE_ERROR_BLOCK_END
}


struct jmtpfs_options
{
	jmtpfs_options() : listDevices(0), displayHelp(0),
			showVersion(0), device(0), listStorage(0) {}

	int	listDevices;
	int displayHelp;
	int showVersion;
	int listStorage;
	char* device;
};

static struct fuse_opt jmtpfs_opts[] = {
		{"-l", offsetof(struct jmtpfs_options, listDevices), 1},
		{"--listDevices", offsetof(struct jmtpfs_options, listDevices), 1},
//		{"-ls", offsetof(struct jmtpfs_options, listStorage), 1},
//		{"--listStorage", offsetof(struct jmtpfs_options, listStorage), 1},
		{"-h", offsetof(struct jmtpfs_options, displayHelp), 1},
		{"--help", offsetof(struct jmtpfs_options, displayHelp),1},
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
	jmtpfs_oper.chmod = jmtpfs_chmod;
	jmtpfs_oper.utime = jmtpfs_utime;

	jmtpfs_options options;

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	if (fuse_opt_parse(&args, &options, jmtpfs_opts,0)==-1)
	{
		std::cerr << "Error parsing arguments" << std::endl;
		return -1;
	}

	if (options.listDevices)
	{
		LIBMTP_Init();
		ConnectedMtpDevices devices;
		std::vector<std::string> devListing;
		std::vector<std::vector<std::string> > storageDevices;
		for(int i=0; i < devices.NumDevices(); i++)
		{
			ConnectedDeviceInfo devInfo = devices.GetDeviceInfo(i);
			std::ostringstream devText;
			devText << devInfo.bus_location << ", " << (uint) devInfo.devnum << ", ";
			devText << "0x" <<  hex << setfill('0') << setw(4) << devInfo.product_id << ", ";
			devText << "0x" << hex << setfill('0') << setw(4)<< devInfo.vendor_id << ", ";
			devText << devInfo.product << ", " << devInfo.vendor;
			devListing.push_back(devText.str());
			if (options.listStorage)
			{
				std::unique_ptr<MtpDevice> device = devices.GetDevice(i);
				std::vector<MtpStorageInfo> storages = device->GetStorageDevices();
				std::vector<std::string> storageList;
				for(std::vector<MtpStorageInfo>::iterator i = storages.begin(); i != storages.end(); i++)
					storageList.push_back(i->description);
				storageDevices.push_back(storageList);
			}
		}
		std::cout << "Available devices (busLocation, devNum, productId, vendorId, product, vendor):" << std::endl;
		for(std::vector<std::string>::iterator i = devListing.begin(); i != devListing.end(); i++)
		for(size_t i=0; i<devListing.size(); i++)
		{
			std::cout << devListing[i] << std::endl;
			if (options.listStorage)
			{
				for(std::vector<std::string>::iterator s = storageDevices[i].begin(); s != storageDevices[i].end(); s++)
					std::cout << "    " << *s << std::endl;
			}
		}
		return EXIT_SUCCESS;
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

	if (options.listStorage)
	{
		LIBMTP_Init();
		std::unique_ptr<MtpDevice> device;
		ConnectedMtpDevices devices;
		if (devices.NumDevices()==0)
		{
			std::cerr << "No mtp devices found." << std::endl;
			return -1;
		}
		try
		{
			if ((requestedBusLocation!=-1) && (requestedDevnum != -1))
				device = devices.GetDevice(0);
			else
				device = devices.GetDevice(requestedBusLocation, requestedDevnum);
		}
		catch(MtpDeviceNotFound&)
		{
			std::cerr << "Requested device not found" << std::endl;
			return -1;
		}
		std::cout << "Storage devices on " << device->Get_Modelname() << ":"<< std::endl;
		std::vector<MtpStorageInfo> storages = device->GetStorageDevices();
		for(std::vector<MtpStorageInfo>::iterator i = storages.begin(); i != storages.end(); i++)
			std::cout << "    " << i->description << std::endl;
		return EXIT_SUCCESS;
	}


	std::unique_ptr<MtpFuseContext> context;

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
		LIBMTP_Init();
		std::unique_ptr<MtpDevice> device;
		ConnectedMtpDevices devices;
		if (devices.NumDevices()==0)
		{
			std::cerr << "No mtp devices found." << std::endl;
			return -1;
		}
		try
		{
			if ((requestedBusLocation==-1) || (requestedDevnum == -1))
				device = devices.GetDevice(0);
			else
				device = devices.GetDevice(requestedBusLocation, requestedDevnum);
		}
		catch(MtpDeviceNotFound&)
		{
			std::cerr << "Requested device not found" << std::endl;
			return -1;
		}

		context = std::unique_ptr<MtpFuseContext>(new MtpFuseContext(std::move(device), getuid(), getgid()));

	}

	if (options.showVersion)
	{
		std::cout << "jmtpfs version: " << JMTPFS_VERSION << std::endl;
	}

#ifdef __APPLE__
	fuse_opt_add_arg(&args, "-f");
	std::cout << "Running in the background disabled because of an imcompatiblity between fork and libmtp under Max OS X" << std::endl;
	fuse_opt_add_arg(&args, "-s"); // bug in fuse4x where multithreaded sometimes doesn't exit correctly.
#endif

	int result = fuse_main(args.argc, args.argv, &jmtpfs_oper, context.get());

	if (options.displayHelp)
	{
		std::cout << std::endl << "jmtpfs options:" << std::endl;
		std::cout << "    -l    --listDevices         list available mtp devices and then exit" << std::endl;
//		std::cout << "    -ls   --listStorage         list the storage areas on the device (or all devices if -l is also specified)" << std::endl;
		std::cout << "    -device=<busnum>,<devnum>   Device to mount. It not specified the first device found is used"<< std::endl;

	}

	return result;
}
