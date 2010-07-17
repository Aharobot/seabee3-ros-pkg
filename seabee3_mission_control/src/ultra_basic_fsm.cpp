#include <ros/ros.h>
//#include <seabee3_driver/SetDesiredRPY.h>
#include <seabee3_driver/SetDesiredXYZ.h>
#include <geometry_msgs/Twist.h>

int main(int argc, char * argv[])
{
	ros::init(argc, argv, "ultra_basic_fsm");
	ros::NodeHandle n("~");
	
	int sleep1, dive_sleep, dive_depth, forward_speed;
	
	n.param("sleep1", sleep1, 5);
	n.param("dive_sleep", dive_sleep, 5);
	
	n.param("dive_depth", dive_depth, 950);
	n.param("forward_speed", forward_speed, 80);
	
	ros::ServiceClient depth_srv = n.serviceClient<seabee3_driver::SetDesiredXYZ>("/seabee3/setDesiredXYZ");
	ros::Publisher cmd_vel_pub = n.advertise<geometry_msgs::Twist>("/seabee3/cmd_vel", 1);
	
	seabee3_driver::SetDesiredXYZ setDesiredXYZ;
	setDesiredXYZ.request.Mask.z = 1;
	setDesiredXYZ.request.DesiredXYZ.z = dive_depth;
	
	geometry_msgs::Twist cmd_vel;
	
	int mState = 0;
	ros::Time lastTime = ros::Time::now();
	
	while( ros::ok() )
	{
		ros::Duration dt = ros::Time::now() - lastTime;
		
		if(mState == 0)
		{
			if(dt.toSec() > sleep1)
			{
				mState = 1;
				lastTime = ros::Time::now();
				//set depth
				depth_srv.call(setDesiredXYZ);
				cmd_vel.linear.x = forward_speed;
			}
			else
			{
				cmd_vel_pub.publish(cmd_vel);
			}
		}
		else if(mState == 1)
		{
			if(dt.toSec() > dive_sleep)
			{
				mState = 2;
			}
		}
		else if(mState == 2)
		{
			//drive forward
			cmd_vel_pub.publish(cmd_vel);
		}
		
		ros::spinOnce();
		ros::Rate(15).sleep();
	}
}
