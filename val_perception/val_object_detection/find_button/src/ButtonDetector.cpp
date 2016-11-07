
#include <find_button/ButtonDetector.h>
using namespace cv;
using namespace std;
using namespace src_perception;

ButtonDetector(src_perception::MultisenseImage *handle, ros::NodeHandle &rosHandle );
{
	assert(handle != nullptr && "Multisesnse pointer is null");
	mi = handle;
    nh = rosHandle;
    //mi(handle);
    pubButtonCenter = nh.advertise<geometry_msgs::PointStamped> ("/buttonCenter", 1);
}


bool ButtonDetector::processImage()
{


	cv::Mat src;
    if(mi->giveImage(src))
    {
        Mat dst;
        flip(src,dst,-1);
        Mat3b hsv;
        cvtColor(dst, hsv, COLOR_BGR2HSV);
        // detect red color in hsv image
        Mat1b mask1, mask2;
        inRange(hsv, Scalar(0, 178, 51), Scalar(5, 255, 128), mask1);
        inRange(hsv, Scalar(170, 204, 140), Scalar(180, 255, 191), mask2);

        Mat1b mask = mask1 | mask2; 
        imshow("Mask", mask);

        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        // find contours
        findContours( mask, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
        RNG rng(12345);
        int largest_area=0;
        int largest_contour_index=0;
        Rect bounding_rect;
        /// Draw contours
        Mat drawing = Mat::zeros( mask.size(), CV_8UC3 );
        vector<Point2f> points;

        for( int i = 0, j = 0; i< contours.size(); i++ )
        {
            //  Find the area of contour
            double a=contourArea( contours[i],false); 
            if(a>largest_area){
                largest_area=a;
                //cout<<i<<" area  "<<a<<endl;
                // Store the index of largest contour
                largest_contour_index=i;               
                // Find the bounding rectangle for biggest contour
                bounding_rect=boundingRect(contours[i]);
            }
            Moments moment = moments((Mat)contours[i]);

            if (moment.m00)
            {
                points.push_back(Point2f(moment.m10/moment.m00,moment.m01/moment.m00));
                std::cout<<"m00"<<moment.m00<<"m10"<<moment.m10<<"m01"<<moment.m01;
                std::cout<<"x:"<<points[j].x<<" y:"<<points[j].y<<"\n";
                buttonCenter.x = points[j].x;
                buttonCenter.y = points[j].y;
                j++;
                getLocation(buttonCenter,dst);
                return true;
            }
        }
 
    }
    return false;
    	
}


bool ButtonDetector::buttonDetected()
{


}

void ButtonDetector::getLocation(geometry_msgs::Point index, cv::Mat src )
{


	StereoPointCloudColor::Ptr organized_cloud(new StereoPointCloudColor);
    geometry_msgs::PointStamped location;

	cv::Mat_<float> disp;
	cv::Mat_<double> Q;

	bool valid_Q=false;
	bool new_color=false;
	bool new_disp=false;

	if(ros::ok())
	{
        
		if(mi->giveDisparityImage(disp))
		{
			new_disp=true;
		}

		if(new_disp&&new_color)
		{
			if(!mi->giveQMatrix(Q))
			{
				ros::spinOnce();
			}
			std::cout<<disp.cols<<" x "<<disp.rows<<std::endl;
			PointCloudHelper::generateOrganizedRGBDCloud(disp,src,Q,organized_cloud);
			ROS_INFO_STREAM("Organized cloud size: "<<organized_cloud->size());
			
			// output.header.frame_id=std::string("left_camera_optical_frame");
			// output.header.stamp=ros::Time::now().toNSec();


		}
		
	}

    pcl::PointXYZRGB point = organized_cloud->at(index.x,index.y);
    location.point.x = point.x;
    location.point.y = point.y;
    location.point.z = point.z;
    location.header.frame_id = std::string("left_camera_optical_frame");
    //location.header.stamp = ros::Time::now().toNSec();
    pubButtonCenter.publish(location);



}