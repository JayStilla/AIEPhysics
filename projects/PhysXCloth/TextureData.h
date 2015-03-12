#ifndef _TEXTURE2D_H_
#define _TEXTURE2D_H_

#include <stb_image.h>
#include <gl\glew.h>

class TextureData
{
public:
	TextureData()
	{
	}

	TextureData(const char * filePath, GLenum target = GL_TEXTURE_2D)
	{
		LoadTexture(filePath, target);
	}

	~TextureData()
	{
		// http://stackoverflow.com/questions/328834/c-delete-vs-free-and-performance
		// call free anyway when it's over!
		//FreeTexture();
	}

	GLuint textureID;

	int width = 0;
	int height = 0;
	int format = 0;

	int LoadTexture(const char * filePath, GLenum target = GL_TEXTURE_2D)
	{
		pixelData = stbi_load(filePath, &width, &height, &format, STBI_default);

		if (pixelData != nullptr)
		{
			printf("--TextureLoad--\nFile: %s\nWidth: %i\nHeight: %i\nFormat: %i\n", filePath, width, height, format);
		}
		else
		{
			printf("Could not load data.\n");
			printf("File: %s\nWidth: %i\nHeight: %i\nFormat: %i\n", filePath, width, height, format);
			return -1;
		}
		printf("\n");

		// Convert format from STBI format to OpenGL formnat
		switch (format)
		{
		case STBI_grey:
		{
						  format = GL_LUMINANCE;
						  break;
		}
		case STBI_grey_alpha:
		{
								format = GL_LUMINANCE_ALPHA;
								break;
		}
		case STBI_rgb:
		{
						 format = GL_RGB;
						 break;
		}
		case STBI_rgb_alpha:
		{
							   format = GL_RGBA;
							   break;
		}
		default:
		{
				   printf("WARNING: Texture format not supported!\n");
		}
		};

		glGenTextures(1, &textureID);
		glBindTexture(target, textureID);
		glTexImage2D(target,
			0,
			format,
			width,
			height,
			0,
			format,
			GL_UNSIGNED_BYTE,
			pixelData);

		if (target == GL_TEXTURE_2D)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		// Set filtering mode
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// look into generating mipmaps

		// Unbind texture so another function won't accidentally change it
		glBindTexture(target, 0);

		return textureID;
	}

	void FreeTexture()
	{
		// http://stackoverflow.com/questions/328834/c-delete-vs-free-and-performance
		// call free anyway when it's over!
		free(pixelData);
	}
private:
	unsigned char * pixelData;
};

#endif