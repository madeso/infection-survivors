class IMedia {
  virtual void load() = 0;
  virtual void unload() = 0;
  bool isLoaded();
  
  bool isLoaded;
  bool isInHardwareDependent; // load/unload when switching basic hw stuff
  IMedia* dependentUpon;
  list<IMedia*> dependentUponMe;
};

class ImageMedia : public Media {
  string file;
  Egine* engine;
  void load() {
    data = new ImageData(file, engine->getQuality());
  }
  void unload() {
    data.release();
  }
  ImageData* getData() {
    return data.get();
  }
  auto_ptr<ImageData> data;
};

class ImageData {
  ImageData(string file, Quality q) {
  }
};

template <class T, class D>
class MediaPointer {
  MediaPointer(T* p) {
    p->increaseUsage();
    t = p;
  }
  ~MediaPointer() {
    t->decreaseUsage();
  }
  D* operator->() {
    assert( t->isLoaded() );
    return t->getData();
  }
  T* t;
};

typedef MediaPointer<ImageMedia> Image

// usage:

class Dude {
  Image standing;
  Image walking[3];
  Image falling;
  CompiledMesh mesh;
  Dude() {
    standing = engine.getImage("data.png");
  }
};
