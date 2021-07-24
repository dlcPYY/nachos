// filehdr.h
//	Data structures for managing a disk file header.
//
//	A file header describes where on disk to find the data in a file,
//	along with other information about the file (for instance, its
//	length, owner, etc.)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef FILEHDR_H
#define FILEHDR_H

#include "disk.h"
#include "bitmap.h"

#define NumDirect ((SectorSize - 2 * sizeof(int)) / sizeof(int))
#define MaxFileSize (NumDirect * SectorSize)

// The following class defines the Nachos "file header" (in UNIX terms,
// the "i-node"), describing where on disk to find all of the data in the file.
// The file header is organized as a simple table of pointers to
// data blocks.
//
// The file header data structure can be stored in memory or on disk.
// When it is on disk, it is stored in a single sector -- this means
// that we assume the size of this data structure to be the same
// as one disk sector.  Without indirect addressing, this
// limits the maximum file length to just under 4K bytes.
//
// There is no constructor; rather the file header can be initialized
// by allocating blocks for the file (if it is a new file), or by
// reading it from disk.

class FileHeader
{
public:
  FileHeader()
  {
    numBytes = 0;                            //文件大小
    numSectors = 0;                          //文件扇区数
    for (int i = 0; i < int(NumDirect); i++) // NumDirector=30:文件最多拥有的扇区数
      dataSectors[i] = 0;                    //文件扇区索引表/
  }
  bool Allocate(BitMap *freeMap, int fileSize, int incrementBytes)
  {
    if (numSectors > 30)
      //限定每个文件最多可分配 30 个扇区
      return false;
    //超出限定的文件大小
    if ((fileSize == 0) && (incrementBytes > 0))
    { //在一个空文件后追加数据
      if (freeMap->NumClear() < 1)
        //至少需要一个扇区块
        return false;
      //磁盘已满,无空闲扇区可分配
      //为添加数据先分配一个空闲磁盘块,并更新文件头信息
      dataSectors[0] = freeMap -> Find();
      numSectors = 1;
      numBytes = 0;
    }
    numBytes = fileSize;

    int offset = numBytes % SectorSize; //原文件最后一个扇区块数据偏移量
    int newSectorBytes = incrementBytes - (SectorSize - (offset + 1));
    //最后一个扇区块剩余空间足以容纳追加数据, 不需分配新的扇区块
    if (newSectorBytes <= 0)
    {
      numBytes = numBytes + incrementBytes; //更新文件头中的文件大小
      return TRUE;
    }
    //最后一个扇区的剩余空间不足以容纳要写入的数据,分配新的磁盘块
    int moreSectors = divRoundUp(newSectorBytes, SectorSize); //新加扇区块数
    if (numSectors + moreSectors > 30)
      return FALSE;                        //文件过大,超出 30 个磁盘块
    if (freeMap->NumClear() < moreSectors) //磁盘无足够的空闲块
      return false;
    //没有超出文件大小的限制,并且磁盘有足够的空闲块
    for (int i = numSectors; i < numSectors + moreSectors; i++)
      dataSectors[i] = freeMap->Find();
    numBytes = numBytes + incrementBytes; //更新文件大小
    numSectors = numSectors + moreSectors;
    //更新文件扇区块数
    return TRUE;
  }
  bool Allocate(BitMap *bitMap, int fileSize); // Initialize a file header,
      //  including allocating space
      //  on disk for the file data
  void Deallocate(BitMap *bitMap); // De-allocate this file's
      //  data blocks

  void FetchFrom(int sectorNumber); // Initialize file header from disk
  void WriteBack(int sectorNumber); // Write modifications to file header
                                    //  back to disk

  int ByteToSector(int offset); // Convert a byte offset into the file
                                // to the disk sector containing
                                // the byte

  int FileLength(); // Return the length of the file
                    // in bytes

  void Print(); // Print the contents of the file.

private:
  int numBytes;               // Number of bytes in the file
  int numSectors;             // Number of data sectors in the file
  int dataSectors[NumDirect]; // Disk sector numbers for each data
                              // block in the file
};

#endif // FILEHDR_H
