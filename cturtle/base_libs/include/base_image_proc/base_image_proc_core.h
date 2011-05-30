/*******************************************************************************
 *
 *      base_image_proc_core
 * 
 *      Copyright (c) 2010, Edward T. Kaszubski (ekaszubski@gmail.com)
 *      All rights reserved.
 *
 *      Redistribution and use in source and binary forms, with or without
 *      modification, are permitted provided that the following conditions are
 *      met:
 *      
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following disclaimer
 *        in the documentation and/or other materials provided with the
 *        distribution.
 *      * Neither the name of "seabee3-ros-pkg" nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *      
 *      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************/

#ifndef BASE_IMAGE_PROC_CORE_H_
#define BASE_IMAGE_PROC_CORE_H_

// for ImageTransport, Publisher
#include <image_transport/image_transport.h>
// for Mat
#include <opencv/cv.h>
#include <cxcore.h>
// for image utilities like imread()
#include "highgui.h"
//for CvBridge
#include <cv_bridge/CvBridge.h>
// for a ton of boost-related shit
#include <boost/thread.hpp>
// for CameraInfo
#include <sensor_msgs/CameraInfo.h>
// for pinhole camera model
#include <image_geometry/pinhole_camera_model.h>
// for the DoSomething service
#include <std_srvs/Empty.h>
#include <base_node/base_node.h>
#include <common_utils/opencv.h>

template<typename _ReconfigureType, typename _ServiceType>
class BaseImageProcCore: public BaseNode<_ReconfigureType>
{

protected:
	image_transport::ImageTransport image_transport_;
	bool new_image_;
	IplImage * ipl_image_;

	sensor_msgs::ImagePtr image_msg_;
	sensor_msgs::CameraInfo camera_info_msg_;

	image_transport::Publisher image_pub_;

	image_transport::Subscriber image_sub_;

	ros::Subscriber camera_info_sub_;

	sensor_msgs::CvBridge image_bridge_;

	boost::mutex image_mutex_, camera_info_mutex_, flag_mutex_;
	std::string image_transport_type_;
	bool publish_image_;
public:
	BaseImageProcCore( ros::NodeHandle & nh, uint threads = 3 );
	virtual ~BaseImageProcCore();

protected:
	virtual void imageCB();
	void publishCvImage( const cv::Mat & image_mat );
	void publishCvImage( const IplImage * ipl_image );

private:
	void imageCB_0( const sensor_msgs::ImageConstPtr& image_msg );
	void infoCB( const sensor_msgs::CameraInfoConstPtr& camera_info_msg );

};

template<typename _ReconfigureType, typename _ServiceType>
BaseImageProcCore<_ReconfigureType, _ServiceType>::BaseImageProcCore( ros::NodeHandle & nh, uint threads ) :
	BaseNode<_ReconfigureType>( nh, threads ), image_transport_( this->nh_priv_ ), new_image_( false ), ipl_image_( NULL )
{
	this->nh_priv_.param( "image_transport", image_transport_type_, std::string( "raw" ) );
	this->nh_priv_.param( "publish_image", publish_image_, true );

	image_sub_ = image_transport_.subscribe( nh.resolveName( "image" ), 1, &BaseImageProcCore::imageCB_0, this, image_transport_type_ );
	camera_info_sub_ = this->nh_priv_.subscribe( nh.resolveName( "camera_info" ), 1, &BaseImageProcCore::infoCB, this );

	if ( publish_image_ ) image_pub_ = image_transport_.advertise( "output_image", 1 );
}

template<typename _ReconfigureType, typename _ServiceType>
BaseImageProcCore<_ReconfigureType, _ServiceType>::~BaseImageProcCore()
{
	//
}

template<typename _ReconfigureType, typename _ServiceType>
void BaseImageProcCore<_ReconfigureType, _ServiceType>::publishCvImage( const IplImage * ipl_image )
{
	opencv_utils::publishCvImage( ipl_image, &image_pub_, &image_bridge_ );
}

template<typename _ReconfigureType, typename _ServiceType>
void BaseImageProcCore<_ReconfigureType, _ServiceType>::publishCvImage( const cv::Mat & image_mat )
{
	opencv_utils::publishCvImage( image_mat, &image_pub_, &image_bridge_ );
}

template<typename _ReconfigureType, typename _ServiceType>
void BaseImageProcCore<_ReconfigureType, _ServiceType>::imageCB_0( const sensor_msgs::ImageConstPtr& image_msg )
{
	ROS_DEBUG( "got image" );
	image_mutex_.lock();

	image_msg_ = boost::const_pointer_cast<sensor_msgs::Image>( image_msg );

	image_mutex_.unlock();

	boost::lock_guard<boost::mutex> guard( flag_mutex_ );
	new_image_ = true;

	imageCB();
}

template<typename _ReconfigureType, typename _ServiceType>
void BaseImageProcCore<_ReconfigureType, _ServiceType>::infoCB( const sensor_msgs::CameraInfoConstPtr& camera_info_msg )
{
	ROS_DEBUG( "got camera info" );
	camera_info_mutex_.lock();

	camera_info_msg_ = *camera_info_msg;

	camera_info_mutex_.unlock();
}

template<typename _ReconfigureType, typename _ServiceType>
void BaseImageProcCore<_ReconfigureType, _ServiceType>::imageCB()
{
	//
}

#endif /* BASE_IMAGE_PROC_CORE_H_ */
