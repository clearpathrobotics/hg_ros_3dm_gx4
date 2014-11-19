/*
 * test.cpp
 *
 *  Created on: Nov 17, 2014
 *      Author: mahisorn
 */

#include <hg_3dm_gx4/3dm_gx4.h>

using namespace std;

int main()
{
  hg_3dm_gx4::Hg3dmGx4 imu;
  //if(imu.openPort("/dev/pts/16", 115200))
  if(imu.openPort("/dev/ttyUSB0", 115200))
  //if(imu.openPort("/dev/ttyS0", 115200))
  {
    try
    {
      imu.ping();

      imu.idle();

      static const int decimation = (500/100);

      imu.setIMUDataRate(decimation, //(500 / 1) for 3DM-GX4-45
                         hg_3dm_gx4::IMUData::SCALED_ACCELEROMETER |
                         hg_3dm_gx4::IMUData::SCALED_GYRO |
                         hg_3dm_gx4::IMUData::SCALED_MAGNETO |
                         0);


      imu.setEFDataRate(decimation,
                        hg_3dm_gx4::EFData::ORIENTATION_EULER |
                        hg_3dm_gx4::EFData::GRAVITY_VECTOR |
                        //hg_3dm_gx4::EFData::FILTER_STATUS |
                        0);

      imu.selectDataStream(
                           //hg_3dm_gx4::DataStream::IMU_DATA |
                           hg_3dm_gx4::DataStream::EF_DATA |
                           0);

      //imu.resume();

      //imu.initializeFilterWithMagneto();

      imu.setInitialAttitude(0, 0, 0);

      imu.receiveDataStream();


    //imu.selectBaudRate(460800);
    /*
    sleep(1);
    imu.idle();
    sleep(1);
    imu.ping();
    sleep(1);
    imu.resume();
    sleep(1);
    imu.ping();
    sleep(1);
    imu.reset();
    sleep(5);
    imu.ping();
    sleep(1);
    */
    }
    catch (std::exception& e)
    {
      std::cout << e.what() << std::endl;
    }
  }
  else
  {
    std::cout << "Shit!\n";
  }
  return 0;
}


