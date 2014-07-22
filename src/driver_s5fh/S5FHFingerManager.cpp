// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars Pfotzer
 * \author  Georg Heppner
 * \date    2014-01-30
 *
 * This file contains the Finger Manager
 * that is managing the Schunk five finger hand on a high level.
 * The fingermanager is responsible to filter all calls and only make
 * allowed calls to the controller. The fingermanager is also responsible
 * for storing any kind of Konfiguration (like current controller settings).
 * The fingermanager is also responsible to poll the controller for continious data (if wanted)
 *
 *
 */
//----------------------------------------------------------------------
#include <driver_s5fh/Logging.h>
#include <driver_s5fh/S5FHFingerManager.h>

#include <icl_core/TimeStamp.h>

namespace driver_s5fh {

S5FHFingerManager::S5FHFingerManager(const bool &autostart, const std::vector<bool> &disable_mask, const std::string &dev_name) :
  m_controller(new S5FHController()),
  m_feedback_thread(NULL),
  m_connected(false),
  m_homing_timeout(10),
  m_home_settings(0),
  m_ticks2rad(0),
  m_position_min(std::vector<int32_t>(eS5FH_DIMENSION, 0)),
  m_position_max(std::vector<int32_t>(eS5FH_DIMENSION, 0)),
  m_position_home(std::vector<int32_t>(eS5FH_DIMENSION, 0)),
  m_is_homed(std::vector<bool>(eS5FH_DIMENSION, false)),
  m_is_switched_off(std::vector<bool>(eS5FH_DIMENSION,false)),
  m_movement_state(eST_DEACTIVATED)
{
  // load home position default parameters
  setHomePositionDefaultParameters();

  // set default reset order of all channels
  m_reset_order.resize(eS5FH_DIMENSION);
  m_reset_order[0] = eS5FH_INDEX_FINGER_PROXIMAL;
  m_reset_order[1] = eS5FH_MIDDLE_FINGER_PROXIMAL;
  m_reset_order[2] = eS5FH_THUMB_OPPOSITION;
  m_reset_order[3] = eS5FH_THUMB_FLEXION;
  m_reset_order[4] = eS5FH_FINGER_SPREAD;
  m_reset_order[5] = eS5FH_MIDDLE_FINGER_DISTAL;
  m_reset_order[6] = eS5FH_INDEX_FINGER_DISTAL;
  m_reset_order[7] = eS5FH_RING_FINGER;
  m_reset_order[8] = eS5FH_PINKY;

  // Order is determined by the channel enum
  m_reset_current_factor.resize(eS5FH_DIMENSION);
  m_reset_current_factor[eS5FH_THUMB_FLEXION]=          0.75;
  m_reset_current_factor[eS5FH_THUMB_OPPOSITION]=       0.75;
  m_reset_current_factor[eS5FH_INDEX_FINGER_DISTAL]=    0.75;
  m_reset_current_factor[eS5FH_INDEX_FINGER_PROXIMAL]=  0.75;
  m_reset_current_factor[eS5FH_MIDDLE_FINGER_DISTAL]=   0.75;
  m_reset_current_factor[eS5FH_MIDDLE_FINGER_PROXIMAL]= 0.75;
  m_reset_current_factor[eS5FH_RING_FINGER]=            0.75;
  m_reset_current_factor[eS5FH_PINKY]=                  0.75;
  m_reset_current_factor[eS5FH_FINGER_SPREAD]=          0.75;

  for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
  {
    m_is_switched_off[i] = disable_mask[i];
    if (m_is_switched_off[i])
    {
      LOGGING_INFO_C(DriverS5FH, S5FHFingerManager, "Joint: " << m_controller->m_channel_description[i] << " was disabled as per user request. It will not do anything!" << endl);
    }
  }


  #ifdef _IC_BUILDER_ICL_COMM_WEBSOCKET_
  m_ws_broadcaster = boost::shared_ptr<icl_comm::websocket::WsBroadcaster>(new icl_comm::websocket::WsBroadcaster(icl_comm::websocket::WsBroadcaster::eRT_S5FH,"/tmp/ws_broadcaster"));
  m_ws_broadcaster->robot->setInputToRadFactor(1);
  #endif

  // Try First Connect and Reset of all Fingers if autostart is enabled
  if (autostart && connect(dev_name))
  {
    resetChannel(eS5FH_ALL);
    LOGGING_INFO_C(DriverS5FH, S5FHFingerManager, "Driver Autostart succesfull! Input can now be sent. Have a safe and productive day" << endl);
  }

}

S5FHFingerManager::~S5FHFingerManager()
{
  disconnect();

  if (m_controller != NULL)
  {
    delete m_controller;
    m_controller = NULL;
  }
}

bool S5FHFingerManager::connect(const std::string &dev_name)
{
  if (m_connected)
  {
    disconnect();
  }

  if (m_controller != NULL)
  {
    if (m_controller->connect(dev_name))
    {
      // Reset the package counts (in case a previous attempt was made)
      m_controller->resetPackageCounts();

      // initialize feedback polling thread
      m_feedback_thread = new S5FHFeedbackPollingThread(icl_core::TimeSpan::createFromMSec(100), this);

      // load default position settings before the fingers are resetted
      std::vector<S5FHPositionSettings> default_position_settings
          = getPositionSettingsDefaultResetParameters();

      // load default current settings
      std::vector<S5FHCurrentSettings> default_current_settings
          = getCurrentSettingsDefaultParameters();

      m_controller->disableChannel(eS5FH_ALL);

      // initialize all channels
      for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
      {
        // request controller feedback
        m_controller->requestControllerFeedback(static_cast<S5FHCHANNEL>(i));

        // set position settings
        m_controller->setPositionSettings(static_cast<S5FHCHANNEL>(i), default_position_settings[i]);

        // set current settings
        m_controller->setCurrentSettings(static_cast<S5FHCHANNEL>(i), default_current_settings[i]);
      }

      // check for correct response from hardware controller
      icl_core::TimeStamp start_time = icl_core::TimeStamp::now();
      bool timeout = false;
      while (!timeout && !m_connected)
      {
        unsigned int send_count = m_controller->getSentPackageCount();
        unsigned int received_count = m_controller->getReceivedPackageCount();
        if (send_count == received_count)
        {
          m_connected = true;
          LOGGING_INFO_C(DriverS5FH, S5FHFingerManager, "Successfully established connection to SCHUNK five finger hand." << endl
                          << "Send packages = " << send_count << ", received packages = " << received_count << endl);

        }
        LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Try to connect to SCHUNK five finger hand: Send packages = " << send_count << ", received packages = " << received_count << endl);

        // check for timeout
        if ((icl_core::TimeStamp::now() - start_time).tsSec() > 5.0)  //TODO: Use parameter
        {
          timeout = true;
          LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Connection timeout! Could not connect to SCHUNK five finger hand." << endl
                          << "Send packages = " << send_count << ", received packages = " << received_count << endl);
        }

        icl_core::os::usleep(50000);
      }

      if (m_connected)
      {
        // start feedback polling thread
        if (m_feedback_thread != NULL)
        {
          m_feedback_thread->start();
        }
      }
    }
  }

  return m_connected;
}

void S5FHFingerManager::disconnect()
{
  m_connected = false;

  if (m_feedback_thread != NULL)
  {
    // wait until thread has stopped
    m_feedback_thread->stop();
    m_feedback_thread->join();

    delete m_feedback_thread;
    m_feedback_thread = NULL;
  }

  if (m_controller != NULL)
  {
    m_controller->disableChannel(eS5FH_ALL);
    m_controller->disconnect();
  }
}

//! reset function for a single finger
bool S5FHFingerManager::resetChannel(const S5FHCHANNEL &channel)
{
  if (m_connected)
  {
    // reset all channels
    if (channel == eS5FH_ALL)
    {
      setMovementState(eST_RESETTING);

      bool reset_all_success = true;
      for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
      {
        // try three times to reset each finger
        size_t max_reset_counter = 3;
        bool reset_success = false;
        while (!reset_success && max_reset_counter > 0)
        {
          S5FHCHANNEL channel = static_cast<S5FHCHANNEL>(m_reset_order[i]);
          reset_success = resetChannel(channel);
          max_reset_counter--;
        }

        LOGGING_INFO_C(DriverS5FH, resetChannel, "Channel " << m_reset_order[i] << " reset success = " << reset_success << endl);

        // set all reset flag
        reset_all_success = reset_all_success && reset_success;
      }

      // TODO: Set better movement state at this point

      return reset_all_success;
    }
    else if (channel > eS5FH_ALL && eS5FH_ALL < eS5FH_DIMENSION)
    {
      LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Start homing channel " << channel << endl);

      if (!m_is_switched_off[channel])
      {
        LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Setting reset position values for controller of channel " << channel << endl);
        m_controller->setPositionSettings(channel, getPositionSettingsDefaultResetParameters()[channel]);

        // DEBUG -> Use different Thumb current controller settings. This is just a quick workaround to use different PID Values and will be fixed soon
//        if (channel == eS5FH_THUMB_OPPOSITION)
//        {
//           S5FHCurrentSettings cur_set_thumb          = {-400.0f, 400.0f, 0.405f, 4e-6f, -400.0f, 400.0f, 0.850f, 85.0f, -500.0f, 500.0f};
//           m_controller->setCurrentSettings(channel,cur_set_thumb);
//        }
        // TODO: DEBUG REMOVE ME

        // reset homed flag
        m_is_homed[channel] = false;

        // read default home settings for channel
        HomeSettings home = m_home_settings[channel];

        S5FHPositionSettings pos_set;
        S5FHCurrentSettings cur_set;
        m_controller->getPositionSettings(channel, pos_set);
        m_controller->getCurrentSettings(channel, cur_set);

        // find home position
        m_controller->disableChannel(eS5FH_ALL);
        int32_t position = 0;

        if (home.direction > 0)
        {
          position = static_cast<int32_t>(pos_set.wmx);
        }
        else
        {
          position = static_cast<int32_t>(pos_set.wmn);
        }

        LOGGING_INFO_C(DriverS5FH, S5FHFingerManager, "Driving channel " << channel << " to hardstop. Detection thresholds: Current MIN: "<< m_reset_current_factor[channel] * cur_set.wmn << "mA MAX: "<< m_reset_current_factor[channel] * cur_set.wmx <<"mA" << endl);

        m_controller->setControllerTarget(channel, position);
        m_controller->enableChannel(channel);

        S5FHControllerFeedback control_feedback_previous;
        S5FHControllerFeedback control_feedback;

        // initialize timeout
        icl_core::TimeStamp start_time = icl_core::TimeStamp::now();

        for (size_t hit_count = 0; hit_count < 10; )
        {
          m_controller->setControllerTarget(channel, position);
          //m_controller->requestControllerFeedback(channel);
          m_controller->getControllerFeedback(channel, control_feedback);

          if ((m_reset_current_factor[channel] * cur_set.wmn >= control_feedback.current) || (control_feedback.current >= m_reset_current_factor[channel] * cur_set.wmx))
          {
            hit_count++;
          }
          else if (hit_count > 0)
          {
            hit_count--;
          }

          // check for time out: Abort, if position does not change after homing timeout.
          if ((icl_core::TimeStamp::now() - start_time).tsSec() > m_homing_timeout)
          {
            m_controller->disableChannel(eS5FH_ALL);
            LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Timeout: Aborted finding home position for channel " << channel << endl);
            return false;
          }

          // reset time of position changes
          if (control_feedback.position != control_feedback_previous.position)
          {
            start_time = icl_core::TimeStamp::now();
          }

          // save previous control feedback
          control_feedback_previous = control_feedback;
        }

        LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Hit counter of " << channel << " reached." << endl);

        m_controller->disableChannel(eS5FH_ALL);

        // set reference values
        m_position_min[channel] = static_cast<int32_t>(control_feedback.position + home.minimumOffset);
        m_position_max[channel] = static_cast<int32_t>(control_feedback.position + home.maximumOffset);
        m_position_home[channel] = static_cast<int32_t>(control_feedback.position + home.idlePosition);
        LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Channel " << channel << " min pos = " << m_position_min[channel]
                        << " max pos = " << m_position_max[channel] << " home pos = " << m_position_home[channel] << endl);

        position = static_cast<int32_t>(control_feedback.position + home.idlePosition);

        // go to idle position
        m_controller->enableChannel(channel);
        while (true)
        {
          m_controller->setControllerTarget(channel, position);
          //m_controller->requestControllerFeedback(channel);
          m_controller->getControllerFeedback(channel, control_feedback);

          if (abs(position - control_feedback.position) < 1000)
          {
            break;
          }
        }
        m_controller->disableChannel(eS5FH_ALL);

        LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Restoring default position values for controller of channel " << channel << endl);
        m_controller->setPositionSettings(channel, getPositionSettingsDefaultParameters()[channel]);

        // TODO DEBUG Quick fix for debugging PID parameters of the controllers this will be fixed soon
//        if (channel == eS5FH_THUMB_OPPOSITION)
//        {
//           m_controller->setCurrentSettings(channel,getCurrentSettingsDefaultParameters()[channel]);
//        }
        // TODO: DEBUG REMOVE ME
      }
      else
      {
         LOGGING_INFO_C(DriverS5FH, S5FHFingerManager, "Channel " << channel << "switched of by user, homing is set to finished" << endl);
      }


      m_is_homed[channel] = true;
      #ifdef _IC_BUILDER_ICL_COMM_WEBSOCKET_
        m_ws_broadcaster->robot->setJointHomed(true,channel);
        if (!m_ws_broadcaster->sendState());
        {
          //LOGGING_INFO_C(DriverS5FH, S5FHFingerManager, "Can't send ws_broadcaster state - reconnect pending..." << endl);
        }
      #endif // _IC_BUILDER_ICL_COMM_WEBSOCKET_

      LOGGING_INFO_C(DriverS5FH, S5FHFingerManager, "Successfully homed channel " << channel << endl);

      return true;
    }
    else
    {
      LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Channel " << channel << " is out side of bounds!" << endl);
      return false;
    }
  }
  else
  {
    LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Could not reset channel " << channel << ": No connection to SCHUNK five finger hand!" << endl);
    return false;
  }
}

//! enables controller of channel
bool S5FHFingerManager::enableChannel(const S5FHCHANNEL &channel)
{
  if (isConnected() && isHomed(channel))
  {
    if (channel == eS5FH_ALL)
    {
      for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
      {
        // Just for safety, enable chanels in the same order as we have resetted them (otherwise developers might geht confused)
        S5FHCHANNEL real_channel = static_cast<S5FHCHANNEL>(m_reset_order[i]);
        if (!m_is_switched_off[real_channel])
        {
          // recursion to get the other updates corresponing with activation of a channel
          enableChannel(real_channel);
        }
      }
    }
    else if (channel > eS5FH_ALL && eS5FH_ALL < eS5FH_DIMENSION)
    {
      // Note: This part is another one of thise places where the names can lead to confusion. I am sorry about that
      // Switched off is a logical term. The user has chosen NOT to use this channel because of hardware trouble.
      // To enable a smooth driver behaviour all replys regarding these channels will be answered in the most positive way
      // the caller could expect. Enabled refers to the actual enabled state of the hardware controller loops that drive the motors.
      // As the user has chosen not to use certain channels we explicitly do NOT enable these but tell a calling driver that we did
      if (!m_is_switched_off[channel])
      {
        m_controller->enableChannel(channel);
      }

    #ifdef _IC_BUILDER_ICL_COMM_WEBSOCKET_
      m_ws_broadcaster->robot->setJointEnabled(true,channel);
      if (!m_ws_broadcaster->sendState())
      {
        //LOGGING_INFO_C(DriverS5FH, S5FHFingerManager, "Can't send ws_broadcaster state - reconnect pending..." << endl);
      }
    #endif // _IC_BUILDER_ICL_COMM_WEBSOCKET_

      setMovementState(eST_PARTIALLY_ENABLED);
      if (isEnabled(eS5FH_ALL))
      {
        setMovementState(eST_ENABLED);
      }
    }
    return true;
  }
  return false;
}

void S5FHFingerManager::disableChannel(const S5FHCHANNEL &channel)
{
  if (channel == eS5FH_ALL)
  {
    for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
    {
      disableChannel(static_cast<S5FHCHANNEL>(i));
    }
  }
  else
  {
    if (!m_is_switched_off[channel])
    {
      m_controller->disableChannel(channel);
    }

  #ifdef _IC_BUILDER_ICL_COMM_WEBSOCKET_
      m_ws_broadcaster->robot->setJointEnabled(false,channel);
      if (!m_ws_broadcaster->sendState())
      {
        //LOGGING_INFO_C(DriverS5FH, S5FHFingerManager, "Can't send ws_broadcaster state - reconnect pending..." << endl);
      }
  #endif // _IC_BUILDER_ICL_COMM_WEBSOCKET_

    setMovementState(eST_PARTIALLY_ENABLED);

    bool all_disabled = true;
    for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
    {
      // Aggain only check channels that are not switched off. Switched off channels will always answer that they are enabled
      all_disabled = all_disabled && (m_is_switched_off[channel] ||!isEnabled(static_cast<S5FHCHANNEL>(i)));
    }
    if (all_disabled)
    {
      setMovementState(eST_DEACTIVATED);
    }

  }
}

bool S5FHFingerManager::requestControllerFeedbackAllChannels()
{
  if (isConnected())
  {
    m_controller->requestControllerFeedbackAllChannels();
    return true;
  }

  return false;
}

bool S5FHFingerManager::requestControllerFeedback(const S5FHCHANNEL &channel)
{
  if (isConnected() && isHomed(channel) && isEnabled(channel))
  {
    m_controller->requestControllerFeedback(channel);
    return true;
  }

  LOGGING_WARNING_C(DriverS5FH, S5FHFingerManager, "Channel " << channel << " is not homed or is not enabled!" << endl);
  return false;
}

//! returns actual position value for given channel
bool S5FHFingerManager::getPosition(const S5FHCHANNEL &channel, double &position)
{
  S5FHControllerFeedback controller_feedback;
  if ((channel >=0 && channel < eS5FH_DIMENSION) && isHomed(channel) && m_controller->getControllerFeedback(channel, controller_feedback))
  {
    // Switched off channels will always remain at zero position as the tics we get back migh be total gibberish
    if (m_is_switched_off[channel])
    {
      position = 0.0;
      return true;
    }

    int32_t cleared_position_ticks = controller_feedback.position;

    if (m_home_settings[channel].direction > 0)
    {
      cleared_position_ticks -= m_position_max[channel];
    }
    else
    {
      cleared_position_ticks -= m_position_min[channel];
    }

    position = static_cast<double>(cleared_position_ticks * m_ticks2rad[channel]);

     // DEBUG for PID Controllers -> this will be removed in future releases
//    if (channel == eS5FH_THUMB_OPPOSITION && !(controller_feedback == debug_feedback))
//    {
//      debug_feedback = controller_feedback;
//      std::cout << "Thumb Oppositon Feedback Ticks: " << controller_feedback.position << " cleared: " <<cleared_position_ticks << " Rad: " << position << "curr: " << controller_feedback.current << std::endl;
//      //m_controller->requestControllerState();
//    }

    // Safety overwrite: If controller drives to a negative position (should not happen but might in case the soft stops are placed badly)
    // we cannot get out because inputs smaller than 0 will be ignored
    if (position < 0)
    {
      position = 0.0;
    }

    LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Channel " << channel << ": position_ticks = " << controller_feedback.position
                    << " | cleared_position_ticks = " << cleared_position_ticks << " | position rad = " << position << endl);
    return true;
  }
  else
  {
    LOGGING_WARNING_C(DriverS5FH, S5FHFingerManager, "Could not get postion for channel " << channel << endl);
    return false;
  }
}



#ifdef _IC_BUILDER_ICL_COMM_WEBSOCKET_
void S5FHFingerManager::updateWebSocket()
{
  double position;
  //double current // will be implemented in future releases
  for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
  {
    // NOTE: Although the call to getPosition and current cann fail due to multiple reason, the only one we would encounter with these calls is a
    // non-homed finger. So it is quite safe to assume that the finger is NOT homed if these calls fail and we can safe multiple acces to the homed variable

    if (getPosition(static_cast<S5FHCHANNEL>(i),position)) // && (getCurrent(i,current))
    {
      m_ws_broadcaster->robot->setJointPosition(position,i);
      //m_ws_broadcaster>robot>setJpintCurrent(current,i); // will be implemented in future releases
    }
    else
    {
      m_ws_broadcaster->robot->setJointHomed(false,i);
    }

    if (!m_ws_broadcaster->sendState())
    {
      //LOGGING_INFO_C(DriverS5FH, S5FHFingerManager, "Can't send ws_broadcaster state - reconnect pending..." << endl);
    }
  }
}
#endif // _IC_BUILDER_ICL_COMM_WEBSOCKET_





//! returns actual current value for given channel
bool S5FHFingerManager::getCurrent(const S5FHCHANNEL &channel, double &current)
{
  S5FHControllerFeedback controller_feedback;
  if ((channel >=0 && channel < eS5FH_DIMENSION) && isHomed(channel) && m_controller->getControllerFeedback(channel, controller_feedback))
  {
    current = controller_feedback.current;
    return true;
  }
  else
  {
    LOGGING_WARNING_C(DriverS5FH, S5FHFingerManager, "Could not get current for channel " << channel << endl);
    return false;
  }
}

bool S5FHFingerManager::getCurrentControllerParams(const S5FHCHANNEL &channel, S5FHCurrentSettings &current_settings)
{
  if (channel >=0 && channel < eS5FH_DIMENSION)
  {
    return m_controller->getCurrentSettings(channel, current_settings);
  }
  else
  {
    LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Could not get current settings for unknown/unsupported channel " << channel << endl);
    return false;
  }
}

bool S5FHFingerManager::getPositionControllerParams(const S5FHCHANNEL &channel, S5FHPositionSettings &position_settings)
{
  if (channel >=0 && channel < eS5FH_DIMENSION)
  {
    return m_controller->getPositionSettings(channel, position_settings);
  }
  else
  {
    LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Could not get position settings for unknown/unsupported channel " << channel << endl);
    return false;
  }
}

//! set all target positions at once
bool S5FHFingerManager::setAllTargetPositions(const std::vector<double>& positions)
{
  if (isConnected())
  {
    // check size of position vector
    if (positions.size() == eS5FH_DIMENSION)
    {
      // create target positions vector
      std::vector<int32_t> target_positions(eS5FH_DIMENSION, 0);

      bool reject_command = false;
      for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
      {
        S5FHCHANNEL channel = static_cast<S5FHCHANNEL>(i);

        // enable all homed and disabled channels.. except its switched of
        if (!m_is_switched_off[channel] && isHomed(channel) && !isEnabled(channel))
        {
          enableChannel(channel);
        }

        // convert all channels to ticks
        target_positions[channel] = convertRad2Ticks(channel, positions[channel]);

        // check for out of bounds (except the switched off channels)
        if (!m_is_switched_off[channel] && !isInsideBounds(channel, target_positions[channel]))
        {
          reject_command = true;
        }
      }

      // send target position vector to controller and SCHUNK hand
      if (!reject_command)
      {
        m_controller->setControllerTargetAllChannels(target_positions);
        return true;
      }
      else
      {
        LOGGING_WARNING_C(DriverS5FH, S5FHFingerManager, "Could not set target position vector: At least one channel is out of bounds!" << endl);
        return false;
      }

    }
    else
    {
      LOGGING_WARNING_C(DriverS5FH, S5FHFingerManager, "Size of target position vector wrong: size = " << positions.size() << " expected size = " << (int)eS5FH_DIMENSION << endl);
      return false;
    }
  }
  else
  {
    LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Could not set target position vector: No connection to SCHUNK five finger hand!" << endl);
    return false;
  }
}

bool S5FHFingerManager::setTargetPosition(const S5FHCHANNEL &channel, double position, double current)
{
  if (isConnected())
  {
    if (channel >= 0 && channel < eS5FH_DIMENSION)
    {
      if (m_is_switched_off[channel])
      {
        // Switched off channels  behave transparent so we return a true value while we ignore the input
        LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Target position for channel " << channel << " was ignored as it is switched off by the user"<< endl);
        return true;
      }


      if (isHomed(channel))
      {
        int32_t target_position = convertRad2Ticks(channel, position);

        LOGGING_DEBUG_C(DriverS5FH, S5FHFingerManager, "Target position for channel " << channel << " = " << target_position << endl);

        // check for bounds
        if (isInsideBounds(channel, target_position))
        {
          if (!isEnabled(channel))
          {
            enableChannel(channel);
          }

          m_controller->setControllerTarget(channel, target_position);
          return true;
        }
        else
        {
          LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Target position for channel " << channel << " out of bounds!" << endl);
          return false;
        }
      }
      else
      {
        LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Could not set target position for channel " << channel << ": Reset first!" << endl);
        return false;
      }
    }
    else
    {
      LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Could not set target position for channel " << channel << ": Illegal Channel" << endl);
      return false;
    }
  }
  else
  {
    LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Could not set target position for channel " << channel << ": No connection to SCHUNK five finger hand!" << endl);
    return false;
  }
}

//! overwrite current parameters
bool S5FHFingerManager::setCurrentControllerParams(const S5FHCHANNEL &channel, const S5FHCurrentSettings &current_settings)
{
  if (channel >=0 && channel < eS5FH_DIMENSION)
  {
    m_controller->setCurrentSettings(channel, current_settings);
    return true;
  }
  else
  {
    return false;
  }
}

//! overwrite position parameters
bool S5FHFingerManager::setPositionControllerParams(const S5FHCHANNEL &channel, const S5FHPositionSettings &position_settings)
{
  if (channel >=0 && channel < eS5FH_DIMENSION)
  {
    m_controller->setPositionSettings(channel, position_settings);
    return true;
  }
  else
  {
    return true;
  }
}

//! return enable flag
bool S5FHFingerManager::isEnabled(const S5FHCHANNEL &channel)
{
  if (channel==eS5FH_ALL)
  {
    bool all_enabled = true;
    for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
    {
      all_enabled = all_enabled && m_controller->isEnabled(static_cast<S5FHCHANNEL>(i));
    }

    return all_enabled;
  }

  // Switched off Channels will aways be reported as enabled to simulate everything is fine. Others need to ask the controller
  // if the channel is realy switched on
  // Note: i can see that based on the names this might lead to a little confusion... sorry about that but there are only limited number of
  // words for not active ;) enabled refers to the actual state of the position and current controllers. So enabled
  // means enabled on a hardware level. Switched off is a logical decission in this case. The user has specified this
  // particular channel not to be used (due to hardware issues) and therefore the driver (aka the finger manager) will act
  // AS IF the channel was enabled but is in fact switched off by the user. If you have a better variable name or a better
  // idea how to handle that you are welcome to change it. (GH 2014-05-26)
  return (m_is_switched_off[channel] || m_controller->isEnabled(channel));
}

//! return homed flag
bool S5FHFingerManager::isHomed(const S5FHCHANNEL &channel)
{
  if (channel == eS5FH_ALL)
  {
    bool all_homed = true;
    for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
    {
      all_homed = all_homed && m_is_homed[i];
    }

    return all_homed;
  }
  // Channels that are switched off will always be reported as homed to simulate everything is fine. Others have to check
  return (m_is_switched_off[channel] || m_is_homed[channel]);
}

void S5FHFingerManager::setMovementState(const S5FHFingerManager::MovementState &state)
{
  m_movement_state = state;

  #ifdef _IC_BUILDER_ICL_COMM_WEBSOCKET_
    m_ws_broadcaster->robot->setMovementState(state);
    if (!m_ws_broadcaster->sendState())
    {
      //LOGGING_INFO_C(DriverS5FH, S5FHFingerManager, "Can't send ws_broadcaster state - reconnect pending..." << endl);
    }
  #endif // _IC_BUILDER_ICL_COMM_WEBSOCKET_


}

void S5FHFingerManager::setHomePositionDefaultParameters()
{
  m_home_settings.resize(eS5FH_DIMENSION);

  // homing parameters are important for software end stops

  // direction, minimum offset, maximum offset, idle position
  HomeSettings home_set_thumb_flexion   = {+1, -175.0e3f,  -5.0e3f, -15.0e3f};  // RE17, thumb flexion
  HomeSettings home_set_thumb_oppsition = {+1, -105.0e3f,  -5.0e3f, -15.0e3f};  // RE17, thumb opposition
  HomeSettings home_set_finger_distal   = {+1,  -47.0e3f,  -2.0e3f,  -8.0e3f};  // RE10, index finger distal joint
  HomeSettings home_set_finger_proximal = {-1,    2.0e3f,  42.0e3f,   8.0e3f};  // RE13, index finger proximal joint   --> Values based on the "limits" described by the hardware table
  //HomeSettings home_set_finger_proximal = {-1,    2.0e3f,  47.0e3f,   8.0e3f}; // Better Looking and more homogeneous maximum ;) but wrog compared to the actual hardware limits
  HomeSettings home_set_ring_finger     = home_set_finger_distal; //{+1,  -47.0e3f,  -2.0e3f,  -8.0e3f};  // RE10, ring finger
  HomeSettings home_set_pinky           = home_set_finger_distal; //{+1,  -47.0e3f,  -2.0e3f,  -8.0e3f};  // RE10, pinky
  HomeSettings home_set_finger_spread   = {+1,  -47.0e3f,  -2.0e3f,  -25.0e3f}; //{+1,  -25.0e3f,  -2.0e3f,  -15.0e3f};  //   // RE13, finger spread

  m_home_settings[0] = home_set_thumb_flexion;    // thumb flexion
  m_home_settings[1] = home_set_thumb_oppsition;  // thumb opposition
  m_home_settings[2] = home_set_finger_distal;    // index finger distal joint
  m_home_settings[3] = home_set_finger_proximal;  // index finger proximal joint
  m_home_settings[4] = home_set_finger_distal;    // middle finger distal joint
  m_home_settings[5] = home_set_finger_proximal;  // middle finger proximal joint
  m_home_settings[6] = home_set_ring_finger;      // ring finger
  m_home_settings[7] = home_set_pinky;            // pinky
  m_home_settings[8] = home_set_finger_spread;    // finger spread

  // calculate factors and offset for ticks to rad conversion
  float range_rad_data[eS5FH_DIMENSION] = { 0.97, 0.99, 1.33, 0.8, 1.33, 0.8, 0.98, 0.98, 0.58 };
  std::vector<float> range_rad(&range_rad_data[0], &range_rad_data[0] + eS5FH_DIMENSION);

  m_ticks2rad.resize(eS5FH_DIMENSION, 0.0);
  for (size_t i = 0; i < eS5FH_DIMENSION; ++i)
  {
    float range_ticks = m_home_settings[i].maximumOffset - m_home_settings[i].minimumOffset;
    m_ticks2rad[i] = range_rad[i] / range_ticks * (-m_home_settings[i].direction);
  }
}

std::vector<S5FHCurrentSettings> S5FHFingerManager::getCurrentSettingsDefaultParameters()
{
  // BEWARE! Only change these values if you know what you are doing !! Setting wrong values could damage the hardware!!!
  // TODO: This will be read from config files in later releases

  std::vector<S5FHCurrentSettings> default_current_settings(eS5FH_DIMENSION);
  // curr min, Curr max,ky(error output scaling),dt(time base),imn (integral windup min), imx (integral windup max), kp,ki,umn,umx (output limter)

  //S5FHCurrentSettings cur_set_thumb          = {-400.0f, 400.0f, 0.405f, 4e-6f, -400.0f, 400.0f, 0.850f, 85.0f, -500.0f, 500.0f}; // Backup values that are based on orginal MeCoVis Software
  S5FHCurrentSettings cur_set_thumb         = {-400.0f, 400.0f, 0.405f, 4e-6f, -500.0f, 500.0f, 0.6f, 0.4f, -400.0f, 400.0f}; // Much Smoother values that produce nice motions and are actually reasonable

  //S5FHCurrentSettings cur_set_thumb_opposition = {-400.0f, 400.0f, 0.405f, 4e-6f, -400.0f, 400.0f, 0.90f, 85.0f, -800.0f, 800.0f}; // Backup values that are based on orginal MeCoVis Software
  S5FHCurrentSettings cur_set_thumb_opposition = {-400.0f, 400.0f, 0.405f, 4e-6f, -500.0f, 500.0f, 0.6f, 0.4f, -400.0f, 400.0f}; // Much Smoother values that produce nice motions and are actually reasonable

  //S5FHCurrentSettings cur_set_distal_joint   = {-176.0f, 176.0f, 0.405f, 4e-6f, -300.0f, 300.0f, 0.850f, 85.0f, -254.0f, 254.0f};  // Backup values that are based on orginal MeCoVis Software
  S5FHCurrentSettings cur_set_distal_joint   = {-300.0f, 300.0f, 0.405f, 4e-6f, -300.0f, 300.0f, 0.85f, 2.0f, -300.0f, 300.0f}; // Much Smoother values that produce nice motions and are actually reasonable
  //S5FHCurrentSettings cur_set_proximal_joint = {-167.0f, 167.0f, 0.405f, 4e-6f, -300.0f, 300.0f, 0.850f, 85.0f, -254.0f, 254.0f};  // Backup values that are based on orginal MeCoVis Software
  S5FHCurrentSettings cur_set_proximal_joint = {-350.0f, 350.0f, 0.405f, 4e-6f, -300.0f, 300.0f, 0.85f, 2.0f, -350.0f, 350.0f}; // Much Smoother values that produce nice motions and are actually reasonable


  //S5FHCurrentSettings cur_set_finger_spread  = {-200.0f, 200.0f, 0.405f, 4e-6f, -300.0f, 300.0f, 0.850f, 85.0f, -254.0f, 254.0f}; // Very Safe values based on MeCoVis software
  S5FHCurrentSettings cur_set_finger_spread  = {-200.0f, 200.0f, 0.405f, 4e-6f, -300.0f, 300.0f, 0.850f, 85.0f, -400.0f, 400.0f}; // Somewhat better values based on the MeCoVis software
  //S5FHCurrentSettings cur_set_finger_spread  = {-200.0f, 200.0f, 0.405f, 4e-6f, -300.0f, 300.0f, 0.85f, 0.4f, -400.0f, 400.0f}; // Much Smoother values that produce nice motions -> not yet very well working with spread

  default_current_settings[0] = cur_set_thumb;              // thumb flexion
  default_current_settings[1] = cur_set_thumb_opposition;   // thumb opposition
  default_current_settings[2] = cur_set_distal_joint;       // index finger distal joint
  default_current_settings[3] = cur_set_proximal_joint;     // index finger proximal joint
  default_current_settings[4] = cur_set_distal_joint;       // middle finger distal joint
  default_current_settings[5] = cur_set_proximal_joint;     // middle finger proximal joint
  default_current_settings[6] = cur_set_distal_joint;       // ring finger
  default_current_settings[7] = cur_set_distal_joint;       // pinky
  default_current_settings[8] = cur_set_finger_spread;      // finger spread

  return default_current_settings;
}


std::vector<S5FHPositionSettings> S5FHFingerManager::getPositionSettingsDefaultResetParameters()
{
  std::vector<S5FHPositionSettings> default_position_settings(eS5FH_DIMENSION);

  //Wmin Wmax DWMax Ky Dt IMin Imax KP KI KD
  S5FHPositionSettings pos_set_thumb = {-1.0e6f, 1.0e6f,  10.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
  S5FHPositionSettings pos_set_finger = {-1.0e6f, 1.0e6f, 15.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
  S5FHPositionSettings pos_set_spread = {-1.0e6f, 1.0e6f, 17.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};


  default_position_settings[0] = pos_set_thumb;   // thumb flexion
  default_position_settings[1] = pos_set_thumb;   // thumb opposition
  default_position_settings[2] = pos_set_finger;  // index finger distal joint
  default_position_settings[3] = pos_set_finger;  // index finger proximal joint
  default_position_settings[4] = pos_set_finger;  // middle finger distal joint
  default_position_settings[5] = pos_set_finger;  // middle finger proximal joint
  default_position_settings[6] = pos_set_finger;  // ring finger
  default_position_settings[7] = pos_set_finger;  // pinky
  default_position_settings[8] = pos_set_spread;  // finger spread

  return default_position_settings;
}


//!
//! \brief returns default parameters for position settings
//!
std::vector<S5FHPositionSettings> S5FHFingerManager::getPositionSettingsDefaultParameters()
{
  std::vector<S5FHPositionSettings> default_position_settings(eS5FH_DIMENSION);

  // Note: there is a multitude of settings here to try out. These will be moved into config files in a later release.

  // Original conservative settings
//  S5FHPositionSettings pos_set_thumb = {-1.0e6f, 1.0e6f,  3.4e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_finger = {-1.0e6f, 1.0e6f,  8.5e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_spread = {-1.0e6f, 1.0e6f, 17.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};

    // All Fingers 0.5rad/sec except the fingers (1.5)
//  S5FHPositionSettings pos_set_thumb_flexion =          {-1.0e6f, 1.0e6f,  26.288e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_thumb_opposition =       {-1.0e6f, 1.0e6f,  15.151e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_finger_index_distal =    {-1.0e6f, 1.0e6f,  16.917e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_finger_index_proximal =  {-1.0e6f, 1.0e6f,  25.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_finger_middle_distal =   {-1.0e6f, 1.0e6f,  16.917e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_finger_middle_proximal = {-1.0e6f, 1.0e6f,  25.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_finger_ring =            {-1.0e6f, 1.0e6f,  22.959e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_finger_pinky =           {-1.0e6f, 1.0e6f,  22.959e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//  S5FHPositionSettings pos_set_spread =                 {-1.0e6f, 1.0e6f, 21.551e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};

  // All Fingers with a speed that will close the complete range of the finger in 1 Seconds    (except the thumb that wikll take 4)
    S5FHPositionSettings pos_set_thumb_flexion =          {-1.0e6f, 1.0e6f,  65.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
    //S5FHPositionSettings pos_set_thumb_opposition =       {-1.0e6f, 1.0e6f,  50.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f}; // original Value for these settings, but the slightly modified one looks better
    S5FHPositionSettings pos_set_thumb_opposition =       {-1.0e6f, 1.0e6f,  50.0e3f, 1.00f, 1e-3f, -4000.0f, 4000.0f, 0.05f, 0.1f, 0.0f};
    S5FHPositionSettings pos_set_finger_index_distal =    {-1.0e6f, 1.0e6f,  45.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
    //S5FHPositionSettings pos_set_finger_index_proximal =  {-1.0e6f, 1.0e6f,  40.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f}; // original Value for these settings, but the slightly modified one looks better
    S5FHPositionSettings pos_set_finger_index_proximal =  {-1.0e6f, 1.0e6f,  40.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.3f, 0.05f, 0.0f};
    S5FHPositionSettings pos_set_finger_middle_distal =   {-1.0e6f, 1.0e6f,  45.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
    //S5FHPositionSettings pos_set_finger_middle_proximal = {-1.0e6f, 1.0e6f,  40.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f}; // original Value for these settings, but the slightly modified one looks better
    S5FHPositionSettings pos_set_finger_middle_proximal = {-1.0e6f, 1.0e6f,  40.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.3f, 0.05f, 0.0f};
    S5FHPositionSettings pos_set_finger_ring =            {-1.0e6f, 1.0e6f,  45.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
    S5FHPositionSettings pos_set_finger_pinky =           {-1.0e6f, 1.0e6f,  45.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
    S5FHPositionSettings pos_set_spread =                 {-1.0e6f, 1.0e6f,  25.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};

    // All Fingers with a speed that will close the complete range of the finger in 0.5 Seconds (except the thumb that will take 4)
//    S5FHPositionSettings pos_set_thumb_flexion =          {-1.0e6f, 1.0e6f,  42.5e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_thumb_opposition =       {-1.0e6f, 1.0e6f,  25.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_finger_index_distal =    {-1.0e6f, 1.0e6f,  90.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_finger_index_proximal =  {-1.0e6f, 1.0e6f,  80.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_finger_middle_distal =   {-1.0e6f, 1.0e6f,  90.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_finger_middle_proximal = {-1.0e6f, 1.0e6f,  80.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_finger_ring =            {-1.0e6f, 1.0e6f,  90.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_finger_pinky =           {-1.0e6f, 1.0e6f,  90.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};
//    S5FHPositionSettings pos_set_spread =                 {-1.0e6f, 1.0e6f,  50.0e3f, 1.00f, 1e-3f, -500.0f, 500.0f, 0.5f, 0.05f, 0.0f};


  // Do not change this order!
  default_position_settings[0] = pos_set_thumb_flexion;   // thumb flexion
  default_position_settings[1] = pos_set_thumb_opposition;   // thumb opposition
  default_position_settings[2] = pos_set_finger_index_distal;  // index finger distal joint
  default_position_settings[3] = pos_set_finger_index_proximal;  // index finger proximal joint
  default_position_settings[4] = pos_set_finger_middle_distal;  // middle finger distal joint
  default_position_settings[5] = pos_set_finger_middle_proximal;  // middle finger proximal joint
  default_position_settings[6] = pos_set_finger_ring;  // ring finger
  default_position_settings[7] = pos_set_finger_pinky;  // pinky
  default_position_settings[8] = pos_set_spread;  // finger spread

  return default_position_settings;
}

// Converts joint positions of a specific channel from RAD to ticks
int32_t S5FHFingerManager::convertRad2Ticks(const S5FHCHANNEL &channel, double position)
{
  int32_t target_position = static_cast<int32_t>(position / m_ticks2rad[channel]);

  if (m_home_settings[channel].direction > 0)
  {
    target_position += m_position_max[channel];
  }
  else
  {
    target_position += m_position_min[channel];
  }

  return target_position;
}

// Check bounds of target positions
bool S5FHFingerManager::isInsideBounds(const S5FHCHANNEL &channel, const int32_t &target_position)
{
  // Switched off channels will always be reported as inside bounds
  return (m_is_switched_off[channel] || ((target_position >= m_position_min[channel]) && (target_position <= m_position_max[channel])));
}

void S5FHFingerManager::requestControllerState()
{
  m_controller->requestControllerState();
}

bool S5FHFingerManager::readParametersFromConfigFile()
{
  // THIS FUNCTIONALITY IS UNTESTED and will be included in a later release

//  bool read_successful = false;

//  // load position settings from config file
//  std::vector<S5FHPositionSettings> position_config_list;
//  read_successful =
//    icc::get(CONFIG_VALUES(
//               CONFIG_LIST(
//                 S5FHPositionSettings, "/S5FH/PositionSettings",
//                 MEMBER_MAPPING(
//                   S5FHPositionSettings,
//                   MEMBER_VALUE_1("WMin", S5FHPositionSettings, wmn)
//                   MEMBER_VALUE_1("WMax", S5FHPositionSettings, wmx)
//                   MEMBER_VALUE_1("DWMax", S5FHPositionSettings, dwmx)
//                   MEMBER_VALUE_1("KY", S5FHPositionSettings, ky)
//                   MEMBER_VALUE_1("DT", S5FHPositionSettings, dt)
//                   MEMBER_VALUE_1("IMin", S5FHPositionSettings, imn)
//                   MEMBER_VALUE_1("IMax", S5FHPositionSettings, imx)
//                   MEMBER_VALUE_1("KP", S5FHPositionSettings, kp)
//                   MEMBER_VALUE_1("KI", S5FHPositionSettings, ki)
//                   MEMBER_VALUE_1("KD", S5FHPositionSettings, kd)),
//                 std::back_inserter(position_config_list))),
//             DriverS5FH::instance());

//  // set controller position settings
//  if (read_successful)
//  {
//    for (size_t i = 0; i < position_config_list.size(); i++)
//    {
//      m_controller->setPositionSettings(i, position_config_list[i]);

//      LOGGING_ERROR_C(DriverS5FH, S5FHController, "new position settings recieved: " << endl <<
//                      "WMin = " << position_config_list[i].wmn << endl);
//    }
//  }
//  else
//  {
//    LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Could not load position settings from config file" << endl);
//  }

//  // load current settings from config file
//  std::vector<S5FHCurrentSettings> current_config_list;
//  read_successful =
//    icc::get(CONFIG_VALUES(
//               CONFIG_LIST(
//                 S5FHCurrentSettings, "/S5FH/CurrentSettings",
//                 MEMBER_MAPPING(
//                   S5FHCurrentSettings,
//                   MEMBER_VALUE_1("WMin", S5FHCurrentSettings, wmn)
//                   MEMBER_VALUE_1("WMax", S5FHCurrentSettings, wmx)
//                   MEMBER_VALUE_1("KY", S5FHCurrentSettings, ky)
//                   MEMBER_VALUE_1("DT", S5FHCurrentSettings, dt)
//                   MEMBER_VALUE_1("IMin", S5FHCurrentSettings, imn)
//                   MEMBER_VALUE_1("IMax", S5FHCurrentSettings, imx)
//                   MEMBER_VALUE_1("KP", S5FHCurrentSettings, kp)
//                   MEMBER_VALUE_1("KI", S5FHCurrentSettings, ki)
//                   MEMBER_VALUE_1("UMin", S5FHCurrentSettings, umn)
//                   MEMBER_VALUE_1("UMax", S5FHCurrentSettings, umx)),
//                 std::back_inserter(current_config_list))),
//             icl_core::logging::Nirwana::instance());

//  // set current position settings
//  if (read_successful)
//  {
//    for (size_t i = 0; i < current_config_list.size(); i++)
//    {
//      m_controller->setCurrentSettings(i, current_config_list[i]);
//    }
//  }
//  else
//  {
//    LOGGING_ERROR_C(DriverS5FH, S5FHFingerManager, "Could not load current settings from config file" << endl);
//  }
  return true;
}

}
