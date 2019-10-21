#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include <iostream>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    //ROS_INFO_STREAM("Moving to robot towards the ball");
	//std::cout << "lin_x: " << lin_x << '\n';
	//std::cout << "ang_z: " << ang_z << '\n';
    // Request linear and angular velocity
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;
	
    // Call the DriveToTarget service and pass the requested joint angles
	//client.call(srv);
    if (!client.call(srv)) {
        ROS_ERROR("Failed to call service DriveToTarget");
	}
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    
    //std::cout << img.data;
	//std::cout << "img width: " << img.width << '\n';
	//std::cout << "img height: " << img.height << '\n';
	//std::cout << "img step: " << img.step << '\n';
	//std::cout << "img size: " << img.data.size() << '\n';
	//std::cout << "img size: " << img.data.size() << '\n';

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    int row = 0;
	int step = 0;
	bool ball_detected = false;
    // Scan middle section of image since ball is level with camera
	// Image height is 800
	for (row = 390; row < 410 && ball_detected == false; row++) {
		for (step = 0; step < img.step && ball_detected == false; step+=3)
		{
			//for (int i = 0; i < (img.width * img.step); i++) {
			//std::cout << "step to 2400" << step << '\n';
			int r_chan = img.data[row * img.step + step];
			int g_chan = img.data[row * img.step + step+1];
			int b_chan = img.data[row * img.step + step+2];
		
			//std::cout << "img.data[i]" << int(img.data[i]) << '\n';

			if (r_chan + g_chan + b_chan == 255*3) 
			//if (img.data[i] == 255) 
			{
				ball_detected = true;
			}
			
		}	
    }
    
	if (ball_detected)
	{
		float x_image_position = (float(step) / float(img.step));
		//std::cout << "x_image_position: " << x_image_position << '\n';
		//ROS_INFO("White ball rel pixel position: %1.2f", (float)x_image_position);
		// map % image location to steer left (positive), straight, right
		float drive_angle = -(x_image_position-0.5);
		//ROS_INFO("Drive Robot Request Ang:%1.2f", (float)drive_angle);
		drive_robot(0.1, drive_angle);
	}
	else
	{
		drive_robot(0.0,0.0);	
    }
        
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
