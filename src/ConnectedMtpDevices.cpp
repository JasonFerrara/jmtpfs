/*
 * ConnectedMtpDevices.cpp
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
#include "ConnectedMtpDevices.h"
#include "MtpLibLock.h"

bool ConnectedMtpDevices::m_instantiated = false;

ConnectedMtpDevices::ConnectedMtpDevices()
{
MtpLibLock	lock;

	if (m_instantiated)
		throw std::runtime_error("There can be only one!");
	m_instantiated  = true;
	LIBMTP_error_number_t err = LIBMTP_Detect_Raw_Devices(&m_devs, &m_numDevs);

	if (err == LIBMTP_ERROR_NO_DEVICE_ATTACHED)
	{
		m_numDevs = 0;
		m_devs = 0;
		return;
	}
	if (err != LIBMTP_ERROR_NONE)
	{
		throw MtpError("", err);
	}
}

int ConnectedMtpDevices::NumDevices()
{
	return m_numDevs;
}

std::unique_ptr<MtpDevice> ConnectedMtpDevices::GetDevice(int index)
{
MtpLibLock	lock;

	return std::unique_ptr<MtpDevice>(new MtpDevice(m_devs[index]));
}

std::unique_ptr<MtpDevice> ConnectedMtpDevices::GetDevice(uint32_t busLocation, uint8_t devnum)
{
MtpLibLock	lock;

	for(int i = 0; i<NumDevices(); i++)
	{
		ConnectedDeviceInfo devInfo = GetDeviceInfo(i);
		if ((devInfo.bus_location == busLocation) &&
			  (devInfo.devnum == devnum))
		{
			return GetDevice(i);;
		}
	}
	throw MtpDeviceNotFound("Requested MTP device not found");
}

LIBMTP_raw_device_t& ConnectedMtpDevices::GetRawDeviceEntry(int index)
{
	return m_devs[index];
}

ConnectedMtpDevices::~ConnectedMtpDevices()
{
MtpLibLock	lock;

	m_instantiated = false;
	if (m_devs)
		free(m_devs);
}

ConnectedDeviceInfo	ConnectedMtpDevices::GetDeviceInfo(int index)
{
	ConnectedDeviceInfo info;
	info.bus_location = m_devs[index].bus_location;
	info.devnum = m_devs[index].devnum;
	info.device_flags = m_devs[index].device_entry.device_flags;
	if (m_devs[index].device_entry.product)
		info.product = m_devs[index].device_entry.product;
	else
		info.product = "UNKNOWN";
	info.product_id = m_devs[index].device_entry.product_id;
	if (m_devs[index].device_entry.vendor)
		info.vendor = m_devs[index].device_entry.vendor;
	else
		info.vendor = "UNKNOWN";
	info.vendor_id = m_devs[index].device_entry.vendor_id;

	return info;
}


