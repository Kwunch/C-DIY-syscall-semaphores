# C-DIY-syscall-semaphores
Pitt CS1550 Project. Create 3 new Syscalls creating my own semaphores and using those syscalls to solve the producer-consumer problem. Sys.c is Linux-5.10.140/kernel/sys.c. Prodcons is ran on the modified modified kernel

# Important
You are unable to take these two files and add it to the kernel. I have not included the files syscalls.h, unistd.h, syscall-64.tbl, and syscall-32.tbl. Those 4 files require modification so that the 3 new syscalls in sys.c work properly. Again without modifications of those 4 files, sys.c will not work. That being said the implementation when put in correctly will work as advertised. 
