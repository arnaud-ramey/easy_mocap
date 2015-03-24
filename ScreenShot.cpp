/*
 * ScreenShot.cpp
 *
 *  Created on: Mar 2, 2010
 *      Author: arnaud
 */
extern "C" {
#include <jpeglib.h>    /* IJG JPEG LIBRARY by Thomas G. Lane */
}

// structure for saving jpeg
GLubyte *jpeg_pixels = 0, *jpeg_flip = 0;

bool save_jpeg(unsigned int width, unsigned int height, char *path, int quality) {
    bool ret = false;

    struct jpeg_compress_struct cinfo; // the JPEG OBJECT
    struct jpeg_error_mgr jerr; // error handler struct
    unsigned char *row_pointer[1]; // pointer to JSAMPLE row[s]
    FILE *shot;
    int row_stride; // width of row in image buffer

    if ((shot = fopen(path, "wb")) != NULL) { // jpeg file
        // initializatoin
        cinfo.err = jpeg_std_error(&jerr); // error handler
        jpeg_create_compress(&cinfo); // compression object
        jpeg_stdio_dest(&cinfo, shot); // tie stdio object to JPEG object
        row_stride = width * 3;

        if (!jpeg_pixels) {
            jpeg_pixels = (GLubyte *) malloc(sizeof(GLubyte) * width * height
                                             * 3);
            jpeg_flip
                    = (GLubyte *) malloc(sizeof(GLubyte) * width * height * 3);
        }

        if (jpeg_pixels != NULL && jpeg_flip != NULL) {
            // save the screen shot into the buffer
            //glReadBuffer(GL_FRONT_LEFT);
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE,
                         jpeg_pixels);

            // give some specifications about the image to save to libjpeg
            cinfo.image_width = width;
            cinfo.image_height = height;
            cinfo.input_components = 3; // 3 for R, G, B
            cinfo.in_color_space = JCS_RGB; // type of image

            jpeg_set_defaults(&cinfo);
            jpeg_set_quality(&cinfo, quality, TRUE);
            jpeg_start_compress(&cinfo, TRUE);

            // OpenGL writes from bottom to top.
            // libjpeg goes from top to bottom.
            // flip lines.
            for (unsigned int y = 0; y < height; ++y) {
                for (unsigned int x = 0; x < width; ++x) {
                    jpeg_flip[(y * width + x) * 3] = jpeg_pixels[((height - 1
                                                                   - y) * width + x) * 3];
                    jpeg_flip[(y * width + x) * 3 + 1] = jpeg_pixels[((height
                                                                       - 1 - y) * width + x) * 3 + 1];
                    jpeg_flip[(y * width + x) * 3 + 2] = jpeg_pixels[((height
                                                                       - 1 - y) * width + x) * 3 + 2];
                }
            }

            // write the lines
            while (cinfo.next_scanline < cinfo.image_height) {
                row_pointer[0] = &jpeg_flip[cinfo.next_scanline * row_stride];
                jpeg_write_scanlines(&cinfo, row_pointer, 1);
            }

            ret = true;
            // finish up and free resources
            jpeg_finish_compress(&cinfo);
            jpeg_destroy_compress(&cinfo);
        }
        fclose(shot);
    }

    //	if (pixels != 0)
    //		free(pixels);
    //	if (flip != 0)
    //		free(flip);

    return ret;
}

GLubyte* tga_pixels = 0;
void save_tga(char* cFileName, GLubyte* pixelsTGA) {
    int nSize = width * height * 3;

    if (pixelsTGA == 0)
        pixelsTGA = new GLubyte[width * height * 3];

    FILE *fScreenshot;
    fScreenshot = fopen(cFileName, "wb");

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixelsTGA);

    //convert to BGR format
    unsigned char temp;
    int i = 0;
    while (i < nSize) {
        temp = pixelsTGA[i]; //grab blue
        pixelsTGA[i] = pixelsTGA[i + 2];//assign red to blue
        pixelsTGA[i + 2] = temp; //assign blue to red

        i += 3; //skip to next blue byte
    }
    unsigned char TGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    unsigned char header[6] = { width % 256, width / 256, height % 256, height
                                / 256, 24, 0 };
    fwrite(TGAheader, sizeof(unsigned char), 12, fScreenshot);
    fwrite(header, sizeof(unsigned char), 6, fScreenshot);
    fwrite(pixelsTGA, sizeof(GLubyte), nSize, fScreenshot);
    fclose(fScreenshot);
}

//this will save a TGA in the screenshots
//folder under incrementally numbered files
// ffmpeg -r 10 -i *.tga out.avi
int nShot = 0;
void screenShot() {
    ++nShot;
    //	if (nShot > 64)
    //		return;
    if (nShot % 10 != 0)
        return;

    char cFileName[64];
    sprintf(cFileName, "export/opengl_screenshot_%06d.jpg", nShot);
    //	sprintf(cFileName, "export/screenshot_%06d.tga", nShot);

    save_jpeg(width, height, cFileName, 85);
    //	save_tga(cFileName, pixels);
}

void screenShot_makeVideo(string schema, string outName) {
    ostringstream command;
    command << "cd export ; ffmpeg ";
    command << "-r 25 "; // fps
    command << "-b 1M "; // bitrate
    command << "-i " << schema << " "; // filename
    command << "-y "; // overwrite
    command << "-an "; // no sound
    command << "-vcodec mpeg4 "; // codec
    command << outName; // outut file
    command << ";";
    int OK = system(command.str().c_str());
    ++OK;
    return;
}

void screenShot_clean() {
    screenShot_makeVideo("opengl_screenshot_%05d0.jpg", "out_opengl.avi");
    screenShot_makeVideo("opencv_illus_%06d.jpg", "out_opencv.avi");

    // clean buffers
    delete tga_pixels;
    free(jpeg_flip);
    free(jpeg_pixels);

    // clean images
    int OK = system("pwd ; rm export/*.jpg");
    ++OK;
}
