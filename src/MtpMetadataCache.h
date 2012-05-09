/*
 * MtpMetadataCache.h
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

#ifndef MTPMETADATACACHE_H_
#define MTPMETADATACACHE_H_

#include "MtpNodeMetadata.h"
#include "MtpLocalFileCopy.h"

#include <list>
#include <unordered_map>

class MtpMetadataCacheFiller
{
public:
	virtual ~MtpMetadataCacheFiller();

	virtual MtpNodeMetadata getMetadata()=0;
};

class MtpMetadataCache
{
public:
	MtpMetadataCache();
	~MtpMetadataCache();

	MtpNodeMetadata getItem(uint32_t id, MtpMetadataCacheFiller& source);
	void clearItem(uint32_t id);

	MtpLocalFileCopy* openFile(MtpDevice& device, uint32_t id);
	MtpLocalFileCopy* getOpenedFile(uint32_t id);

	uint32_t closeFile(uint32_t id);

private:
	void clearOld();
	struct CacheEntry
	{
		MtpNodeMetadata data;
		time_t			whenCreated;
	};

	typedef std::list<CacheEntry> cache_type;
	typedef std::unordered_map<uint32_t, cache_type::iterator> cache_lookup_type;
	typedef std::unordered_map<uint32_t, MtpLocalFileCopy*> local_file_cache_type;

	cache_type				m_cache;
	cache_lookup_type		m_cacheLookup;
	local_file_cache_type	m_localFileCache;

};


#endif /* MTPMETADATACACHE_H_ */
