#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

GLuint loadBMP_custom(const char * imagepath){
        // Load BMP
    // GLuint image = loadBMP_custom("uvtemplate.bmp");
    unsigned char header[54]; // Each BMP file begins by a 54-bytes header
    unsigned int dataPos;     // Position in the file where the actual data begins
    unsigned int widthBMP, heightBMP;
    unsigned int imageSizeBMP;   // = width*height*3
    // Actual RGB data
    unsigned char * data;

    FILE * file = fopen(imagepath,"rb");
    if (!file){printf("Image could not be opened\n"); return 0;}

    // Check if this is a bmp file by assessing the number of bytes in the 
    if ( fread(header, 1, 54, file)!=54 ){ // If not 54 bytes read : problem
        printf("Not a correct BMP file\n");
        return false;
    }
    // Check this is reall a bmp file
    if ( header[0]!='B' || header[1]!='M' ){
        printf("Not a correct BMP file\n");
        return 0;
    }
    // Read in the file info
    dataPos    = *(int*)&(header[0x0A]);
    imageSizeBMP  = *(int*)&(header[0x22]);
    widthBMP      = *(int*)&(header[0x12]);
    heightBMP     = *(int*)&(header[0x16]);

    if (imageSizeBMP==0)    imageSizeBMP=widthBMP*heightBMP*3; // 3 : one byte for each Red, Green and Blue component
    if (dataPos==0)      dataPos=54; // The BMP header is done that way

    // Allocate some memory for the image given the size we just loaded
    data = new unsigned char [imageSizeBMP]; // create the buffer
    fread(data, 1, imageSizeBMP, file); // Read the data into the buffer
    fclose(file);

    // Create the texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    // Bind it 
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Read image in OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthBMP, heightBMP, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    
    
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // When MAGnifying the image (no bigger mipmap available), use LINEAR filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // When MINifying the image, use a LINEAR blend of two mipmaps, each filtered LINEARLY too
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // Generate mipmaps, by the way.
    glGenerateMipmap(GL_TEXTURE_2D);

    return textureID;
}



#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

GLuint loadDDS(const char * imagepath){

    unsigned char header[124];

    FILE *fp;

    /* try to open the file */
    fp = fopen(imagepath, "rb");
    if (fp == NULL)
        return 0;

    /* verify the type of file */
    char filecode[4];
    fread(filecode, 1, 4, fp);
    if (strncmp(filecode, "DDS ", 4) != 0) {
        fclose(fp);
        return 0;
    }

    /* get the surface desc */
    fread(&header, 124, 1, fp); 

    unsigned int height      = *(unsigned int*)&(header[8 ]);
    unsigned int width         = *(unsigned int*)&(header[12]);
    unsigned int linearSize     = *(unsigned int*)&(header[16]);
    unsigned int mipMapCount = *(unsigned int*)&(header[24]);
    unsigned int fourCC      = *(unsigned int*)&(header[80]);

    if (mipMapCount == 0) mipMapCount = 1;
    
    // Assign a buffer for our mipmaps
    unsigned char * buffer;
    unsigned int bufsize;
    // Set the sizing and reserve some memory for the mipmaps
    bufsize = mipMapCount > 1 ? linearSize *2 : linearSize;
    buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
    // Read file into the buffer
    fread(buffer, 1, bufsize, fp);
    fclose(fp);
    
    unsigned int components  = (fourCC == FOURCC_DXT1) ? 3 : 4;
    unsigned int format;
    switch(fourCC)
    {
    case FOURCC_DXT1:
        format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case FOURCC_DXT3:
        format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case FOURCC_DXT5:
        format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    default:
        free(buffer);
        return 0;
    }

    //Create the texture as usual
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);	

    // Now fill the textur using bugger info
    unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16; //Assuming the DXT1 format uses smaller blocks
    unsigned int offset = 0;

    for (unsigned int level =0; level < mipMapCount && (width || height); level++){
        
        // DXT1,3,5... store tex as 4x4 pixel blocks. 
        //  (width + 3) / 4 is equivalent to ceil(width / 4.0), as we store size as an int so it ceils.
        //   - this is a generate rule for ceil(a/b) == (a+ b-1)/b
        unsigned int size = ((width + 3)/4)*((height+3)/4)*blockSize;
        glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, buffer + offset);

        offset += size; // Move the offset to the next point of the mipmap
        width /= 2; //
        height /=2;
		if(width < 1) width = 1;
		if(height < 1) height = 1;

        
    }

    // we allocated memory for the buffer for passing the mimap data. Now that's done lets free the buffer memory
    free(buffer);

    return textureID;
}