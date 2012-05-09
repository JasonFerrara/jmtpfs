/*
 * MtpDevice.cpp
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
#include "MtpDevice.h"
#include "MtpLibLock.h"
#include "ConnectedMtpDevices.h"
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <magic.h>



MtpFileInfo::MtpFileInfo(LIBMTP_file_t& info)
{
	id = info.item_id;
	parentId = info.parent_id;
	storageId = info.storage_id;
	name = info.filename;
	filetype = info.filetype;
	filesize = info.filesize;
	modificationdate = info.modificationdate;
}

NewLIBMTPFile::NewLIBMTPFile(const std::string& filename, uint32_t parentId, uint32_t storageId, uint64_t size)
{
	m_fileInfo = LIBMTP_new_file_t();
	m_fileInfo->filename = strdup(filename.c_str());
	m_fileInfo->parent_id = parentId;
	m_fileInfo->storage_id = storageId;
	m_fileInfo->filesize = size;
}

NewLIBMTPFile::~NewLIBMTPFile()
{
	LIBMTP_destroy_file_t(m_fileInfo);
}

NewLIBMTPFile::operator LIBMTP_file_t*()
{
	return m_fileInfo;
}

MtpDevice::MtpDevice(LIBMTP_raw_device_t& rawDevice)
{
MtpLibLock	lock;

	m_mtpdevice = LIBMTP_Open_Raw_Device_Uncached(&rawDevice);
	if (m_mtpdevice == 0)
		throw MtpErrorCantOpenDevice();
	m_busLocation = rawDevice.bus_location;
	m_devnum = rawDevice.devnum;
	LIBMTP_Clear_Errorstack(m_mtpdevice);
	m_magicCookie = magic_open(MAGIC_MIME_TYPE);
	if (m_magicCookie == 0)
		throw std::runtime_error("Couldn't init magic");
	if (magic_load(m_magicCookie, 0))
		throw std::runtime_error(magic_error(m_magicCookie));

}

MtpDevice::~MtpDevice()
{
MtpLibLock	lock;

	LIBMTP_Release_Device(m_mtpdevice);
}

std::string MtpDevice::Get_Modelname()
{
MtpLibLock	lock;

	char* fn = LIBMTP_Get_Modelname(m_mtpdevice);
	if (fn)
	{
		std::string result(fn);
		free(fn);
		return result;
	}
	else
	{
		CheckErrors(false);
		return "";
	}
}


std::vector<MtpStorageInfo> MtpDevice::GetStorageDevices()
{
MtpLibLock	lock;

	if (LIBMTP_Get_Storage(m_mtpdevice, LIBMTP_STORAGE_SORTBY_NOTSORTED))
	{
		CheckErrors(true);
	}

	LIBMTP_devicestorage_t* storage = m_mtpdevice->storage;
	std::vector<MtpStorageInfo> result;
	while(storage)
	{
		result.push_back(MtpStorageInfo(storage->id, storage->StorageDescription,
				storage->FreeSpaceInBytes, storage->MaxCapacity));
		storage = storage->next;
	}
	return result;

}

MtpStorageInfo MtpDevice::GetStorageInfo(uint32_t storageId)
{
	std::vector<MtpStorageInfo> storages = GetStorageDevices();
	for(std::vector<MtpStorageInfo>::iterator i = storages.begin(); i != storages.end(); i++)
		if (i->id == storageId)
			return *i;
	throw MtpStorageNotFound("storage not found");
}

std::vector<MtpFileInfo> MtpDevice::GetFolderContents(uint32_t storageId, uint32_t folderId)
{
MtpLibLock lock;

	std::vector<MtpFileInfo> result;
	LIBMTP_file_t* files = LIBMTP_Get_Files_And_Folders(m_mtpdevice, storageId, folderId);
	if (files == 0)
	{
		CheckErrors(false);
		return result;
	}
	LIBMTP_file_t* filesWalk = files;
	while(filesWalk)
	{
		MtpFileInfo fileInfo(MtpFileInfo(filesWalk->item_id, filesWalk->parent_id, filesWalk->storage_id, filesWalk->filename,
					filesWalk->filetype, filesWalk->filesize));
		result.push_back(fileInfo);
		filesWalk = filesWalk->next;
	}
	if (files)
		LIBMTP_destroy_file_t(files);
	return result;
}



MtpFileInfo MtpDevice::GetFileInfo(uint32_t id)
{
MtpLibLock lock;

	LIBMTP_file_t* fileInfoP = LIBMTP_Get_Filemetadata(m_mtpdevice, id);
	if (fileInfoP==0)
	{
		CheckErrors(true);
	}
	MtpFileInfo fileInfo(*fileInfoP);
	LIBMTP_destroy_file_t(fileInfoP);
	return fileInfo;

}


void MtpDevice::GetFile(uint32_t id, int fd)
{
MtpLibLock lock;

	if (LIBMTP_Get_File_To_File_Descriptor(m_mtpdevice, id, fd,0,0))
		CheckErrors(true);
}

void MtpDevice::CreateFolder(const std::string& name, uint32_t parentId, uint32_t storageId)
{
MtpLibLock lock;

	if (LIBMTP_Create_Folder(m_mtpdevice, (char*) name.c_str(), parentId, storageId)==0)
		CheckErrors(true);
}

void MtpDevice::CheckErrors(bool throwEvenWithNoError)
{
MtpLibLock lock;

	LIBMTP_error_t* errors = LIBMTP_Get_Errorstack(m_mtpdevice);
	if (errors)
	{
		LIBMTP_error_number_t errorCode = errors->errornumber;
		std::string errorText(errors->error_text);
		LIBMTP_Clear_Errorstack(m_mtpdevice);
		switch(errorCode)
		{
		case LIBMTP_ERROR_NO_DEVICE_ATTACHED:
			throw MtpDeviceDisconnected(errorText);
		default:
			throw MtpError(errorText, errorCode);
		}

	}
	LIBMTP_Clear_Errorstack(m_mtpdevice);
	if (throwEvenWithNoError)
		throw ExpectedMtpErrorNotFound();
}

void MtpDevice::DeleteObject(uint32_t id)
{
MtpLibLock lock;
	if (LIBMTP_Delete_Object(m_mtpdevice, id))
		CheckErrors(true);
}

void MtpDevice::SendFile(LIBMTP_file_t* destination, int fd)
{
MtpLibLock lock;

const char* mimeType = 0;
	if (destination->filesize > 0)
	{
		// We want to use magic_descriptor here, but there is a bug
		// in magic_descriptor that closes the file descriptor, which
		// then prevents the LIBMTP_Send_File_From_File_Descriptor from working.
		// I tried calling dup on fd and passing the new file descriptor to
		// magic_descriptor, but for some reason that still didn't work.
		// Perhaps because our file descriptors were created with tmpfile
		// there is some other bug that makes the file go away on the first close.
		// So as a work around we copy the beginning of the file into memory and use
		// magic_buffer. Hopefully our buffer is big enough that libmagic has
		// enough data to work its magic.
		lseek(fd, 0, SEEK_SET);
		ssize_t bytesRead = read(fd, m_magicBuffer, MAGIC_BUFFER_SIZE);
		lseek(fd,0,SEEK_SET);
		mimeType = magic_buffer(m_magicCookie, m_magicBuffer, bytesRead);
	}
	if (mimeType)
		destination->filetype = PropertyTypeFromMimeType(mimeType);


	if (LIBMTP_Send_File_From_File_Descriptor(m_mtpdevice, fd, destination, 0,0))
		CheckErrors(true);
}


void MtpDevice::RenameFile(uint32_t id, const std::string& newName)
{
	MtpLibLock lock;
	LIBMTP_file_t* fileInfo = LIBMTP_Get_Filemetadata(m_mtpdevice, id);
	if (fileInfo==0)
	{
		CheckErrors(true);
	}
	int result = LIBMTP_Set_File_Name(m_mtpdevice, fileInfo, newName.c_str());
	LIBMTP_destroy_file_t(fileInfo);
	if (result)
		CheckErrors(true);
}

void MtpDevice::SetObjectProperty(uint32_t id, LIBMTP_property_t property, const std::string& value)
{
	MtpLibLock lock;
	if (LIBMTP_Set_Object_String(m_mtpdevice, id, property, value.c_str()))
		CheckErrors(true);
}

LIBMTP_filetype_t MtpDevice::PropertyTypeFromMimeType(const std::string& mimeType)
{
	if (mimeType == "video/quicktime")
		return LIBMTP_FILETYPE_QT;
	if (mimeType == "video/x-sgi-movie")
		return LIBMTP_FILETYPE_UNDEF_VIDEO;
	if (mimeType == "video/mp4")
		return LIBMTP_FILETYPE_MP4;
	if (mimeType == "video/3gpp")
		return LIBMTP_FILETYPE_MP4;
	if (mimeType == "audio/mp4")
		return LIBMTP_FILETYPE_M4A;
	if (mimeType == "video/mpeg")
		return LIBMTP_FILETYPE_MPEG;
	if (mimeType == "video/mpeg4-generic")
		return LIBMTP_FILETYPE_MPEG;
	if (mimeType == "audio/x-hx-aac-adif")
		return LIBMTP_FILETYPE_AAC;
	if (mimeType == "audio/x-hx-aac-adts")
		return LIBMTP_FILETYPE_AAC;
	if (mimeType ==  "audio/x-mp4a-latm")
		return LIBMTP_FILETYPE_M4A;
	if (mimeType == "video/x-fli")
		return LIBMTP_FILETYPE_UNDEF_VIDEO;
	if (mimeType == "video/x-flc")
		return LIBMTP_FILETYPE_UNDEF_VIDEO;
	if (mimeType == "video/x-unknown")
		return LIBMTP_FILETYPE_UNDEF_VIDEO;
	if (mimeType == "video/x-ms-asf")
		return LIBMTP_FILETYPE_ASF;
	if (mimeType == "video/x-mng")
		return LIBMTP_FILETYPE_UNDEF_VIDEO;
	if (mimeType == "video/x-jng")
		return LIBMTP_FILETYPE_UNDEF_VIDEO;
	if (mimeType == "video/h264")
		return LIBMTP_FILETYPE_UNDEF_VIDEO;
	if (mimeType == "audio/basic")
		return LIBMTP_FILETYPE_UNDEF_AUDIO;
	if (mimeType == "audio/midi")
		return LIBMTP_FILETYPE_UNDEF_AUDIO;
	if (mimeType == "image/jp2")
		return LIBMTP_FILETYPE_JP2;
	if (mimeType == "audio/x-unknown")
		return LIBMTP_FILETYPE_UNDEF_AUDIO;
	if (mimeType == "audio/x-pn-realaudio")
		return LIBMTP_FILETYPE_UNDEF_AUDIO;
	if (mimeType == "audio/x-mod")
		return LIBMTP_FILETYPE_UNDEF_AUDIO;
	if (mimeType == "audio/x-flac")
		return LIBMTP_FILETYPE_FLAC;
	if (mimeType == "image/tiff")
		return LIBMTP_FILETYPE_TIFF;
	if (mimeType == "image/png")
		return LIBMTP_FILETYPE_PNG;
	if (mimeType == "image/gif")
		return LIBMTP_FILETYPE_GIF;
	if (mimeType == "image/x-ms-bmp")
		return LIBMTP_FILETYPE_BMP;
	if (mimeType == "image/jpeg")
		return LIBMTP_FILETYPE_JPEG;
	if (mimeType =="text/calendar")
		return LIBMTP_FILETYPE_VCALENDAR2;
	if (mimeType =="text/x-vcard")
		return LIBMTP_FILETYPE_VCARD2;
	if (mimeType == "application/x-dosexec")
		return LIBMTP_FILETYPE_WINEXEC;
	if (mimeType == "application/msword")
		return LIBMTP_FILETYPE_DOC;
	if (mimeType == "application/vnd.ms-excel")
		return LIBMTP_FILETYPE_XLS;
	if (mimeType == "application/vnd.ms-powerpoint")
		return LIBMTP_FILETYPE_PPT;
	if (mimeType == "audio/x-wav")
		return LIBMTP_FILETYPE_WAV;
	if (mimeType == "video/x-msvideo")
		return LIBMTP_FILETYPE_AVI;
	if (mimeType == "LIBMTP_FILETYPE_AVI")
		return LIBMTP_FILETYPE_WAV;
	if (mimeType == "text/html")
		return LIBMTP_FILETYPE_HTML;
	if (mimeType == "application/xml")
		return LIBMTP_FILETYPE_TEXT;
	if (mimeType == "application/ogg")
		return LIBMTP_FILETYPE_OGG;
	if (mimeType == "audio/mpeg")
		return LIBMTP_FILETYPE_MP3;
	if (mimeType == "")


	if (mimeType.substr(0,5) == "text/")
		return LIBMTP_FILETYPE_TEXT;
	if (mimeType.substr(0,6) == "video/")
		return LIBMTP_FILETYPE_UNDEF_VIDEO;
	if (mimeType.substr(0,6) == "audio/")
		return LIBMTP_FILETYPE_UNDEF_AUDIO;
	return LIBMTP_FILETYPE_UNKNOWN;
}
