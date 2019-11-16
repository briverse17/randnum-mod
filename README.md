RANDOM NUMBERS GENERATING KERNEL MODULE
=======
I - Descriptions:
-------
1. Feature:
	This module generate random numbers in a range of (-limit, limit).
2. Files and Folders:
	- **randnum_mod.c**: main source
	- **Makefile** and **Kbuild**: this module is built using Kbuild method
	- **README.md**
	- **~/user_app**: contains *randnum.c* and its *Makefile* (for the user to get random numbers)
	- **~/test** (not recommended): contains *randnum_mod.ko* and *randnum.exe* that are already built
	>Built and tested successfully on:
	>>- [x] ubuntu 16.04 LTS 64-bit
	>>- [x] kernel version: 4.15.0-70-generic
3. Funtions in ***randnum_mod.c***:
	- **number_len(int num)** : get number of digits
	- **mypow(int base, int exp)** : simple power function
	- **number_to_str(int num, char\* str)** : convert from int to char* - equivalent to **string::itoa()**
	- **str_to_number(int \*num, char* str)** : convert from char* to int - equivalent to **string::atoi()**
	- **randnum_open(struct inode \*i, struct file \*f)** : will be assigned to syscall "open"
	- **randnum_close(struct inode \*i, struct file \*f)** : will be assigned to syscall "release"
	- **randnum_read(struct file \*f, char __user \*buf, size_t len, loff_t \*off)** : will be assigned to syscall "read"
	- **randnum_write(struct file \*f, const char __user \*buf, size_t len, loff_t \*off)** : will be assigned to syscall "write" (in this program, its specific task is to change the range of generation)
	- **__init randnum_init(void)** : initialize the module
	- **__exit randnum_exit(void)** : exit the module

II - Makefiles:
-------
1. *randnum_mod.c*
- `$ ~/Randnum$ make`
- The result should look like:
	>	```
	>	make -C /lib/modules/``uname -r``/build M=``pwd`` modules
	>	make[1]: Entering directory '/usr/src/linux-headers-4.15.0-70-generic'
	>	CC [M]  /home/osboxes/Modules/randnum_mod.o
	>	Building modules, stage 2.
	>	MODPOST 1 modules
	>	  CC      /home/osboxes/Modules/randnum_mod.mod.o
	>	  LD [M]  /home/osboxes/Modules/randnum_mod.ko
	>	make[1]: Leaving directory '/usr/src/linux-headers-4.15.0-70-generic'
	>	```
2. *randnum.c*
- `$ ~/Randnum/user_app$ make`
- The result should look like:
	>	`cc -o randnum randnum.c`

III - Module manipulations:
-------
1. insert:
	- `$ ~/Randnum$ sudo insmod randnum_mod.ko`
2. remove:
	- `$ ~/Randnum$ sudo rmmod randnum_mod.ko`
3. get permission to access device file:
	- `$ ~/Randnum$ sudo chmod 666 dev/randnum_dev`

IV - Use the app:
-------
1. Run:
- `$ ~/Randnum/user_app$ ./randnum`
- The result should look like:
	>	```
	>	Select options:
	>		o (to open a device node)
	>		c (to close the device node)
	>		r (to read data from device node)
	>		s (to set a new random range, max = 1000000)
	>		q (to quit the application)
	>	Enter your option:
	>	```
2. Commands:
- o : open the device file
- c : close the device file
- r : read data from device file
- s : set a new random range, max = 1000000
- q : quit the application

V - References:
-------
1. Core knowledges and user interface:
	>https://vimentor.com/vi/course/linux-device-driver?type=lesson
2. **randnum_mod.c** is based on:
	>https://sites.google.com/site/embedded247/ddcourse/device-drivers-phan-5-character-device-files
	
<div style="text-align: justify"> Nguyen Minh Vu - Nov, 2019 </div>
