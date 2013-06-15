A project developed in C++ using Qt and the OpenCV Library with the aim of
generating a mosaiced image in real time from a video.

Although the OpenCV library already has a stitching module that makes the whole
mosaicing process much more easy it was not useful for this project because
with that implementation the user would have to wait for reading the whole
video to start generating the stitched image. But instead, with this approach
the resulting image is being generated while the video is running which make it
more like "in real time". 

Is also worth to mention that the resulting image in this case is not very good
because of the registration process after warping the new frames. This have to
be improved.

--
Alberto 
