/***************************************************************************
 *  include/seabee3_common/movement.h
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

#ifndef SEABEE3COMMON_MOVEMENT_H_
#define SEABEE3COMMON_MOVEMENT_H_

#include <map>
#include <vector>
#include <array>

namespace seabee3_common
{
namespace movement
{

static unsigned int const NUM_MOTOR_CONTROLLERS = 9;
static unsigned int const NUM_THRUSTERS = 6;
namespace MotorControllerIDs
{
    static int const FWD_RIGHT_THRUSTER = 3;
    static int const FWD_LEFT_THRUSTER = 1;
    static int const DEPTH_FRONT_THRUSTER = 0; // front
    static int const DEPTH_BACK_THRUSTER = 5; // back
    static int const STRAFE_TOP_THRUSTER = 4; // top
    static int const STRAFE_BOTTOM_THRUSTER = 2; // bottom
    static int const SHOOTER = 6;
    static int const DROPPER_STAGE1 = 7;
    static int const DROPPER_STAGE2 = 8;

    static inline bool isThruster( int const & id )
    {
        return id != SHOOTER && id != DROPPER_STAGE1 && id != DROPPER_STAGE2;
    }
}

static unsigned int const NUM_FIRING_DEVICES = 4;
namespace FiringDeviceIDs
{
    static int const SHOOTER1 = 0;
    static int const SHOOTER2 = 1;
    static int const DROPPER_STAGE1 = 2;
    static int const DROPPER_STAGE2 = 3;
}

namespace Axes
{
    static int const SPEED = 0;
    static int const STRAFE = 1;
    static int const DEPTH = 2;

    static int const ROLL = 3;
    static int const PITCH = 4;
    static int const YAW = 5;

    static int const SPEED_REL = 6;
    static int const STRAFE_REL = 7;
    static int const DEPTH_REL = 8;

    static int const ROLL_REL = 9;
    static int const PITCH_REL = 10;
    static int const YAW_REL = 11;
}

namespace ThrusterPairs
{
    static std::array<std::array<int, 2>, 6> const values =
    {{
        // speed
        {{
            MotorControllerIDs::FWD_LEFT_THRUSTER,
            MotorControllerIDs::FWD_RIGHT_THRUSTER
        }},
        // strafe
        {{
            MotorControllerIDs::STRAFE_TOP_THRUSTER,
            MotorControllerIDs::STRAFE_BOTTOM_THRUSTER
        }},
        // depth
        {{
            MotorControllerIDs::DEPTH_FRONT_THRUSTER,
            MotorControllerIDs::DEPTH_BACK_THRUSTER
        }},
        // roll
        {{
            MotorControllerIDs::STRAFE_TOP_THRUSTER,
            MotorControllerIDs::STRAFE_BOTTOM_THRUSTER
        }},
        // pitch
        {{
            MotorControllerIDs::DEPTH_FRONT_THRUSTER,
            MotorControllerIDs::DEPTH_BACK_THRUSTER
        }},
        // yaw
        {{
            MotorControllerIDs::FWD_LEFT_THRUSTER,
            MotorControllerIDs::FWD_RIGHT_THRUSTER
        }}
    }};
}

} // movement
} // seabee3_common

#endif // SEABEE3COMMON_MOVEMENT_H_
