/*******************************************************************************
 *
 *      seabee3_physics
 * 
 *      Copyright (c) 2010
 *
 *      Edward T. Kaszubski (ekaszubski@gmail.com)
 *      Randolph C. Voorhies (voorhies at usc dot edu)
 *
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
 *      * Neither the name of the USC Underwater Robotics Team nor the names of its
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

#include <base_node/base_node.h>
#include <common_utils/tf.h>

class TfFrameLock: public BaseNode<>
{
  public:
    TfFrameLock( ros::NodeHandle &nh ) : BaseNode<>( nh )  
    {
      nh_local_.param<std::string>("from_frame", from_frame_,"");
      nh_local_.param<std::string>("to_frame", to_frame_,"");

      transform_sub_ = nh_local_.subscribe<geometry_msgs::Twist>("transform", 10, &TfFrameLock::transformCB, this);
    }

    void transformCB(const geometry_msgs::Twist::ConstPtr & transform_msg)
    { *transform_msg >> locked_transform_; }

    void spinOnce()
    { tf_utils::publishTfFrame(locked_transform_, from_frame_, to_frame_); }

    ~TfFrameLock()
    { }

  private:
    std::string from_frame_;
    std::string to_frame_;

    ros::Subscriber transform_sub_;

    tf::Transform locked_transform_;
};

int main( int argc, char** argv )
{

	ros::init( argc, argv, "tf_frame_lock" );
	ros::NodeHandle nh( "~" );

	TfFrameLock framelock( nh );
	framelock.spin();

	return 0;
}
