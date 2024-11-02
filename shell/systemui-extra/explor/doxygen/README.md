# Generate html document

## How to
I have provided a configure file to generate the html document of this project.
The document is autogen from the file Doxygen.in and the whole project codes.
If you want to generate the doc correctly, you should make sure:
- doxygen and graphviz have been installed in your system. If you're user of Debian or Ubuntu, you can use this command for packages installation.


- - sudo apt install doxygen graphviz


- Then just clone this project, cd into this directory, run command:


- - doxygen Doxygen.in


- After command is executed, you should see a new folder named 'out' generated in this directory. Find index.html in this new folder and open it in your browser. Then you can view the project document locally.

By the way, in progress of generating document, you can also use doxywizard to configure the .in file by yourself.
This is very helpful if you want to adjust the scope, content, and format of doc generation. doxywizard is provided by doxygen-gui in Debian and Ubuntu.
