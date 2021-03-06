#include "LogStream.h"

#include <algorithm>
#include <limits>
#include <boost/static_assert.hpp>
#include <string.h>
#include <stdint.h>

using namespace muduo;
using namespace muduo::detail;

namespace muduo
{
namespace detail
{

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
BOOST_STATIC_ASSERT(sizeof(digits) == 20);

const char digitsHex[] = "0123456789ABCDEF";
BOOST_STATIC_ASSERT(sizeof digitsHex == 17);

// Efficient Integer to String Conversions, by Matthew Wilson.
template<typename T>
int convert(char buf[], T value)
{
  T i = value;
  char* p = buf;

  do
  {
    int lsd = i % 10;
    i /= 10;
    *p++ = zero[lsd];
  } while (i != 0);

  if (value < 0)
  {
    *p++ = '-';
  }
  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

int convertHex(char buf[], uintptr_t value)
{
  uintptr_t i = value;
  char* p = buf;

  do
  {
    int lsd = i % 16;
    i /= 16;
    *p++ = digitsHex[lsd];
  } while (i != 0);

  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

}
}

FixedBuffer::FixedBuffer()
{
  cur_ = data_;
}

void FixedBuffer::append(const char* /*restrict*/ buf, int len)
{
  if (cur_+len < end());
  {
    memcpy(cur_, buf, len);
    cur_ += len;
  }
}

const char* FixedBuffer::debugString()
{
  *cur_ = '\0';
  return data_;
}

void LogStream::staticCheck()
{
  BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<double>::digits10);
  BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10);
  BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<long>::digits10);
  BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10);
}

template<typename T>
void LogStream::formatInteger(T v)
{
  if (buffer_.avail() >= kMaxNumericSize)
  {
    int len = convert(buffer_.buffer(), v);
    buffer_.add(len);
  }
}

LogStream::LogStream()
{
}

LogStream& LogStream::operator<<(bool v)
{
  buffer_.append(v ? "1" : "0", 1);
  return *this;
}

LogStream& LogStream::operator<<(short v)
{
  *this << static_cast<int>(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned short v)
{
  *this << static_cast<unsigned int>(v);
  return *this;
}

LogStream& LogStream::operator<<(int v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned int v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long long v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(const void* p)
{
  uintptr_t v = reinterpret_cast<uintptr_t>(p);
  if (buffer_.avail() >= kMaxNumericSize)
  {
    char* buf = buffer_.buffer();
    buf[0] = '0';
    buf[1] = 'x';
    int len = convertHex(buf+2, v);
    buffer_.add(len+2);
  }
  return *this;
}

LogStream& LogStream::operator<<(float v)
{
  *this << static_cast<double>(v);
  return *this;
}

// FIXME: replace this with Grisu3 by Florian Loitsch.
LogStream& LogStream::operator<<(double v)
{
  if (buffer_.avail() >= kMaxNumericSize)
  {
    int len = snprintf(buffer_.buffer(), kMaxNumericSize, "%.12g", v);
    buffer_.add(len);
  }
  return *this;
}

LogStream& LogStream::operator<<(char v)
{
  buffer_.append(&v, 1);
  return *this;
}

LogStream& LogStream::operator<<(const char* v)
{
  buffer_.append(v, strlen(v));
  return *this;
}

LogStream& LogStream::operator<<(const string& v)
{
  buffer_.append(v.c_str(), v.size());
  return *this;
}

