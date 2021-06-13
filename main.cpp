#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <stdlib.h>

//-std=gnu++20;
namespace fs = std::filesystem;
using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// Define Rootdir and Masks here, if some locaction change just edit this part				 ////
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define ZIPTAIL " -ms=off -mx=9  -xr!*.adi  -xr!*.vhd  -xr!*.vdi  -xr!*.vdmk  -xr!*.iso  -xr!*.ova -xr!*.vmdk -xr!*\\tor-browser_en-US -xr!.* -xr!*\\BackUpLogExlog -xr!*\\Downloads -xr!*\\eclipse -xr!*\\eclipse-workspace \\ >> /home/johannes/Documents/Skript/BackUpLogExlog/log$TODAY$TIME.txt 2>&1 ";
#define ZIPFRONT "7z a -bb1 -p\"geheim\" -mhe "
#define DDIR "/home/johannes/Documents/OutPutTest" //"/mnt/Sicherungen/Sicherung_Meine_Ablage"
#define FULLBACKUPNAME "MeinAblgaeFullBackUp.7z"
#define ROOTDIR "/home/johannes/Documents/OutPutTest"
#define SRCDIR "/home/johannes/Documents/Skript"
#define OLDLIST "/BackUpList.txt"
#define ZIPLIST "/ZipList.sh"
#define XCLUDE "/home/johannes/Documents/Skript/xclude.txt"
#define STRINGSPLIT ","
#define ROOTMASK 0b10
#define ZIPMASK 0b01
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Struct to hold the data
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct FileInfo
{
  unsigned long long ulHash;
  string strPath;
  int iStatus=0;
};
std::vector<FileInfo> mvctFileInfoOld;
std::vector<FileInfo> mvctFileInfoNew;
std::vector<FileInfo> mvctFileInfoZIP;
int m_iXcludes=0;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// Methode to check input          										                     ////
/////////////////////////////////////////////////////////////////////////////////////////////////////
	inline string checkInput(string strInput)
	{
		string strTemp=strInput;
		strTemp.replace(strTemp.find("("), 2, "\\(");
		strTemp.replace(strTemp.find(")"), 2, "\\)");
		return strTemp;
	}

	inline bool checkXClude(string strInput, string strMask)
	{
		if (strInput.find(strMask) != string::npos)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// Method to iteratr through the Directory, recursive											 ////
/////////////////////////////////////////////////////////////////////////////////////////////////////
int ItterateDir(fs::path dirPath, int iSwitch)
{
  //cout << "Path "<<dirPath<<"\n";
	int i=0;
    //read xcludelist
	bool bXClude=false;
	vector<string> vctXcludes;
	ifstream xCludeStream;
	xCludeStream.open(XCLUDE);
	string strXLine;
	while(getline(xCludeStream, strXLine))
	{

		 vctXcludes.push_back(strXLine);
	}
	xCludeStream.close();
	string strMask;
		for ( auto& entry : fs::directory_iterator(dirPath))
		{
			//cout << "i= "<<i<<"\n";
			i++;
			auto filenameStr = entry.path().filename().string();
			string strPath_= fs::absolute(entry);
			fs::path p=strPath_;
				for( auto &cmpXclude:vctXcludes)
					{
						bXClude=checkXClude(strPath_, cmpXclude);
						m_iXcludes++;
						if(bXClude)
							break;
					}
			if(bXClude)
				continue;
			if (entry.is_directory())
			{
				ItterateDir(p,1);
			}
				else
				{
					FileInfo fi;
					fi.strPath=strPath_;
					std::ifstream input(fi.strPath, std::ios::binary);
					std::vector<char> bytes(
					(std::istreambuf_iterator<char>(input)),
					(std::istreambuf_iterator<char>()));
					input.close();
					unsigned long long ullNumber = 0;
					for( auto &bytes:bytes )
						{
							ullNumber += (unsigned long long)bytes;
						}
					cout << "Number: "<<ullNumber<<"\n";
					fi.ulHash=ullNumber;
					mvctFileInfoNew.push_back(fi);
				}

  }
  return 0;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// Methode to create Zip cli for file										                     ////
/////////////////////////////////////////////////////////////////////////////////////////////////////
string create7ZipCmd(string strInput)
{
	//define 7ZIPTAIL
	//define 7ZIPFRONT
	//  $DIRR/$MONTH.7z -spf2 /home/johannes
	//strInput=checkInput(strInput);
	string strReturn="";
	if(fs::exists(DDIR "/" FULLBACKUPNAME))
	{
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];

		time (&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer,sizeof(buffer),"%Y-%m-%d_%H:%M:%S",timeinfo);
		std::string strTimestamp(buffer);
		strReturn  = ZIPFRONT DDIR "/";
		strReturn += strTimestamp;
		strReturn += " -spf2 ";
		strReturn += "\"";
		strReturn += strInput;
		strReturn += "\"";
		strReturn += ZIPTAIL;
	}
	else
	{
		strReturn	 = ZIPFRONT  DDIR "/" FULLBACKUPNAME " -spf2 ";
		strReturn 	+= "\"";
		strReturn	+= strInput;
		strReturn 	+= "\"";
		strReturn	+= ZIPTAIL;
	}

	return strReturn;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// Mainmethod to start the itteration and write the Lists										 ////
/////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
  //read old Filelist-------------------------------------------------------------------

  std::cout << "Read List: " << ROOTDIR <<OLDLIST<<'\n';
  std::cout << "Source: " << SRCDIR <<'\n';
  FileInfo fi;
  int i=0;
  string strLine;
  ifstream ifStream;
  ifStream.open(ROOTDIR OLDLIST);
  while(getline(ifStream, strLine))// finde /
  {
     int pos=0;
     pos=strLine.find(STRINGSPLIT);
	 fi.strPath= strLine.substr(0, strLine.find(STRINGSPLIT));//stoul
		string delimiter=STRINGSPLIT;
		string strTemp=strLine;
		int k=pos+delimiter.length();
		strTemp.erase(0, k);
		fi.ulHash = stoul(strTemp);//stoul
		//fi.iStatus=0;
		mvctFileInfoOld.push_back(fi);

     i++;
  }
  ifStream.close();
 //make fallbackcopy to keep
	string strToCheck= ROOTDIR "/temp" OLDLIST;
	if(fs::exists(strToCheck))
		fs::remove(strToCheck);
	if(fs::exists(ROOTDIR OLDLIST))
		fs::copy(ROOTDIR OLDLIST, ROOTDIR "/temp" OLDLIST);
	//remove old to write a new one
	if(fs::exists(ROOTDIR OLDLIST))
		fs::remove(ROOTDIR OLDLIST);
	if(fs::exists(ROOTDIR ZIPLIST))
		fs::remove(ROOTDIR ZIPLIST);

  if(i==0)
     std::cout << "readline was empty!" << '\n';
    else
      std::cout << "ListOld: " << mvctFileInfoNew.size()<<'\n';
  std::cout << "Create FileList."<<'\n';
  //create new Filelist-------------------------------------------------------------------

   fs::path pathToShow{ fs::path(SRCDIR) }; //Dir to be backed up

  //~~~~~~~~~~~~~~~~~~CALL~~~~~~~~~~~~~~~~~~~~~~~~~~
  int iState= ItterateDir(pathToShow,1);
  //Call end---------------------------------------------------------------------------
  std::cout << "ListNew: " << mvctFileInfoNew.size()<<'\n';
  if(iState==(-1))
    return (-1);
	////////////////////////////////////////////////////////////////////////////////////////////
	//// Loop to compare the New List with the Old one										////
	////////////////////////////////////////////////////////////////////////////////////////////
    //compare lists-------------------------------------------------------------------
	bool bFound=false;
   for( auto &cmpNew:mvctFileInfoNew)
    {
      //std::cout << "Compare:"<< cmpNew.strPath<<","<<cmpNew.ulHash<<"with:"<<"\n";
      bFound=false;
      for( auto &cmpOld:mvctFileInfoOld )
      {
		  // Check if both have the same path-------------------------------------------------------------------
          if(cmpNew.strPath.compare(cmpOld.strPath)==0)
          {
            //check if the have still the same hash-------------------------------------------------------------------
            if((cmpNew.ulHash != cmpOld.ulHash))//filecontent changed
            {
              cmpNew.iStatus=(cmpNew.iStatus | ZIPMASK); // set -1 if file exsists but got changed-------------------------------------------------------------------
			  cmpNew.iStatus=(cmpNew.iStatus | ROOTMASK);

			  //continue;
            }
            else //filecontent is the same, only write to OldFile/Rootfile for next BackUp
            {
				cmpOld.iStatus=(cmpOld.iStatus | ROOTMASK);
				mvctFileInfoZIP.push_back(cmpOld);
				//continue;
            }
			bFound=true;

          }
		  if(bFound)
			break;
      }
      if(!bFound)//file is new and not contained in the list
          {
            cmpNew.iStatus=(cmpNew.iStatus | ZIPMASK);
			cmpNew.iStatus=(cmpNew.iStatus | ROOTMASK);
          }
    }//end outer forloop
    //write new Filelist for next Backup-------------------------------------------------------------------
	////////////////////////////////////////////////////////////////////////////////////////////
	////Write to new Filelists. FileListOld/Root contains the updatet Data for the next     ////
	////BackUp.FileListDif is the list that contains the Files to be backed up.				////
	//// Write to RootList if iStatus in binary & matches with ROOTMASK.					////
	//// Write to ZipList if iStatus in binary & matches with ZIPMASK.						////
	////////////////////////////////////////////////////////////////////////////////////////////

	//#define ROOTDIR "/home/johannes/Documents/OutPutTest"
	//#define SRCDIR "/home/johannes/Documents/Skript"
	//#define OLDLIST "/BackUpList.txt"
	//#define ZIPLIST "/ZipList.txt"
    ofstream WriteOldStream;
	ofstream WriteZipStream;
    WriteZipStream.open(ROOTDIR ZIPLIST, fstream::app);
    WriteOldStream.open(ROOTDIR OLDLIST, fstream::app);
	std::cout << "List to WriteOldStream ListOld: " << mvctFileInfoOld.size()<<'\n';
	//Write from OldList
	for( auto &arr:mvctFileInfoOld) //Old Data
	{
		int iCheck=(arr.iStatus & ROOTMASK);
		if(iCheck==0)
			continue;
		if(iCheck==ROOTMASK)
		{
		WriteOldStream <<arr.strPath<< ","<< arr.ulHash<< "\n";
		WriteOldStream.flush();
		//std::cout << "7ZipFile:"<< arr.strPath<<","<<arr.ulHash<<"\n";
		}
		iCheck=(arr.iStatus & ZIPMASK);
		if(iCheck==ZIPMASK)
		{
		WriteZipStream << create7ZipCmd( arr.strPath )<< "\n";
		WriteZipStream.flush();
		//std::cout << "7ZipFile:"<< arr.strPath<<","<<arr.ulHash<<"\n";
		}
	}
	//Write from NewList
	for( auto &arr:mvctFileInfoNew) //Old Data
	{
		int iCheck=(arr.iStatus & ROOTMASK);
		if(iCheck==0)
			continue;
		if(iCheck==ROOTMASK)
		{
		WriteOldStream << arr.strPath<< ","<< arr.ulHash<< "\n";
		WriteOldStream.flush();
		//std::cout << "7ZipFile:"<< arr.strPath<<","<<arr.ulHash<<"\n";
		}
		iCheck=(arr.iStatus & ZIPMASK);
		if(iCheck==ZIPMASK)
		{
		WriteZipStream << create7ZipCmd( arr.strPath )<< "\n";
		WriteZipStream.flush();
		//std::cout << "7ZipFile:"<< arr.strPath<<","<<arr.ulHash<<"\n";
		}
	}
	WriteOldStream.close();
	WriteZipStream.close();

	for( auto &arr:mvctFileInfoOld) //Old Data
	{
		if(arr.iStatus==0)
			continue;
		if((arr.iStatus & ROOTMASK)==ROOTMASK)
		{
		WriteZipStream <<"187 zipold"<< arr.strPath<< ","<< arr.ulHash<< "\n";
		WriteZipStream.flush();
		//std::cout << "7ZipFile:"<< arr.strPath<<","<<arr.ulHash<<"\n";
		}

	}
	for( auto &arr:mvctFileInfoNew) //New Data
	{
		if(arr.iStatus==0)
			continue;
		if((arr.iStatus & ROOTMASK)==ROOTMASK)
		{
		WriteZipStream <<"199 zipnew"<< arr.strPath<< ","<< arr.ulHash<< "\n";
		WriteZipStream.flush();
		//std::cout << "7ZipFile:"<< arr.strPath<<","<<arr.ulHash<<"\n";
		}
	}
	WriteZipStream.close();
	//Write to Oldlist
   for( auto &arr:mvctFileInfoOld) //Old Data
	{
		if(arr.iStatus==0)
			continue;
		if((arr.iStatus & ZIPMASK)==ZIPMASK)
		{
		WriteOldStream<<"212 oldold" << arr.strPath<< ","<< arr.ulHash<< "\n";
		WriteOldStream.flush();
		//std::cout << "7ZipFile:"<< arr.strPath<<","<<arr.ulHash<<"\n";
		}

	}
	for( auto &arr:mvctFileInfoNew) //New Data
	{
		if(arr.iStatus==0)
			continue;
		if((arr.iStatus & ZIPMASK)==ZIPMASK)
		{
		WriteOldStream <<"224 oldNew"<< arr.strPath<< ","<< arr.ulHash<< "\n";
		WriteOldStream.flush();
		//std::cout << "7ZipFile:"<< arr.strPath<<","<<arr.ulHash<<"\n";
		}
	}


	//ofstream WriteZipStream;
    //WriteZipStream.open(ROOTDIR ZIPLIST, fstream::app);
	//Write to Ziplist

   for( auto &arr:mvctFileInfoOld) //Old Data
	{
		if(arr.iStatus==0)
			continue;
		if((arr.iStatus & ROOTMASK)==ROOTMASK)
		{
		WriteZipStream <<"187 zipold"<< arr.strPath<< ","<< arr.ulHash<< "\n";
		WriteZipStream.flush();
		//std::cout << "7ZipFile:"<< arr.strPath<<","<<arr.ulHash<<"\n";
		}

	}
	for( auto &arr:mvctFileInfoNew) //New Data
	{
		if(arr.iStatus==0)
			continue;
		if((arr.iStatus & ROOTMASK)==ROOTMASK)
		{
		WriteZipStream <<"199 zipnew"<< arr.strPath<< ","<< arr.ulHash<< "\n";
		WriteZipStream.flush();
		//std::cout << "7ZipFile:"<< arr.strPath<<","<<arr.ulHash<<"\n";
		}
	}
	WriteZipStream.close();
	//Write to Oldlist
   for( auto &arr:mvctFileInfoOld) //Old Data
	{
		if(arr.iStatus==0)
			continue;
		if((arr.iStatus & ZIPMASK)==ZIPMASK)
		{
		WriteOldStream<<"212 oldold" << arr.strPath<< ","<< arr.ulHash<< "\n";
		WriteOldStream.flush();
		//std::cout << "7ZipFile:"<< arr.strPath<<","<<arr.ulHash<<"\n";
		}

	}
	for( auto &arr:mvctFileInfoNew) //New Data
	{
		if(arr.iStatus==0)
			continue;
		if((arr.iStatus & ZIPMASK)==ZIPMASK)
		{
		WriteOldStream <<"224 oldNew"<< arr.strPath<< ","<< arr.ulHash<< "\n";
		WriteOldStream.flush();
		//std::cout << "7ZipFile:"<< arr.strPath<<","<<arr.ulHash<<"\n";
		}
	}
	WriteOldStream.close();
	cout<<"Found "<< m_iXcludes<<" Objects to exclude from archieve.\n";
	system("/bin/bash " ROOTDIR ZIPLIST);
	//system(ROOTDIR ZIPLIST);
	cout<<"ende!"<<"\n";
	mvctFileInfoNew.clear();
	mvctFileInfoOld.clear();
	mvctFileInfoZIP.clear();
	if(fs::exists(ROOTDIR ZIPLIST))
		fs::remove(ROOTDIR ZIPLIST);
	return 0;
}
