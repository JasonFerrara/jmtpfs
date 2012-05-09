/*
 * MtpFile.h
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

#ifndef MTPFILE_H_
#define MTPFILE_H_

#include "MtpNode.h"
#include <stdio.h>

class MtpFile : public MtpNode
{
public:
	MtpFile(MtpDevice& device,  MtpMetadataCache& cache, uint32_t id);
	~MtpFile();

	std::unique_ptr<MtpNode> getNode(const FilesystemPath& path);
	void getattr(struct stat& info);

	void Open();
	void Close();
	int Read(char *buf, size_t size, off_t offset);
	int Write(const char* buf, size_t size, off_t offset);
	void Remove();

	void Fsync();
	void Truncate(off_t length);
	void Rename(MtpNode& newParent, const std::string& newName);

	MtpNodeMetadata getMetadata();

protected:
	MtpFileInfo	m_info;
	bool		m_opened;
	FILE*		m_localFile;
	bool		m_needWrite;
};



#endif /* MTPFILE_H_ */
