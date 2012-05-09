/*
 * MtpMetadataCache.cpp
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
#include "MtpMetadataCache.h"

#include <time.h>
#include <assert.h>

MtpMetadataCacheFiller::~MtpMetadataCacheFiller()
{

}

MtpMetadataCache::MtpMetadataCache()
{

}
MtpMetadataCache::~MtpMetadataCache()
{
	for(local_file_cache_type::iterator i = m_localFileCache.begin(); i != m_localFileCache.end(); i++)
	{
		delete i->second;
	}
}



MtpNodeMetadata MtpMetadataCache::getItem(uint32_t id, MtpMetadataCacheFiller& source)
{

	clearOld();
	cache_lookup_type::iterator i = m_cacheLookup.find(id);
	if (i != m_cacheLookup.end())
		return i->second->data;
	CacheEntry newData;
	newData.data = source.getMetadata();
	assert(newData.data.self.id == id);
	newData.whenCreated = time(0);
	m_cacheLookup[id] = m_cache.insert(m_cache.end(), newData);
	return newData.data;
}

void MtpMetadataCache::clearItem(uint32_t id)
{

	cache_lookup_type::iterator i = m_cacheLookup.find(id);
	if (i != m_cacheLookup.end())
	{
		m_cache.erase(i->second);
		m_cacheLookup.erase(i);
	}
}

void MtpMetadataCache::clearOld()
{

	time_t now = time(0);
	for(cache_type::iterator i = m_cache.begin(); i != m_cache.end();)
	{
		if ((now - i->whenCreated) > 5)
		{
			m_cacheLookup.erase(m_cacheLookup.find(i->data.self.id));
			i = m_cache.erase(i);
		}
		else
			return;
	}
}

MtpLocalFileCopy* MtpMetadataCache::openFile(MtpDevice& device, uint32_t id)
{
	local_file_cache_type::iterator i = m_localFileCache.find(id);
	if (i != m_localFileCache.end())
		return i->second;
	MtpLocalFileCopy* newFile = new MtpLocalFileCopy(device, id);
	m_localFileCache[id] = newFile;
	return newFile;
}

MtpLocalFileCopy* MtpMetadataCache::getOpenedFile(uint32_t id)
{
	local_file_cache_type::iterator i = m_localFileCache.find(id);
	if (i != m_localFileCache.end())
		return i->second;
	else
		return 0;
}

uint32_t MtpMetadataCache::closeFile(uint32_t id)
{
	local_file_cache_type::iterator i = m_localFileCache.find(id);
	if (i != m_localFileCache.end())
	{
		uint32_t newId = i->second->close();
		delete i->second;
		m_localFileCache.erase(i);
		return newId;
	}
	return id;
}
