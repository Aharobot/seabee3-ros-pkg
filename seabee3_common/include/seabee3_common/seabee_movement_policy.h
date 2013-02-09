/***************************************************************************
 *  include/seabee3_common/seabee_movement_policy.h
 *  --------------------
 *
 *  Copyright (c) 2011, Edward T. Kaszubski ( ekaszubski@gmail.com )
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of seabee3-ros-pkg nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **************************************************************************/

#ifndef SEABEE3COMMON_SEABEEMOVEMENTPOLICY_H_
#define SEABEE3COMMON_SEABEEMOVEMENTPOLICY_H_

// policies
#include <quickdev/tf_tranceiver_policy.h>
#include <quickdev/node_handle_policy.h>
//#include <quickdev/action_client_policy.h>

// objects
#include <seabee3_common/motion_primitives.h>
#include <quickdev/multi_subscriber.h>
#include <quickdev/multi_publisher.h>

// utils
#include <quickdev/numeric_unit_conversions.h>
#include <quickdev/math.h>

// actions
//#include <seabee3_actions/MakeTrajectoryAction.h>
//#include <seabee3_actions/FollowTrajectoryAction.h>

// msgs
//#include <seabee3_msgs/TrajectoryWaypoint.h>

using namespace seabee;

QUICKDEV_DECLARE_POLICY_NS( SeabeeMovement )
{
//    typedef seabee3_msgs::TrajectoryWaypoint _TrajectoryWaypointMsg;

//    typedef seabee3_actions::MakeTrajectoryAction _MakeTrajectoryAction;
//    typedef seabee3_actions::FollowTrajectoryAction _FollowTrajectoryAction;

    typedef quickdev::TfTranceiverPolicy _TfTranceiverPolicy;
    typedef quickdev::NodeHandlePolicy _NodeHandlePolicy;
//    typedef quickdev::ActionClientPolicy<_MakeTrajectoryAction> _MakeTrajectoryActionClientPolicy;
//    typedef quickdev::ActionClientPolicy<_FollowTrajectoryAction> _FollowTrajectoryActionClientPolicy;
}

QUICKDEV_DECLARE_POLICY( SeabeeMovement, _TfTranceiverPolicy, _NodeHandlePolicy )

QUICKDEV_DECLARE_POLICY_CLASS( SeabeeMovement )
{
public:
//    typedef QUICKDEV_GET_POLICY_NS( SeabeeMovement )::_TrajectoryWaypointMsg _TrajectoryWaypointMsg;

//    typedef QUICKDEV_GET_POLICY_NS( SeabeeMovement )::_MakeTrajectoryAction _MakeTrajectoryAction;
//    typedef QUICKDEV_GET_POLICY_NS( SeabeeMovement )::_FollowTrajectoryAction _FollowTrajectoryAction;

    typedef QUICKDEV_GET_POLICY_NS( SeabeeMovement )::_TfTranceiverPolicy _TfTranceiverPolicy;
    typedef QUICKDEV_GET_POLICY_NS( SeabeeMovement )::_NodeHandlePolicy _NodeHandlePolicy;
//    typedef QUICKDEV_GET_POLICY_NS( SeabeeMovement )::_MakeTrajectoryActionClientPolicy _MakeTrajectoryActionClientPolicy;
//    typedef QUICKDEV_GET_POLICY_NS( SeabeeMovement )::_FollowTrajectoryActionClientPolicy _FollowTrajectoryActionClientPolicy;

//    typedef _TrajectoryWaypointMsg _PhysicsStateMsg;
//    typedef __QUICKDEV_FUNCTION_TYPE<void( _TrajectoryWaypointMsg::ConstPtr const & )> _PhysicsStateCallback;

    typedef std::function<bool()> _TermCriteria;

    typedef geometry_msgs::Twist _TwistMsg;
/*
protected:
    _PhysicsStateMsg::ConstPtr physics_state_msg_;
*/
private:
    ros::MultiSubscriber<> multi_sub_;
    ros::MultiPublisher<> multi_pub_;

    QUICKDEV_MAKE_POLICY_FUNCS( SeabeeMovement )

    QUICKDEV_DECLARE_POLICY_CONSTRUCTOR( SeabeeMovement ),
        initialized_( false )
    {
        printPolicyActionStart( "create", this );
        printPolicyActionDone( "create", this );
    }

    void init_add_publishers( ros::NodeHandle & nh_rel )
    {
        multi_pub_.addPublishers<_TwistMsg>( nh_rel, { "/seabee3/cmd_vel" } );
    }

    QUICKDEV_ENABLE_INIT()
    {
        auto & nh_rel = _NodeHandlePolicy::getNodeHandle();

//        multi_sub_.addSubscriber( nh_rel, "physics_state", &SeabeeMovementPolicy::physicsStateCB, this );
        init_add_publishers( nh_rel );

//        _MakeTrajectoryActionClientPolicy::registerDoneCB( quickdev::auto_bind( &SeabeeMovementPolicy::makeTrajectoryActionDoneCB, this ) );
//        _FollowTrajectoryActionClientPolicy::registerDoneCB( quickdev::auto_bind( &SeabeeMovementPolicy::followTrajectoryActionDoneCB, this ) );

//        initPolicies<_MakeTrajectoryActionClientPolicy>( "action_name_param", std::string( "make_trajectory" ) );
//        initPolicies<_FollowTrajectoryActionClientPolicy>( "action_name_param", std::string( "follow_trajectory" ) );

        initPolicies<quickdev::policy::ALL>();

        QUICKDEV_SET_INITIALIZED();
    }

/*
    QUICKDEV_DECLARE_ACTION_DONE_CALLBACK( makeTrajectoryActionDoneCB, _MakeTrajectoryAction )
    {
        //
    }

    QUICKDEV_DECLARE_ACTION_DONE_CALLBACK( followTrajectoryActionDoneCB, _FollowTrajectoryAction )
    {
        //
    }
*/
/*
    QUICKDEV_DECLARE_MESSAGE_CALLBACK( physicsStateCB, _PhysicsStateMsg )
    {
        physics_state_msg_ = msg;
    }
*/
    // #########################################################################################################################################
    tf::Transform getTransform( std::string const & to_frame, std::string const & from_frame = "seabee3/current_pose" )
    {
        return tf::Transform( _TfTranceiverPolicy::tryLookupTransform( from_frame, to_frame ) );
    }

    tf::Transform getCurrentTransform()
    {
        return getTransform( "seabee3/current_pose", "world" );
    }

    tf::Transform getDesiredTransform()
    {
        return getTransform( "seabee3/desired_pose", "world" );
    }

    // #########################################################################################################################################
    //! Return the Pose of the landmark with the given name
    Pose getPose( std::string const & to_frame, std::string const & from_frame = "seabee3/current_pose" )
    {
        return unit::convert<Pose>( getTransform( to_frame, from_frame ) );
    }

    //! Return the Pose of the sub
    /*!
     * - Calls getPose( "seabee" )
     */
    Pose getCurrentPose()
    {
        return getPose( "seabee3/current_pose", "world" );
    }

    Pose getDesiredPose()
    {
        return getPose( "seabee3/desired_pose", "world" );
    }

    // #########################################################################################################################################
    //! Fire the given device
    quickdev::SimpleActionToken fireDevice( FiringDevice const & device );

    // #########################################################################################################################################
    //! Fire the given shooter
    /*!
     * - Calls fireDevice( Shooter( shooter_id ) )
     */
    quickdev::SimpleActionToken fireShooter( Shooter::Id const & shooter_id );

    //! Fire the given dropper
    /*!
     * - Calls fireDevice( Dropper( dropper_id ) )
     */
    quickdev::SimpleActionToken fireDropper( Dropper::Id const & dropper_id );

    // #########################################################################################################################################
    //! Move to the given pose
    /*!
     * - moveTo( getCurrentPose() + Orientation( 90 ) ) will rotate 90 degrees CCW (see faceTo( Orientation ) )
     * - moveTo( getCurrentPose() + Position( 0, 0, -1 ) will dive one meter
     */
    /*
    _FollowTrajectoryActionClientPolicy::_ActionToken moveTo( Pose const & pose )
    {
        if( !physics_state_msg_ ) return _FollowTrajectoryActionClientPolicy::_ActionToken();

        _MakeTrajectoryActionClientPolicy::_GoalMsg make_trajectory_goal;

        make_trajectory_goal.waypoints.push_back( *physics_state_msg_ );

        _TrajectoryWaypointMsg ending_waypoint;
        ending_waypoint.pose.pose = unit::implicit_convert( pose );
        // velocity will default to zero
        make_trajectory_goal.waypoints.push_back( ending_waypoint );

        auto make_trajectory_token = _MakeTrajectoryActionClientPolicy::sendGoal( make_trajectory_goal );
        auto make_trajectory_result = make_trajectory_token.get( 1.0 );

        if( make_trajectory_result->trajectory.intervals.size() > 0 )
        {
            _FollowTrajectoryActionClientPolicy::_GoalMsg follow_trajectory_goal;
            follow_trajectory_goal.trajectory = make_trajectory_result->trajectory;

            return _FollowTrajectoryActionClientPolicy::sendGoal( follow_trajectory_goal );
        }
        return _FollowTrajectoryActionClientPolicy::_ActionToken();
    }*/

    quickdev::SimpleActionToken diveTo( double const & depth, _TermCriteria term_criteria = _TermCriteria() )
    {
        quickdev::SimpleActionToken result;
        result.start( quickdev::auto_bind( quickdev::auto_bind( &SeabeeMovementPolicy::diveToImpl, this ), depth, result, term_criteria ) );

        return result;
    }

    void diveToImpl( double const & depth, quickdev::SimpleActionToken token, _TermCriteria term_criteria )
    {
        ros::Rate publish_rate( 10 );

        while( ( !term_criteria || !term_criteria() ) && token.ok() && ros::ok() )
        {
            auto world_to_desired_pose = _TfTranceiverPolicy::tryLookupTransform( "/world", "/seabee3/desired_pose" );

            world_to_desired_pose.getOrigin().setZ( depth );
            _TfTranceiverPolicy::publishTransform( world_to_desired_pose, "/world", "/seabee3/desired_pose" );

            if( _TfTranceiverPolicy::transformExists( "/world", "/seabee3/current_pose" ) )
            {
                auto world_to_current_pose = _TfTranceiverPolicy::tryLookupTransform( "/world", "/seabee3/current_pose" );

                if( fabs( world_to_current_pose.getOrigin().getZ() - world_to_desired_pose.getOrigin().getZ() ) < 0.05 )
                {
                    token.complete( true );
                    return;
                }
            }

            publish_rate.sleep();
        }

        token.cancel();
    }

    quickdev::SimpleActionToken faceTo( double const & heading, _TermCriteria term_criteria = _TermCriteria() )
    {
        quickdev::SimpleActionToken result;
        result.start( quickdev::auto_bind( quickdev::auto_bind( &SeabeeMovementPolicy::faceToImpl, this ), heading, result, term_criteria ) );

        return result;
    }

    void faceToImpl( double const & heading, quickdev::SimpleActionToken token, _TermCriteria term_criteria )
    {
        ros::Rate publish_rate( 10 );

        while( ( !term_criteria || !term_criteria() ) && token.ok() && ros::ok() )
        {
            auto world_to_desired_pose = _TfTranceiverPolicy::tryLookupTransform( "/world", "/seabee3/desired_pose" );

            world_to_desired_pose.setRotation( tf::Quaternion( heading, 0, 0 ) );
            _TfTranceiverPolicy::publishTransform( world_to_desired_pose, "/world", "/seabee3/desired_pose" );

            if( _TfTranceiverPolicy::transformExists( "/world", "/seabee3/current_pose" ) )
            {
                auto world_to_current_pose = _TfTranceiverPolicy::tryLookupTransform( "/world", "/seabee3/current_pose" );

                // constraints met
                if( fabs( quickdev::angleBetween( unit::convert<tf::Vector3>( world_to_current_pose.getRotation() ).getZ(), unit::convert<tf::Vector3>( world_to_desired_pose.getRotation() ).getZ() ) ) < 0.05 * M_PI )
                {
                    token.complete( true );
                    return;
                }
            }

            publish_rate.sleep();
        }

        token.cancel();
    }

    quickdev::SimpleActionToken moveAtVelocity( tf::Transform const & velocity, _TermCriteria term_criteria = _TermCriteria() )
    {
        quickdev::SimpleActionToken result;
        result.start( quickdev::auto_bind( quickdev::auto_bind( &SeabeeMovementPolicy::moveAtVelocityImpl, this ), velocity, result, term_criteria ) );

        return result;
    }

    void moveAtVelocityImpl( tf::Transform const & velocity, quickdev::SimpleActionToken token, _TermCriteria term_criteria )
    {
        _TwistMsg twist_msg = unit::implicit_convert( velocity );
        ros::Rate publish_rate( 10 );

        while( ( !term_criteria || !term_criteria() ) && token.ok() && ros::ok() )
        {
            multi_pub_.publish( "/seabee3/cmd_vel", twist_msg );
            publish_rate.sleep();
        }

        token.cancel();
        multi_pub_.publish( "/seabee3/cmd_vel", _TwistMsg() );
    }

    quickdev::SimpleActionToken moveAtVelocity( Pose const & velocity )
    {
        return moveAtVelocity( unit::convert<tf::Transform>( velocity ) );
    }

    quickdev::SimpleActionToken moveAtVelocity( _TwistMsg const & velocity )
    {
        return moveAtVelocity( unit::convert<tf::Transform>( velocity ) );
    }

    void setDesiredPose( tf::Transform const & pose )
    {
        _TfTranceiverPolicy::publishTransform( pose, "/world", "/seabee3/desired_pose" );
    }

    void setDesiredPose( Pose const & pose )
    {
        setDesiredPose( unit::convert<tf::Transform>( pose ) );
    }

    // move within a certain pose error of the given target; there must exist a transform from /seabee/base_link to <target>
    quickdev::SimpleActionToken moveRelativeTo( std::string const & target, tf::Transform const & desired_distance_from_target, _TermCriteria term_criteria = _TermCriteria() )
    {
        quickdev::SimpleActionToken result;
        result.start( quickdev::auto_bind( quickdev::auto_bind( &SeabeeMovementPolicy::moveRelativeToImpl, this ), target, desired_distance_from_target, result, term_criteria ) );

        return result;
    }

    void moveRelativeToImpl( std::string const & target, tf::Transform const & desired_distance_from_target, quickdev::SimpleActionToken token, _TermCriteria term_criteria )
    {
        ros::Rate publish_rate( 10 );

        tf::Transform world_to_self = _TfTranceiverPolicy::tryLookupTransform( "/world", "/seabee3/current_pose" );
        tf::Transform world_to_target = _TfTranceiverPolicy::tryLookupTransform( "/world", target ) * desired_distance_from_target;

        while( ( !term_criteria || !term_criteria() ) && token.ok() && ros::ok() )
        {
            world_to_self = _TfTranceiverPolicy::tryLookupTransform( "/world", "/seabee3/current_pose" );
            world_to_target = _TfTranceiverPolicy::tryLookupTransform( "/world", target ) * desired_distance_from_target;

            tf::Vector3 position_error = world_to_target.getOrigin() - world_to_self.getOrigin();
            //double heading_error = 0; //unit::convert<tf::Vector3>( world_to_target.getRotation() ).getZ() - world_to_self.getRotation() ).getZ();
            double heading_error = unit::convert<tf::Vector3>( world_to_self.getRotation().inverse() * world_to_target.getRotation() ).getZ();

            if( world_to_target.getOrigin() != world_to_target.getOrigin() ) continue;

            //_TfTranceiverPolicy::publishTransform( tf::Transform( tf::Quaternion( unit::convert<tf::Vector3>( world_to_target.getRotation() ).getZ(), 0, 0 ), world_to_target.getOrigin() ), "/world", "/seabee3/desired_pose" );
            _TfTranceiverPolicy::publishTransform( tf::Transform( tf::Quaternion( unit::convert<tf::Vector3>( world_to_self.getRotation() ).getZ(), 0, 0 ), world_to_target.getOrigin() ), "/world", "/seabee3/desired_pose" );

/*
            if( position_error.length() < 0.2 && fabs( heading_error ) < M_PI_2 / 9 )
            {
                token.complete( true );

                // publish transform with current pose's rpy xy and the target frame's z
                //_TfTranceiverPolicy::publishTransform( tf::Transform( world_to_self.getRotation(), tf::Vector3( world_to_self.getOrigin().getX(), world_to_self.getOrigin().getY(), world_to_target.getOrigin().getZ() ) ), "/world", "/seabee3/desired_pose" );

                return;
            }
*/
            publish_rate.sleep();

        }

        //_TfTranceiverPolicy::publishTransform( tf::Transform( world_to_self.getRotation(), tf::Vector3( world_to_self.getOrigin().getX(), world_to_self.getOrigin().getY(), world_to_target.getOrigin().getZ() ) ), "/world", "/seabee3/desired_pose" );

        token.cancel();
    }

    quickdev::SimpleActionToken rotateSearch( quickdev::SimpleActionToken parent_token, Radian const & min, Radian const & max, Radian const & velocity, _TermCriteria term_criteria = _TermCriteria() )
    {
        quickdev::SimpleActionToken result;
        result.start( quickdev::auto_bind( quickdev::auto_bind( &SeabeeMovementPolicy::rotateSearchImpl, this ), parent_token, min, max, velocity, result, term_criteria ) );

        return result;
    }

    void rotateSearchImpl( quickdev::SimpleActionToken parent_token, double const & min, double const & max, double const & velocity, quickdev::SimpleActionToken token, _TermCriteria term_criteria )
    {
        tf::Transform const world_to_self = _TfTranceiverPolicy::tryLookupTransform( "/world", "/seabee3/current_pose" );

        ros::Rate update_rate( 20 );
        double const range = max - min;
        ros::Time const start_time = ros::Time::now();

        while( ( !term_criteria || !term_criteria() ) && parent_token.ok() && token.ok() && ros::ok() )
        {
            tf::Transform world_to_desired = _TfTranceiverPolicy::tryLookupTransform( "/world", "/seabee3/desired_pose" );

            world_to_desired.setRotation( world_to_self.getRotation() * tf::Quaternion( ( range / 2 ) * cos( ( ros::Time::now() - start_time ).toSec() * M_PI * ( velocity / range ) ), 0, 0 ) );

            _TfTranceiverPolicy::publishTransform( world_to_desired, "/world", "/seabee3/desired_pose" );

            update_rate.sleep();
        }

        if( token.ok() ) token.complete( parent_token.success() );
        else token.cancel();
    }

    //! Move to some relative position
    /*!
     * - Same as moveTo( getCurrentPose() + position )
     */
/*
    _FollowTrajectoryActionClientPolicy::_ActionToken moveTo( Position const & position )
    {
        return moveTo( getCurrentPose() + position );
    }
*/
    //! Move to some relative orientation
    /*!
     * - Same as moveTo( getCurrentPose() + orientation )
     */
/*
    _FollowTrajectoryActionClientPolicy::_ActionToken moveTo( Orientation const & orientation )
    {
        return moveTo( getCurrentPose() + orientation );
    }
*/
/*
    // #########################################################################################################################################
    //! Face the given position
    _FollowTrajectoryActionClientPolicy::_ActionToken faceTo( Position const & position );

    //! Face at the given orientation
    _FollowTrajectoryActionClientPolicy::_ActionToken faceTo( Orientation const & orientation );

    // #########################################################################################################################################
    //! Strafe around pose.position at current distance until our orientation matches pose.orientation + Degrees( 180 )
    _FollowTrajectoryActionClientPolicy::_ActionToken strafeAround( Pose const & pose );

    //! Strafe around pose.position at current distance until our orientation matches orientation
    _FollowTrajectoryActionClientPolicy::_ActionToken strafeAround( Pose const & pose, Orientation const & orientation );

    //! Strafe around pose.position at distance until ... (see above)
    _FollowTrajectoryActionClientPolicy::_ActionToken strafeAround( Pose const & pose, double distance, ... );
*/
    /* This is the client's main means of accessing and updating a policy; un-comment and change args as appropriate

    void update( args... )
    {
        // When using init(), it's useful to be able to detect whether our init() function as been called
        // One option is to simply read the member "initialized_"
        // Alternatively, there are some simple macros for notifying/responding to situations where init() has not been called
        // These macros can be used anywhere within our policy as long as our init() function has been declared with QUICKDEV_ENABLE_INIT{}
        //
        // If we're not initialized yet, warn and continue
        // QUICKDEV_CHECK_INITIALIZED();
        //
        // If we're not initialized yet, warn and return immediately
        // QUICKDEV_ASSERT_INITIALIZED();
        //
    }*/
};

#endif // SEABEE3COMMON_SEABEEMOVEMENTPOLICY_H_
