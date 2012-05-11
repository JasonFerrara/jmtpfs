/*
 * MtpFuseContext.h
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

#ifndef MTPFUSECONTEXT_H_
#define MTPFUSECONTEXT_H_

#include "MtpDevice.h"
#include "MtpMetadataCache.h"
#include "MtpNode.h"
#include <memory>
#include <sys/types.h>

class MtpFuseContext
{
public:
	MtpFuseContext(std::unique_ptr<MtpDevice> device,  uid_t uid, gid_t gid);

	std::unique_ptr<MtpNode> getNode(const FilesystemPath& path);

	uid_t uid() const;
	gid_t gid() const;

protected:
	uid_t						m_uid;
	gid_t						m_gid;
	std::unique_ptr<MtpDevice>	m_device;
	MtpMetadataCache 		  	m_cache;
};


#endif /* MTPFUSECONTEXT_H_ */
