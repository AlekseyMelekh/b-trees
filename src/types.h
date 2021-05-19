#ifndef TYPES_H
#define TYPES_H

struct Address
{
  void *blockAddress;
  short int offset;
};

struct Record
{
  char tconst[10];    
  float averageRating; 
  int numVotes;        
};

#endif