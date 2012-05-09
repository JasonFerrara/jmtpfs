/*
 * MtpRoot.h
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

#ifndef MTPROOT_H_
#define MTPROOT_H_

#include "MtpNode.h"

class MtpRoot : public MtpNode
{
public:
	MtpRoot(MtpDevice& device, MtpMetadataCache& cache);

	std::unique_ptr<MtpNode> getNode(const FilesystemPath& path);
	void getattr(struct stat& info);

	std::vector<std::string> readDirectory();

	void mkdir(const std::string& name);
	void Remove();
	MtpNodeMetadata getMetadata();

	MtpStorageInfo GetStorageInfo();
};



#endif /* MTPROOT_H_ */
