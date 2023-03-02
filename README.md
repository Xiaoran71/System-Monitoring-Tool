
# System Monitoring Tool

This is a C program that will report different metrics of the utilization of a given system.


## How did I solve the problem

1. I set the main function so that it will accept command line arguments. The program reacts differently after receiving different arguments.

2. I divide the output into 5 sections: 
    - Output data about sample and delay. Output the memory usage of the current program.
    - Output user information.
    - Output information about the system.
    - Output used memory utilization, including physical and virtual. 
    - Output core number. Output cpu utilization.

    (I set the order of these 5 sections like this so that most of the data that don't change during iteration will be displayed first.)

3. I work on these sections one by one. I write corresponding function calls for them, and also write code in main() in order to output these information as requested.

4. Then I work on those command line argument to make them work correctly. For example, when the program receives "--sequential", the information will be output sequentially without needing to "refresh" the screen.

5. Test the program and debug.

6. Improve comments and documentation in order to make it more readable.


## Overview of the functions

** Some detailed info are written in comments of my code.

1. void get_program_mem()
    - A function to get and print the memory usage of the current program.
    - This function takes no parameters and does not return anything.

2. void get_sys_info()
    - A function to get and print the system information.
    - This function takes no parameters and does not return anything.

3. void get_core_info()
    - A function to get and print the number of cores.
    - This function takes no parameters and does not return anything.

4. double get_cpu_usage()
    - A function to get and return the cpu utilization information.
    - This function takes no parameters.
    - I use the formula: 
        cpu_utilization = |cpu_usage_2 - cpu_usage_1| / cpu_usage_1 * 100.
    -  For cpu uutilization, I want to report the difference between how much the cpu worked in one instance of time and the next. To achieve this, I get the first cpu data, wait for tdelay time, and get the second cpu data. Then I subtract the second cpu data from the first cpu data to show their difference. At last I divide the difference value by the first cpu data and multiply by 100 to get the result in %.

5. void get_sys_mem()
    -  A function to get and print the system memory utilization information, including (used and total) physical and virtual memory.
    - This function takes no parameters and does not return anything.
    - Detailed explaination are in comments.

6. int get_user_info()
    - A function to get and print the information of users connected to the system. 
    - This function takes no parameters.
    - It return 0 if open the file successfully, return 1 otherwise.

7. int main(int argc, char *argv[])
    - This main function can take 5 command line argument and output data accordingly: --system, --user, --sequential --samples, --tdelay.
    - This funtion return 1 if everything goes successfully, return 1 otherwise.
    - If you use --system and --user arguments at the same time, a message will be displayed and the fuction will returns 1 immediately.
    - Detailed explaination are in comments.

    
## How to run my program

- You can choose to use command line arguments: --system, --user, --sequential --samples, --tdelay. They work just as what is said in the assignment descrption. You can change their order. The last two arguments can also be considered as positional arguments if not flag is indicated in the corresponding order: samples tdelay.
- If you use --system and --user arguments at the same time, a message will be displayed and the main fuction will end immediately.


## Other

- I wrote a different version of this program that could display the information in the same order as what was shown in the video demo. It worked well when I connect to the BV473 lab and ran it on the terminal of Visual Studio Code. However, when I connected to the lab and ran on the Terminal on Mac, the ouput became disorganized.
- To avoid uncertainty, I change the output sections order so that most of the data that don't change alongside iteration will be displayed first. Then used memory utilization will be displayed. Finally it outputs core number and cpu utilization.

