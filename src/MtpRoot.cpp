/*
 * MtpRoot.cpp
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
#include "MtpRoot.h"
#include "mtpFilesystemErrors.h"
#include "MtpStorage.h"
#include <limits>

MtpRoot::MtpRoot(MtpDevice& device, MtpMetadataCache& cache) : MtpNode(device, cache, std::numeric_limits<uint32_t>::max())
{
}

void MtpRoot::getattr(struct stat& info)
{
	info.st_mode = S_IFDIR | 0755;
	info.st_nlink = 2 + readDirectory().size();

}

std::unique_ptr<MtpNode> MtpRoot::getNode(const FilesystemPath& path)
{
	MtpNodeMetadata md = m_cache.getItem(m_id, *this);

	if (path.Empty())
		throw FileNotFound(path.str());
	std::string storageName = path.Head();
	for(std::vector<MtpStorageInfo>::iterator i = md.storages.begin(); i != md.storages.end(); i++)
	{
		if (i->description == storageName)
		{
			std::unique_ptr<MtpNode> storageDevice(new MtpStorage(m_device, m_cache, i->id));
			FilesystemPath childPath = path.Body();
			if (childPath.Empty())
				return storageDevice;
			else
				return storageDevice->getNode(childPath);
		}
	}
	throw FileNotFound(path.str());
}



std::vector<std::string> MtpRoot::readDirectory()
{
	MtpNodeMetadata md = m_cache.getItem(m_id, *this);

	std::vector<std::string> result;
	for(std::vector<MtpStorageInfo>::iterator i = md.storages.begin(); i != md.storages.end(); i++)
		result.push_back(i->description);
	return result;
}


void MtpRoot::mkdir(const std::string& name)
{
	throw ReadOnly();
}

void MtpRoot::Remove()
{
	throw ReadOnly();
}

MtpNodeMetadata MtpRoot::getMetadata()
{
	MtpNodeMetadata md;
	md.self.id = m_id;
	md.self.parentId = 0;
	md.self.storageId = 0;
	md.storages = m_device.GetStorageDevices();

	return md;
}

MtpStorageInfo MtpRoot::GetStorageInfo()
{
	return MtpStorageInfo(0,"",0,0);
}

void MtpRoot::statfs(struct statvfs *stat)
{
	size_t totalSize = 0;
	size_t totalFree = 0;

	std::vector<std::string> storages = readDirectory();
	for(std::vector<std::string>::iterator s = storages.begin(); s != storages.end(); s++)
	{
		MtpStorageInfo info = getNode(FilesystemPath(s->c_str()))->GetStorageInfo();
		totalSize += info.maxCapacity;
		totalFree += info.freeSpaceInBytes;
	}

	stat->f_bsize = 512;  // We have to pick some block size, so why not 512?
	stat->f_blocks = totalSize / stat->f_bsize;
	stat->f_bfree = totalFree / stat->f_bsize;
	stat->f_bavail = stat->f_bfree;
	stat->f_namemax = 233;

}
