/*
 * MtpDevice.h
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

#ifndef MTPDEVICE_H_
#define MTPDEVICE_H_

#include "libmtp.h"
#include <string>
#include <vector>
#include <stdexcept>
#include <string.h>
#include <magic.h>

#define MAGIC_BUFFER_SIZE 8192

class MtpError : public std::runtime_error
{
public:
	explicit MtpError(const std::string& message, LIBMTP_error_number_t errorCode) : std::runtime_error(message), m_errorCode(errorCode) {}
	LIBMTP_error_number_t ErrorCode() { return m_errorCode; }

protected:
	LIBMTP_error_number_t	m_errorCode;
};

class ExpectedMtpErrorNotFound : public MtpError
{
public:
	ExpectedMtpErrorNotFound() : MtpError("Expected an error condition but found none", LIBMTP_ERROR_GENERAL) {}
};

class MtpErrorCantOpenDevice : public MtpError
{
public:
	MtpErrorCantOpenDevice() : MtpError("Can't open device", LIBMTP_ERROR_GENERAL) {}
};


class MtpDeviceDisconnected : public MtpError
{
public:
	MtpDeviceDisconnected(const std::string& message) : MtpError(message, LIBMTP_ERROR_NO_DEVICE_ATTACHED) {}
};

class MtpDeviceNotFound : public MtpError
{
public:
	MtpDeviceNotFound(const std::string& message) : MtpError(message, LIBMTP_ERROR_GENERAL) {}
};

class MtpStorageNotFound : public MtpError
{
public:
	MtpStorageNotFound(const std::string& message) : MtpError(message, LIBMTP_ERROR_GENERAL) {}
};


struct MtpStorageInfo
{
	MtpStorageInfo() {}
	MtpStorageInfo(uint32_t i, std::string s, uint64_t fs, uint64_t mc) :
		id(i), description(s), freeSpaceInBytes(fs), maxCapacity(mc) {}

	uint32_t id;
	std::string description;
	uint64_t freeSpaceInBytes;
	uint64_t maxCapacity;
};

struct MtpFileInfo
{
	MtpFileInfo() {}
	MtpFileInfo(LIBMTP_file_t& info);
	MtpFileInfo(uint32_t i, uint32_t p, uint32_t storage,   std::string s, LIBMTP_filetype_t t, uint64_t fs) :
			id(i), parentId(p), storageId(storage), name(s), filetype(t),
			filesize(fs){}

	uint32_t id;
	uint32_t parentId;
	uint32_t storageId;
	std::string name;
	LIBMTP_filetype_t	filetype;
	uint64_t	filesize;
	time_t	modificationdate;
};


class NewLIBMTPFile
{
public:
	NewLIBMTPFile(const std::string& filename, uint32_t parentId, uint32_t storageId, uint64_t size=0);
	~NewLIBMTPFile();

	operator LIBMTP_file_t* ();

protected:
	LIBMTP_file_t*	m_fileInfo;

private:
	NewLIBMTPFile(const NewLIBMTPFile&);
	NewLIBMTPFile& operator=(const NewLIBMTPFile&);
};

class MtpDevice
{
public:
	MtpDevice(LIBMTP_raw_device_t& rawDevice);
	~MtpDevice();

	std::string Get_Modelname();
	std::vector<MtpStorageInfo> GetStorageDevices();
	MtpStorageInfo GetStorageInfo(uint32_t storageId);
	std::vector<MtpFileInfo> GetFolderContents(uint32_t storageId, uint32_t folderId);
	MtpFileInfo GetFileInfo(uint32_t id);
	void GetFile(uint32_t id, int fd);
	void SendFile(LIBMTP_file_t* destination, int fd);
	void CreateFolder(const std::string& name, uint32_t parentId, uint32_t storageId);
	void DeleteObject(uint32_t id);
	void RenameFile(uint32_t id, const std::string& newName);
	void SetObjectProperty(uint32_t id, LIBMTP_property_t property, const std::string& value);
	static LIBMTP_filetype_t PropertyTypeFromMimeType(const std::string& mimeType);


protected:
	void CheckErrors(bool throwEvenIfNoError);
	LIBMTP_mtpdevice_t* m_mtpdevice;
	uint32_t		m_busLocation;
	uint8_t			m_devnum;
	magic_t			m_magicCookie;
	char			m_magicBuffer[MAGIC_BUFFER_SIZE];
};



#endif /* MTPDEVICE_H_ */
