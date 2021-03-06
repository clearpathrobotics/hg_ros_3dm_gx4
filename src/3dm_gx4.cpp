/*
 * 3dm_gx4.cpp
 *
 *  Created on: Nov 17, 2014
 *      Author: mahisorn
 */
#include <hg_ros_3dm_gx4/3dm_gx4.h>
#include <boost/asio.hpp>
#include <boost/chrono.hpp>

using namespace hg_3dm_gx4;

const float Hg3dmGx4::GAUSS_TO_TESLA = 0.0001;
const float Hg3dmGx4::G_TO_ACCELERATION = 9.81;

void Hg3dmGx4::ping()
{
  MIP p(CMD_CLASS_BASE);
  p.beginField(CMD_PING);
  p.endField();
  p.updateChecksum();
  sendAndReceivePacket(p);
}

void Hg3dmGx4::idle()
{
  MIP p(CMD_CLASS_BASE);
  p.beginField(CMD_IDLE);
  p.endField();
  p.updateChecksum();
  sendAndReceivePacket(p);
}

void Hg3dmGx4::resume()
{
  MIP p(CMD_CLASS_BASE);
  p.beginField(CMD_RESUME);
  p.endField();
  p.updateChecksum();
  sendAndReceivePacket(p);
}

void Hg3dmGx4::reset()
{
  MIP p(CMD_CLASS_BASE);
  p.beginField(CMD_RESET);
  p.endField();
  p.updateChecksum();
  sendAndReceivePacket(p);
}

void Hg3dmGx4::selectBaudRate(unsigned int baud)
{
  //search for device
  static const size_t num_rates = 6;
  static const unsigned int rates[num_rates] = {9600, 19200, 115200, 230400, 460800, 921600};

  bool found_rate = false;
  int i;
  for(i = 0; i < num_rates; i++)
  {
    std::cout << "Try: " << rates[i] << std::endl;
    changeBaudRate(rates[i]);
    try
    {
      ping();
    }
    catch (std::exception& e)
    {
      std::cout << e.what() << std::endl;
      continue;
    }
    break;
  }

  //Change baudrate

  MIP p(CMD_CLASS_3DM);
  p.length = 7;
  p.payload[0] = 7;
  p.payload[1] = CMD_UART_BAUD_RATE;
  p.payload[2] = FUNCTION_APPLY;
  p.payload[3] = (baud >> 24) & 0xff;
  p.payload[4] = (baud >> 16) & 0xff;
  p.payload[5] = (baud >> 8) & 0xff;
  p.payload[6] = baud & 0xff ;
  p.updateChecksum();

  //std::cout << p.toString() << std::endl;

  try
  {
    sendAndReceivePacket(p);
    sleep(1);
    ping();
  }
  catch (std::exception& e)
  {
    std::cout << e.what() << std::endl;
  }
}

void Hg3dmGx4::setCFSettings()
{
  MIP p(CMD_CLASS_3DM);
  p.beginField(CMD_CF_SETTINGS);
  //p.app
  //p.updateChecksum();
}

void Hg3dmGx4::setIMUDataRate(unsigned int decimation, const std::bitset<IMUData::NUM_IMU_DATA>& sources)
{
  static const uint8_t data_fields[] =
  {
    FIELD_IMU_SCALED_ACCELEROMETER,
    FIELD_IMU_SCALED_GYRO,
    FIELD_IMU_SCALED_MAGNETO,
    FIELD_IMU_SCALED_PRESSURE,
    FIELD_IMU_DELTA_THETA,
    FIELD_IMU_DELTA_VELOCITY,
    FIELD_IMU_CF_ORIENTATION_MATRIX,
    FIELD_IMU_CF_QUATERNION,
    FIELD_IMU_CF_EULAR_ANGLES,
    FIELD_IMU_CF_STABILIZED_MAG_VECTOR,
    FIELD_IMU_CF_STABILIZED_ACCEL_VECTOR,
    FIELD_IMU_GPS_CORRELATION_TIMESTAMP
  };

  assert(sizeof(data_fields) == sources.size());

  std::vector<uint8_t> fields;

  for(int i = 0; i < sources.size(); i++)
  {
    if(sources[i])
    {
      fields.push_back(data_fields[i]);
    }
  }

  MIP p(CMD_CLASS_3DM);
  p.beginField(CMD_IMU_MESSAGE_FORMAT);
  p.append(FUNCTION_APPLY);
  p.append(u8(fields.size()));

  for(int i = 0; i < fields.size(); i++)
  {
    p.append(fields[i]);
    p.append(u16(decimation));
  }

  p.endField();
  p.updateChecksum();

  //std::cout << p.toString() << std::endl;

  sendAndReceivePacket(p);
}

void Hg3dmGx4::setGPSDataRate(unsigned int decimation, const std::bitset<GPSData::NUM_GPS_DATA>& sources)
{
  static const uint8_t data_fields[] =
  {
    FIELD_GPS_LLH_POSITION,
    FIELD_GPS_ECEF_POSITION,
    FIELD_GPS_NED_VELOCITY,
    FIELD_GPS_ECEF_VELOCITY,
    FIELD_GPS_DOP_DATA,

    FIELD_GPS_UTC_TIME,
    FIELD_GPS_TIME,
    FIELD_GPS_CLOCK_INFORMATION,
    FIELD_GPS_FIX_INFORMATION,
    FIELD_GPS_SPACE_VEHICLE_INFORMATION,

    FIELD_GPS_HARDWARE_STATUS,
    FIELD_DGPS_INFORMATION,
    FIELD_DGPS_CHANNEL_STATUS
  };

  assert(sizeof(data_fields) == sources.size());

  std::vector<uint8_t> fields;

  for(int i = 0; i < sources.size(); i++)
  {
    if(sources[i])
    {
      fields.push_back(data_fields[i]);
    }
  }

  MIP p(CMD_CLASS_3DM);
  p.beginField(CMD_GPS_MESSAGE_FORMAT);
  p.append(FUNCTION_APPLY);
  p.append(u8(fields.size()));

  for(int i = 0; i < fields.size(); i++)
  {
    p.append(fields[i]);
    p.append(u16(decimation));
  }

  p.endField();
  p.updateChecksum();

  //std::cout << p.toString() << std::endl;

  sendAndReceivePacket(p);

}

void Hg3dmGx4::setEFDataRate(unsigned int decimation, const std::bitset<EFData::NUM_EF_DATA>& sources)
{
  static const uint8_t data_fields[] =
  {
    FIELD_EF_FILTER_STATUS,
    FIELD_EF_GPS_TIMESTAMP,
    FIELD_EF_LLH_POSITION,
    FIELD_EF_NED_VELOCITY,
    FIELD_EF_ORIENTATION_QUATERNION,

    FIELD_EF_ORIENTATION_MATRIX,
    FIELD_EF_ORIENTATION_EULER,
    FIELD_EF_GYRO_BIAS,
    FIELD_EF_ACCEL_BIAS,
    FIELD_EF_LLH_POSITION_UNCERTAINTY,

    FIELD_EF_NED_VELOCITY_UNCERTAINTY,
    FIELD_EF_ALTITUDE_UNCERTAINTY,
    FIELD_EF_GYRO_BIAS_UNCERTAINTY,
    FIELD_EF_ACCEL_BIAS_UNCERTAINTY,
    FIELD_EF_LINEAER_ACCELERATION,

    FIELD_EF_COMPENSATED_ACCELERATION,
    FIELD_EF_COMPENSATED_ANGULAR_RATE,
    FIELD_EF_WGS84_LOCAL_GRAVITY_MAGNITUDE,
    FIELD_EF_ALTITUDE_UNCERTAINTY_QUATERNION_ELEMENT,
    FIELD_EF_GRAVITY_VECTOR,

    FIELD_EF_HEADING_UPDATE_SOURCE_STATE,
    FIELD_EF_MAGNETIC_MODEL_SOLUTION,
    FIELD_EF_GYRO_SCALE_FACTOR,
    FIELD_EF_ACCEL_SCALE_FACTOR,
    FIELD_EF_GYRO_SCALE_FACTOR_UNCERTAINTY,

    FIELD_EF_ACCEL_SCALE_FACTOR_UNCERTAINTY,
    FIELD_EF_STANDARD_ATMOSPHERE_MODEL,
    FIELD_EF_PRESSURE_ALTITUDE,
    FIELD_EF_GPS_ANTENNA_OFFSET_CORRECTION,
    FIELD_EF_GPS_ANTENNA_OFFSET_CORRECTION_UNCERTAINTY,
  };

  assert(sizeof(data_fields) == sources.size());

  std::vector<uint8_t> fields;

  for(int i = 0; i < sources.size(); i++)
  {
    if(sources[i])
    {
      fields.push_back(data_fields[i]);
    }
  }

  MIP p(CMD_CLASS_3DM);
  p.beginField(CMD_EF_MESSAGE_FORMAT);
  p.append(FUNCTION_APPLY);
  p.append(u8(fields.size()));

  for(int i = 0; i < fields.size(); i++)
  {
    p.append(fields[i]);
    p.append(u16(decimation));
  }

  p.endField();
  p.updateChecksum();

  //std::cout << p.toString() << std::endl;

  sendAndReceivePacket(p);
}

void Hg3dmGx4::selectDataStream(const std::bitset<3>& streams)
{
  static const uint8_t selection[] =
  {
    SELECT_IMU,
    SELECT_GPS,
    SELECT_EF,
  };

  assert(sizeof(selection) == streams.size());

  MIP p(CMD_CLASS_3DM);

  for(int i = 0; i < streams.size(); i++)
  {
    p.beginField(CMD_ENABLE_DATA_STREAM);
    p.append(FUNCTION_APPLY);
    p.append(selection[i]);
    p.append(streams[i]);
    p.endField();
  }


  p.updateChecksum();
  //std::cout << p.toString() << std::endl;

  sendAndReceivePacket(p);
}

void Hg3dmGx4::initializeFilterWithMagneto()
{
  MIP p(CMD_CLASS_EF);
  p.beginField(CMD_INITIAL_ATTITUDE_WITH_MAGNETOMETER);
  p.append(float(0));
  p.endField();

  p.updateChecksum();
  //std::cout << p.toString() << std::endl;

  sendAndReceivePacket(p);

  //std::cout << received_packet_.toString() << std::endl;
}

void Hg3dmGx4::setInitialAttitude(float roll, float pitch, float yaw)
{
  MIP p(CMD_CLASS_EF);
  p.beginField(CMD_SET_INITIAL_ATTITUDE);
  p.append(roll);
  p.append(pitch);
  p.append(yaw);
  p.endField();

  p.updateChecksum();
  //std::cout << p.toString() << std::endl;

  sendAndReceivePacket(p);

  //std::cout << received_packet_.toString() << std::endl;
}

void Hg3dmGx4::setInitialHeading(float heading)
{

}

void Hg3dmGx4::sendPacket(const MIP& p, int timeout)
{
  try
  {
    writePacket(p, timeout);
  }
  catch (std::exception &e)
  {
    std::cout << e.what() << std::endl;
  }
}



void Hg3dmGx4::sendAndReceivePacket(const MIP& p)
{
  using namespace boost::chrono;

  sendPacket(p, COMMAND_RW_TIMEOUT);

  static hg_ros_serial::DataStream buffer(MIP::MAX_PAYLOAD);

  high_resolution_clock::time_point tstart = high_resolution_clock::now();
  high_resolution_clock::time_point tstop = tstart + milliseconds(300);

  while(true)
  {
    if (tstop < high_resolution_clock::now())
    {
      throw hg_ros_serial::TimeoutError(false, 300);
    }

    try
    {
      //Get header
      asyncReadBlockOfData(buffer, 1, COMMAND_RW_TIMEOUT);
      if(buffer[0] != MIP::SYNC1)
        continue;

      asyncReadBlockOfData(buffer, 1, COMMAND_RW_TIMEOUT);
      if (buffer[0] != MIP::SYNC2)
        continue;

      //Get descriptor and length
      asyncReadBlockOfData(buffer, 2, COMMAND_RW_TIMEOUT);
      received_packet_.descriptor = buffer[0];
      received_packet_.length = buffer[1];

      //Get payload
      asyncReadBlockOfData(received_packet_.payload, received_packet_.length, COMMAND_RW_TIMEOUT);

      //Get checksum
      asyncReadBlockOfData(buffer, 2, COMMAND_RW_TIMEOUT);

      received_packet_.updateChecksum();

      if(!received_packet_.compareCheckSum(buffer[0], buffer[1]))
      {
        std::cout << "Warning: Dropped packet with mismatched checksum\n" << std::endl;
      }
      else
      {
        processPacket();
        return;
      }
    }
    catch (std::exception& e)
    {
      std::cout << e.what() << std::endl;
    }
  }
}

void Hg3dmGx4::receiveDataStream()
{
  static hg_ros_serial::DataStream buffer(MIP::MAX_PAYLOAD);

  try
  {
    //Get header
    asyncReadBlockOfData(buffer, 1, COMMAND_RW_TIMEOUT);
    if (buffer[0] != MIP::SYNC1)
      return;//continue;

    asyncReadBlockOfData(buffer, 1, COMMAND_RW_TIMEOUT);
    if (buffer[0] != MIP::SYNC2)
      return;//continue;

    //Get descriptor and length
    asyncReadBlockOfData(buffer, 2, COMMAND_RW_TIMEOUT);
    received_packet_.descriptor = buffer[0];
    received_packet_.length = buffer[1];

    //Get payload
    asyncReadBlockOfData(received_packet_.payload, received_packet_.length, COMMAND_RW_TIMEOUT);

    //Get checksum
    asyncReadBlockOfData(buffer, 2, COMMAND_RW_TIMEOUT);

    received_packet_.updateChecksum();

    if(!received_packet_.compareCheckSum(buffer[0], buffer[1]))
    {
      std::cout << "Warning: Dropped packet with mismatched checksum\n" << std::endl;
    }
    else
    {
      processPacket();
    }
  }
  catch (std::exception& e)
  {
    std::cout << e.what() << std::endl;
  }

}

void Hg3dmGx4::processPacket()
{
  switch(received_packet_.descriptor)
  {
    case DATA_CLASS_IMU: processIMUPacket(); break;
    case DATA_CLASS_GPS: processGPSPacket(); break;
    case DATA_CLASS_EF: processEFPacket(); break;
    default: processRespondPacket(); break;
  }
  received_packet_.reset();
}

void Hg3dmGx4::processIMUPacket()
{
  IMUData data;
  uint8_t fields = 0;
  while (fields < IMUData::NUM_IMU_DATA)
  {
    switch (received_packet_.getFieldDescriptor())
    {
      case FIELD_IMU_SCALED_ACCELEROMETER:
        received_packet_.extract(3, data.scaled_accelerometer);
        data.fields |= IMUData::SCALED_ACCELEROMETER;
        break;
      case FIELD_IMU_SCALED_GYRO:
        received_packet_.extract(3, data.scaled_gyro);
        data.fields |= IMUData::SCALED_GYRO;
        break;
      case FIELD_IMU_SCALED_MAGNETO:
        received_packet_.extract(3, data.scaled_magneto);
        data.fields |= IMUData::SCALED_MAGNETO;
        break;
      case FIELD_IMU_SCALED_PRESSURE:
        break;
      case FIELD_IMU_DELTA_THETA:
        break;
      case FIELD_IMU_DELTA_VELOCITY:
        break;
      case FIELD_IMU_CF_ORIENTATION_MATRIX:
        break;
      case FIELD_IMU_CF_QUATERNION:
        received_packet_.extract(4, data.orientation_quaternion);
        data.fields |= IMUData::CF_QUATERNION;
        break;
      case FIELD_IMU_CF_EULAR_ANGLES:
        break;
      case FIELD_IMU_CF_STABILIZED_MAG_VECTOR:
        break;
      case FIELD_IMU_CF_STABILIZED_ACCEL_VECTOR:
        break;
      case FIELD_IMU_GPS_CORRELATION_TIMESTAMP:
        break;
      default:
        //  no more data
        if(imu_data_callback_)
        {
          imu_data_callback_(data);
        }
        return;
    }
    received_packet_.nextField();
    fields++;
  }
}

void Hg3dmGx4::processGPSPacket()
{
  GPSData data;
  uint8_t fields = 0;
  while (fields < GPSData::NUM_GPS_DATA)
  {
    switch (received_packet_.getFieldDescriptor())
    {
      case FIELD_GPS_LLH_POSITION:
        data.fields |= GPSData::LLH_POSITION;
        received_packet_.extract(4, data.llh);
        received_packet_.extract(2, data.llh_accuracy);
        break;
      case FIELD_GPS_ECEF_POSITION: break;
      case FIELD_GPS_NED_VELOCITY:
        received_packet_.extract(3, data.vel);
        received_packet_.extract(1, &data.vel_accuracy);
        break;
      case FIELD_GPS_ECEF_VELOCITY:break;
      case FIELD_GPS_DOP_DATA: break;
      case FIELD_GPS_UTC_TIME:
        received_packet_.extract(1, &data.year);
        received_packet_.extract(5, data.date);
        received_packet_.extract(1, &data.mills);
        struct tm time_str;
        time_str.tm_year = data.year - 1900;
        time_str.tm_mon = data.date[0] - 1;
        time_str.tm_mday = data.date[1] -1;
        time_str.tm_hour = data.date[2];
        time_str.tm_min = data.date[3];
        time_str.tm_sec = data.date[4];
        time_str.tm_isdst = -1;
        data.posix_time = mktime(&time_str);
        break;
      case FIELD_GPS_TIME: break;
      case FIELD_GPS_CLOCK_INFORMATION: break;
      case FIELD_GPS_FIX_INFORMATION:
      {
        data.fields |= GPSData::FIX_INFORMATION;
        uint8_t d12[2];
        uint16_t d34[2];
        received_packet_.extract(2, d12);
        received_packet_.extract(2, d34);
        data.status = d12[0];
        data.status_flag = d34[1];
        break;
      }
      case FIELD_GPS_SPACE_VEHICLE_INFORMATION: break;

      case FIELD_GPS_HARDWARE_STATUS: break;
      case FIELD_DGPS_INFORMATION: break;
      case FIELD_DGPS_CHANNEL_STATUS: break;
      default:
        if(gps_data_callback_)
        {
          gps_data_callback_(data);
        }
        return;
    }
    received_packet_.nextField();
    fields++;
  }
}

void Hg3dmGx4::processEFPacket()
{
  EFData data;
  uint8_t fields = 0;
  while (fields < EFData::NUM_EF_DATA)
  {
    switch (received_packet_.getFieldDescriptor())
    {
      case FIELD_EF_FILTER_STATUS:
      {
        received_packet_.extract(1, &data.status);
        received_packet_.extract(1, &data.dynamics_mode);
        received_packet_.extract(1, &data.status_flag);
        break;
      }
      case FIELD_EF_GPS_TIMESTAMP: break;
      case FIELD_EF_LLH_POSITION:
        data.fields |= EFData::LLH_POSITION;
        received_packet_.extract(3, data.llh);
        break;
      case FIELD_EF_NED_VELOCITY:
        data.fields |= EFData::NED_VELOCITY;
        received_packet_.extract(3, data.vel);
        break;
      case FIELD_EF_ORIENTATION_QUATERNION:
        received_packet_.extract(4, data.orientation_quaternion);
        data.fields |= EFData::ORIENTATION_QUATERNION;
        break;

      case FIELD_EF_ORIENTATION_MATRIX: break;
      case FIELD_EF_ORIENTATION_EULER: break;
      case FIELD_EF_GYRO_BIAS: break;
      case FIELD_EF_ACCEL_BIAS: break;
      case FIELD_EF_LLH_POSITION_UNCERTAINTY:
        data.fields |= EFData::LLH_POSITION_UNCERTAINTY;
        received_packet_.extract(3, data.llh_uncertainty);
        break;
      case FIELD_EF_NED_VELOCITY_UNCERTAINTY:
        data.fields |= EFData::NED_VELOCITY_UNCERTAINTY;
        received_packet_.extract(3, data.vel_uncertainty);
        break;
      case FIELD_EF_ALTITUDE_UNCERTAINTY:
        received_packet_.extract(4, data.orientation_uncertainty);
        data.fields |= EFData::ALTITUDE_UNCERTAINTY_QUATERNION_ELEMENT;
        break;
      case FIELD_EF_GYRO_BIAS_UNCERTAINTY:
        received_packet_.extract(3, data.uncertainty_angular_rate);
        data.fields |= EFData::GYRO_BIAS_UNCERTAINTY;
        break;
      case FIELD_EF_ACCEL_BIAS_UNCERTAINTY:
        received_packet_.extract(3, data.uncertainty_acceleration);
        data.fields |= EFData::ACCEL_BIAS_UNCERTAINTY;
        break;
      case FIELD_EF_LINEAER_ACCELERATION: break;

      case FIELD_EF_COMPENSATED_ACCELERATION:
        received_packet_.extract(3, data.compensated_acceleration);
        data.fields |= EFData::COMPENSATED_ACCELERATION;
        break;
      case FIELD_EF_COMPENSATED_ANGULAR_RATE:
        received_packet_.extract(3, data.compensated_angular_rate);
        data.fields |= EFData::COMPENSATED_ANGULAR_RATE;
        break;
      case FIELD_EF_WGS84_LOCAL_GRAVITY_MAGNITUDE: break;
      case FIELD_EF_ALTITUDE_UNCERTAINTY_QUATERNION_ELEMENT: break;
      case FIELD_EF_GRAVITY_VECTOR:
        //  received_packet_.extract(3, data.gravity_vector);
        //  data.fields |= EFData::GRAVITY_VECTOR;
        break;

      case FIELD_EF_HEADING_UPDATE_SOURCE_STATE: break;
      case FIELD_EF_MAGNETIC_MODEL_SOLUTION: break;
      case FIELD_EF_GYRO_SCALE_FACTOR: break;
      case FIELD_EF_ACCEL_SCALE_FACTOR: break;
      case FIELD_EF_GYRO_SCALE_FACTOR_UNCERTAINTY: break;
      case FIELD_EF_ACCEL_SCALE_FACTOR_UNCERTAINTY: break;
      case FIELD_EF_STANDARD_ATMOSPHERE_MODEL: break;
      case FIELD_EF_PRESSURE_ALTITUDE: break;
      case FIELD_EF_GPS_ANTENNA_OFFSET_CORRECTION: break;
      case FIELD_EF_GPS_ANTENNA_OFFSET_CORRECTION_UNCERTAINTY: break;
      default:
        //  no more data
        if (ef_data_callback_)
        {
          ef_data_callback_(data);
        }
        return;
    }
    received_packet_.nextField();
    fields++;
  }
}

void Hg3dmGx4::processRespondPacket()
{
  //  Find ACK/NACK
  if(received_packet_.payload[1] == FIELD_ACK_NACK)
  {
    if(received_packet_.payload[3] == 0x00)
    {
      //  Got ACK
      //  std::cout << "Found ACK field" << std::endl;
    }
    else
    {
      //Got NACK
      std::cout << "Received NACK packet (class, command, code): ";
      std::cout << std::hex << static_cast<int>(received_packet_.descriptor) << ", ";
      std::cout << static_cast<int>(received_packet_.payload[2]) << ", ";
      std::cout << static_cast<int>(received_packet_.payload[3]) << std::endl;
    }
  }
}
