/*
 * MtpFuseContext.cpp
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
#include "MtpFuseContext.h"
#include "mtpFilesystemErrors.h"
#include "MtpRoot.h"

MtpFuseContext::MtpFuseContext(std::unique_ptr<MtpDevice> device,  uid_t uid, gid_t gid) :
	m_device(std::move(device)), m_uid(uid), m_gid(gid)
{

}

std::unique_ptr<MtpNode> MtpFuseContext::getNode(const FilesystemPath& path)
{
	std::unique_ptr<MtpNode> root(new MtpRoot(*m_device, m_cache));
	if (path.Head()!="/")
		throw FileNotFound(path.str());
	if (path.str()=="/")
		return root;
	else
		return root->getNode(path.Body());
}

uid_t MtpFuseContext::uid() const
{
	return m_uid;
}

gid_t MtpFuseContext::gid() const
{
	return m_gid;
}
