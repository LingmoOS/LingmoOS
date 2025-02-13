// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
/* ######################################################################
   
   APT Extract Templates - Program to extract debconf config and template
                           files

   This is a simple program to extract config and template information 
   from Debian packages. It can be used to speed up the preconfiguration
   process for debconf-enabled packages
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include <config.h>

#include <apt-pkg/cmndline.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/debfile.h>
#include <apt-pkg/deblistparser.h>
#include <apt-pkg/dirstream.h>
#include <apt-pkg/error.h>
#include <apt-pkg/fileutl.h>
#include <apt-pkg/init.h>
#include <apt-pkg/mmap.h>
#include <apt-pkg/pkgcache.h>
#include <apt-pkg/pkgcachegen.h>
#include <apt-pkg/pkgsystem.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/tagfile-keys.h>
#include <apt-pkg/tagfile.h>
#include <apt-pkg/version.h>

#include <apt-private/private-cmndline.h>
#include <apt-private/private-main.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "apt-extracttemplates.h"

#include <apti18n.h>
									/*}}}*/

using namespace std;

pkgCache *DebFile::Cache = 0;

// DebFile::DebFile - Construct the DebFile object			/*{{{*/
// ---------------------------------------------------------------------
/* */
DebFile::DebFile(const char *debfile)
	: File(debfile, FileFd::ReadOnly), Control(NULL), ControlLen(0),
	  DepOp(0), PreDepOp(0), Config(0), Template(0), Which(None)
{
}
									/*}}}*/
// DebFile::~DebFile - Destruct the DebFile object			/*{{{*/
// ---------------------------------------------------------------------
/* */
DebFile::~DebFile()
{
	delete [] Control;
	delete [] Config;
	delete [] Template;
}
									/*}}}*/
// DebFile::GetInstalledVer - Find out the installed version of a pkg	/*{{{*/
// ---------------------------------------------------------------------
/* */
string DebFile::GetInstalledVer(const string &package)
{
	pkgCache::PkgIterator Pkg = Cache->FindPkg(package);
	if (Pkg.end() == false) 
	{
		pkgCache::VerIterator V = Pkg.CurrentVer();
		if (V.end() == false)
		{
			return V.VerStr();
		}
	}

	return string();
}
									/*}}}*/
// DebFile::Go - Start extracting a debian package			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool DebFile::Go()
{
	debDebFile Deb(File);

	return Deb.ExtractTarMember(*this, "control.tar");
}
									/*}}}*/
// DebFile::DoItem examine element in package and mark			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool DebFile::DoItem(Item &I, int &Fd)
{
	if (strcmp(I.Name, "control") == 0)
	{
		delete [] Control;
		Control = new char[I.Size+3];
		Control[I.Size] = '\n';
		Control[I.Size + 1] = '\n';
		Control[I.Size + 2] = '\0';
		Which = IsControl;
		ControlLen = I.Size + 3;
		// make it call the Process method below. this is so evil
		Fd = -2;
	}
	else if (strcmp(I.Name, "config") == 0)
	{
		delete [] Config;
		Config = new char[I.Size+1];
		Config[I.Size] = 0;
		Which = IsConfig;
		Fd = -2; 
	} 
	else if (strcmp(I.Name, "templates") == 0)
	{
		delete [] Template;
		Template = new char[I.Size+1];
		Template[I.Size] = 0;
		Which = IsTemplate;
		Fd = -2;
	} 
	else 
	{
		// Ignore it
		Fd = -1;
	}
	return true;
}
									/*}}}*/
// DebFile::Process examine element in package and copy			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool DebFile::Process(Item &/*I*/, const unsigned char *data,
		unsigned long long size, unsigned long long pos)
{
	switch (Which)
	{
	case IsControl:
		memcpy(Control + pos, data, size);
		break;
	case IsConfig:
		memcpy(Config + pos, data, size);
		break;
	case IsTemplate:
		memcpy(Template + pos, data, size);
		break;
	default: /* throw it away */ ;
	}
	return true;
}
									/*}}}*/
// DebFile::ParseInfo - Parse control file for dependency info		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool DebFile::ParseInfo()
{
	if (Control == NULL) return false;

	pkgTagSection Section;
	if (Section.Scan(Control, ControlLen) == false)
		return false;

	Package = Section.Find(pkgTagSection::Key::Package).to_string();
	Version = GetInstalledVer(Package);

	const char *Start, *Stop;
	if (Section.Find(pkgTagSection::Key::Depends, Start, Stop))
	{
		while (1)
		{
			string P, V;
			unsigned int Op;
			Start = debListParser::ParseDepends(Start, Stop, P, V, Op);
			if (Start == 0) return false;
			if (P == "debconf")
			{
				DepVer = V;
				DepOp = Op;
				break;
			}
			if (Start == Stop) break;
		}
	}

	if (Section.Find(pkgTagSection::Key::Pre_Depends, Start, Stop))
	{
		while (1)
		{
			string P, V;
			unsigned int Op;
			Start = debListParser::ParseDepends(Start, Stop, P, V, Op);
			if (Start == 0) return false;
			if (P == "debconf")
			{
				PreDepVer = V;
				PreDepOp = Op;
				break;
			}
			if (Start == Stop) break;
		}
	}
	
	return true;
}
									/*}}}*/
static bool ShowHelp(CommandLine &)					/*{{{*/
{
	cout <<
		_("Usage: apt-extracttemplates file1 [file2 ...]\n"
		"\n"
		"apt-extracttemplates is used to extract config and template files\n"
		"from debian packages. It is used mainly by debconf(1) to prompt for\n"
		"configuration questions before installation of packages.\n");
	return true;
}
									/*}}}*/
// WriteFile - write the contents of the passed string to a file	/*{{{*/
// ---------------------------------------------------------------------
/* */
static string WriteFile(const char *package, const char *prefix, const char *data)
{
   FileFd f;
   std::string tplname;
   strprintf(tplname, "%s.%s", package, prefix);
   GetTempFile(tplname, false, &f);
   if (data != nullptr)
      f.Write(data, strlen(data));
   return f.Name();
}
									/*}}}*/
// WriteConfig - write out the config data from a debian package file	/*{{{*/
// ---------------------------------------------------------------------
/* */
static void WriteConfig(const DebFile &file)
{
	string templatefile = WriteFile(file.Package.c_str(), "template", file.Template);
	string configscript = WriteFile(file.Package.c_str(), "config", file.Config);

	if (templatefile.empty() == true || configscript.empty() == true)
		return;
	cout << file.Package << " " << file.Version << " " 
	     << templatefile << " " << configscript << endl;
}
									/*}}}*/
// InitCache - initialize the package cache				/*{{{*/
// ---------------------------------------------------------------------
/* */
static bool Go(CommandLine &CmdL)
{	
	// Initialize the apt cache
	MMap *Map = 0;
	pkgSourceList List;
	List.ReadMainList();
	pkgCacheGenerator::MakeStatusCache(List,NULL,&Map,true);
	if (Map == 0)
	   return false;
	DebFile::Cache = new pkgCache(Map);
	if (_error->PendingError() == true)
		return false;

	// Find out what version of debconf is currently installed
	string debconfver = DebFile::GetInstalledVer("debconf");
	if (debconfver.empty() == true)
		return _error->Error( _("Cannot get debconf version. Is debconf installed?"));

	auto const tmpdir = _config->Find("APT::ExtractTemplates::TempDir");
	if (tmpdir.empty() == false)
	   setenv("TMPDIR", tmpdir.c_str(), 1);

	// Process each package passsed in
	for (unsigned int I = 0; I != CmdL.FileSize(); I++)
	{
		// Will pick up the errors later..
		DebFile file(CmdL.FileList[I]);
		if (file.Go() == false)
		{
		        _error->Error("Prior errors apply to %s",CmdL.FileList[I]);
			continue;
		}

		// Does the package have templates?
		if (file.Template != 0 && file.ParseInfo() == true)
		{
			// Check to make sure debconf dependencies are
			// satisfied
			// cout << "Check " << file.DepVer << ',' << debconfver << endl;
			if (file.DepVer != "" &&
			    DebFile::Cache->VS->CheckDep(debconfver.c_str(),
					file.DepOp,file.DepVer.c_str()
							 ) == false)
				continue;
			if (file.PreDepVer != "" &&
			    DebFile::Cache->VS->CheckDep(debconfver.c_str(),
			                file.PreDepOp,file.PreDepVer.c_str()
							 ) == false) 
				continue;

			WriteConfig(file);
		}
	}
	

	delete Map;
	delete DebFile::Cache;
	
	return !_error->PendingError();
}
									/*}}}*/
static std::vector<aptDispatchWithHelp> GetCommands()			/*{{{*/
{
   return {
	{nullptr, nullptr, nullptr}
   };
}
									/*}}}*/
int main(int argc, const char **argv)					/*{{{*/
{
	CommandLine CmdL;
	ParseCommandLine(CmdL, APT_CMD::APT_EXTRACTTEMPLATES, &_config, &_system, argc, argv, &ShowHelp, &GetCommands);

	Go(CmdL);

	return DispatchCommandLine(CmdL, {});
}
									/*}}}*/
