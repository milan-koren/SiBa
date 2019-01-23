# SiBa
Simple Backup Application

The SiBa is a simple application used to create the one-way incremental backup of a source directory. A target directory is synchronized with the source directory. Only the target directory is modified by backup. New files are copied from the source to the target directory. Modified files overwrite the existing ones. Files deleted from the source directory are deleted from the target directory. 
The source directory is not modified.

To prevent accidental overwriting of files and directories, it is advisable to place a blank file called "source.siba" in the source directory and a blank file named "target.siba" in the target directory. The program distinguishes the source and target directory by the presence of these files.

Build in Qt Creator 4.7.0
