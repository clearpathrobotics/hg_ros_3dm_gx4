/*
 * 3dm_gx4.cpp
 *
 *  Created on: Nov 17, 2014
 *      Author: mahisorn
 */
#include <hg_3dm_gx4/3dm_gx4.h>
#include <boost/asio.hpp>
#include <boost/chrono.hpp>

#define u8(x) static_cast<uint8_t>((x))
#define u16(x) static_cast<uint16_t>((x))

using namespace hg_3dm_gx4;

void Hg3dmGx4::ping()
{
  MIP p(CMD_CLASS_BASE);
  p.beginField(CMD_PING);
  p.endField();
  p.updateCheckSum();
  sendAndReceivePacket(p);
}

void Hg3dmGx4::idle()
{
  MIP p(CMD_CLASS_BASE);
  p.beginField(CMD_IDLE);
  p.endField();
  p.updateCheckSum();
  sendAndReceivePacket(p);
}

void Hg3dmGx4::resume()
{
  MIP p(CMD_CLASS_BASE);
  p.beginField(CMD_RESUME);
  p.endField();
  p.updateCheckSum();
  sendAndReceivePacket(p);
}

void Hg3dmGx4::reset()
{
  MIP p(CMD_CLASS_BASE);
  p.beginField(CMD_RESET);
  p.endField();
  p.updateCheckSum();
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
  p.updateCheckSum();

  std::cout << p.toString() << std::endl;

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

void Hg3dmGx4::setIMUDataRate(unsigned int decimation, const std::bitset<IMUData::NUM_IMU_DATA>& sources)
{
  static const uint8_t data_fields[] =
  {
    FILED_IMU_SCALED_ACCELEROMETER,
    FILED_IMU_SCALED_GYRO,
    FILED_IMU_SCALED_MAGNETO,
    FILED_IMU_SCALED_PRESSURE,
    FILED_IMU_DELTA_THETA,
    FILED_IMU_DELTA_VELOCITY,
    FILED_IMU_GPS_CORRELATION_TIMESTAMP
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
  p.updateCheckSum();

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
  p.updateCheckSum();

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


  p.updateCheckSum();
  //std::cout << p.toString() << std::endl;

  sendAndReceivePacket(p);
}

void Hg3dmGx4::initializeFilterWithMagneto()
{
  MIP p(CMD_CLASS_EF);
  p.beginField(CMD_INITIAL_ATTITUDE_WITH_MAGNETOMETER);
  p.append(float(0));
  p.endField();

  p.updateCheckSum();
  std::cout << p.toString() << std::endl;

  sendAndReceivePacket(p);

  std::cout << received_packet_.toString() << std::endl;
}

void Hg3dmGx4::setInitialAttitude(float roll, float pitch, float yaw)
{
  MIP p(CMD_CLASS_EF);
  p.beginField(CMD_SET_INITIAL_ATTITUDE);
  p.append(roll);
  p.append(pitch);
  p.append(yaw);
  p.endField();

  p.updateCheckSum();
  std::cout << p.toString() << std::endl;

  sendAndReceivePacket(p);

  std::cout << received_packet_.toString() << std::endl;
}

void Hg3dmGx4::setInitialHeading(float heading)
{

}

void Hg3dmGx4::sendPacket(const MIP& p, int timeout)
{
  using namespace boost::chrono;

  Serial::Data buffer;
  buffer.reserve(MIP::HEADER_LENGTH + MIP::MAX_PAYLOAD + 2);

  buffer.push_back(p.sync1);
  buffer.push_back(p.sync2);
  buffer.push_back(p.descriptor);
  buffer.push_back(p.length);
  for (size_t i = 0; i < p.length; i++) {
    buffer.push_back(p.payload[i]);
  }
  buffer.push_back(p.crc1);
  buffer.push_back(p.crc2);

  try
  {
    writeData(buffer, timeout);
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

  static Serial::Data buffer(MIP::HEADER_LENGTH + MIP::MAX_PAYLOAD + 2);

  high_resolution_clock::time_point tstart = high_resolution_clock::now();
  high_resolution_clock::time_point tstop = tstart + milliseconds(300);

  while(true)
  {
    if (tstop < high_resolution_clock::now())
    {
      throw std::runtime_error("Got no respond");
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

      received_packet_.updateCheckSum();

      if(received_packet_.crc1 != buffer[0] || received_packet_.crc2 != buffer[1])
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
  //using namespace boost::chrono;

  static Serial::Data buffer(MIP::HEADER_LENGTH + MIP::MAX_PAYLOAD + 2);

  is_running_  = true;

  while (is_running_)
  {
    try
    {
      //Get header
      asyncReadBlockOfData(buffer, 1, COMMAND_RW_TIMEOUT);
      if (buffer[0] != MIP::SYNC1)
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

      received_packet_.updateCheckSum();

      if (received_packet_.crc1 != buffer[0] || received_packet_.crc2 != buffer[1])
      {
        std::cout << "Warning: Dropped packet with mismatched checksum\n" << std::endl;
      }
      else
      {
        processPacket();
        //return;
      }
    }
    catch (std::exception& e)
    {
      std::cout << e.what() << std::endl;
      is_running_ = false;
    }
    usleep(1000);
  }

}

void Hg3dmGx4::processPacket()
{
  //std::cout << received_packet_.toString() << std::endl;
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
  float data[10];
  while (true)
  {
    switch (received_packet_.getFieldDescriptor())
    {
      case FILED_IMU_SCALED_ACCELEROMETER:
        received_packet_.extract(3, data);
        printf("acc: %8.3f %8.3f %8.3f\n", data[0], data[1], data[2]);
        break;
      case FILED_IMU_SCALED_GYRO:
        received_packet_.extract(3, data);
        printf("gyr: %8.3f %8.3f %8.3f\n", data[0], data[1], data[2]);
        break;
      case FILED_IMU_SCALED_MAGNETO:
        received_packet_.extract(3, data);
        printf("mag: %8.3f %8.3f %8.3f\n", data[0], data[1], data[2]);
        break;
      case FILED_IMU_SCALED_PRESSURE:
        break;
      case FILED_IMU_DELTA_THETA:
        break;
      case FILED_IMU_DELTA_VELOCITY:
        break;
      case FILED_IMU_GPS_CORRELATION_TIMESTAMP:
        break;
      default:
        return;
    }
    received_packet_.nextField();
  }
}

void Hg3dmGx4::processGPSPacket()
{
  std::cout << __FUNCTION__ << std::endl;
}

void Hg3dmGx4::processEFPacket()
{
  float data[10];
  while (true)
  {
    switch (received_packet_.getFieldDescriptor())
    {
      case FIELD_EF_FILTER_STATUS:
      {
        uint16_t status[3];
        received_packet_.extract(3, status);
        printf("sta: 0x%04x 0x%04x 0x%04x\n", status[0], status[1], status[2]);
        break;
      }
      case FIELD_EF_GPS_TIMESTAMP: break;
      case FIELD_EF_LLH_POSITION: break;
      case FIELD_EF_NED_VELOCITY: break;
      case FIELD_EF_ORIENTATION_QUATERNION: break;

      case FIELD_EF_ORIENTATION_MATRIX: break;
      case FIELD_EF_ORIENTATION_EULER:
        received_packet_.extract(3, data);
        printf("rpy: %8.3f %8.3f %8.3f\n", data[0], data[1], data[2]);
        break;
      case FIELD_EF_GYRO_BIAS: break;
      case FIELD_EF_ACCEL_BIAS: break;
      case FIELD_EF_LLH_POSITION_UNCERTAINTY: break;

      case FIELD_EF_NED_VELOCITY_UNCERTAINTY: break;
      case FIELD_EF_ALTITUDE_UNCERTAINTY: break;
      case FIELD_EF_GYRO_BIAS_UNCERTAINTY: break;
      case FIELD_EF_ACCEL_BIAS_UNCERTAINTY: break;
      case FIELD_EF_LINEAER_ACCELERATION: break;

      case FIELD_EF_COMPENSATED_ACCELERATION: break;
      case FIELD_EF_COMPENSATED_ANGULAR_RATE: break;
      case FIELD_EF_WGS84_LOCAL_GRAVITY_MAGNITUDE: break;
      case FIELD_EF_ALTITUDE_UNCERTAINTY_QUATERNION_ELEMENT: break;
      case FIELD_EF_GRAVITY_VECTOR:
        received_packet_.extract(3, data);
        printf("grv: %8.3f %8.3f %8.3f\n", data[0], data[1], data[2]);
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
        return;
    }
    received_packet_.nextField();
  }
}

void Hg3dmGx4::processRespondPacket()
{
  //Find ACK/NACK
  if(received_packet_.payload[1] == FIELD_ACK_NACK)
  {
    if(received_packet_.payload[3] == 0x00)
    {
      //Got ACK
      std::cout << "Found ACK field" << std::endl;
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
