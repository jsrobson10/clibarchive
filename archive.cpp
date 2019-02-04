#include <vector>
#include <string>
#include <fstream>
#include <streambuf>
#include <cstring>

#include "archive.h"
#include "compression.h"

std::vector<std::string> archives;

void stringRead(std::string str, int* pos, char* data, int size)
{
  // Loop over part of the string
  for(int i = 0; i < size && i < str.size(); i++)
  {
    // Set part of the data to part of the string
    data[i] = str[*pos+i];
  }

  // Add to the position
  *pos += size;
}

void stringWrite(std::string &str, const char* data, int size)
{
  // Loop over the data
  for(int i = 0; i < size; i++)
  {
    // Add it to the string
    str += data[i];
  }
}

int archiveGen(const char* data, int size, bool compression)
{
  // Create the loaded archive data file
  std::string sdata;

  // Is there decompression needed
  if(compression)
  {
    // Get the size of the uncompressed string
    uint64_t csize;

    // Decompress the string
    const char* cdata = decompress_string(data, size, &csize);

    // Load the archives data
    sdata.append(cdata, csize);
  }

  else
  {
    // Load the archives data
    sdata.append(data, size);
  }

  // Push back the archive
  archives.push_back(sdata);

  // Return the archives position
  return archives.size()-1;
}

int archiveLoad(const char* dir, bool compression)
{
  // Open the file
  std::ifstream file(dir, std::ios::binary);

  // Is the file good
  if(file.good())
  {
    // Read the data
    std::istreambuf_iterator<char> begin(file), end;
    std::string data(begin, end);

    // Is there decompression needed
    if(compression)
    {
      // Convert the string to a c string
      int csize = data.size();
      char* cdata = new char[csize];

      // Loop over the string
      for(int i=0;i<csize;i++)
      {
        // Push the string item to the cstring
        cdata[i] = data[i];
      }

      // Get the decompressed size
      uint64_t nsize;

      // Clear the data varible
      data = "";

      // Decompress the data
      const char* ndata = decompress_string(cdata, csize, &nsize);
      data.append(ndata, nsize);
    }

    // Push back the archive
    archives.push_back(data);

    // Return the archives position
    return archives.size()-1;
  }

  else
  {
    // Return an error message, -1
    return -1;
  }
}

const char* archiveGetData(int a, uint64_t* size, bool compression)
{
  // Get the data
  const char* data = archives[a].c_str();
  uint64_t dsize = archives[a].size();

  // Does the user want compression
  if(compression)
  {
    // Return compressed data
    return compress_string(data, dsize, size);
  }

  else
  {
    // Set the size
    *size = dsize;

    // Return the normal data
    return data;
  }
}

bool archiveSave(int a, const char* dir, bool compression)
{
  // Open the file
  std::ofstream file(dir, std::ios::binary);

  // Is the file good
  if(file.good())
  {
    // Is there compression
    if(compression)
    {
      // Convert string to a c string
      uint64_t c_size = archives[a].size();
      char* c_str = new char[c_size];

      // Loop over the string
      for(int i=0;i<c_size;i++)
      {
        // Push the string element to the c string
        c_str[i] = archives[a][i];
      }

      // Get the compressed size
      uint64_t csize;

      // Get the compressed data
      const char* cdata = compress_string(c_str, c_size, &csize);

      // Write the compressed data to the file
      file.write(cdata, csize);
    }

    else
    {
      // Write the archive to the file
      file.write(archives[a].c_str(), archives[a].size());
    }

    // Close the file
    file.close();

    // Return true
    return true;
  }

  else
  {
    // Close the file
    file.close();

    // Return false
    return false;
  }
}

void archiveFree(int a)
{
  // Delete the archives reference
  delete &archives[a];
}

ArchivePos archiveGetEnd()
{
  // Create the position
  ArchivePos pos;
  pos.end = true;
  pos.found = false;

  // Return the position
  return pos;
}

ArchivePos archiveGetPos(int a, const char* dir)
{
  // Loop over the archive
  int i=0;
  while(i<archives[a].size())
  {
    // Get the name size
    uint32_t namesize;
    stringRead(archives[a], &i, (char*)&namesize, sizeof(namesize));

    // Get the name
    char* name = new char[namesize];
    stringRead(archives[a], &i, name, namesize);

    // Get the data size
    uint32_t datasize;
    stringRead(archives[a], &i, (char*)&datasize, sizeof(datasize));

    // Does the name size equal the directory name size
    if(namesize == strlen(dir))
    {
      // Check if both files are the same
      bool same = true;

      // Loop over the name size
      for(int i=0;i<namesize;i++)
      {
        // Do the names not match
        if(name[i] != dir[i])
        {
          // Set same to false
          same = false;
        }
      }

      // Are the directories the same
      if(same)
      {
        // Set the position
        ArchivePos pos;
        pos.end = false;
        pos.found = true;
        pos.size = datasize;
        pos.pos = i;

        // Return the position
        return pos;
      }
    }

    // Free the name
    free(name);

    // Skip the datas bytes
    i += datasize;
  }

  // Set the position
  ArchivePos pos;
  pos.found = false;
  pos.end = true;

  // Return the pos variable
  return pos;
}

void archiveWrite(int a, ArchivePos pos, const char* filename, const char* data, int size)
{
  // Is this for the end of the file
  if(pos.end)
  {
    // Write the filename to the end of the file
    uint32_t filename_size = strlen(filename);
    stringWrite(archives[a], (char*)&filename_size, sizeof(filename_size));
    stringWrite(archives[a], filename, filename_size);

    // Write the data to the end of the file
    uint32_t usize = size;
    stringWrite(archives[a], (char*)&usize, sizeof(usize));
    stringWrite(archives[a], data, size);
  }

  else
  {
    // Setup a variable the substring
    int substr_pos;

    // Is the size too large for substr
    if(pos.pos+size >= archives[a].size())
    {
      // Set the substr pos to the archives size
      substr_pos = archives[a].size();
    }

    else
    {
      // Set the substr pos to the write bytes and size
      substr_pos = pos.pos + size;
    }

    // Get the first part and the last part of the string
    std::string start = archives[a].substr(0, pos.pos-sizeof(uint32_t));
    std::string end = archives[a].substr(substr_pos);

    // Write the data to the end of the start sring
    stringWrite(start, (char*)&size, sizeof(size));
    stringWrite(start, data, size);

    // Add the start and end to the archive
    archives[a] = start + end;
  }
}

void archiveRead(int a, ArchivePos pos, char* data)
{
  // Get the data from the position
  int ipos = pos.pos;
  stringRead(archives[a], &ipos, data, pos.size);
}
