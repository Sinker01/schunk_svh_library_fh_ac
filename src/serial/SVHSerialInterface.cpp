// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// This file is part of the SCHUNK SVH Driver suite.
//
// This program is free software licensed under the LGPL
// (GNU LESSER GENERAL PUBLIC LICENSE Version 3).
// You can find a copy of this license in LICENSE folder in the top
// directory of the source code.
//
// © Copyright 2014 SCHUNK Mobile Greifsysteme GmbH, Lauffen/Neckar Germany
// © Copyright 2014 FZI Forschungszentrum Informatik, Karlsruhe, Germany
//
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars Pfotzer
 * \date    2014-01-30
 * \date    2014-07-16
 *
 * This file contains the SVHSerialInterface class that is used to
 * handle the protocoll overhead of the serial communication.
 * It uses a serial device that opens the physical connection and
 * is responsible to manage this hardware resource as well as protocoll issues
 * like sync bytes, checksum calculation and counting of packets send and received.
 */
//----------------------------------------------------------------------
#include "schunk_svh_library/serial/SVHSerialInterface.h"
#include "schunk_svh_library/Logger.h"

#include <memory>
#include <schunk_svh_library/serial/ByteOrderConversion.h>
#include <boost/bind/bind.hpp>
#include <thread>
#include <chrono>


using driver_svh::serial::SerialFlags;
#if BOOST_VERSION >= 106000 // Moved to namespace in boost 1.60
using namespace boost::placeholders;
#endif

namespace driver_svh {

SVHSerialInterface::SVHSerialInterface(ReceivedPacketCallback const & received_packet_callback) :
  m_connected(false),
  m_received_packet_callback(received_packet_callback),
  m_packets_transmitted(0)
{
}

SVHSerialInterface::~SVHSerialInterface()
{
  //close();
}

bool SVHSerialInterface::connect(const std::string &dev_name)
{
  // close device if already opened
  close();

  // create serial device
  m_serial_device.reset(new Serial(dev_name.c_str(), SerialFlags(SerialFlags::eBR_921600, SerialFlags::eDB_8)));

  if (m_serial_device)
  {
    // open serial device
    if (!m_serial_device->Open())
    {
      SVH_LOG_ERROR_STREAM("SVHSerialInterface", "Could not open serial device: " << dev_name.c_str());
      return false;
    }
  }
  else
  {
    SVH_LOG_ERROR_STREAM("SVHSerialInterface", "Could not create serial device handle: " << dev_name.c_str());
    return false;
  }

  m_svh_receiver = std::make_unique<SVHReceiveThread>(
      std::chrono::microseconds(500),
      m_serial_device,
      boost::bind(&SVHSerialInterface::receivedPacketCallback,this,_1,_2)
  );

  // create receive thread
  m_receive_thread = std::thread([this]{m_svh_receiver->run(); });

  m_connected = true;
  SVH_LOG_DEBUG_STREAM("SVHSerialInterface", "Serial device  " << dev_name.c_str()  << " opened and receive thread started. Communication can now begin.");

  return true;
}

void SVHSerialInterface::close()
{
  m_connected = false;

  // cancel and delete receive packet thread
  if (m_svh_receiver)
  {
    m_svh_receiver->stop();
  }
  if (m_receive_thread.joinable())
  {
    m_receive_thread.join();
    SVH_LOG_DEBUG_STREAM("SVHSerialInterface", "Serial device receive thread was terminated.");
  }

  // close and delete serial device handler
  if (m_serial_device)
  {
    m_serial_device->Close();

    m_serial_device.reset();
    SVH_LOG_DEBUG_STREAM("SVHSerialInterface", "Serial device handle was closed and terminated.");
  }
}

bool SVHSerialInterface::sendPacket(SVHSerialPacket& packet)
{
  if (m_serial_device != NULL)
  {
    // For alignment: Always 64Byte data, padded with zeros
    packet.data.resize(64,0);
  
    uint8_t check_sum1 = 0;
    uint8_t check_sum2 = 0;

    // Calculate Checksum for the packet
    for (size_t i = 0; i < packet.data.size(); i++)
    {
      check_sum1 += packet.data[i];
      check_sum2 ^= packet.data[i];
    }

    // set packet counter
    packet.index = static_cast<uint8_t>(m_packets_transmitted % uint8_t(-1));

    if (m_serial_device->IsOpen())
    {
      // Prepare arraybuilder
      size_t size = packet.data.size() + cPACKET_APPENDIX_SIZE;
      driver_svh::ArrayBuilder send_array(size);
      // Write header and packet information and checksum
      send_array << PACKET_HEADER1 << PACKET_HEADER2 << packet << check_sum1 << check_sum2;

      // actual hardware call to send the packet
      size_t bytes_send = 0;
      while (bytes_send < size)
      {
        bytes_send += m_serial_device->Write(send_array.array.data() + bytes_send, size - bytes_send);
      }

      // Small delay -> THIS SHOULD NOT BE NECESSARY as the communication speed should be handable by the HW. However, it will die if this sleep is
      // not used and this may also depend on your computer speed -> This issue might stem also from the hardware and will hopefully be fixed soon.
      // 782µs are needed to send 72bytes via a baudrate of 921600
      std::this_thread::sleep_for(std::chrono::microseconds(782));
      // Instead you could wait for the response of the packet (or on of the previous n packets). This slows down the speed to the 2-way latency, which is platform dependent

    }
    else
    {
      SVH_LOG_DEBUG_STREAM("SVHSerialInterface", "sendPacket failed, serial device was not properly initialized.");
      return false;
    }

    m_packets_transmitted++;
  }

  return true;
}

void SVHSerialInterface::resetTransmitPackageCount()
{
  m_packets_transmitted = 0;
  // Only the receive thread knows abotu the accurate number it has received
  m_svh_receiver->resetReceivedPackageCount();
}

void SVHSerialInterface::printPacketOnConsole(SVHSerialPacket &packet)
{

  uint8_t check_sum1 = 0;
  uint8_t check_sum2 = 0;

  // Calculate Checksum for the packet
  for (size_t i = 0; i < packet.data.size(); i++)
  {
    check_sum1 += packet.data[i];
    check_sum2 ^= packet.data[i];
  }

  // set packet counter
  packet.index = static_cast<uint8_t>(m_dummy_packets_printed % uint8_t(-1));


  // Prepare arraybuilder
  size_t size = packet.data.size() + cPACKET_APPENDIX_SIZE;
  driver_svh::ArrayBuilder send_array(size);
  // Write header and packet information and checksum
  send_array << PACKET_HEADER1 << PACKET_HEADER2 << packet << check_sum1 << check_sum2;

  std::cout << send_array << std::endl;

  m_dummy_packets_printed++;
}

void SVHSerialInterface::receivedPacketCallback(const SVHSerialPacket &packet, unsigned int packet_count)
{
  last_index=packet.index;
  m_received_packet_callback(packet,packet_count);
}

}
