/*
 * mtpFilesystemErrors.h
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
#ifndef MTPFILESYSTEMERRORS_H_
#define MTPFILESYSTEMERRORS_H_

#include <stdexcept>

// There appears to be a bug in Android 4.0.4 on the Galaxy Nexus (and maybe
// other Android versions as well) where if you specify a filename greater
// that 233 characters the MTP stack looses it mind. Creating the file
// reports an error. The file or may not have actually been created on the
// device filesystem, but either way MTP will show the file as existing
// but generate an error if you try to delete the file. You have to
// reboot the device to get MTP to start behaving again.
// So to prevent this from happening we set a limit on the filename
// length.
#define MAX_MTP_NAME_LENGTH 233

class MtpFilesystemError : public std::runtime_error
{
public:
	MtpFilesystemError(const std::string& what) : std::runtime_error(what) {};
};

class FileNotFound : public MtpFilesystemError
{
public:
	FileNotFound(const std::string& what) : MtpFilesystemError(std::string("File not found: ") + what)
	{
	}
};

class NotImplemented : public MtpFilesystemError
{
public:
	NotImplemented(const std::string& what) : MtpFilesystemError(std::string("Not implemented: ") + what)
	{
	}
};

class MtpFilesystemErrorWithErrorCode : public MtpFilesystemError
{
public:
	MtpFilesystemErrorWithErrorCode(int errorCode, const std::string& message) : MtpFilesystemError(message), m_errorCode(errorCode)
	{

	}

	int ErrorCode() const { return m_errorCode; };

private:
	int m_errorCode;
};

class CantCreateTempFile : public MtpFilesystemErrorWithErrorCode
{
public:
	CantCreateTempFile(int errorCode) : MtpFilesystemErrorWithErrorCode(errorCode, "Can't create temp file") {}
};

class ReadError : public MtpFilesystemErrorWithErrorCode
{
public:
	ReadError(int errorCode) : MtpFilesystemErrorWithErrorCode(errorCode, "Can't read file") {}
};

class WriteError : public MtpFilesystemErrorWithErrorCode
{
public:
	WriteError(int errorCode) : MtpFilesystemErrorWithErrorCode(errorCode, "Can't write file") {}
};

class ReadOnly : public MtpFilesystemErrorWithErrorCode
{
public:
	ReadOnly() : MtpFilesystemErrorWithErrorCode(EROFS, "read only") {}
};

class MtpDirectoryNotEmpty : public MtpFilesystemErrorWithErrorCode
{
public:
	MtpDirectoryNotEmpty() : MtpFilesystemErrorWithErrorCode(ENOTEMPTY, "Directoy not empty") {};
};

class NotADirectory : public MtpFilesystemError
{
public:
	NotADirectory() : MtpFilesystemError("Not a directory: ") {}
};

class MtpNameTooLong : public MtpFilesystemErrorWithErrorCode
{
public:
	MtpNameTooLong() : MtpFilesystemErrorWithErrorCode(ENAMETOOLONG, "Filename too long") {};
};
#endif /* MTPFILESYSTEMERRORS_H_ */
