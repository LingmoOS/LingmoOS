// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
/* ######################################################################
   
   Package Records - Allows access to complete package description records
                     directly from the file.
     
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include <config.h>

#include <apt-pkg/error.h>
#include <apt-pkg/indexfile.h>
#include <apt-pkg/pkgcache.h>
#include <apt-pkg/pkgrecords.h>

#include <vector>
#include <stddef.h>

#include <apti18n.h>
									/*}}}*/

// Records::pkgRecords - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* This will create the necessary structures to access the status files */
pkgRecords::pkgRecords(pkgCache &aCache) : d(NULL), Cache(aCache),
  Files(Cache.HeaderP->PackageFileCount)
{
   for (pkgCache::PkgFileIterator I = Cache.FileBegin();
        I.end() == false; ++I)
   {
      const pkgIndexFile::Type *Type = pkgIndexFile::Type::GetType(I.IndexType());
      if (Type == 0)
      {
         _error->Error(_("Index file type '%s' is not supported"),I.IndexType());
         return;
      }

      Files[I->ID] = Type->CreatePkgParser(I);
      if (Files[I->ID] == 0)
         return;
   }
}
									/*}}}*/
// Records::~pkgRecords - Destructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgRecords::~pkgRecords()
{
   for ( std::vector<Parser*>::iterator it = Files.begin();
     it != Files.end();
     ++it)
   {
      delete *it;
   }
}
									/*}}}*/
// Records::Lookup - Get a parser for the package version file		/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgRecords::Parser &pkgRecords::Lookup(pkgCache::VerFileIterator const &Ver)
{
   Files[Ver.File()->ID]->Jump(Ver);
   return *Files[Ver.File()->ID];
}
									/*}}}*/
// Records::Lookup - Get a parser for the package description file	/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgRecords::Parser &pkgRecords::Lookup(pkgCache::DescFileIterator const &Desc)
{
   Files[Desc.File()->ID]->Jump(Desc);
   return *Files[Desc.File()->ID];
}
									/*}}}*/

pkgRecords::Parser::Parser() : d(NULL) {}
pkgRecords::Parser::~Parser() {}
