#!/bin/bash

echo ""
echo -n 'Install compiler tools? (y/n) '
read res
if [ "$res" == "y" ]; then
  sudo apt-get update && sudo apt-get install build-essential python-yaml cmake subversion wget
#  wget --no-check-certificate http://ros.org/rosinstall -O ~/rosinstall
#  chmod 755 ~/rosinstall
fi
echo "----------"

echo ""
echo -n 'Install ros-diamondback-base? (y/n) '
read res
if [ "$res" == "y" ]; then
  sudo sh -c 'echo "deb http://code.ros.org/packages/ros/ubuntu maverick main" > /etc/apt/sources.list.d/ros-latest.list'
  wget http://code.ros.org/packages/ros.key -O - | sudo apt-key add -
  sudo apt-get update
  sudo apt-get install ros-diamondback-desktop-full ros-diamondback-joystick-drivers
fi
echo "----------"

echo ""
echo -n 'Configure environment variables? (y/n) '
read res
if [ "$res" == "y" ]; then
  sudo sh -c 'echo "source /opt/ros/diamondback/setup.bash" >> ~/.bashrc'
  sudo sh -c 'echo "\nexport ROS_PACKAGE_PATH=\$ROS_PACKAGE_PATH:~/workspace/ros" >> ~/.bashrc'
  sudo sh -c 'echo "\nexport ROS_PARALLEL_JOBS=-j4" >> ~/.bashrc'
  . ~/.bashrc
fi
echo "----------"

echo ""
echo -n 'Install quickdev? (y/n) '
read res
if [ "$res" == "y" ]; then
  mkdir -p ~/workspace/ros
  echo "Downloading source..."
  svn co https://usc-ros-pkg.svn.sourceforge.net/svnroot/usc-ros-pkg/trunk/quickdev/ ~/workspace/ros/quickdev
  echo "Installing dependencies..."
  rosdep install quickdev
  echo "Compiling quickdev..."
  rosmake quickdev
fi
echo "----------"

echo ""
echo -n 'Download seabee3-ros-pkg? (y/n) '
read res
if [ "$res" == "y" ]; then
  mkdir -p ~/workspace
  echo "Downloading source..."
  svn co https://seabee3-ros-pkg.googlecode.com/svn/trunk/diamondback-latest/seabee3-ros-pkg/ ~/workspace/ros/seabee3-ros-pkg
fi
echo "----------"

echo ""
echo -n 'Compile seabee3-ros-pkg? (y/n) '
read res
if [ "$res" == "y" ]; then
  echo "Installing dependencies..."
  rosdep install seabee3-ros-pkg
  echo "Compiling seabee3-ros-pkg..."
  rosmake seabee3-ros-pkg
fi
echo "----------"

echo ""
echo -n 'Update permissions for video devices? (y/n) '
read res
if [ "$res" == "y" ]; then
  sudo sh `rosstack find seabee3-ros-pkg`/seabee3_util/config-1394.sh
fi
echo "----------"

echo ""
echo 'Done'
