# Plotting-using-ImGui
This is a project to plot data from binary file realised in c++ using ImGui as UI.

Install OpenCV by following the [link](https://www.learnopencv.com/install-opencv-3-4-4-on-ubuntu-18-04/?fbclid=IwAR2fN4nVyeWYnkW8Zb2wvBsFMsCTURHzm3kfQFGI8OxrO7u0NF_CQO8rGzA). You can skip step 3, because in this project we do not need the python extensio of the openCV. After that execute the following commands:

1. *sudo cp $(cwd)/installation/lib/pkgconfig/opencv.pc /usr/lib/pkgconfig/opencv.pc* <br> 
**or** <br> 
add the next environmental variable to ~/.bashrc file: <br>
*export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$(cwd)/installation/OpenCV-3.4.4/lib/pkgconfig*
2. *cd $(cwd)/installation/lib* <br>
*sudo pwd > /etc/ld.so.conf.d/opencv.conf*<br>
**or**<br>
*sudo cp $(cwd)/installation/lib/.\* /usr/lib*
