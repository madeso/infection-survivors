#include "wx.hpp"
#include "wx/filedlg.h"
#include <GL/glew.h>
#include "wx_opengl.hpp"
#include "wx/filename.h"
#include "wx/dnd.h"
#include <string>
#include <sstream>
#include <GL/glext.h>
#include "IL/il.h"
#include "boost/smart_ptr.hpp"
#include <fstream>

#define APP_NAME "Build Image"

void DisplayError(const std::string& iMessage) {
	wxMessageBox(iMessage.c_str(), APP_NAME " Error", wxOK | wxICON_ERROR);
}

struct ConvertData {
	std::string inFile;
	std::string outFile;
	bool compress;
	bool uncompress;
	GLint min;
	GLint mag;
	GLint wrapS;
	GLint wrapT;
	GLint enviroment;
	GLint compressionAlgo;
	GLint color;

	bool needMipmap() const {
		bool minFilter = FilterNeedsMipmap(min);
		bool magFilter = FilterNeedsMipmap(mag);
		return minFilter || magFilter;
	}

	static bool FilterNeedsMipmap(GLint iFilter) {
		return ( iFilter != GL_NEAREST
			&& iFilter != GL_LINEAR );
	}
};

void DisplayGLError(GLint error) {
	switch(error) {
#define HANDLE_ERROR(e) case e: DisplayError("OpenGL error: " #e "!!!"); break;
	HANDLE_ERROR(GL_NO_ERROR)
	HANDLE_ERROR(GL_INVALID_ENUM)
	HANDLE_ERROR(GL_INVALID_VALUE)
	HANDLE_ERROR(GL_INVALID_OPERATION)
	HANDLE_ERROR(GL_STACK_OVERFLOW)
	HANDLE_ERROR(GL_STACK_UNDERFLOW)
	HANDLE_ERROR(GL_OUT_OF_MEMORY)
#undef HANDLE_ERROR
	default:
			DisplayError( (char*)gluErrorString(error) );
			break;
	}
}

void DisplayError(int error) {
	switch(error) {
#define HANDLE_ERROR(e) case e: DisplayError("Devil error: " #e "!!!"); break;
		HANDLE_ERROR(IL_NO_ERROR)
		HANDLE_ERROR(IL_INVALID_ENUM)
		HANDLE_ERROR(IL_OUT_OF_MEMORY)
		HANDLE_ERROR(IL_FORMAT_NOT_SUPPORTED)
		HANDLE_ERROR(IL_INTERNAL_ERROR)
		HANDLE_ERROR(IL_INVALID_VALUE)
		HANDLE_ERROR(IL_ILLEGAL_OPERATION)
		HANDLE_ERROR(IL_ILLEGAL_FILE_VALUE)
		HANDLE_ERROR(IL_INVALID_FILE_HEADER)
		HANDLE_ERROR(IL_INVALID_PARAM)
		HANDLE_ERROR(IL_COULD_NOT_OPEN_FILE)
		HANDLE_ERROR(IL_INVALID_EXTENSION)
		HANDLE_ERROR(IL_FILE_ALREADY_EXISTS)
		HANDLE_ERROR(IL_OUT_FORMAT_SAME)
		HANDLE_ERROR(IL_STACK_OVERFLOW)
		HANDLE_ERROR(IL_STACK_UNDERFLOW)
		HANDLE_ERROR(IL_INVALID_CONVERSION)
		HANDLE_ERROR(IL_BAD_DIMENSIONS)
		HANDLE_ERROR(IL_FILE_READ_ERROR)
		//HANDLE_ERROR(IL_FILE_WRITE_ERROR)

		HANDLE_ERROR(IL_LIB_GIF_ERROR)
		HANDLE_ERROR(IL_LIB_JPEG_ERROR)
		HANDLE_ERROR(IL_LIB_PNG_ERROR)
		HANDLE_ERROR(IL_LIB_TIFF_ERROR)
		HANDLE_ERROR(IL_LIB_MNG_ERROR)
		HANDLE_ERROR(IL_UNKNOWN_ERROR)
#undef HANDLE_ERROR
		default:
			DisplayError("Unkown default Devil error, shouldnt ever happen");
			break;
	}
}

enum {
	DIR_BUTTON = 100,
	CONVERT_BUTTON
};

class ImgFrame;

class ConvertImageTarget : public wxFileDropTarget {
public:
	ConvertImageTarget(ImgFrame* frame) : mFrame(frame) {
	}
	bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);
private:
	ImgFrame* mFrame;
};

class ImgFrame : public wxFrame {
public:
	ImgFrame() : wxFrame(NULL, wxID_ANY, APP_NAME, wxDefaultPosition, wxDefaultSize, wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN /*| wxSTAY_ON_TOP*/){
		wxPanel* panel = new wxPanel(this);
		sizer = new wxFlexGridSizer(2,0);
		const int space = 5;
		
		const int actionCount = 5;
		const wxString actionChoises[actionCount] = {wxString("LLT"),
			wxString("BMP"),
			wxString("Jpeg"),
			wxString("PNG"),
			wxString("TIF")};
		mAction = new wxComboBox(panel, -1, actionChoises[0], wxDefaultPosition, wxDefaultSize, actionCount, actionChoises, wxCB_DROPDOWN|wxCB_READONLY);
		sizer->Add( new wxStaticText(panel, -1, "Convert to", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), 0, wxEXPAND | wxALL, space);
		sizer->Add( mAction, 0, wxEXPAND | wxALL, space);

		sizer->AddStretchSpacer();
		sizer->Add( new wxStaticText(panel, -1, "Drop files or hit convert", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), 0, wxALL, space);

		const int minCount = 6;
		const wxString minChoises[minCount] = {wxString("Nearest"),
			wxString("Linear"),
			wxString("Nearest mipmap Nearest"),
			wxString("Nearest mipmap Linear"),
			wxString("Linear mipmap Nearest"),
			wxString("Linear mipmap Linear")};
		const int magCount = 2;
		const wxString magChoises[magCount] = {wxString("Nearest"), wxString("Linear")};
		const int wrapCount = 2;
		const wxString wrapChoises[wrapCount] = {wxString("Repeat"), wxString("Clamp")};

		mMin = new wxComboBox(panel, -1, minChoises[5], wxDefaultPosition, wxDefaultSize, minCount, minChoises, wxCB_DROPDOWN|wxCB_READONLY);
		mMag = new wxComboBox(panel, -1, magChoises[1], wxDefaultPosition, wxDefaultSize, magCount, magChoises, wxCB_DROPDOWN|wxCB_READONLY);
		mWrapS = new wxComboBox(panel, -1, wrapChoises[0], wxDefaultPosition, wxDefaultSize, wrapCount, wrapChoises, wxCB_DROPDOWN|wxCB_READONLY);
		mWrapT = new wxComboBox(panel, -1, wrapChoises[0], wxDefaultPosition, wxDefaultSize, wrapCount, wrapChoises, wxCB_DROPDOWN|wxCB_READONLY);
		
		sizer->Add( new wxStaticText(panel, -1, "Min filter", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), 0, wxEXPAND | wxALL, space);
		sizer->Add( mMin, 0, wxEXPAND | wxALL, space);

		sizer->Add( new wxStaticText(panel, -1, "Mag filter", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), 0, wxEXPAND | wxALL, space);
		sizer->Add( mMag, 0, wxEXPAND | wxALL, space);

		sizer->Add( new wxStaticText(panel, -1, "Wrap S", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), 0, wxEXPAND | wxALL, space);
		sizer->Add( mWrapS, 0, wxEXPAND | wxALL, space);

		sizer->Add( new wxStaticText(panel, -1, "Wrap T", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), 0, wxEXPAND | wxALL, space);
		sizer->Add( mWrapT, 0, wxEXPAND | wxALL, space);

		const int enviromentCount = 4;
		const wxString enviromentChoises[enviromentCount] = {wxString("Modulate"), wxString("Decal"),
			wxString("Blend"),
			wxString("Replace")};
		mEnviroment = new wxComboBox(panel, -1, enviromentChoises[0], wxDefaultPosition, wxDefaultSize, enviromentCount, enviromentChoises, wxCB_DROPDOWN|wxCB_READONLY);
		sizer->Add( new wxStaticText(panel, -1, "Enviroment", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), 0, wxEXPAND | wxALL, space);
		sizer->Add( mEnviroment, 0, wxEXPAND | wxALL, space);

		const int compressionCount = 4;
		const wxString compressionChoises[compressionCount] = {
			wxString("Opaque DXT1 8:1"),
			wxString("1-bit alpha DXT1 6:1"),
			wxString("Explicit alpha DXT3 4:1"),
			wxString("Interpolated alpha DXT5 4:1")};
		mCompressionAlgo = new wxComboBox(panel, -1, compressionChoises[0], wxDefaultPosition, wxDefaultSize, compressionCount, compressionChoises, wxCB_DROPDOWN|wxCB_READONLY);
		sizer->Add( new wxStaticText(panel, -1, "Compression Algo", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), 0, wxEXPAND | wxALL, space);
		sizer->Add( mCompressionAlgo, 0, wxEXPAND | wxALL, space);

		mIncludeCompressed = new wxCheckBox(panel, -1, "Include Compressed");
		sizer->AddStretchSpacer();
		sizer->Add( mIncludeCompressed, 0, wxEXPAND | wxALL, space);
		mIncludeCompressed->SetValue( true );

		mIncludeUncompressed = new wxCheckBox(panel, -1, "Include Un-compressed");
		sizer->AddStretchSpacer();
		sizer->Add( mIncludeUncompressed, 0, wxEXPAND | wxALL, space);
		mIncludeUncompressed->SetValue( true );
		
		mGLCanvas = new wxGLCanvas(panel);
		sizer->Add( mGLCanvas, 0, wxEXPAND | wxALL, space);
		sizer->Add( new wxButton(panel, CONVERT_BUTTON, "Convert!"), 0, wxALL, space);
		panel->SetSizer(sizer);

		SetDropTarget( new ConvertImageTarget(this) );

		Layout();
		panel->Layout();
		Fit();
		SetMinSize(sizer->Fit(this));
	}
	static GLint ConvertToGLWrap(int iIndex) {
		if( iIndex == 0 ) {
			return GL_REPEAT;
		}
		else {
			return GL_CLAMP;
		}
	}
	static GLint ConvertToGLFilter(int iIndex) {
		switch(iIndex) {
			case 0: return GL_NEAREST;
			case 1: return GL_LINEAR;
			case 2: return GL_NEAREST_MIPMAP_NEAREST;
			case 3: return GL_NEAREST_MIPMAP_LINEAR;
			case 4: return GL_LINEAR_MIPMAP_NEAREST;
			case 5:
			default: return GL_LINEAR_MIPMAP_LINEAR;
		}
	}

	static GLint ConvertToGLEnviroment(int iIndex) {
		switch(iIndex) {
			case 0: return GL_MODULATE;
			case 1: return GL_DECAL;
			case 2: return GL_BLEND;
			case 3:
			default: return GL_REPLACE;
		}
	}
	static GLint ConvertToGLAlgo(int iIndex) {
		switch(iIndex) {
			case 0:
				return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			case 1:
				return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			case 2:
				return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			case 3:
			default:
				return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		}
	}
	static GLint ConvertToGLColor(int iIndex) {
		switch(iIndex) {
			case 0:
				return GL_RGB;
			case 1:
				return GL_RGBA;
			case 2:
				return GL_RGBA;
			case 3:
			default:
				return GL_RGBA;
		}
	}
	~ImgFrame() {
	}

#pragma warning(push)
#pragma warning(disable:4100)
	void OnConvert(wxCommandEvent& event) {
		wxFileDialog dialog(this, "Select files to convert", wxEmptyString, wxEmptyString,
			"Image files|*.bmp;*.cut;*.dcx;*.dds;*.ico;*.gif;"
			"*.jpg;*.lbm;*.lif;*.mdl;*.pcd;*.pcx;*.pic;*.png;"
			"*.pnm;*.psd;*.psp;*.raw;*.sgi;*.tga;*.tif;*.wal;"
			"*.act;*.pal;*.hdr;*.jpeg", wxOPEN | wxFILE_MUST_EXIST |wxMULTIPLE);
		if( dialog.ShowModal() == wxID_OK ) {
			wxArrayString fileNames;
			dialog.GetPaths(fileNames);
			convert(fileNames);
		}
	}
#pragma warning(pop)

	void convert(const wxArrayString fileNames) {
		Raise();
		wxBeginBusyCursor();
		bool special = false;
		std::string ext = "";
		const int action = mAction->GetSelection();
		switch( action ) {
			case 0:
				special = true;
				ext = "llt";
				break;
			case 1:
				ext = "bmp";
				special = false;
				break;
			case 2:
				ext = "jpeg";
				special = false;
				break;
			case 3:
				ext = "png";
				special = false;
				break;
			case 4:
				ext = "tif";
				special = false;
				break;
		}

		mGLCanvas->SetCurrent();
		glEnable(GL_TEXTURE_2D);

		for(unsigned int i=0; i<fileNames.Count(); ++i) {
			wxFileName file(fileNames[i]);
			const std::string inFileName = file.GetFullPath();
			file.Assign(file.GetPath(), file.GetName(), ext.c_str(), wxPATH_NATIVE);
			const std::string outFileName = file.GetFullPath();

			if( special ) {
				ConvertData data;
				data.inFile = inFileName;
				data.outFile = outFileName;
				data.compress = mIncludeCompressed->GetValue();
				data.uncompress = mIncludeUncompressed->GetValue();
				data.wrapS = ConvertToGLWrap(mWrapS->GetSelection());
				data.wrapT = ConvertToGLWrap(mWrapT->GetSelection());
				data.mag = ConvertToGLFilter(mMag->GetSelection());
				data.min = ConvertToGLFilter(mMin->GetSelection());
				data.compressionAlgo = ConvertToGLAlgo( mCompressionAlgo->GetSelection() );
				data.enviroment = ConvertToGLEnviroment(mEnviroment->GetSelection());
				data.color = ConvertToGLColor( mCompressionAlgo->GetSelection() );
				convert(data);
			}
			else {
				ILuint ImageName = 0;
				ILenum Error;
#define DEVIL(x) if(Error == IL_NO_ERROR ) do { x; Error = ilGetError(); } while(false)
				Error = ilGetError();
				DEVIL(ilGenImages(1, &ImageName));
				DEVIL(ilBindImage(ImageName));
				DEVIL(ilLoadImage(inFileName.c_str()));
				DEVIL(ilEnable(IL_FILE_OVERWRITE));
				DEVIL(ilSaveImage(outFileName.c_str()));

				if( Error != IL_NO_ERROR ) {
					DisplayError(Error);
				}
			}
		}
		wxEndBusyCursor();
		wxBell();
		::wxMessageBox("Conversion done", APP_NAME, wxICON_INFORMATION | wxOK);
	}

	static void convert(const ConvertData& iConvertData) {
		ILuint ImageName = 0;
		ILenum Error = 0;
		GLint errorGl = glGetError();
#define DEVIL(x) if(Error == IL_NO_ERROR ) do { x; Error = ilGetError(); } while(false)
#define OPENGL(x) if(errorGl == GL_NO_ERROR ) do { x; errorGl = glGetError(); } while(false)
		Error = ilGetError();
		DEVIL(ilGenImages(1, &ImageName));
		DEVIL(ilBindImage(ImageName));
		DEVIL(ilLoadImage( iConvertData.inFile.c_str() ));
		if( Error != IL_NO_ERROR ) {
			DisplayError(Error);
			return;
		}
		GLuint tex = 0;
		OPENGL( glGenTextures(1, &tex) );
		OPENGL( glBindTexture(GL_TEXTURE_2D, tex) );
		OPENGL( glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, iConvertData.enviroment) );
		OPENGL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, iConvertData.min) );
		OPENGL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, iConvertData.mag) );
		OPENGL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, iConvertData.wrapS) );
		OPENGL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, iConvertData.wrapT) );
		
		std::ofstream file(iConvertData.outFile.c_str(), std::ios::out | std::ios::binary);
		if( !file ) {
			DisplayError("Failed to open file for writing");
			return;
		}
		char haveCompressedImage = (iConvertData.compress)?1:0;
		char haveUncompressedImage = (iConvertData.uncompress)?1:0;
		file.write(&haveCompressedImage, sizeof(char));
		file.write(&haveUncompressedImage, sizeof(char));
		file.write((char*) &iConvertData.enviroment, sizeof(GLint));
		file.write((char*) &iConvertData.min, sizeof(GLint));
		file.write((char*) &iConvertData.mag, sizeof(GLint));
		file.write((char*) &iConvertData.wrapS, sizeof(GLint));
		file.write((char*) &iConvertData.wrapT, sizeof(GLint));
		file.write((char*) &iConvertData.color, sizeof(GLint));

		// save "uncompressed" data
		if( iConvertData.uncompress ) {
			std::ifstream texture(iConvertData.inFile.c_str(), std::ios::out | std::ios::binary);
			if( texture ) {
				texture.seekg(0, std::ios::end);
				unsigned int fileSize = texture.tellg();
				texture.seekg(0, std::ios::beg);
				texture.clear();

				boost::scoped_array<char> buffer( new char[fileSize] );
				texture.read(buffer.get(), fileSize);

				file.write((char*) &fileSize, sizeof(unsigned int));
				file.write((char*) buffer.get(), fileSize);
			}
			else {
				DisplayError("Failed to open texture file");
				return;
			}
		}

		// save compressed data
		if( iConvertData.compress ) {
			GLint colorMode = iConvertData.compressionAlgo;
			if( iConvertData.needMipmap() ) {
				OPENGL( gluBuild2DMipmaps(GL_TEXTURE_2D, colorMode,
					ilGetInteger(IL_IMAGE_WIDTH),
					ilGetInteger(IL_IMAGE_HEIGHT),
					ilGetInteger(IL_IMAGE_FORMAT), IL_UNSIGNED_BYTE, ilGetData() ));
			}
			else {
				OPENGL( glTexImage2D(GL_TEXTURE_2D, 0, colorMode, ilGetInteger(IL_IMAGE_WIDTH),
					ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), IL_UNSIGNED_BYTE, ilGetData()) );
			}
			// if error free:
			if( Error == IL_NO_ERROR && errorGl == GL_NO_ERROR ) {
				GLint height = 0;
				GLint width = 0;
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &height);
				for(unsigned int level=0; height!=0 && width!=0; ++level) {
					glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &width);
					glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &height);
					if( height!=0 && width!=0) {
						GLint compressed = GL_FALSE;
						glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_COMPRESSED_ARB, &compressed);
						if( compressed == GL_TRUE ) {
							GLint internalFormat = 0;
							glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
							GLint compressedSize = 0;
							glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &compressedSize);

							boost::scoped_array<unsigned char> compressedData( new unsigned char[ compressedSize * sizeof(unsigned char) ] );
							glGetCompressedTexImageARB(GL_TEXTURE_2D, level, compressedData.get());
							//SaveTexture(width, height, compressedSize, compressedData, internalFormat, level);
							file.write((char*) &level, sizeof(GLint));
							file.write((char*) &internalFormat, sizeof(GLint));
							file.write((char*) &compressedSize, sizeof(GLint));
							file.write((char*) &width, sizeof(GLint));
							file.write((char*) &height, sizeof(GLint));
							file.write((char*) compressedData.get(), compressedSize * sizeof(unsigned char));
						}
						else {
							std::stringstream str;
							str << "Failed to compress " << iConvertData.inFile << " to " << iConvertData.outFile << "." << std::endl;
							str << "Level: " << level << ", (w,h)=(" << width << ", " << height << ")";
							DisplayError( str.str() );
						}
					}
				}
			}
			glDeleteTextures( 1, &tex );
			glGetError();

			if( errorGl != GL_NO_ERROR ) {
				DisplayGLError(errorGl);
			}

			ilDeleteImage(ImageName);
			ilGetError();

			if( Error != IL_NO_ERROR ) {
				DisplayError(Error);
				return;
			}
		}
	}

	
	DECLARE_EVENT_TABLE()
private:
	wxGLCanvas* mGLCanvas;
	wxComboBox* mCompressionAlgo;

	wxCheckBox* mIncludeCompressed;
	wxCheckBox* mIncludeUncompressed;

	wxComboBox* mMin;
	wxComboBox* mMag;
	wxComboBox* mWrapS;
	wxComboBox* mWrapT;
	wxComboBox* mAction;
	wxComboBox* mEnviroment;
	wxFlexGridSizer* sizer;
};

BEGIN_EVENT_TABLE(ImgFrame, wxFrame)
	EVT_BUTTON(CONVERT_BUTTON, ImgFrame::OnConvert)
END_EVENT_TABLE()

#pragma warning(push)
#pragma warning(disable:4100)
bool ConvertImageTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) {
	mFrame->convert(filenames);
	return true;
}
#pragma warning(pop)

class FontApp : public wxApp {
public:
	virtual bool OnInit();
};

DECLARE_APP(FontApp)
IMPLEMENT_APP(FontApp)

bool FontApp::OnInit() {
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION) {
		DisplayError("DevIL version is different...exiting!");
		return false;
	}
	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

	ImgFrame* frame = new ImgFrame();
	frame->Show(true);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::stringstream str;
		str << "Glew error: " << glewGetErrorString(err);
		DisplayError(str.str());
		return false;
	}

	if( ! (GLEW_ARB_texture_compression && GLEW_EXT_texture_compression_s3tc) ) {
		DisplayError("Missing s3tc texture expression, please don't use or application might crash");
	}

	//SetTopWindow(frame);
	return true;
}