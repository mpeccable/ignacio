Ignacio

Ignacio is an open source mini rover with an omnidirectional drivetrain. By using four 5V DC motors and special mecanum wheels, Ignacio is able to go forward, backwards, and strafe left/right without turning. Ignacio communicates with most commercially available video game controllers. This project is primarily designed as a budget option for sparking interest in robotics for a younger audience, but it could also be a great start for a more advanced robot with a simultaneous localization and mapping (SLAM) algorithm, as Ignacio uses a wifi-enabled microcontroller that is compatible with IMUs, dead wheel odometry, and other motion tracking devices. 

Learn more about the project on my personal website. 

https://maxxiba.wordpress.com/Ignacio

Trouble shooting:
    
    1. Nacho doesn't drive in the correct direction 
        
        Check out the following online resource about mecanum drive trains: 
        https://seamonsters-2605.github.io/archive/mecanum/

        Pick nacho up and rest the base on a box, allowing the wheels to freely rotate as you control him. Compare the rotation of the wheels to the rotation picture on the seamonsters GitHub to figure out if some wheels are turning
        the opposite direction. If your wheels are turning the wrong way, pick and complete ONLY ONE of the below options

        Option A: Modify lines 5-12 in the code to flip the two inputs for the wheel and reupload the code to the robot. Verify correct behavior.

	Option B: Find the wheel that turns the wrong direction. Carefully note which color wire is connected to which numbered input. Disconnect the wires, and swap the connections to reverse the motor direction. 

    2. I cannot upload the code to Nacho

        If you haven't uploaded code to an esp32 before, it can be quite tricky. Ensure that you have 