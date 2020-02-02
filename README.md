DEFRAG(1)

NAME
	defrag - defragmentation of disk images

DESCRIPTION
	This program will be used to defragment disk images.
USAGE
	To execute this program, first execute the command 'make' in order to build the executable binary file called "defrag".
	Then use the following command to defragment your disk:
		./defrag [disk_image_name]
INPUT
	This execution takes one argument which is the name of the disk image. For an image which is not in the same location as the executable 'defrag', use the image name alog with its path.
	Example:./defrag disk_frag_0
		./defrag ../PA3/disk_frag_0
OUTPUT
	The output of the program will be a disk_image with the name 'disk_defrag' at the same location as the executable 'defrag'
AUTHOR
	Gurman Singh, gsingh23@ncsu.edu 
