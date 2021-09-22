#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "reader.h"

using std::string;
using std::vector;

bool FileExists(const char *filename)
{
    int fd = open(filename, O_RDONLY, 0);
    if (fd == -1)
    {
        if (errno == ENOENT)
        {
            return false;
        }
        fprintf(stderr, "Failed to open \"%s\", errno %i\n", filename, errno);
        if (errno == 24)
        {
            fprintf(stderr, "errno 24 may indicate too many open files\n");
        }
        exit(-1);
    }
    else
    {
        close(fd);
        return true;
    }
}

long long int FileSize(const char *filename)
{
    struct stat stbuf;
    if (stat(filename, &stbuf) == -1)
    {
        fprintf(stderr, "FileSize: Couldn't access: %s\n", filename);
        exit(-1);
    }
    return stbuf.st_size;
}

void Reader::OpenFile(const char *filename)
{
    filename_ = filename;
    struct stat stbuf;
    if (stat(filename, &stbuf) == -1)
    {
        fprintf(stderr, "Reader::OpenFile: Couldn't access: %s\n", filename);
        exit(-1);
    }
    file_size_ = stbuf.st_size;
    remaining_ = file_size_;

    fd_ = open(filename, O_RDONLY, 0);
    if (fd_ == -1)
    {
        fprintf(stderr, "Failed to open \"%s\", errno %i\n", filename, errno);
        if (errno == 24)
        {
            fprintf(stderr, "errno 24 may indicate too many open files\n");
        }
        exit(-1);
    }

    overflow_size_ = 0;
    byte_pos_ = 0;
}

Reader::Reader(const char *filename)
{
    OpenFile(filename);

    buf_size_ = kBufSize;
    if (remaining_ < buf_size_)
        buf_size_ = remaining_;

    buf_.reset(new unsigned char[buf_size_]);
    buf_ptr_ = buf_.get();
    end_read_ = buf_.get();

    if (!Refresh())
    {
        fprintf(stderr, "Warning: empty file: %s\n", filename);
    }
}

Reader::Reader(const char *filename, long long int file_size)
{
    filename_ = filename;
    file_size_ = file_size;
    remaining_ = file_size;

    fd_ = open(filename, O_RDONLY, 0);
    if (fd_ == -1)
    {
        fprintf(stderr, "Failed to open \"%s\", errno %i\n", filename, errno);
        if (errno == 24)
        {
            fprintf(stderr, "errno 24 may indicate too many open files\n");
        }
        exit(-1);
    }

    overflow_size_ = 0;
    byte_pos_ = 0;

    buf_size_ = kBufSize;
    if (remaining_ < buf_size_)
        buf_size_ = remaining_;

    buf_.reset(new unsigned char[buf_size_]);
    buf_ptr_ = buf_.get();
    end_read_ = buf_.get();

    if (!Refresh())
    {
        fprintf(stderr, "Warning: empty file: %s\n", filename);
    }
}

Reader *NewReaderMaybe(const char *filename)
{
    struct stat stbuf;
    if (stat(filename, &stbuf) == -1)
    {
        return NULL;
    }
    long long int file_size = stbuf.st_size;
    return new Reader(filename, file_size);
}

Reader::~Reader(void)
{
    close(fd_);
}

bool Reader::AtEnd(void) const
{
    return (buf_ptr_ == end_read_ && remaining_ == 0 && overflow_size_ == 0);
}

void Reader::SeekTo(long long int offset)
{
    long long int ret = lseek(fd_, offset, SEEK_SET);
    if (ret == -1)
    {
        fprintf(stderr, "lseek failed, offset %lli, ret %lli, errno %i, fd %i\n",
                offset, ret, errno, fd_);
        fprintf(stderr, "File: %s\n", filename_.c_str());
        exit(-1);
    }
    remaining_ = file_size_ - offset;
    overflow_size_ = 0;
    byte_pos_ = offset;
    Refresh();
}

bool Reader::Refresh(void)
{
    if (remaining_ == 0 && overflow_size_ == 0)
        return false;

    if (overflow_size_ > 0)
    {
        memcpy(buf_.get(), overflow_, overflow_size_);
    }
    buf_ptr_ = buf_.get();

    unsigned char *read_into = buf_.get() + overflow_size_;

    int to_read = buf_size_ - overflow_size_;
    if (to_read > remaining_)
        to_read = remaining_;

    int ret;
    if ((ret = read(fd_, read_into, to_read)) != to_read)
    {
        fprintf(stderr, "Read returned %i not %i\n", ret, to_read);
        fprintf(stderr, "File: %s\n", filename_.c_str());
        fprintf(stderr, "remaining_ %lli\n", remaining_);
        exit(-1);
    }

    remaining_ -= to_read;
    end_read_ = read_into + to_read;
    overflow_size_ = 0;

    return true;
}

bool Reader::GetLine(string *s)
{
    s->clear();
    while (true)
    {
        if (buf_ptr_ == end_read_)
        {
            if (!Refresh())
            {
                return false;
            }
        }
        if (*buf_ptr_ == '\r')
        {
            ++buf_ptr_;
            ++byte_pos_;
            continue;
        }
        if (*buf_ptr_ == '\n')
        {
            ++buf_ptr_;
            ++byte_pos_;
            break;
        }
        s->push_back(*buf_ptr_);
        ++buf_ptr_;
        ++byte_pos_;
    }
    return true;
}

bool Reader::ReadInt(int *i)
{
    if (buf_ptr_ + sizeof(int) > end_read_)
    {
        if (buf_ptr_ < end_read_)
        {
            overflow_size_ = (int)(end_read_ - buf_ptr_);
            memcpy(overflow_, buf_ptr_, overflow_size_);
        }
        if (!Refresh())
        {
            return false;
        }
    }
    char my_buf[4];
    my_buf[0] = *buf_ptr_++;
    my_buf[1] = *buf_ptr_++;
    my_buf[2] = *buf_ptr_++;
    my_buf[3] = *buf_ptr_++;
    byte_pos_ += 4;
    int *int_ptr = reinterpret_cast<int *>(my_buf);
    *i = *int_ptr;
    return true;
}

int Reader::ReadIntOrDie(void)
{
    int i;
    if (!ReadInt(&i))
    {
        fprintf(stderr, "Couldn't read int; file %s byte pos %lli\n",
                filename_.c_str(), byte_pos_);
        exit(-1);
    }
    return i;
}

bool Reader::ReadUnsignedInt(unsigned int *u)
{
    if (buf_ptr_ + sizeof(int) > end_read_)
    {
        if (buf_ptr_ < end_read_)
        {
            overflow_size_ = (int)(end_read_ - buf_ptr_);
            memcpy(overflow_, buf_ptr_, overflow_size_);
        }
        if (!Refresh())
        {
            return false;
        }
    }
    char my_buf[4];
    my_buf[0] = *buf_ptr_++;
    my_buf[1] = *buf_ptr_++;
    my_buf[2] = *buf_ptr_++;
    my_buf[3] = *buf_ptr_++;
    byte_pos_ += 4;
    unsigned int *u_int_ptr = reinterpret_cast<unsigned int *>(my_buf);
    *u = *u_int_ptr;
    return true;
}

unsigned int Reader::ReadUnsignedIntOrDie(void)
{
    unsigned int u;
    if (!ReadUnsignedInt(&u))
    {
        fprintf(stderr, "Couldn't read unsigned int\n");
        fprintf(stderr, "File: %s\n", filename_.c_str());
        fprintf(stderr, "Byte pos: %lli\n", byte_pos_);
        exit(-1);
    }
    return u;
}

bool Reader::ReadLong(long long int *l)
{
    if (buf_ptr_ + sizeof(long long int) > end_read_)
    {
        if (buf_ptr_ < end_read_)
        {
            overflow_size_ = (int)(end_read_ - buf_ptr_);
            memcpy(overflow_, buf_ptr_, overflow_size_);
        }
        if (!Refresh())
        {
            return false;
        }
    }
    *l = *(long long int *)buf_ptr_;
    buf_ptr_ += sizeof(long long int);
    byte_pos_ += sizeof(long long int);
    return true;
}

long long int Reader::ReadLongOrDie(void)
{
    long long int l;
    if (!ReadLong(&l))
    {
        fprintf(stderr, "Couldn't read long\n");
        exit(-1);
    }
    return l;
}

bool Reader::ReadUnsignedLong(unsigned long long int *u)
{
    if (buf_ptr_ + sizeof(unsigned long long int) > end_read_)
    {
        if (buf_ptr_ < end_read_)
        {
            overflow_size_ = (int)(end_read_ - buf_ptr_);
            memcpy(overflow_, buf_ptr_, overflow_size_);
        }
        if (!Refresh())
        {
            return false;
        }
    }
    *u = *(unsigned long long int *)buf_ptr_;
    buf_ptr_ += sizeof(unsigned long long int);
    byte_pos_ += sizeof(unsigned long long int);
    return true;
}

unsigned long long int Reader::ReadUnsignedLongOrDie(void)
{
    unsigned long long int u;
    if (!ReadUnsignedLong(&u))
    {
        fprintf(stderr, "Couldn't read unsigned long\n");
        exit(-1);
    }
    return u;
}

bool Reader::ReadShort(short *s)
{
    if (buf_ptr_ + sizeof(short) > end_read_)
    {
        if (buf_ptr_ < end_read_)
        {
            overflow_size_ = (int)(end_read_ - buf_ptr_);
            memcpy(overflow_, buf_ptr_, overflow_size_);
        }
        if (!Refresh())
        {
            return false;
        }
    }
    // Possible alignment issue?
    *s = *(short *)buf_ptr_;
    buf_ptr_ += sizeof(short);
    byte_pos_ += sizeof(short);
    return true;
}

short Reader::ReadShortOrDie(void)
{
    short s;
    if (!ReadShort(&s))
    {
        fprintf(stderr, "Couldn't read short\n");
        exit(-1);
    }
    return s;
}

bool Reader::ReadUnsignedShort(unsigned short *u)
{
    if (buf_ptr_ + sizeof(unsigned short) > end_read_)
    {
        if (buf_ptr_ < end_read_)
        {
            overflow_size_ = (int)(end_read_ - buf_ptr_);
            memcpy(overflow_, buf_ptr_, overflow_size_);
        }
        if (!Refresh())
        {
            return false;
        }
    }
    *u = *(unsigned short *)buf_ptr_;
    buf_ptr_ += sizeof(unsigned short);
    byte_pos_ += sizeof(unsigned short);
    return true;
}

unsigned short Reader::ReadUnsignedShortOrDie(void)
{
    unsigned short s;
    if (!ReadUnsignedShort(&s))
    {
        fprintf(stderr, "Couldn't read unsigned short; file %s byte pos %lli "
                "file_size %lli\n",
                filename_.c_str(), byte_pos_, file_size_);
        exit(-1);
    }
    return s;
}

bool Reader::ReadChar(char *c)
{
    if (buf_ptr_ + sizeof(char) > end_read_)
    {
        if (buf_ptr_ < end_read_)
        {
            overflow_size_ = (int)(end_read_ - buf_ptr_);
            memcpy(overflow_, buf_ptr_, overflow_size_);
        }
        if (!Refresh())
        {
            return false;
        }
    }
    *c = *(char *)buf_ptr_;
    buf_ptr_ += sizeof(char);
    byte_pos_ += sizeof(char);
    return true;
}

char Reader::ReadCharOrDie(void)
{
    char c;
    if (!ReadChar(&c))
    {
        fprintf(stderr, "Couldn't read char\n");
        exit(-1);
    }
    return c;
}

bool Reader::ReadUnsignedChar(unsigned char *u)
{
    if (buf_ptr_ + sizeof(unsigned char) > end_read_)
    {
        if (buf_ptr_ < end_read_)
        {
            overflow_size_ = (int)(end_read_ - buf_ptr_);
            memcpy(overflow_, buf_ptr_, overflow_size_);
        }
        if (!Refresh())
        {
            return false;
        }
    }
    *u = *(unsigned char *)buf_ptr_;
    buf_ptr_ += sizeof(unsigned char);
    byte_pos_ += sizeof(unsigned char);
    return true;
}

unsigned char Reader::ReadUnsignedCharOrDie(void)
{
    unsigned char u;
    if (!ReadUnsignedChar(&u))
    {
        fprintf(stderr, "Couldn't read unsigned char\n");
        fprintf(stderr, "File: %s\n", filename_.c_str());
        fprintf(stderr, "Byte pos: %lli\n", byte_pos_);
        exit(-1);
    }
    return u;
}

void Reader::ReadOrDie(unsigned char *c)
{
    *c = ReadUnsignedCharOrDie();
}

void Reader::ReadOrDie(unsigned short *s)
{
    *s = ReadUnsignedShortOrDie();
}

void Reader::ReadOrDie(unsigned int *u)
{
    *u = ReadUnsignedIntOrDie();
}

void Reader::ReadOrDie(int *i)
{
    *i = ReadIntOrDie();
}

void Reader::ReadOrDie(double *d)
{
    *d = ReadDoubleOrDie();
}

void Reader::ReadNBytesOrDie(unsigned int num_bytes, unsigned char *buf)
{
    for (unsigned int i = 0; i < num_bytes; ++i)
    {
        if (buf_ptr_ + 1 > end_read_)
        {
            if (!Refresh())
            {
                fprintf(stderr, "Couldn't read %i bytes\n", num_bytes);
                fprintf(stderr, "Filename: %s\n", filename_.c_str());
                fprintf(stderr, "File size: %lli\n", file_size_);
                fprintf(stderr, "Before read byte pos: %lli\n", byte_pos_);
                fprintf(stderr, "Overflow size: %i\n", overflow_size_);
                fprintf(stderr, "i %i\n", i);
                exit(-1);
            }
        }
        buf[i] = *buf_ptr_++;
        ++byte_pos_;
    }
}

void Reader::ReadEverythingLeft(unsigned char *data)
{
    unsigned long long int data_pos = 0ULL;
    unsigned long long int left = file_size_ - byte_pos_;
    while (left > 0)
    {
        unsigned long long int num_bytes = end_read_ - buf_ptr_;
        memcpy(data + data_pos, buf_ptr_, num_bytes);
        buf_ptr_ = end_read_;
        data_pos += num_bytes;
        if (data_pos > left)
        {
            fprintf(stderr, "ReadEverythingLeft: read too much?!?\n");
            exit(-1);
        }
        else if (data_pos == left)
        {
            break;
        }
        if (!Refresh())
        {
            fprintf(stderr, "ReadEverythingLeft: premature EOF?!?\n");
            exit(-1);
        }
    }
}

bool Reader::ReadCString(string *s)
{
    *s = "";
    while (true)
    {
        if (buf_ptr_ + 1 > end_read_)
        {
            if (!Refresh())
            {
                return false;
            }
        }
        char c = *buf_ptr_++;
        ++byte_pos_;
        if (c == 0)
            return true;
        *s += c;
    }
}

string Reader::ReadCStringOrDie(void)
{
    string s;
    if (!ReadCString(&s))
    {
        fprintf(stderr, "Couldn't read string\n");
        exit(-1);
    }
    return s;
}

bool Reader::ReadDouble(double *d)
{
    if (buf_ptr_ + sizeof(double) > end_read_)
    {
        if (buf_ptr_ < end_read_)
        {
            overflow_size_ = (int)(end_read_ - buf_ptr_);
            memcpy(overflow_, buf_ptr_, overflow_size_);
        }
        if (!Refresh())
        {
            return false;
        }
    }
    *d = *(double *)buf_ptr_;
    buf_ptr_ += sizeof(double);
    byte_pos_ += sizeof(double);
    return true;
}

double Reader::ReadDoubleOrDie(void)
{
    double d;
    if (!ReadDouble(&d))
    {
        fprintf(stderr, "Couldn't read double: file %s byte pos %lli\n",
                filename_.c_str(), byte_pos_);
        exit(-1);
    }
    return d;
}

bool Reader::ReadFloat(float *f)
{
    if (buf_ptr_ + sizeof(float) > end_read_)
    {
        if (buf_ptr_ < end_read_)
        {
            overflow_size_ = (int)(end_read_ - buf_ptr_);
            memcpy(overflow_, buf_ptr_, overflow_size_);
        }
        if (!Refresh())
        {
            return false;
        }
    }
    *f = *(float *)buf_ptr_;
    buf_ptr_ += sizeof(float);
    byte_pos_ += sizeof(float);
    return true;
}

float Reader::ReadFloatOrDie(void)
{
    float f;
    if (!ReadFloat(&f))
    {
        fprintf(stderr, "Couldn't read float: file %s\n", filename_.c_str());
        exit(-1);
    }
    return f;
}

bool Reader::ReadReal(double *d)
{
    if (buf_ptr_ + sizeof(double) > end_read_)
    {
        if (buf_ptr_ < end_read_)
        {
            overflow_size_ = (int)(end_read_ - buf_ptr_);
            memcpy(overflow_, buf_ptr_, overflow_size_);
        }
        if (!Refresh())
        {
            return false;
        }
    }
    *d = *(double *)buf_ptr_;
    buf_ptr_ += sizeof(double);
    byte_pos_ += sizeof(double);
    return true;
}

bool Reader::ReadReal(float *f)
{
    if (buf_ptr_ + sizeof(float) > end_read_)
    {
        if (buf_ptr_ < end_read_)
        {
            overflow_size_ = (int)(end_read_ - buf_ptr_);
            memcpy(overflow_, buf_ptr_, overflow_size_);
        }
        if (!Refresh())
        {
            return false;
        }
    }
    *f = *(float *)buf_ptr_;
    buf_ptr_ += sizeof(float);
    byte_pos_ += sizeof(float);
    return true;
}
