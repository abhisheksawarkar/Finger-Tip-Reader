# Finger-Tip-Reader
Humans have naturally tendency to point fingers at various objects. Why not use this ability to get information about our world?
Here is a project called Finger Tip Reader which helps visually impaired people to read any kind of text without the need to 
learn braille. 
A camera mounted on the finger inputs text and pre-processes the image. An image to text engine called Tesseract OCR converts
the image into text. 
Another engine called microsoft SAPI converts this text into speech output. Hence the camera input is output into speech format.

The project is done in visual studio 2013, using visual C++
Libraries used are OpenCV + Tesseract + Microsoft SAPI

Instructions:
1. Download Visual Studio 2013

2. Download this project as zip or git clone it

3. Install opencv and tesseract libraries. Installation guide is provided on their official website

4. Install SAPI text to speech engine

5. Run the solution. Main program is in the source.cpp file 

Inspired from:
http://fluid.media.mit.edu/projects/fingerreader

Video:
https://www.youtube.com/watch?v=p-SkIimUeqs
