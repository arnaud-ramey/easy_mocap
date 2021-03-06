                  +----------------------+
                  |      easy_mocap      |
                  +----------------------+

[![Build Status](https://travis-ci.org/arnaud-ramey/easy_mocap.svg)](https://travis-ci.org/arnaud-ramey/easy_mocap)

The aim is to be able to perform some motion capture with only a simple
camera. The output of the motion capture (the detected pose of the human
body) is here used to animate a OpenGL character.

License :                  see the LICENSE file.
Authors :                  see the AUTHORS file.
How to build the program:  see the INSTALL file.

The idea is to make an actor move in front of a camera. He is equipped with
markers on his body. These markers are in fact simple pieces of fabric, with
a vivid color. We detect the position and orientation of these markers. By
those means, we can "rebuild" the body and send the data of its movement to
a computer generated figure.

1. Image analysis : finding where the markers are on the video. The aim of
this part is to extract the relevant portions of the image that correspond
to the members of the person doing the motion capture. In input, we have a
video recorded with a random camera. In output, we have the orientation of
every zone of pixels with a color matching the colors of the members. This
is an OpenCV program.

2. Human body : find the orientation on the actor on the video, according to
the markers. We convert the information we extract from the video images to
compute the orientations of the human body.

3. Representation of the body : send the data about the orientation to the
computer generated body. We send to the OpenGL animated body the orientation
of the human body that we computed earlier. Thus, we create an animation of
a computer generated character, controlled by a real human in real time.
This is an OpenGL program.

(you can find a more complete documentation in doc/report.pdf)

________________________________________________________________________________

How to use the program
________________________________________________________________________________
To display the help, just launch in a terminal:
$ ./build/test_easy_mocap -g -v samples/motion.MPG
It will display the help of the program.

Keys:
'8' : camera go forward
'2' : camera go backward
'1' : camera strafe left
'3' : camera strafe right
'4' : camera rotate left
'6' : camera rotate right
'+' : camera go higher
'-' : camera go lower
'q' : quit

