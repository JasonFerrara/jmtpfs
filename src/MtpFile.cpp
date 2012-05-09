/*
 * MtpFile.cpp
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
#include "MtpFile.h"
#include "mtpFilesystemErrors.h"
#include <errno.h>

MtpFile::MtpFile(MtpDevice& device,  MtpMetadataCache& cache, uint32_t id) : MtpNode(device, cache, id), m_opened(false)
{
}

MtpFile::~MtpFile()
{
}

std::unique_ptr<MtpNode> MtpFile::getNode(const FilesystemPath& path)
{
	throw FileNotFound(path.str());
}

MtpNodeMetadata MtpFile::getMetadata()
{
	MtpNodeMetadata md;
	md.self = m_device.GetFileInfo(m_id);
	return md;
}

void MtpFile::getattr(struct stat& info)
{
	MtpNodeMetadata md = m_cache.getItem(m_id, *this);

	info.st_mode = S_IFREG | 0644;
	info.st_nlink = 1;
	info.st_mtime = md.self.modificationdate;
	MtpLocalFileCopy* localFile = m_cache.getOpenedFile(m_id);
	if (localFile)
	{
		info.st_size = localFile->getSize();
	}
	else
		info.st_size = md.self.filesize;
}


void MtpFile::Open()
{
	m_cache.openFile(m_device, m_id);
}

int MtpFile::Read(char *buf, size_t size, off_t offset)
{
	MtpLocalFileCopy* localFile = m_cache.openFile(m_device, m_id);

	localFile->seek(offset);
	return localFile->read(buf, size);

}

int MtpFile::Write(const char* buf, size_t size, off_t offset)
{

	MtpLocalFileCopy* localFile = m_cache.openFile(m_device, m_id);
	localFile->seek(offset);
//	m_cache.clearItem(m_id);
	return localFile->write(buf, size);
}

void MtpFile::Fsync()
{
	uint32_t parentId = GetParentNodeId();
	m_cache.clearItem(m_id);
	m_id = m_cache.closeFile(m_id);
	m_cache.clearItem(parentId);
}

void MtpFile::Close()
{
	Fsync();
}

void MtpFile::Truncate(off_t length)
{

	struct stat info;
	getattr(info);
	if (info.st_size == length)
		return;
	uint32_t parentId = GetParentNodeId();
	MtpLocalFileCopy* localFile = m_cache.openFile(m_device, m_id);
	localFile->truncate(length);
	m_id = m_cache.closeFile(m_id);
	m_cache.clearItem(m_id);
	m_cache.clearItem(parentId);
}



void MtpFile::Remove()
{
	uint32_t parentId = GetParentNodeId();
	m_device.DeleteObject(m_id);
	m_cache.clearItem(parentId);
	m_cache.clearItem(m_id);

}

void MtpFile::Rename(MtpNode& newParent, const std::string& newName)
{
	if (newName.length() > MAX_MTP_NAME_LENGTH)
		throw MtpNameTooLong();
	Fsync();
	MtpNodeMetadata md = m_cache.getItem(m_id, *this);
	uint32_t parentId = GetParentNodeId();
	/* This true in place rename seems to confuse apps on the android device. The Gallery app
	 * for example, won't notice image files that have been renamed. So to prevent this strangeness
	 * real rename is disabled, and instead we make a copy of the file and delete the original
	if ((newParent.FolderId() == md.self.parentId) && (newParent.StorageId() == md.self.storageId))
	{
		// we can do a real rename
		m_device.RenameFile(md.self.id, newName);
	}
	else
	*/
	{
		//we have to do a copy and delete
		MtpLocalFileCopy* localFile = m_cache.openFile(m_device, md.self.id);
		NewLIBMTPFile newFile(newName, newParent.FolderId(), newParent.StorageId(), localFile->getSize());
		localFile->CopyTo(m_device, newFile);
		m_cache.clearItem(md.self.id);
		m_cache.clearItem(((LIBMTP_file_t*)newFile)->item_id);
		m_device.DeleteObject(md.self.id);
		m_id = ((LIBMTP_file_t*)newFile)->item_id;

	}
	m_cache.clearItem(parentId);
}
