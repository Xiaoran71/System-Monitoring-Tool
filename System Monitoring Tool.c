#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <utmp.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

/** A function to get and print the memory usage of the current program. */
void get_program_mem(){
    struct rusage program_usage;

    getrusage(RUSAGE_SELF, &program_usage);
    printf(" Memory usage: %ld kilobytes\n",program_usage.ru_maxrss);
}

/** A function to get and print the system information. */
void get_sys_info() {
    struct utsname sys_data;

    uname(&sys_data);
    printf("### System Information ###\n");
    printf(" System Name = %s\n", sys_data.sysname);
    printf(" Machine Name = %s\n", sys_data.nodename);
    printf(" Version = %s\n", sys_data.release);
    printf(" Release = %s\n", sys_data.version);
    printf(" Architecture = %s\n", sys_data.machine);
    printf("---------------------------------------\n");
}

/** A function to get and print the number of cores. */
void get_core_info(){
    char buffer[1024];
    int cores = 0;
    FILE *fp = fopen("/proc/cpuinfo", "r");

    // Handle error when we cannot successfully open the file.
    if (fp == NULL) {
        perror("Error opening file");
        exit(1);
    }

    while (fgets(buffer, 1024, fp) != NULL) {
        if (strstr(buffer, "cpu cores") != NULL) {
            sscanf(buffer, "cpu cores\t: %d", &cores);
            break;
        }
    }

    fclose(fp);
    printf("Number of cores: %d\n", cores);
}

/** A function to get and return the cpu utilization information. */
double get_cpu_usage(){
    FILE *fp;
    char line[100];
    unsigned long long int user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

    // Read the "/proc/stat" file and get the data we want about cpu usage.
    fp = fopen("/proc/stat", "r");
    fgets(line, 100, fp);
    fclose(fp);
    sscanf(line, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);

    // Use the data we obtain to calculate cpu utilization:
    // First, cuculate the current total total utilization value and total idle value.
    unsigned long long int total = user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
    unsigned long long int total_idle = idle + iowait;

    // Second, use static variables to store the utilization data from the previous iteration.
    // When we first call this function, these value would be 0 which is bad for calculation,
    // so I will calculate cpu usage starting from the second time we call this function.
    static unsigned long long int previous_total = 0, previous_idle = 0;

    //Third, calculate the used parts by subtracting idle.
    unsigned long long int current_used = total - total_idle;
    unsigned long long int previous_used = previous_total - previous_idle;

    // Forth, store the current utilization value in order to use them in next iteration.
    previous_total = total;
    previous_idle = total_idle;
    
    // Calculate cpu utilization using formula and return it.
    return (double)(current_used - previous_used) / previous_used * 100.0;
}

/** A function to get and print the system memory utilization information, 
 * including (used and total) physical and virtual memory.
*/
void get_sys_mem() {
    struct sysinfo info;
    sysinfo(&info);

    // Calculate total memory.
    long phy_total = info.totalram * info.mem_unit;

    // Calculate used physical memory by subtracting free memory from total memory.
    long freeRam = info.freeram * info.mem_unit;
    long phy_used = phy_total - freeRam;

    // Calculate total virtual memory by adding physical memory and swap space.
    long vir_total = (info.totalram + info.totalswap) * info.mem_unit;

    // Calculate used virtual memory by subtracting free memory and free swap space
    // from total virtual memory.
    long vir_free = info.freeswap;
    long vir_used = vir_total - vir_free - freeRam;

    // Transfer to GB.
    double phy_total_gb = (double) phy_total / (1024 * 1024 * 1024);
    double phy_used_gb = (double) phy_used / (1024 * 1024 * 1024);
    double vir_total_gb = (double) vir_total / (1024 * 1024 * 1024);
    double vir_used_gb = (double) vir_used / (1024 * 1024 * 1024);

    printf("%.2f GB / %.2f GB  --  %.2f GB / %.2f GB\n", phy_used_gb, phy_total_gb, vir_used_gb, vir_total_gb);
}

/** A function to get and print the information of users connected to the system. 
 * It return 0 if open the file successfully, return 1 otherwise.
*/
int get_user_info() {
    struct utmp connected;
    int utmpfd;
    int len = sizeof(connected);

    // Handle error when we having trouble opening the file.
    if ((utmpfd = open(_PATH_UTMP, O_RDONLY)) == -1) {
        perror("open");
        return 1;
    }

    while (read(utmpfd, &connected, len) == len) {
        if (connected.ut_type == USER_PROCESS) {
            printf("\033[s");
            printf("%s", connected.ut_user);
            printf("\033[u");
            printf("\033[15C"); // Keep users' IP Adress in vertical alignment.
            printf("%s (%s)\n",connected.ut_line, connected.ut_host);
        }
    }
    close(utmpfd);

    return 0;
}


int main(int argc, char *argv[]) {

    // Set command line arguments.
    int system = 0, user = 0, sequential = 0, samples = 0, tdelay = 0;
    int samplesV = 10, tdelayV = 1;
    int opt;

    static struct option long_options[] = {     
        {"samples", required_argument, 0, '1'},
        {"tdelay", required_argument, 0, '2'},
        {"system", no_argument, 0, '3'},
        {"user", no_argument, 0, '4'},
        {"sequential", no_argument, 0, '5'},
        {0, 0, 0, 0}
    };

    // Set what will happen after we get certain command line argument.
    while ((opt = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (opt) {
            case '1':
                samples = 1;
                samplesV = atoi(optarg);
                break;
            case '2':
                tdelay = 1;
                tdelayV = atoi(optarg);
                break;
            case '3':
                system = 1;
                break;
            case '4':
                user = 1;
                break;
            case '5':
                sequential = 1;
                break;                
            default:
                fprintf(stderr, "Usage: %s [--samples samplesV] [--tdelay tdelayV] [--system] [--user] [--sequential]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // When no flag is given, samples and tdelay can also be considered as positional arguments.
    // Optind represents the position of file name in argv[], so here we are considering the situation
    // that there is given arguments with no flag (other than the file name).
    if (!samples && optind < argc) {
        samplesV = atoi(argv[optind++]); // Here argv[optind++] points to the former arguments with no flag.
        samples = 1;
    }

    // I set the value tdelayV after samplesV because the optind would be bigger by one here, which satisfies 
    // the assignment descrption. (--samples and --tdelay to be indicated in the corresponding order: samples tdelay.)
    if (!tdelay && optind < argc) {
        tdelay = 1;
        tdelayV = atoi(argv[optind++]); // Here argv[optind++] points to the latter arguments with no flag.
    }

    if(system == 1 && user == 1){
        printf("You cannot use --system and --user at the same time. There's a contradiction.\n");
        return 1;
    }

    // Output all the content we want.
    // When we did not receive "--sequential" argument:
    if(!sequential){

        //* Section 1.
        printf("Nbr of samples: %d -- every %d secs\n", samplesV, tdelayV); // Output data about sample and delay.
        get_program_mem(); //Output the memory usage of the current program.
        printf("---------------------------------------\n");

        //* Section 2.
        // We do not output this section if we receive "--system" argument.
        if(!system){
            printf("### Sessions/users ###\n");
            get_user_info();
            printf("---------------------------------------\n");
        } // Output user information.

        // If we receive "--user" argument, we don't need to out put the following information,
        // so we just return directly.
        if(user){
            return 0;
        }

        //* Section 3.
        get_sys_info(); // Output information about the system.

        //* Section 4.
        // Output used memory utilization, including physical and virtual.
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");
        for(int i = 0; i < samplesV; i++){
            get_sys_mem();
            sleep(tdelayV);
        }
        printf("---------------------------------------\n");

        //* Section 5.
        // Output core number.
        get_core_info();

        // Output cpu utilization.
        double cpu_usage_0 = get_cpu_usage(); // To avoid problems as stated in the comment of function get_cpu_usage().
        sleep(tdelayV);
        double cpu_usage_1 = get_cpu_usage();
        double cpu_usage_2;
        double result;
        for(int i = 0; i < samplesV; i++){

            sleep(tdelayV); 
            cpu_usage_2 = get_cpu_usage(); // Get cpu data again after the given delay time.
            result = fabs(cpu_usage_2-cpu_usage_1)/cpu_usage_1; // Use formula to calculate cpu usage.
            printf("Total CPU usage: %.2lf%%\n", result);

            // In order to rewrite the previous cpu utilization data every iteration:
            if(i+1 < samplesV){
                printf("\033[A");  // Move the cursor up one line.
                printf("\033[K");  // Clear the current line.
            }
            
            cpu_usage_1 = cpu_usage_2; // Store the current cpu data so that it can be used in next iteration.
        }

        printf("---------------------------------------\n");  

    }else{
        // When we receive "--sequential" argument:
        int i = 0;
        double cpu_usage_0 = get_cpu_usage(); // To avoid problems as stated in the comment of function get_cpu_usage(). 
        sleep(tdelayV);
        double cpu_usage_1 = get_cpu_usage(); // Get the first cpu data.
        double cpu_usage_2, result;
        // For every iteration:
        for(i = 0; i < samplesV; i++){

            //* Section 1.
            printf(">>>>>>interation %d<<<<<<\n", i);
            printf("Nbr of samples: %d -- every %d secs\n", samplesV, tdelayV);
            get_program_mem();
            printf("---------------------------------------\n");

            //* Section 2.
            // Output user information when we did not receive "--system" argument.
            if(!system){
                printf("### Sessions/users ###\n");
                get_user_info();
                printf("---------------------------------------\n");
            }

            // Return here if we receive "--user" argument since we don't need to output the following information.
            if(user){
                return 0;
            }

            //* Section 3.
            get_sys_info(); // Output information about the system.

            //* Section 4.
            // Output used memory utilization, including physical and virtual.
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");
            for(int j = 0; j < samplesV; j++){
                if(i==j){
                    get_sys_mem(); // We only output the line of information of the current iteration.
                }else{
                    printf("\n"); // Make other lines blank.
                }
            }
            printf("---------------------------------------\n");

            //* Section 5.
            get_core_info(); // Output core number.

            sleep(tdelayV);
            cpu_usage_2 = get_cpu_usage(); // Wait for the given delay time and get the second cpu data.
            result = fabs(cpu_usage_2-cpu_usage_1)/cpu_usage_1; // Use formula to calculate cpu usage.
            printf("Total CPU usage: %.2lf%%\n", result); // Output cpu usage.
            printf("---------------------------------------\n");                       
        }

    }

    return 0;
}
