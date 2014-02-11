// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Georg Heppner
 * \date    2014-02-10
 *
 */
//----------------------------------------------------------------------
#include <driver_s5fh/S5FHSerialPacket.h>
//#include <sstream>

namespace driver_s5fh {

//! overload stream operator to easily serialize data
icl_comm::ArrayBuilder& operator << (icl_comm::ArrayBuilder& ab, const S5FHSerialPacket& data)
{
  ab << data.index << data.address << static_cast<u_int16_t>(data.data.size()) << data.data;
  return ab;
}

//! overload stream operator to easily Deserialize data
icl_comm::ArrayBuilder& operator >> (icl_comm::ArrayBuilder& ab, S5FHSerialPacket& data)
{
  // Disregard the size when deserializing as we get that anyway
  u_int16_t size ;
  ab >> data.data >> size >> data.address  >> data.index;
  return ab;
}

//! Output Stream operator
std::ostream& operator << (std::ostream& o, const S5FHSerialPacket& sp)
{
  o << "index: " << static_cast<int>(sp.index) << " address: " << "0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(sp.address) << " Data: ";
  for (size_t i = 0; i < sp.data.size(); i++)
  {
    o << "0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(sp.data[i]) << " ";
  }
  // Reset Output stream to decimal output .. otherwise it may confuse people
  std::cout << std::dec ;
  return o;
}

////! overload logging stream operator
//icl_core::logging::ThreadStream& operator << (icl_core::logging::ThreadStream& stream, const S5FHSerialPacket& value)
//{
//  //.... well.. its out own fault... now we have to use this -> convert to stringstream witht the already overloaded ostream operator then convert the resulting string...
//  std::stringstream interpreter;
//  interpreter << value;
//  std::string test;
//  interpreter >> test;
//  stream << test;
//  return stream;
//}

}

