/*
 * MtpFolder.cpp
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

#include "MtpFolder.h"
#include "MtpFile.h"
#include "mtpFilesystemErrors.h"
#include "TemporaryFile.h"

MtpFolder::MtpFolder(MtpDevice& device, MtpMetadataCache& cache, uint32_t storageId,
		uint32_t folderId) : MtpNode(device, cache, folderId ? folderId : storageId),
		m_storageId(storageId), m_folderId(folderId)
{

}

MtpNodeMetadata MtpFolder::getMetadata()
{
	MtpNodeMetadata md;
	uint32_t folderId = m_folderId;
	if (folderId==0)
	{
		folderId = 0xFFFFFFFF;
		md.self.id = m_storageId;
		md.self.parentId = 0;
		md.self.storageId = m_storageId;
	}
	else
	{
		md.self = m_device.GetFileInfo(m_id);
	}
	md.children = m_device.GetFolderContents(m_storageId, folderId);

	return md;
}

void MtpFolder::getattr(struct stat& info)
{
	MtpNodeMetadata md = m_cache.getItem(m_id, *this);
	info.st_mode = S_IFDIR | 0755;
	info.st_nlink = 2;
	info.st_mtime = md.self.modificationdate;
	for(std::vector<MtpFileInfo>::iterator i = md.children.begin(); i!=md.children.end(); i++)
	{
		if (i->filetype == LIBMTP_FILETYPE_FOLDER)
			info.st_nlink++;
	}
}

std::unique_ptr<MtpNode> MtpFolder::getNode(const FilesystemPath& path)
{
	MtpNodeMetadata md = m_cache.getItem(m_id, *this);

	std::string filename = path.Head();
	for(std::vector<MtpFileInfo>::iterator i = md.children.begin(); i!=md.children.end(); i++)
	{
		if (i->name == filename)
		{
			if (i->filetype != LIBMTP_FILETYPE_FOLDER)
				return std::unique_ptr<MtpNode>(new MtpFile(m_device, m_cache, i->id));
			else
			{
				std::unique_ptr<MtpNode> n(new MtpFolder(m_device, m_cache, m_storageId, i->id));
				FilesystemPath childPath = path.Body();
				if (childPath.Empty())
					return n;
				else
					return n->getNode(childPath);
			}
		}
	}
	throw FileNotFound(path.str());
}

std::vector<std::string> MtpFolder::readDirectory()
{
	MtpNodeMetadata md = m_cache.getItem(m_id, *this);

	std::vector<std::string> result;

	for(std::vector<MtpFileInfo>::iterator i = md.children.begin(); i != md.children.end(); i++)
		result.push_back(i->name);
	return result;
}


void MtpFolder::Remove()
{
	if (readDirectory().size()>0)
		throw MtpDirectoryNotEmpty();
	uint32_t parentId = GetParentNodeId();
	m_device.DeleteObject(m_id);
	m_cache.clearItem(parentId);
	m_cache.clearItem(m_id);

}

void MtpFolder::mkdir(const std::string& name)
{
	if (name.length() > MAX_MTP_NAME_LENGTH)
		throw MtpNameTooLong();

	m_device.CreateFolder(name, m_folderId, m_storageId);
	m_cache.clearItem(m_id);
}



void MtpFolder::CreateFile(const std::string& name)
{
	if (name.length() > MAX_MTP_NAME_LENGTH)
		throw MtpNameTooLong();

	NewLIBMTPFile newFile(name, m_folderId, m_storageId);
	TemporaryFile empty;
	m_device.SendFile(newFile, empty.FileNo());
	m_cache.clearItem(((LIBMTP_file_t*)newFile)->item_id);
	m_cache.clearItem(m_id);
}

uint32_t MtpFolder::FolderId()
{
	return m_folderId;
}

uint32_t MtpFolder::StorageId()
{
	return m_storageId;
}

void MtpFolder::Rename(MtpNode& newParent, const std::string& newName)
{
	if (newName.length() > MAX_MTP_NAME_LENGTH)
		throw MtpNameTooLong();

	MtpNodeMetadata md = m_cache.getItem(m_id, *this);

	uint32_t parentId = GetParentNodeId();
	if ((newParent.FolderId() == md.self.parentId) && (newParent.StorageId() == m_storageId))
	{
		// we can do a real rename
		m_device.RenameFile(m_id, newName);
	}
	else
	{
		// we have to do a copy and delete
		newParent.mkdir(newName);
		std::unique_ptr<MtpNode> destDir(newParent.getNode(FilesystemPath(newName.c_str())));
		std::vector<std::string> contents = readDirectory();
		for(std::vector<std::string>::iterator i = contents.begin(); i != contents.end(); i++)
		{
			std::unique_ptr<MtpNode> child(getNode(FilesystemPath(i->c_str())));
			child->Rename(*destDir, *i);
		}
		Remove();
	}
	m_cache.clearItem(newParent.Id());
	m_cache.clearItem(parentId);

}

