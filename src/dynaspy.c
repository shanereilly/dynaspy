/* Needed because of this:
 * https://stackoverflow.com/questions/23477817/implicit-declaration-of-process-vm-readv-but-i-am-including-sys-uio-h
 */
#define _GNU_SOURCE

#define BUFFER_LEN 64

#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <syscall.h>
#include <unistd.h>

// Can enable for debugging purposes
bool VERBOSE = false;

void handle_error(int errnum)
{
	fprintf(stderr, "DynaSpy: %s\n", strerror(errnum));
	exit(EXIT_FAILURE);
}

bool check_if_dynamic(char* filename)
{
	char buffer[BUFFER_LEN];
	memset(buffer, 0, BUFFER_LEN);

	/* First, check if file is an ELF file by checking file signature */
	int fd = open(filename, O_RDONLY);
	ssize_t nread = read(fd, buffer, 4);

	/* Don't want to just exit here, as there can be many reasons we aren't able to open files */
	if (nread == -1)
	{ 
		switch (errno) {
			/* Directories aren't shared libraries! */
			case EISDIR:
				return false;
			/* Access denied */
			case EACCES:
				if (VERBOSE)
					fprintf(stderr, "Problem opening %s, access denied\n", filename);
				return false;
		}
		if (VERBOSE)
			fprintf(stderr, "Problem opening %s, %s\n", filename, strerror(errno));
		return false;
	}

	if (nread == 4 && buffer[0] == 0x7f && buffer[1] == 0x45 && buffer[2] == 0x4c && buffer[3] == 0x46) {
		/* File is elf, now check 17th byte to ensure it is a shared object (16th if big endian) */
		read(fd, buffer + 4, 13);

		if (nread == -1)
			handle_error(errno);

		int dynamic_byte = (buffer[0x5] == 1) ? 0x10 : 0x11;
		if (buffer[dynamic_byte] == ET_DYN)
			return true;
	}
	return false;
}

void locate_syscalls(char **argv)
{
	pid_t pid = fork();
	switch (pid) {
		/* Error */
		case -1:
			handle_error(errno);
		/* Child process */
		case 0:
			ptrace(PTRACE_TRACEME, 0, NULL, NULL);
			execvp(argv[0], argv);
	}
	/* Parent process */
	/* Terminate tracee with its parent */
	waitpid(pid, 0, 0); 
	ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);

	/* Loop until child process exits */
	while(1) {
		/* Enter next system call */
		if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1)
		    handle_error(errno);
		if (waitpid(pid, 0, 0) == -1)
		    handle_error(errno);

		/* Get the system call arguments from registers*/
		struct user_regs_struct regs;
		if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1)
		    handle_error(errno);
		struct iovec local[1];
		struct iovec remote[1];
		char local_buffer[BUFFER_LEN];
		local[0].iov_base = local_buffer;
		local[0].iov_len = BUFFER_LEN;
		remote[0].iov_base = NULL;
		remote[0].iov_len = BUFFER_LEN;

		switch(regs.orig_rax) {
    			/* int open(const char *pathname, int flags); */
			case SYS_open:
				remote[0].iov_base = (void*) regs.rdi;
				break;
       			/* int openat(int dirfd, const char *pathname, int flags); */
			case SYS_openat:
				remote[0].iov_base = (void*) regs.rsi;
				break;
       			/* int openat2(int dirfd, const char *pathname, const struct open_how *how, size_t size); */
			case SYS_openat2:
				remote[0].iov_base = (void*) regs.rsi;
				break;
       			/* int creat(const char *pathname, mode_t mode); */
			case SYS_creat:
				remote[0].iov_base = (void*) regs.rdi;
				break;
		}
		if (remote[0].iov_base != NULL) {
			ssize_t total_read = process_vm_readv(pid, local, 1, remote, 1, 0);
			if (total_read == -1)
				handle_error(errno);

			/* Get length of filename and copy to string */
			size_t len = strlen(local[0].iov_base);
			char filename[len + 1];
			strncpy(filename, local[0].iov_base, len);
			filename[len] = '\0';

			/* Check if filename is a dynamic library */
			bool is_dynamic = check_if_dynamic(filename);
			if (is_dynamic) {
				fprintf(stderr, "Dynamic library opened: %s\n", filename);
			}
		}

		/* Run system call */
		if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1)
		    handle_error(errno);
		if (waitpid(pid, 0, 0) == -1)
		    handle_error(errno);

		/* Test if call was exit() */
		if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) {
		    if (errno == ESRCH)
			exit(regs.rdi); 
		    handle_error(errno);
		}
	}
	
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Usage: %s path_to_executable output_file [arguments]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	locate_syscalls(argv + 1);
	exit(EXIT_SUCCESS);
}
