/*
 * MtpStorage.cpp
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
#include "MtpStorage.h"
#include "mtpFilesystemErrors.h"
#include <iostream>

MtpStorage::MtpStorage(MtpDevice& device, MtpMetadataCache& cache, uint32_t id) : MtpFolder(device, cache, id, 0)
{

}

std::unique_ptr<MtpNode> MtpStorage::Clone()
{
	return std::unique_ptr<MtpNode>(new MtpStorage(m_device, m_cache, m_id));
}



void MtpStorage::Remove()
{
	throw ReadOnly();
}

void MtpStorage::Rename(MtpNode& newParent, const std::string& newName)
{
	throw ReadOnly();
}
