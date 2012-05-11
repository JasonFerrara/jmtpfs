/*
 * ConnectedMtpDevices.h
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

#ifndef CONNECTEDMTPDEVICES_H_
#define CONNECTEDMTPDEVICES_H_
#include "MtpDevice.h"
#include <memory>

struct ConnectedDeviceInfo
{
	uint32_t	bus_location;
	uint8_t		devnum;
	uint32_t	device_flags;
	std::string	product;
	uint16_t	product_id;
	std::string	vendor;
	uint16_t	vendor_id;
};

class ConnectedMtpDevices
{
public:
	ConnectedMtpDevices();
	~ConnectedMtpDevices();

	int NumDevices();
	std::unique_ptr<MtpDevice> GetDevice(int index);
	std::unique_ptr<MtpDevice> GetDevice(uint32_t busLocation, uint8_t devnum);

	ConnectedDeviceInfo	GetDeviceInfo(int index);
	LIBMTP_raw_device_t& GetRawDeviceEntry(int index);

protected:
	static bool m_instantiated;
	LIBMTP_raw_device_t* m_devs;
	int m_numDevs;
};


#endif /* CONNECTEDMTPDEVICES_H_ */
