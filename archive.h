struct ArchivePos
{
  bool end;
  bool found;
  uint32_t size;
  uint32_t pos;
};

int archiveGen(const char* data, int size, bool compression=false);
int archiveLoad(const char* dir, bool compression=false);
bool archiveSave(int a, const char* dir, bool compression=false);
const char* archiveGetData(int a, int &size, bool compression=false);
void archiveFree(int a);
ArchivePos archiveGetEnd();
ArchivePos archiveGetPos(int a, const char* dir);
void archiveWrite(int a, ArchivePos pos, const char* filename, const char* data, int size);
void archiveRead(int a, ArchivePos pos, char* data);
