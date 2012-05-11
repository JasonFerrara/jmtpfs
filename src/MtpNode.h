/*
 * MtpNode.h
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

#ifndef MTPNODE_H_
#define MTPNODE_H_

#include "MtpMetadataCache.h"
#include "MtpFilesystemPath.h"
#include "MtpDevice.h"
#include "FuseHeader.h"
#include <time.h>
#include <vector>
#include <string>
#include <memory>

class MtpNode : public MtpMetadataCacheFiller
{
public:
	MtpNode(MtpDevice& device, MtpMetadataCache& cache, uint32_t id);
	virtual ~MtpNode();

	virtual std::vector<std::string> readdir();

	virtual uint32_t Id();

	virtual std::unique_ptr<MtpNode> getNode(const FilesystemPath& path)=0;

	virtual std::vector<std::string> readDirectory();
	virtual void getattr(struct stat& info) = 0;

	virtual void Open();
	virtual void Close();
	virtual int Read(char *buf, size_t size, off_t offset);
	virtual int Write(const char* buf, size_t size, off_t offset);

	virtual void mkdir(const std::string& name);
	virtual void Remove();

	virtual void CreateFile(const std::string& name);

	virtual void Rename(MtpNode& newParent, const std::string& newName);

	virtual void Truncate(off_t length);

	virtual MtpStorageInfo GetStorageInfo();


	virtual uint32_t FolderId();
	virtual uint32_t StorageId();

	virtual std::unique_ptr<MtpNode> Clone();

	virtual void statfs(struct statvfs *stat);

protected:
	uint32_t GetParentNodeId();

	MtpDevice&					m_device;
	MtpMetadataCache&			m_cache;
	uint32_t					m_id;
};



#endif /* MTPNODE_H_ */
