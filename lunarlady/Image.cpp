#include "lunarlady/Image.hpp"
#include "sgl/sgl_opengl.hpp"
#include "lunarlady/File.hpp"
#include "lunarlady/OpenGL_util.hpp"
#include "lunarlady/Error.hpp"
#include "IL/il.h"
#include <sstream>
#include "lunarlady/Log.hpp"
#include "lunarlady/Game.hpp"

namespace lunarlady {
	ImageDescriptor::ImageDescriptor(const std::string& iFileName) : mFileName(iFileName) {
	}

	const std::string& ImageDescriptor::getFileName() const {
		return mFileName;
	}
	bool ImageDescriptor::operator<(const ImageDescriptor& iOther) const {
		return mFileName < iOther.mFileName;
	}

	ImageLoaded::ImageLoaded(const ImageDescriptor& iDescription) {
		ReadFile file(iDescription.getFileName());

		unsigned int offset = 0;
		const bool haveCompressedImage = file.getBuffer()[0]==1;
		const bool haveUncompressedImage = file.getBuffer()[1]==1;
		offset += sizeof(char)*2;

		const GLint enviroment = ((GLint*)(file.getBuffer()+offset))[0];
		const GLint min = ((GLint*)(file.getBuffer()+offset))[1];
		const GLint mag = ((GLint*)(file.getBuffer()+offset))[2];
		const GLint wrapS = ((GLint*)(file.getBuffer()+offset))[3];
		const GLint wrapT = ((GLint*)(file.getBuffer()+offset))[4];
		const GLint color = ((GLint*)(file.getBuffer()+offset))[5];
		offset += sizeof(GLint) * 6;

		glGenTextures(1, &mId);
		glBindTexture(GL_TEXTURE_2D, mId);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, enviroment);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
				
		const bool useCompressedImage = haveCompressedImage && GL_ARB_texture_compression && GL_EXT_texture_compression_s3tc;
		if( useCompressedImage ) {
			// load image setting up all mipmaps
			{
				const unsigned int size = ((unsigned int*)(file.getBuffer()+offset))[0];
				offset+= sizeof(unsigned int);
				offset+=size;
			}

			do {
				testGl();
				GLint level = ((GLint*)(file.getBuffer()+offset))[0];
				GLint internalFormat = ((GLint*)(file.getBuffer()+offset))[1];
				GLint compressedSize = ((GLint*)(file.getBuffer()+offset))[2];
				GLint width = ((GLint*)(file.getBuffer()+offset))[3];
				GLint height = ((GLint*)(file.getBuffer()+offset))[4];
				offset += sizeof(GLint)*5;
				glCompressedTexImage2D(GL_TEXTURE_2D, level, internalFormat, width, height, 0, compressedSize, (unsigned char*)(file.getBuffer()+offset));
				offset += compressedSize * sizeof(unsigned char) ;
				testGl();
			} while( offset < file.getSize() );
		}
		else {
			ILuint ImageName;
			ilGenImages(1, &ImageName);
			ilBindImage(ImageName);
			unsigned int size = ((unsigned int*)(file.getBuffer()+offset))[0];
			offset+= sizeof(unsigned int);
			ILboolean result = ilLoadL(IL_TYPE_UNKNOWN, (char*)(file.getBuffer()+offset), size);
			if( result == IL_FALSE ) {
				std::ostringstream str;
				str << "Failed to load uncompressd image from: " << iDescription.getFileName();
				throw DevILError(str.str());
			}

			bool needMipMap = true;
			const GLuint colorMode = color;

			if( needMipMap ) {
				gluBuild2DMipmaps(GL_TEXTURE_2D, colorMode,
					ilGetInteger(IL_IMAGE_WIDTH),
					ilGetInteger(IL_IMAGE_HEIGHT),
					ilGetInteger(IL_IMAGE_FORMAT), IL_UNSIGNED_BYTE, ilGetData() );
			}
			else {
				glTexImage2D(GL_TEXTURE_2D, 0, colorMode, ilGetInteger(IL_IMAGE_WIDTH),
					ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), IL_UNSIGNED_BYTE, ilGetData());
			}
		}
	}
	ImageLoaded::~ImageLoaded() {
		glDeleteTextures(1, &mId);
	}

	ImageLoaded* ImageLoaded::Load(const ImageDescriptor& iDescription) {
		//return new ImageLoaded(iDescription);
		return ::lunarlady::Load(iDescription);
	}
	void ImageLoaded::Unload(ImageLoaded* iLoadedImage) {
		::lunarlady::Unload(iLoadedImage);
	}

	int ImageLoaded::getId() const {
		return mId;
	}
}