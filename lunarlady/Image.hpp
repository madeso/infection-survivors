#ifndef LL_IMAGE_HPP
#define LL_IMAGE_HPP

#include <string>

#include "lunarlady/TemplatedMedia.hpp"
#include "lunarlady/Loaded.hpp"

namespace lunarlady {

	class ImageDescriptor {
	public:
		ImageDescriptor(const std::string& iFileName);

		const std::string& getFileName() const;

		bool operator<(const ImageDescriptor& iOther) const ;
	private:
		const std::string mFileName;
	};

	class ImageLoaded : public Loaded {
	public:
		ImageLoaded(const ImageDescriptor& iDescription);
		~ImageLoaded();

		int getId() const;

		static ImageLoaded* Load(const ImageDescriptor& iDescription);
		static void Unload(ImageLoaded* iLoadedImage);
	private:
		unsigned int mId;
	};

	typedef TemplatedMedia<ImageLoaded, ImageDescriptor> Image;
}

#endif