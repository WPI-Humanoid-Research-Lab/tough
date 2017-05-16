#include <val_task_common/val_task_common_utils.h>
#include <tf/tf.h>
#include <val_control/val_chest_navigation.h>
#include <val_control/val_pelvis_navigation.h>
#include <val_control/val_gripper_control.h>
#include <val_control/val_head_navigation.h>
#include <val_control/val_arm_navigation.h>

bool taskCommonUtils::isPoseChanged(geometry_msgs::Pose2D pose_old, geometry_msgs::Pose2D pose_new)
{
    bool ret = false;
    if (sqrt(pow((pose_new.y - pose_old.y),2) + pow((pose_new.x - pose_old.x),2)) > DISTANCE_TOLERANCE)
    {
        ret = true;
    }
    else if (fabs(fmod(pose_new.theta,(2*M_PI)) - fmod(pose_new.theta,(2*M_PI))) > ANGLE_TOLERANCE)
    {
        ret = true;
    }

    return ret;
}


bool taskCommonUtils::isGoalReached(geometry_msgs::Pose2D pose_old, geometry_msgs::Pose2D pose_new)
{
//    return !taskCommonUtils::isPoseChanged(pose_old, pose_new);
    // cannot reuse isPoseChanged, need a larger tolerance for goal
    bool ret = true;
    if (sqrt(pow((pose_new.y - pose_old.y),2) + pow((pose_new.x - pose_old.x),2)) > GOAL_DISTANCE_TOLERANCE)
    {
        ret = false;
    }

    else if (fabs(fmod(pose_new.theta,(2*M_PI)) - fmod(pose_new.theta,(2*M_PI))) > GOAL_ANGLE_TOLERANCE)
    {
        ret = false;
    }

    return ret;
}


bool taskCommonUtils::isGoalReached(geometry_msgs::Pose pose_old, geometry_msgs::Pose2D pose_new)
{
    geometry_msgs::Pose2D pose2d_old;
    pose2d_old.x = pose_old.position.x;
    pose2d_old.y = pose_old.position.y;
    pose2d_old.theta = tf::getYaw(pose_old.orientation);
    return taskCommonUtils::isGoalReached(pose2d_old, pose_new);
}

void taskCommonUtils::moveToWalkSafePose(ros::NodeHandle nh)
{
    ROS_INFO("Moving to walk safe pose");
    chestTrajectory chest_controller(nh);
    chest_controller.controlChest(0, 0, 0, 1);
    ros::Duration(0.5).sleep();

    pelvisTrajectory pelvis_controller(nh);
    pelvis_controller.controlPelvisHeight(0.9);
    ros::Duration(0.5).sleep();

    HeadTrajectory  head_controller(nh);
    head_controller.moveHead(0, 0, 0);
//    ros::Duration(1).sleep();

    gripperControl gripper_controller(nh);
    gripper_controller.openGripper(armSide::RIGHT);
    gripper_controller.openGripper(armSide::LEFT);
//    ros::Duration(0.2).sleep();

    armTrajectory arm_controller(nh);
    arm_controller.moveToDefaultPose(armSide::RIGHT);
    arm_controller.moveToDefaultPose(armSide::LEFT);
    ros::Duration(0.5).sleep();

}



void taskCommonUtils::moveToInitPose(ros::NodeHandle nh)
{
    armTrajectory armTraj(nh);
    armTrajectory::armJointData l;
    l.side = LEFT;
    l.arm_pose = {-0.28f, -1.14f, 1.28f, -1.07f, 1.27f, 0.0f, 0.0f};
    l.time = 0.5;

    // Set the pose of the right arm to extend it to the front
    armTrajectory::armJointData r;
    r.side = RIGHT;
    r.arm_pose = {-0.28f, 1.14f, 1.28f, 1.07f, 1.27f, 0.0f, 0.0f};
    r.time = 0.5;

    // Combine the left and right arm movements
    std::vector<armTrajectory::armJointData> initPose;
    initPose.push_back(r);
    initPose.push_back(l);

    armTraj.moveArmJoints(initPose);
    ros::Duration(0.5).sleep();
}