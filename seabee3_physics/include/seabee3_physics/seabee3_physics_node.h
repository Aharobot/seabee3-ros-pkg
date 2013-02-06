/***************************************************************************
 *  include/seabee3_physics/seabee3_physics_node.h
 *  --------------------
 *
 *  Copyright (c) 2013, Edward T. Kaszubski ( ekaszubski@gmail.com ), Dhruv Monga ( dhruvmonga@gmail.com )
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

#ifndef SEABEE3PHYSICS_SEABEE3PHYSICSNODE_H_
#define SEABEE3PHYSICS_SEABEE3PHYSICSNODE_H_

#include <quickdev/node.h>

// policies
#include <quickdev/reconfigure_policy.h>
#include <quickdev/tf_tranceiver_policy.h>

// objects
#include <quickdev/time.h>
#include <quickdev/multi_publisher.h>
#include <quickdev/multi_subscriber.h>
#include <quickdev/threading.h>

// utils
#include <seabee3_common/movement.h>
#include "btBulletDynamicsCommon.h"
#include <Eigen/Core>

// msgs
#include <seabee3_msgs/MotorVals.h>
#include <seabee3_msgs/KillSwitch.h>

// cfgs
#include <seabee3_physics/Seabee3PhysicsConfig.h>

using namespace seabee3_common;
using namespace Eigen;

typedef seabee3_msgs::MotorVals _MotorValsMsg;
typedef seabee3_msgs::KillSwitch _KillSwitchMsg;

typedef seabee3_physics::Seabee3PhysicsConfig _Seabee3PhysicsConfig;

typedef quickdev::ReconfigurePolicy<_Seabee3PhysicsConfig> _Seabee3PhysicsLiveParams;
typedef quickdev::TfTranceiverPolicy _TfTranceiverPolicy;

QUICKDEV_DECLARE_NODE( Seabee3Physics, _Seabee3PhysicsLiveParams, _TfTranceiverPolicy )

QUICKDEV_DECLARE_NODE_CLASS( Seabee3Physics )
{
    ros::MultiPublisher<> multi_pub_;
    ros::MultiSubscriber<> multi_sub_;

    boost::shared_ptr<btBroadphaseInterface> broadphase_;
    boost::shared_ptr<btDefaultCollisionConfiguration> collision_configuration_;
    boost::shared_ptr<btCollisionDispatcher> dispatcher_;
    boost::shared_ptr<btSequentialImpulseConstraintSolver> solver_;
    boost::shared_ptr<btDiscreteDynamicsWorld> dynamics_world_;
    boost::shared_ptr<btDefaultMotionState> seabee_motion_state_;
    boost::shared_ptr<btRigidBody> seabee_body_;
    boost::shared_ptr<btCollisionShape> seabee_shape_;

    std::map<std::string, boost::shared_ptr<btCollisionShape> > structure_dynamics_map_;
    std::map<std::string, boost::shared_ptr<btCollisionShape> > structures_map_;
    std::map<std::string, XmlRpc::XmlRpcValue> structure_shapes_map_;
    std::map<std::string, double> structures_mass_;
    std::map<std::string, btTransform> transforms_map_;

    btScalar seabee_mass;

    quickdev::Timer timer_;

    std::vector<btTransform> thruster_transforms_;
    std::vector<int> thruster_values_;
    bool is_killed_;

    QUICKDEV_DECLARE_NODE_CONSTRUCTOR( Seabee3Physics ),
        thruster_transforms_( movement::NUM_THRUSTERS ),
        thruster_values_( movement::NUM_THRUSTERS ),
        is_killed_( true )
    {
        //
    }

    boost::shared_ptr<btCollisionShape> makeBtCollisionShape( XmlRpc::XmlRpcValue && value )
    {
        boost::shared_ptr<btCollisionShape> result;

        std::string const type = value["type"];

        std::cout << "creating collision shape: [ " << type << " ]" << std::endl;

        if( type == "box" )
        {
            double const length = value["length"];
            double const width = value["width"];
            double const height = value["height"];

            result = boost::shared_ptr<btCollisionShape>( new btBoxShape( btVector3( length / 2, width / 2, height / 2 ) ) );
        }

        if( type == "cylinder" )
        {
            double const length = value["length"];
            double const width = value.hasMember( "width" ) ? double( value["width"] ) : 2 * double( value["radius"] );
            double const height = value.hasMember( "height" ) ? double( value["height"] ) : 2 * double( value["radius"] );

            result = boost::shared_ptr<btCollisionShape>( new btCylinderShapeX( btVector3( length / 2, width / 2, height / 2 ) ) );
        }

        // if( result ) result->calculateLocalInertia( double( value["mass"] ), btVector3( 0, 0, 0 ) );

        return result;
    }

    QUICKDEV_SPIN_FIRST()
    {
        QUICKDEV_GET_RUNABLE_NODEHANDLE( nh_rel );

        _Seabee3PhysicsLiveParams::registerCallback( quickdev::auto_bind( &Seabee3PhysicsNode::reconfigureCB, this ) );

        multi_sub_.addSubscriber( nh_rel, "motor_vals", &Seabee3PhysicsNode::motorValsCB, this );
        multi_sub_.addSubscriber( nh_rel, "kill_switch", &Seabee3PhysicsNode::killSwitchCB, this );

        // read in structures from parameter server
        auto structures_param = quickdev::ParamReader::readParam<XmlRpc::XmlRpcValue>( nh_rel, "structures" );
        // read in structure dynamics from parameter server
        auto structure_dynamics_param = quickdev::ParamReader::readParam<XmlRpc::XmlRpcValue>( nh_rel, "structure_dynamics" );

        // build map of collision shapes from structure dynamics parameter
        PRINT_INFO( "Building map of collision shapes" );
        for( auto structure_dynamics_it = structure_dynamics_param.begin(); structure_dynamics_it != structure_dynamics_param.end(); ++structure_dynamics_it )
        {
            structures_mass_[structure_dynamics_it->first] = structure_dynamics_it->second["mass"];
            structure_dynamics_map_[structure_dynamics_it->first] = makeBtCollisionShape( structure_dynamics_it->second );
        }

        // build map of named collision shapes and corresponding transforms from structures parameter
        PRINT_INFO( "Building map of named shapes" );
        for( auto structures_it = structures_param.begin(); structures_it != structures_param.end(); ++structures_it )
        {
            // look up the dynamics entry for this structures_it
            auto structure_dynamics_it = structure_dynamics_map_.find( structures_it->second["dynamics"] );
            // get the shape entry for this structure
//            auto structure_shape_it = structure_dynamics_param[structures_it->second["dynamics"]];
            // if it exists
            if( structure_dynamics_it != structure_dynamics_map_.end() )
            {
                structures_map_[structures_it->first] = structure_dynamics_it->second;
//                structure_shapes_map_[structures_it->first] = structure_shape_it->second;

                btTransform current_transform = btTransform( btQuaternion( 0, 0, 0, 1 ), btVector3( 0, 0, 0 ) );

                // attempt to look up the transform from the parent of this structure to this structure
                if( structures_it->second.hasMember( "parent" ) )
                {
                    std::string const parent_name = structures_it->second["parent"];
                    std::string const from_frame = structures_param[parent_name]["frame"];
                    std::string const to_frame = structures_it->second["frame"];
                    current_transform = _TfTranceiverPolicy::tryLookupTransform( from_frame, to_frame );
                }

                transforms_map_[structures_it->first] = current_transform;
            }
        }

        // Build the broadphase; this object helps determine which objects are about to collide
        broadphase_ = quickdev::make_shared( new btDbvtBroadphase() );

        // Set up the collision configuration and dispatcher
        collision_configuration_ = quickdev::make_shared( new btDefaultCollisionConfiguration() );
        dispatcher_ = quickdev::make_shared( new btCollisionDispatcher( collision_configuration_.get() ) );

        // The actual physics solver
        solver_ = quickdev::make_shared( new btSequentialImpulseConstraintSolver );

        // The world
        dynamics_world_ = quickdev::make_shared( new btDiscreteDynamicsWorld( dispatcher_.get(), broadphase_.get(), solver_.get(), collision_configuration_.get() ) );
        dynamics_world_->setGravity( btVector3( 0, 0, 0 ) );

        // Seabee's Body
        seabee_shape_ = structures_map_["hull"];

        seabee_motion_state_ = quickdev::make_shared( new btDefaultMotionState( btTransform( btQuaternion( 0, 0, 0, 1 ), btVector3( 0, 0, -0.1 ) ) ) );

        seabee_mass = structures_mass_["hull"];
        std::cout << "hull has mass: " << seabee_mass << std::endl;
        btVector3 seabee_inertia( 0, 0, 0 );
        seabee_shape_->calculateLocalInertia( seabee_mass, seabee_inertia );

        btRigidBody::btRigidBodyConstructionInfo seabee_body_construction_info( seabee_mass, seabee_motion_state_.get(), seabee_shape_.get(), seabee_inertia );
        seabee_body_ = quickdev::make_shared( new btRigidBody( seabee_body_construction_info ) );
        // prevent deactivation of seabee at low velocities
        seabee_body_->setActivationState( DISABLE_DEACTIVATION );
        dynamics_world_->addRigidBody( seabee_body_.get() );

        // Seabee's initial position
        //btTransform trans;
        //seabee_body_->getMotionState()->getWorldTransform(trans);
        //seabee_body_->translate( btVector3( 0, 0, -0.534973 - trans.getOrigin().getZ() ) );

        timer_.reset();

        initPolicies<quickdev::policy::ALL>();

        // attempt to fetch thruster transforms
        if( !quickdev::tryFunc( quickdev::auto_bind( &Seabee3PhysicsNode::fetchThrusterTransforms, this ), 10, 500000 ) ) RunablePolicy::interrupt();
    }

    void fetchThrusterTransforms()
    {
        // Fill in all of our thruster transforms
        for( size_t i = 0; i < movement::NUM_THRUSTERS; ++i )
        {
            if( !movement::MotorControllerIDs::isThruster( i ) ) continue;

            std::ostringstream stream;
            stream << "/seabee3/thruster" << ( i + 1 );

            thruster_transforms_[i] = lookupTransform( "/seabee3/hull", stream.str() );
        }
    }

    QUICKDEV_SPIN_ONCE()
    {

        auto const is_killed = is_killed_ && config_.is_killed;

        btVector3 const & linear_velocity = seabee_body_->getLinearVelocity();
        btVector3 const & angular_velocity = seabee_body_->getAngularVelocity();
        btVector3 buoyant_force( 0, 0, 0 );
        btVector3 gravity_force( 0, 0, 0 );
        btVector3 gravity_vector(0, 0, -9.8 );

        // model the entire vehicle as a cylinder
        double const vehicle_radius = 0.1000125;
        btVector3 const vehicle_dims( 0.3302, 2 * vehicle_radius, 2 * vehicle_radius );
        double const vehicle_volume = vehicle_dims.getX() * vehicle_dims.getY() * vehicle_dims.getZ() + config_.extra_buoyant_volume;

        ROS_INFO( "linear_velocity: %f, %f, %f", linear_velocity.x(), linear_velocity.y(), linear_velocity.z() );
        ROS_INFO( "angular_velocity: %f, %f, %f", angular_velocity.x(), angular_velocity.y(), angular_velocity.z() );

        btTransform world_transform;

        seabee_body_->clearForces();

        seabee_body_->getMotionState()->getWorldTransform( world_transform );

        // apply forces from thrusters
        if( !is_killed )
        {
            for ( size_t i = 0; i < movement::NUM_THRUSTERS; i++ )
            {
                if( !movement::MotorControllerIDs::isThruster( i ) ) continue;

                auto const thrust_force = is_killed ? 0.0 : thruster_values_.at( i ) * config_.motor_val_to_force;

                auto const & current_thruster_tf = thruster_transforms_.at(i);

                // our thrust comes out of the x-axis of the motor
                btVector3 const force_vec( -thrust_force, 0, 0 );

                // the position of the thruster with respect to the center of the sub
                btVector3 const & force_rel_pos = current_thruster_tf.getOrigin();

                // the direction of the thruster with respect to the world
                btTransform const force_orientation_tf( world_transform.getRotation() * current_thruster_tf.getRotation() );

                // the final, rotated force
                btVector3 const rotated_force( force_orientation_tf * force_vec );

                // the orientation of the hull
                btTransform const hull_orientation_tf( world_transform.getRotation() );

                // the position of the thruster on the hull (taking into account the hull's orientation)
                btVector3 const rotated_force_rel_pos( hull_orientation_tf * force_rel_pos );

                ROS_INFO( "MOTOR%Zu THRUST: %f (%f,%f,%f) @ (%f,%f,%f) : (%f, %f, %f)",
                          i,
                          thrust_force,
                          force_vec.getX(),
                          force_vec.getY(),
                          force_vec.getZ(),
                          force_rel_pos.getX(),
                          force_rel_pos.getY(),
                          force_rel_pos.getZ(),
                          rotated_force.getX(),
                          rotated_force.getY(),
                          rotated_force.getZ() );

                // apply @rotated_force at @force_rel_pos
                seabee_body_->applyForce( rotated_force, rotated_force_rel_pos );
            }
        }

        //if( config_.gravity_enabled )
        //{
        ////dynamics_world_->setGravity( btVector3( 0, 0, -9.8 ) );
        ////seabee_body_->applyForce( btVector3( 0, 0, -9.8 ), btVector3( 0, 0, 0) );
        ////dynamics_world_->setGravity( btVector3( 0, 0, 0 ) );
        ////}
        //else
        //{
        ////dynamics_world_->setGravity( btVector3( 0, 0, 0 ) );
        ////seabee_body_->applyForce( btVector3( 0, 0, 0 ), btVector3( 0, 0, 0) );
        //gravity_force.setZ( 0 );
        //}

        // Apply buoyant force in upward direction, this is a temporary simple model of the actual buoyancy
        // Specifically, we assume the vehicle is cylidrical, then look at the volume of the vehicle below the water, which will be the same as
        // the volume of water displaced by the vehicle. Eventually, to maximize the accuracy of this calculation, we want to model each
        // component of the hull (in terms of volume and mass) and apply buoyant forces at the center of each of those components
        //if( config_.buoyancy )
        //{

        gravity_force.setZ( gravity_vector.getZ() * seabee_mass );

        double const water_density = 1000;

        double volume_displaced = 0;

        // fully below water
        if( world_transform.getOrigin().getZ() < (-vehicle_dims.getZ() / 2 ) )
        {
            volume_displaced = vehicle_volume;
        }
        // fully above water
        else if( world_transform.getOrigin().getZ() > (vehicle_dims.getZ() / 2 ) )
        {
            volume_displaced = 0;
        }
        // somewhere in-between
        else
        {
            volume_displaced = ( ( vehicle_dims.getZ() / 2 - world_transform.getOrigin().getZ() ) / vehicle_dims.getZ() ) * vehicle_volume;
        }

        // density of water * acceleration due to gravity * volume of water displaced
        // we're assuming a calm surface, so we can make the buoyant force point straight up
        buoyant_force.setZ( -1 * water_density * gravity_vector.getZ() * volume_displaced );

        ROS_INFO( "Buoyancy force: %f; volume displaced / max volume displaced: %f / %f", buoyant_force.getZ(), volume_displaced, vehicle_volume );

        // apply the buoyant force at the center of the vehicle
        //seabee_body_->applyForce( buoyant_force, btVector3( 0, 0, 0 ) );
        //}

        // sum the gravity and buouyancy vectors to get net force on submarine
        btVector3 net_buoyancy_force = gravity_force + buoyant_force;

        ROS_INFO( "Buoyancy force: %f, gravity: %f, total buoyant force from gravity and buoyancy: %f", buoyant_force.getZ(), gravity_force.getZ(), net_buoyancy_force.getZ() );

        seabee_body_->applyForce( net_buoyancy_force, btVector3( 0, 0, 0 ) );

        if( config_.drag_enabled )
        {
            auto const num_samples = config_.angular_drag_num_samples;
            // x-y plane
            {
                for( int i = 0; i < num_samples; ++i )
                {
                    Eigen::Vector2d sample_pt_u( vehicle_dims.getX() / 2, -( 2 * i + 1 ) * vehicle_dims.getY() / ( 4 * num_samples ) );
                    Eigen::Vector2d sample_pt_v( ( 2 * i + 1 ) * vehicle_dims.getX() / ( 4 * num_samples ), vehicle_dims.getY() / 2 );

                    Eigen::Matrix2d normal_tf; //( 0, 1, -1, 0 );
                    normal_tf << 0, -1, 1, 0;

                    Eigen::Vector2d sample_vec_u = normal_tf * sample_pt_u;
                    sample_vec_u.normalize();
                    Eigen::Vector2d sample_vec_v = normal_tf * sample_pt_v;
                    sample_vec_v.normalize();

                    double const angular_velocity_axis = angular_velocity.getZ();
                    double const angular_direction = angular_velocity_axis >= 0 ? 1 : -1;

                    btVector3 drag_vec_u( angular_direction * -sample_vec_u.x() * pow( angular_velocity_axis * 2 * M_PI * sample_pt_u.norm(), 2 ) * config_.angular_drag_constant, 0, 0 );
                    btVector3 drag_vec_v( 0, angular_direction * -sample_vec_v.y() * pow( angular_velocity_axis * 2 * M_PI * sample_pt_v.norm(), 2 ) * config_.angular_drag_constant, 0 );
                    btVector3 drag_pt_u( sample_pt_u.x(), sample_pt_u.y(), 0 );
                    btVector3 drag_pt_v( sample_pt_v.x(), sample_pt_v.y(), 0 );

                    //printf( "ang drag force u: ( %f %f )[%f] @ ( %f %f ); ( %f %f %f ) @ ( %f %f %f )\n", sample_vec_u.x(), sample_vec_u.y(), sample_vec_u.norm(), sample_pt_u.x(), sample_pt_u.y(), drag_vec_u.x(), drag_vec_u.y(), drag_vec_u.z(), drag_pt_u.x(), drag_pt_u.y(), drag_pt_u.z() );
                    //printf( "ang drag force v: ( %f %f )[%f] @ ( %f %f ); ( %f %f %f ) @ ( %f %f %f )\n", sample_vec_v.x(), sample_vec_v.y(), sample_vec_v.norm(), sample_pt_v.x(), sample_pt_v.y(), drag_vec_v.x(), drag_vec_v.y(), drag_vec_v.z(), drag_pt_v.x(), drag_pt_v.y(), drag_pt_v.z() );

                    seabee_body_->applyForce( drag_vec_u, drag_pt_u );
                    seabee_body_->applyForce( -drag_vec_u, -drag_pt_u );

                    seabee_body_->applyForce( drag_vec_v, drag_pt_v );
                    seabee_body_->applyForce( -drag_vec_v, -drag_pt_v );
                }
            }

            // apply linear drag force on a per-axis basis
            btVector3 const linear_velocity_unit_vec = linear_velocity.length() ? linear_velocity.normalized() : btVector3( 0, 0, 0 );

            btVector3 drag_force = linear_velocity;
            drag_force *= drag_force;
            drag_force *= -config_.drag_constant;

            if( linear_velocity.length() != 0 )
            {
                seabee_body_->applyForce( drag_force * linear_velocity_unit_vec, btVector3( 0, 0, 0 ) );
            }

            ROS_INFO( "Drag Force: %f ... Sub Speed: %f", drag_force.length(), linear_velocity.length() );

        }

        auto const & dt = timer_.update();

        // Step the physics simulation; increase the simulation's time by dt (dt is variable); interpolate by up to 2 steps when dt is not the ideal @loop_rate_seconds_ time
        dynamics_world_->stepSimulation( dt, 10, getLoopRateSeconds() );

        btVector3 const & final_linear_velocity = seabee_body_->getLinearVelocity();

        ROS_INFO( "final linear_velocity: %f, %f, %f", final_linear_velocity.x(), final_linear_velocity.y(), final_linear_velocity.z() );


        ROS_INFO( "Body Pos: %f %f %f\n",
                world_transform.getOrigin().getX(),
                world_transform.getOrigin().getY(),
                world_transform.getOrigin().getZ() );

        _TfTranceiverPolicy::publishTransform( world_transform, "/world", "/seabee3/physics/pose" );

    }

    QUICKDEV_DECLARE_MESSAGE_CALLBACK( motorValsCB, _MotorValsMsg )
    {
        for( size_t i = 0; i < msg->mask.size(); ++i )
        {
            if( movement::MotorControllerIDs::isThruster( i ) && msg->mask.at( i ) ) thruster_values_[i] = msg->motors.at( i );
        }
    }

    QUICKDEV_DECLARE_RECONFIGURE_CALLBACK( reconfigureCB, _Seabee3PhysicsConfig )
    {
        //
    }

    QUICKDEV_DECLARE_MESSAGE_CALLBACK( killSwitchCB, _KillSwitchMsg )
    {
        is_killed_ = msg->is_killed;
    }
};

#endif // SEABEE3PHYSICS_SEABEE3PHYSICSNODE_H_
