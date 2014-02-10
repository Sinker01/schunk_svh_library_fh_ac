// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars pfotzer
 * \date    2014-01-30
 *
 */
//----------------------------------------------------------------------
#ifndef DRIVER_S5FH_S5FH_FINGER_MANAGER_H_INCLUDED
#define DRIVER_S5FH_S5FH_FINGER_MANAGER_H_INCLUDED

#include "driver_s5fh/ImportExport.h"
#include "driver_s5fh/S5FHController.h"

namespace driver_s5fh {

/*! This class manages controller parameters and the finger reset.
 */
class DRIVER_S5FH_IMPORT_EXPORT S5FHFingerManager
{
public:

  /*! Constructs a finger manager for the SCHUNK five finger hand.
   */
  S5FHFingerManager();

  ~S5FHFingerManager();

  enum {
    eS5FH_THUMB_FLEXION,
    eS5FH_THUMB_OPPOSITION, // wrist
    eS5FH_INDEX_FINGER_DISTAL,
    eS5FH_INDEX_FINGER_PROXIMAL,
    eS5FH_MIDDLE_FINGER_DISTAL,
    eS5FH_MIDDLE_FINGER_PROXIMAL,
    eS5FH_RING_FINGER,
    eS5FH_PINKY,
    eS5FH_FINGER_SPREAD
  } typedef S5FHDOF;


  //! reset function for a single finger
  bool resetFinger(const S5FHDOF &index);

  //! set target position of a single finger
  bool setTargetPosition(const S5FHDOF &index, double position, double current);

  //! overwrite current parameters
  bool setCurrentControllerParams(const S5FHDOF &index);

  //! overwrite position parameters
  bool setPositionControllerParams(const S5FHDOF &index);


private:

  //! pointer to s5fh controller
  S5FHController *m_controller;

};

}

#endif
