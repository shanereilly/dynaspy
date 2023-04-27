% DYNASPY(1)
% Shane Reilly
% April 2023

# NAME
dynaspy - prints out the dynamic libraries used by an executable to standard error

# SYNOPSIS
**dynaspy** *TARGET* [*ARGUMENTS*]

# DESCRIPTION
**dynaspy** writes to standard error the names of shared libraries used by a program as the program opens them. It uses *ptrace* to watch all system calls made by the application. Any variation of the *open()* syscall has the corresponding filename opened and checked to see if it is a dynamic library by examining the file header.

# EXAMPLES
**dynaspy /bin/ls**
: Displays all shared libraries used by the **ls** program. Also displays the output of the **ls** program.

**dynaspy /bin/ls 1> /dev/null**
: Displays all shared libraries used by the **ls** program. Omits the text written by **ls** to standard out.

**dynaspy /bin/cat testfile.txt**
: Displays all shared libraries used by the **cat** program as it opens **testfile.txt**.

# EXIT VALUES
**0**
: Success

**1**
: Failure
