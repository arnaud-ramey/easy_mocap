/*
 * Textures.cpp
 *
 *  Created on: Jan 24, 2010
 *      Author: arnaud
 *
 * textures and co
 */"
class Tex {
public:
        string filename;
        int id;
};

vector<Tex> textures;

/*!
 * load a texture
 * @param id the id of the texture
 */
void load_texture(int idTex, int idTexUnit) {
        glActiveTextureARB(idTexUnit);
        glEnable( GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, idTex);
}

void unload_texture_unit(int idTexUnit) {
        glActiveTextureARB(idTexUnit);
        glDisable( GL_TEXTURE_2D);
}

/*!
 * begin a texture
 * @param id the id of the texture
 */
void create_texture_begin(int id) {
        glEnable( GL_TEXTURE_2D);

        //The first thing that must take place in the process of uploading the texture is a call to glBindTexture.
        // What glBindTexture does is it tells OpenGL which texture "id" we will be working with.
        // A texture "id" is just a number that you will use to access your textures.
        glBindTexture(GL_TEXTURE_2D, id);

        // The glTexParameteri sets the various parameters for the current OpenGL texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        // The glTexEnvf call sets environment variables for the current texture.
        // What this does is tell OpenGL how the texture will act when it is rendered into a scene.
        //GL_MODULATE, GL_DECAL, GL_BLEND, or GL_REPLACE.
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

/*!
 * load a texture into the texture number id
 * @param filename the name of the bitmap file
 * @param id the index of the texture
 */
void read_texture_from_file(const char* filename, int id_tex) {
        cout << endl << endl << "***" << filename << endl;
        create_texture_begin(id_tex);

        Bitmap* img = new Bitmap(filename);

        //This call tells OpenGL that the pixel data which is going to be passed to it is aligned in byte order,
        //this means that the data has one byte for each component, one for red, green and blue.
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // The glTexImage2D call is our goal.
        // This call will upload the texture to the video memory where it will be ready for us to use in our programs
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->width, img->height, 0, GL_RGB,
                        GL_UNSIGNED_BYTE, img->data);

        Tex texture;
        texture.filename = filename;
        texture.id = id_tex;
        textures.push_back(texture);
}

/*!
 * create a black texture
 * @param id the id
 * @param w the width
 * @param h the height
 */
void create_black_texture(int id, int w, int h) {
        create_texture_begin(id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE,
                        NULL);
}

/*!
 * return the index of the good texture if loaded, -1 else
 * @param filename
 * @return
 */
int find_texture_index(const char* filename) {
        for (vector<Tex>::iterator tex = textures.begin(); tex != textures.end(); ++tex) {
                if (((string) tex->filename).find(filename) != string::npos)
                        return tex->id;
        }
        return -1;
}

/*!
 * load texture from a slot if it is already loaded,
 * from the file otherwise
 * @param filename
 * @param idTexUnit
 */
void load_texture(const char* filename, int idTexUnit) {
        int index_in_vector = find_texture_index(filename);
        /*
         * read the texture if needed
         */
        if (index_in_vector == -1) {
                // find an available index
                int first_available_texture_index = 0;
                bool OK = false;
                while (!OK) {
                        ++first_available_texture_index;
                        OK = true;
                        for (vector<Tex>::iterator tex = textures.begin(); tex
                                        != textures.end(); ++tex) {
                                if (tex->id == first_available_texture_index) {
                                        OK = false;
                                        break;
                                }
                        } // end loop on textures
                } // end loop while

                // create filename
                ostringstream filename_full;
                filename_full << "textures/" << filename;

                // display
                cout << "*** Texture required !!!" << endl;
                cout << "We will read the texture '" << filename << "' in slot "
                                << first_available_texture_index << endl;
                cout << "Loaded textures : " << endl;
                for (vector<Tex>::iterator tex = textures.begin(); tex
                                != textures.end(); ++tex)
                        cout << " - " << tex->filename << endl;

                read_texture_from_file(filename_full.str().c_str(),
                                first_available_texture_index);
        } // end of texture not found

        load_texture(index_in_vector, idTexUnit);
}
