#include "proc_reader.h"
#include <sys/types.h> /* for ssize_t */

/* -------- helper : is_number --------*/
int is_number(const char*str) { 
	if(!str || !*str) return 0; 
	const unsigned char *p = (const unsigned char*)str;
	while (*p) { 
		if (!isdigit(*p)) return 0;
		++p;
	}
	return 1;
}

/*--------helper : read_file_with_syscalls --------*/
int read_file_with_syscalls(const char* filename) { 
	int fd = open(filename, O_RDONLY);
	if (fd == -1) { perror("open"); return -1; } 
	
	char buffer[1024];
	for(;;) { 
	ssize_t n = read(fd, buffer, sizeof(buffer) -1); 
	if (n > 0) { 
		buffer[n] = '\0';
		fputs(buffer, stdout); 
	      } else if (n == 0) { 
	      	break; /* EOF */
	      } else { 
	      	perror("read"); 
	      	close(fd); 
	      	return -1;
	      }
	}
	if (close(fd) == -1) { perror("close"); return -1; } 
	return 0;
}

/*--------helper : read_file_with_library--------*/
int read_file_with_library(const char* filename) { 
	FILE *fp = fopen(filename, "r");
	if (!fp) { perror("fopen"); return -1; }
	
	char line[256];
	while (fgets(line, sizeof line, fp)) { 
		fputs(line, stdout); 
	}
	if (fclose(fp) == EOF) { perror("fclose"); return -1; }
	return 0; 
}

/*--------required : list_process_directories--------*/

int list_process_directories(void) {
	DIR *dir = opendir("/proc");
	if (!dir) { perror("opendir"); return -1; } 
	
	struct dirent *entry;
	int count = 0;

    printf("Process directories in /proc:\n");
    printf("%-8s %-20s\n", "PID", "Type");
    printf("%-8s %-20s\n", "---", "----");

    while ((entry = readdir(dir)) != NULL) { 
    	if (is_number(entry->d_name)) { 
    		printf("%-8s %-20s\n", entry->d_name, "process");
    		++count;
    	}
 }
    if (closedir(dir) == -1) { perror("closedir"); return -1; } 
    printf("Found %d process directories\n", count);
    return 0; // Replace with proper error handling
}

/*--------required: read_process_info--------*/
int read_process_info(const char* pid) {
    char path[256];

    /* /proc/<pid>/status */
    snprintf(path, sizeof path, "/proc/%s/status", pid);
    printf("\n--- Process Information for PID %s ---\n", pid);
    if (read_file_with_syscalls(path) == -1) return -1;
    
    
    /* /proc/<pid>/cmdline (NUL-separated) */
    snprintf(path, sizeof path, "/proc/%s/cmdline", pid);
    printf("\n--- Command Line ---\n");

    int fd = open(path, O_RDONLY);
    if (fd == -1) { perror("open cmdline"); return -1; } 
    
    char buf[4096];
    ssize_t n = read(fd,buf, sizeof buf);
    if (n < 0) { perror("read cmdline"); close(fd); return -1; } 
    
    if (n == 0) { 
    	printf("(empty)\n");
  } else { 
  	for(ssize_t i=0; i < n; i++) { 
  		char c = (buf[i] == '\0') ? ' ' : buf[i];
  		if (write(STDOUT_FILENO, &c, 1) !=1) { perror("write"); close(fd); return -1; } 
    }
    if (buf[n -1] != '\n') (void)write(STDOUT_FILENO, "\n", 1);
  }
  if (close(fd) == -1) { perror("close cmdline") ; return -1; } 

    printf("\n"); // Add extra newline for readability
    return 0; // Replace with proper error handling
}

/*--------required: show_system_info--------*/
int show_system_info(void) {
    const int MAX_LINES = 10;

    printf("\n--- CPU Information (first %d lines) ---\n", MAX_LINES);
    FILE *fcpu = fopen("/proc/cpuinfo", "r");
    if (!fcpu) { perror("fopen /proc/cpuinfo"); return -1; }
    char line[512]; int lines = 0;
    while (lines < MAX_LINES && fgets(line, sizeof line, fcpu)) {
        fputs(line, stdout);
        ++lines;
    }
    if (fclose(fcpu) == EOF) { perror("fclose /proc/cpuinfo"); return -1; }

    printf("\n--- Memory Information (first %d lines) ---\n", MAX_LINES);
    FILE *fmem = fopen("/proc/meminfo", "r");
    if (!fmem) { perror("fopen /proc/meminfo"); return -1; }
    lines = 0;
    while (lines < MAX_LINES && fgets(line, sizeof line, fmem)) {
        fputs(line, stdout);
        ++lines;
    }
    if (fclose(fmem) == EOF) { perror("fclose /proc/meminfo"); return -1; }

    return 0; // Replace with proper error handling
}

/*--------required: compare_file_methods--------*/

void compare_file_methods(void) {
    const char* test_file = "/proc/version";

    printf("Comparing file reading methods for: %s\n\n", test_file);

    printf("=== Method 1: Using System Calls ===\n");
    if (read_file_with_syscalls(test_file) !=0)
    	fprintf(stderr, "(syscall method failed)\n");

    printf("\n=== Method 2: Using Library Functions ===\n");
    if (read_file_with_library(test_file) !=0) 
    	fprintf(stderr, "(library method failed)\n");

    printf("\nNOTE: Run this program with strace to see the difference!\n");
    printf("Example: strace -e trace=openat,read,write,close ./lab2\n");
}


