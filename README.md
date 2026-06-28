# Ignacio

Ignacio is an open source mini rover with an omnidirectional drivetrain.   
By using four 5V DC motors and special mecanum wheels, Ignacio is able 
to go forward, backwards, and strafe left/right without turning. Ignacio 
communicates with most commercially available video game controllers by 
using an open source library called Bluepad32, by Ricardo Quesada et al.

This project has a budget-friendly option for sparking interest in 
robotics for a younger audience, and it is a robust starting point for 
a more advanced robot with a simultaneous localization and mapping 
(SLAM) algorithm. By default, Ignacio comes with an ESP-32 micro controller, allowing it to interface with wifi or bluetooth devices. You may also opt for a more powerful processor (Arduino, Teensy, Raspberry Pi, NVIDIA Chipset). As a programmable device, Nacho is compatible with IMUs, dead wheel odometry, and other popular motion tracking sensors.  

# Bill of Materials

The easiest way to get all the tools and components that you need for 
this project is by purchasing them as a kit on my etsy shop below:

[ Insert Etsy Link]

If you want more of a DIY experience, you can instead buy the following parts from Amazon using my affiliate links below: 

| Description | Quantity | Cost | Link | 
| :---------- | :------: | ---: | :--- |
| Batteries & Charger | 1 | $18.99 | https://amzn.to/4tDavKv |
| Wheels & Motors | 1 | $15.50 | https://amzn.to/42jCN1w |
| Battery Hat | 1 | $9.99 | https://amzn.to/4ft6nZU |
| ESP 32 Micro controller | 1 | $8.99 | https://amzn.to/3PmH4hE |
| L298n (2- pack) | 1 | $6.98 | https://amzn.to/4ndZDBb |
| Assorted Jumper Wires | 1 | $6.98 | https://amzn.to/4exS9a0 |

You will also need a chassis, and I think making your own is a fun part of this project. If you'd prefer to buy one instead then you could use the following option instead of the "Wheels & Motors" Option above: 


# Assembly Instructions
I have posted a detailed instructional video posted on Youtube. You can watch here in the browser, or continue scrolling for written instructions.

[embed youtube video]

The easiest way to view assembly instructions for Ignacio is to look at 
the CAD via onshape. This digital model is a (nearly) 1:1 version of the 
robot as you see it in the video. 

[embed onshape link here]

Ignacio is an open source mini rover with an omnidirectional drivetrain. By using four 5V DC motors and special mecanum wheels, Ignacio is able to go forward, backwards, and strafe left/right without turning. Ignacio communicates with most commercially available video game controllers. This project is primarily designed as a budget option for sparking interest in robotics for a younger audience. Nacho is a great start for a more advanced robot with a simultaneous localization and mapping (SLAM) algorithm, as Ignacio uses a wifi-enabled microcontroller that is compatible with IMUs, dead wheel odometry, and other motion tracking sensors such as lidar & GPS.

Learn more about the project on my personal website. 

https://maxxiba.wordpress.com/Ignacio

#Trouble shooting:
    
    1. Nacho doesn't drive in the correct direction 
        
        Check out the following online resource about mecanum drive trains: 
        https://seamonsters-2605.github.io/archive/mecanum/

        Pick nacho up and rest the base on a box, allowing the wheels to freely rotate as you control him. Compare the rotation of the wheels to the rotation picture on the seamonsters GitHub to figure out if some wheels are turning
        the opposite direction. If your wheels are turning the wrong way, pick and complete ONLY ONE of the below options

        Option A: Modify lines 5-12 in the code to flip the two inputs for the wheel and reupload the code to the robot. Verify correct behavior.

	Option B: Find the wheel that turns the wrong direction. Carefully note which color wire is connected to which numbered input. Disconnect the wires, and swap the connections to reverse the motor direction. 

    2. I cannot upload the code to Nacho

        If you haven't uploaded code to an esp32 before, it can be quite tricky. Ensure that you have 