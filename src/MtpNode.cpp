/*
 * MtpNode.cpp
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
#include "MtpNode.h"
#include "mtpFilesystemErrors.h"
MtpNode::MtpNode(MtpDevice& device, MtpMetadataCache& cache, uint32_t id) : m_device(device), m_cache(cache), m_id(id)
{
}

MtpNode::~MtpNode()
{

}

uint32_t MtpNode::Id()
{
	return m_id;
}

uint32_t MtpNode::GetParentNodeId()
{
	MtpNodeMetadata md = m_cache.getItem(m_id, *this);

	if (md.self.parentId == 0)
		return md.self.storageId;
	else
		return md.self.parentId;
}


std::vector<std::string> MtpNode::readdir()
{
	std::vector<std::string> result;
	result.push_back(".");
	result.push_back("..");
	std::vector<std::string> entries = readDirectory();
	result.insert(result.end(), entries.begin(), entries.end());
	return result;
}

std::vector<std::string> MtpNode::readDirectory()
{
	throw NotADirectory();
}


void MtpNode::Open()
{
	throw NotImplemented("Open");
}

void MtpNode::Close()
{
	throw NotImplemented("Close");
}

int MtpNode::Read(char *buf, size_t size, off_t offset)
{
	throw NotImplemented("Read");
}

void MtpNode::mkdir(const std::string& name)
{
	throw NotImplemented("mkdir");
}

void MtpNode::Remove()
{
	throw NotImplemented("mkdir");
}


void MtpNode::CreateFile(const std::string& name)
{
	throw NotImplemented("CreateFile");
}

int MtpNode::Write(const char* buf, size_t size, off_t offset)
{
	throw NotImplemented("Write");
}

void MtpNode::Truncate(off_t length)
{
	throw NotImplemented("Truncate");
}

void MtpNode::Rename(MtpNode& newParent, const std::string& newName)
{
	throw NotImplemented("Rename");
}

uint32_t MtpNode::FolderId()
{
	throw NotImplemented("FolderId");
}

uint32_t MtpNode::StorageId()
{
	throw NotImplemented("StorageId");
}

MtpStorageInfo MtpNode::GetStorageInfo()
{
	MtpNodeMetadata md = m_cache.getItem(m_id, *this);
	return m_device.GetStorageInfo(md.self.storageId);
}

void MtpNode::statfs(struct statvfs *stat)
{

	MtpStorageInfo storageInfo = GetStorageInfo();

	stat->f_bsize = 512;  // We have to pick some block size, so why not 512?
	stat->f_blocks = storageInfo.maxCapacity / stat->f_bsize;
	stat->f_bfree = storageInfo.freeSpaceInBytes / stat->f_bsize;
	stat->f_bavail = stat->f_bfree;
	stat->f_namemax = 233;

}

std::unique_ptr<MtpNode> MtpNode::Clone()
{
	throw NotImplemented("Clone");
}
