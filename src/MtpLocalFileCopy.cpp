/*
 * MtpLocalFileCopy.cpp
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
#include "MtpLocalFileCopy.h"
#include "mtpFilesystemErrors.h"
#include <sys/stat.h>
#include <iostream>
#include <unistd.h>

MtpLocalFileCopy::MtpLocalFileCopy(MtpDevice& device, uint32_t id) :
	m_device(device), m_remoteId(id), m_needWriteBack(false)
{
	m_localFile = tmpfile();
	if (m_localFile == 0)
		throw CantCreateTempFile(errno);
	m_device.GetFile(m_remoteId, fileno(m_localFile));

}

MtpLocalFileCopy::~MtpLocalFileCopy()
{
	close();
}


uint32_t MtpLocalFileCopy::close()
{
	if (m_localFile)
	{
		if (m_needWriteBack)
		{
			m_needWriteBack = false;
			try
			{
				fflush(m_localFile);
				if (fseek(m_localFile, 0, SEEK_SET))
					throw WriteError(errno);
				struct stat tempInfo;
				if (fstat(fileno(m_localFile), &tempInfo))
					throw ReadError(errno);
				MtpFileInfo remoteInfo = m_device.GetFileInfo(m_remoteId);
				NewLIBMTPFile newFile(remoteInfo.name, remoteInfo.parentId, remoteInfo.storageId, tempInfo.st_size);
				m_device.DeleteObject(m_remoteId);
				std::cout << "************ sending file" << std::endl;
				m_device.SendFile(newFile, fileno(m_localFile));
				m_remoteId = ((LIBMTP_file_t*)newFile)->item_id;
			}
			catch(...)
			{
				fclose(m_localFile);
				m_localFile=0;
				throw;
			}
		}

		fclose(m_localFile);
		m_localFile = 0;
	}
	return m_remoteId;
}

off_t MtpLocalFileCopy::getSize()
{
	fflush(m_localFile);
	struct stat tempInfo;
	if (fstat(fileno(m_localFile), &tempInfo))
			throw ReadError(errno);
	return tempInfo.st_size;
}

void MtpLocalFileCopy::seek(long offset)
{
	if (fseek(m_localFile, offset, SEEK_SET))
		throw MtpFilesystemErrorWithErrorCode(errno, "seek failed");
}

size_t MtpLocalFileCopy::write(const void* ptr, size_t size)
{
	size_t wroteBytes = fwrite(ptr, 1, size, m_localFile);
	m_needWriteBack = true;
	if (wroteBytes!= size)
		if (ferror(m_localFile))
			throw WriteError(errno);
	return wroteBytes;
}

size_t MtpLocalFileCopy::read(void* ptr, size_t size)
{

	size_t readBytes = fread(ptr, 1, size, m_localFile);
	if (readBytes!= size)
		if (ferror(m_localFile))
			throw ReadError(errno);
	return readBytes;
}

void MtpLocalFileCopy::truncate(off_t length)
{
	if (ftruncate(fileno(m_localFile), length))
		throw WriteError(errno);
	m_needWriteBack = true;
}

void MtpLocalFileCopy::CopyTo(MtpDevice& device, NewLIBMTPFile& destination)
{
	fflush(m_localFile);
	if (fseek(m_localFile, 0, SEEK_SET))
		throw WriteError(errno);
	struct stat tempInfo;
	if (fstat(fileno(m_localFile), &tempInfo))
			throw ReadError(errno);

	device.SendFile(destination, fileno(m_localFile));
}
