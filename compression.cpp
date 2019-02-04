#include <string>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>

#include <zlib.h>

char* compress_string(const char* cstr, uint64_t size, uint64_t *size_out)
{
  // Set the compression level
  int compressionlevel = Z_BEST_COMPRESSION;

  // Setup the string
  std::string str(cstr, size);

  z_stream zs;                        // z_stream is zlib's control structure
  memset(&zs, 0, sizeof(zs));

  if (deflateInit(&zs, compressionlevel) != Z_OK)
    throw(std::runtime_error("deflateInit failed while compressing."));

  zs.next_in = (Bytef*)str.data();
  zs.avail_in = str.size();           // set the z_stream's input

  int ret;
  char outbuffer[32768];
  std::string outstring;

  // retrieve the compressed bytes blockwise
  do {
    zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
    zs.avail_out = sizeof(outbuffer);

    ret = deflate(&zs, Z_FINISH);

    if (outstring.size() < zs.total_out) {
      // append the block to the output string
      outstring.append(outbuffer,
                       zs.total_out - outstring.size());
    }
  } while (ret == Z_OK);

  deflateEnd(&zs);

  if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
    std::ostringstream oss;
    oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
    throw(std::runtime_error(oss.str()));
  }

  // Get the size
  *size_out = outstring.size();

  // Get the char array to return
  char* out = new char[*size_out];

  // Loop over the outstring varible
  for(int i=0;i<*size_out;i++)
  {
    // Add the string item to the char array
    out[i] = outstring[i];
  }

  // Return out
  return out;
}

char* decompress_string(const char* cstr, uint64_t size, uint64_t *size_out)
{
  // Setup the string
  std::string str(cstr, size);

  z_stream zs;                        // z_stream is zlib's control structure
  memset(&zs, 0, sizeof(zs));

  if (inflateInit(&zs) != Z_OK)
    throw(std::runtime_error("inflateInit failed while decompressing."));

  zs.next_in = (Bytef*)str.data();
  zs.avail_in = str.size();

  int ret;
  char outbuffer[32768];
  std::string outstring;

  // get the decompressed bytes blockwise using repeated calls to inflate
  do {
    zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
    zs.avail_out = sizeof(outbuffer);

    ret = inflate(&zs, 0);

    if (outstring.size() < zs.total_out) {
      outstring.append(outbuffer,
                       zs.total_out - outstring.size());
    }

  } while (ret == Z_OK);

  inflateEnd(&zs);

  if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
    std::ostringstream oss;
    oss << "Exception during zlib decompression: (" << ret << ") "
        << zs.msg;
    throw(std::runtime_error(oss.str()));
  }

  // Get the size
  *size_out = outstring.size();

  // Get the char array to return
  char* out = new char[*size_out];

  // Loop over the outstring varible
  for(int i=0;i<*size_out;i++)
  {
    // Add the string item to the char array
    out[i] = outstring[i];
  }

  // Return out
  return out;
}
