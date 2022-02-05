# SiBa
Simple Backup Application

The SiBa is a simple application used to create the one-way incremental backup of a source directory. A target directory is synchronized with the source directory. Only the target directory is modified by backup. New files are copied from the source to the target directory. Modified files overwrite the existing ones. Files deleted from the source directory are deleted from the target directory. 
The source directory is not modified.

To prevent accidental overwriting of files and directories, it is advisable to place a blank file called "source.siba" in the source directory and a blank file named "target.siba" in the target directory. The program distinguishes the source and target directory by the presence of these files.


LICENCE
 * Licence: EUPL v. 1.2
 * https://joinup.ec.europa.eu/collection/eupl

Permission is hereby granted to any person obtaining a copy of this software  and associated documentation files to use the software and documentation (the "software") free of charge. 
The software is provided "as is" and the author disclaims all warranties with regard to this software including all implied warranties of merchantability and fitness.In no event shall the author be liable for any special, direct, indirect, or consequential damages or any damages whatsoever resulting from loss of use, data or profits, whether in an action of contract, negligence or other tortious action, arising out of or in connection with the use or performance of this software. 

The disclaimer
The software is provided "as is", without warranty of any kind.
You use the software at your own risk.
The software is a work in progress and may contain some defects. We do not guarantee that the software will be error-free, the results obtained from the use of the software will be accurate and reliable.
We will not be liable for any direct or indirect damages resulting from the use or misuse of the software, even if the author has been advised of the possibility of such damage.

The agreement
By downloading or using the software, you agree to be bound by the terms and conditions of the end user licence agreement. If you do not agree to the terms and conditions of this end user licence agreement, you may not download or use the software.

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the software.

Build in Qt Creator 4.15.2
