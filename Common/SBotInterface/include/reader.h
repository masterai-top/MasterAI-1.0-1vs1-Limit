#ifndef _IO_H_
#define _IO_H_

#include <memory>
#include <string>
#include <vector>

class Reader {
public:
  Reader(void) {}
  Reader(const char *filename);
  Reader(const char *filename, long long int file_size);
  virtual ~Reader(void);
  bool AtEnd(void) const;
  void SeekTo(long long int offset);
  bool ReadInt(int *i);
  int ReadIntOrDie(void);
  bool ReadUnsignedInt(unsigned int *i);
  unsigned int ReadUnsignedIntOrDie(void);
  bool ReadLong(long long int *l);
  long long int ReadLongOrDie(void);
  bool ReadUnsignedLong(unsigned long long int *u);
  unsigned long long int ReadUnsignedLongOrDie(void);
  bool ReadShort(short *s);
  short ReadShortOrDie(void);
  bool ReadChar(char *c);
  char ReadCharOrDie(void);
  bool ReadUnsignedChar(unsigned char *u);
  unsigned char ReadUnsignedCharOrDie(void);
  bool ReadUnsignedShort(unsigned short *u);
  unsigned short ReadUnsignedShortOrDie(void);
  bool ReadDouble(double *d);
  double ReadDoubleOrDie(void);
  bool ReadFloat(float *f);
  float ReadFloatOrDie(void);
  bool ReadReal(float *f);
  bool ReadReal(double *d);
  void ReadOrDie(unsigned char *c);
  void ReadOrDie(unsigned short *s);
  void ReadOrDie(unsigned int *u);
  void ReadOrDie(int *i);
  void ReadOrDie(double *d);
  bool GetLine(std::string *s);
  bool ReadCString(std::string *s);
  std::string ReadCStringOrDie(void);
  long long int BytePos(void) const {return byte_pos_;}
  long long int FileSize(void) const {return file_size_;}
  void ReadNBytesOrDie(unsigned int num_bytes, unsigned char *buf);
  void ReadEverythingLeft(unsigned char *data);
  int FD(void) const {return fd_;}
  const std::string &Filename(void) const {return filename_;}

 protected:
  void OpenFile(const char *filename);
  virtual bool Refresh(void);

  static const int kBufSize = 65536;

  int fd_;
  std::unique_ptr<unsigned char []> buf_;
  unsigned char *end_read_;
  unsigned char *buf_ptr_;
  int buf_size_;
  long long int file_size_;
  long long int remaining_;
  unsigned char overflow_[100];
  int overflow_size_;
  long long int byte_pos_;
  std::string filename_;
};

Reader *NewReaderMaybe(const char *filename);

bool FileExists(const char *filename);
long long int FileSize(const char *filename);
#endif
