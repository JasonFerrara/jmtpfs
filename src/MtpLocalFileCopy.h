/*
 * MtpLocalFileCopy.h
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

#ifndef MTPLOCALFILECOPY_H_
#define MTPLOCALFILECOPY_H_

#include "MtpDevice.h"

class MtpLocalFileCopy
{
public:
	MtpLocalFileCopy(MtpDevice& device, uint32_t id);
	~MtpLocalFileCopy();

	/*
	 * Close the local copy and write changes back to the remote if needed.
	 * The return value is the id for the remote file, which may have
	 * changed if we had to write back changes.
	 */
	uint32_t close();

	off_t getSize();

	void seek(long offset);
	size_t write(const void* ptr, size_t size);
	void truncate(off_t length);
	size_t read(void* ptr, size_t size);

	void CopyTo(MtpDevice& device, NewLIBMTPFile& destination);

private:
	MtpLocalFileCopy(const MtpLocalFileCopy&);
	MtpLocalFileCopy& operator=(const MtpLocalFileCopy&);

	MtpDevice&			m_device;
	FILE*				m_localFile;
	uint32_t			m_remoteId;
	bool				m_needWriteBack;
};


#endif /* MTPLOCALFILECOPY_H_ */
