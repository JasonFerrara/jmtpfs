/*
 * MtpFilesystemPath.cpp
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



#include "MtpFilesystemPath.h"
#include "mtpFilesystemErrors.h"

FilesystemPath::FilesystemPath(const char* path) : m_path(path)
{

}

std::string FilesystemPath::Head() const
{
	std::string result;
	size_t p = m_path.find("/");
	if (p==0)
		return "/";
	if (p==std::string::npos)
		result =  m_path;
	else
		result = m_path.substr(0,p);
	if (result.size() > MAX_MTP_NAME_LENGTH)
		throw MtpNameTooLong();
	return result;
}

std::string FilesystemPath::str() const
{
	return m_path;
}

FilesystemPath	FilesystemPath::Body() const
{
	size_t p = m_path.find("/");
	if (p==std::string::npos)
		return FilesystemPath("");
	return FilesystemPath(m_path.substr(p+1).c_str());
}

bool FilesystemPath::Empty() const
{
	return m_path.length() == 0;
}

FilesystemPath FilesystemPath::AllButTail() const
{
	size_t p = m_path.rfind("/");
	if (p == std::string::npos)
		return FilesystemPath("");
	if (p==0)
		return FilesystemPath("/");
	return FilesystemPath(m_path.substr(0,p).c_str());
}

std::string FilesystemPath::Tail() const
{
	std::string result;
	size_t p = m_path.rfind("/");
	if (p == std::string::npos)
		result = m_path;
	else
		result = m_path.substr(p+1,std::string::npos);
	return result;
}

