/*
 * MtpFolder.h
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

#ifndef MTPFOLDER_H_
#define MTPFOLDER_H_

#include "MtpNode.h"

class MtpFolder : public MtpNode
{
public:
	MtpFolder(MtpDevice& device, MtpMetadataCache& cache, uint32_t storageId, uint32_t folderId);

	std::unique_ptr<MtpNode> getNode(const FilesystemPath& path);
	void getattr(struct stat& info);

	std::vector<std::string> readDirectory();
	void Remove();

	void mkdir(const std::string& name);
	void CreateFile(const std::string& name);

	uint32_t FolderId();
	uint32_t StorageId();

	void Rename(MtpNode& newParent, const std::string& newName);

	MtpNodeMetadata getMetadata();

protected:


	std::vector<MtpFileInfo> m_files;
	uint32_t m_storageId, m_folderId;
};


#endif /* MTPFOLDER_H_ */
