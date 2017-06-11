#include <val_task3/climb_stairs.h>

climbStairs::climbStairs(ros::NodeHandle nh): nh_(nh)
{
    walker_ = new ValkyrieWalker(nh_, 0.8f, 0.8f, 0, 0.3);
    chest_  = new chestTrajectory(nh_);
    pelvis_ = new pelvisTrajectory(nh_);
    arm_    = new armTrajectory(nh_);
}

climbStairs::~climbStairs()
{
    if (walker_ != nullptr) delete walker_;
    if (chest_ != nullptr) delete chest_;
    if (pelvis_ != nullptr) delete pelvis_;
    if (arm_ != nullptr) delete arm_;
}

void climbStairs::climb_stairs()
{
    // set the chest orientation
    chest_->controlChest(0,20,0);
    ros::Duration(1).sleep();

    // set the arms orientation
    arm_->moveArmJoint(RIGHT, 1, 0.7);
    ros::Duration(1).sleep();
    arm_->moveArmJoint(LEFT, 1, -0.7);
    ros::Duration(1).sleep();

    // first step
    walker_->walkNSteps(1, 0.3, 0.0, true, RIGHT);
    ros::Duration(1).sleep();
    pelvis_->controlPelvisHeight(1.0);
    ros::Duration(1).sleep();
    walker_->load_eff(armSide::RIGHT, EE_LOADING::LOAD);
    walker_->walkNSteps(1, 0.3, 0.0, true, LEFT);

    // next 8 steps
    for (int i=0; i<8; i++)
    {
        ros::Duration(1).sleep();
        pelvis_->controlPelvisHeight(1.0);
        ros::Duration(1).sleep();
        walker_->load_eff(armSide::LEFT, EE_LOADING::LOAD);
        walker_->walkNSteps(1, 0.25, 0.0, true, RIGHT);
        ros::Duration(1).sleep();
        pelvis_->controlPelvisHeight(1.0);
        ros::Duration(1).sleep();
        walker_->load_eff(armSide::RIGHT, EE_LOADING::LOAD);
        walker_->walkNSteps(1, 0.25, 0.0, true, LEFT);
    }
}
