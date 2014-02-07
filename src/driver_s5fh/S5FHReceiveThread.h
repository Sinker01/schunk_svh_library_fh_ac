// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars Pfotzer
 * \date    2014-01-30
 *
 */
//----------------------------------------------------------------------
#ifndef DRIVER_S5FH_S5FH_RECEIVE_THREAD_H_INCLUDED
#define DRIVER_S5FH_S5FH_RECEIVE_THREAD_H_INCLUDED

#include <icl_core/TimeSpan.h>
#include <icl_core_thread/PeriodicThread.h>
#include <icl_comm_serial/Serial.h>

#include <driver_s5fh/Logging.h>
#include <driver_s5fh/S5FHSerialPacket.h>

using icl_core::TimeSpan;
using icl_core::thread::PeriodicThread;
using icl_comm::serial::Serial;

namespace driver_s5fh {

class S5FHSerialInterface;

/*! Thread for periodically receiving messages from the serial device.
 */
class S5FHReceiveThread : public PeriodicThread
{
public:
  S5FHReceiveThread(const TimeSpan& period, S5FHSerialInterface* interface, Serial* device);

  virtual ~S5FHReceiveThread();

  //! run method of the thread
  virtual void run();

  //! received packets count getter
  unsigned int receivedPacketCount() { return m_packets_received; }

private:

  //! pointer to serial interface object
  S5FHSerialInterface* m_serial_interface;

  //! pointer to serial device object
  Serial* m_serial_device;

  //! enum for receive packet state machine
  enum
  {
    eRS_HEADER1,
    eRS_HEADER2,
    eRS_INDEX,
    eRS_ADDRESS,
    eRS_LENGTH,
    eRS_DATA,
    eRS_CHECKSUM,
    eRS_COMPLETE
  } typedef tState;

  //! current state of the state machine
  tState m_received_state;

  //! received packet created from serial data in the state machine
  S5FHSerialPacket* m_received_packet;

  //! packets counter
  unsigned int m_packets_received;

  //! state machine processing received data
  bool receiveData();

};

}

#endif
