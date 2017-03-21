#include "stdifm.h"
#include "AutoExportHeads_Trans.h"

IfmModule g_pMod;  /* Global handle related to this plugin */

#pragma region IFM_Definitions
/* --- IFMREG_BEGIN --- */
/*  -- Do not edit! --  */

static IfmResult OnBeginDocument (IfmDocument);
static void OnEndDocument (IfmDocument);
static void OnActivate (IfmDocument, Widget);
static void OnEditDocument (IfmDocument, Widget);
static void PreSimulation (IfmDocument);
static void PostSimulation (IfmDocument);
static void PostTimeStep (IfmDocument);
static void PreFlowSimulation (IfmDocument);
static void PostFlowSimulation (IfmDocument);

/*
 * Enter a short description between the quotation marks in the following lines:
 */
static const char szDesc[] = 
  "Please, insert a plug-in description here!";

#ifdef __cplusplus
extern "C"
#endif /* __cplusplus */

IfmResult RegisterModule(IfmModule pMod)
{
  if (IfmGetFeflowVersion (pMod) < IFM_REQUIRED_VERSION)
    return False;
  g_pMod = pMod;
  IfmRegisterModule (pMod, "SIMULATION", "AUTOEXPORTHEADS_TRANS", "AutoExportHeads_Trans", 0x1000);
  IfmSetDescriptionString (pMod, szDesc);
  IfmSetCopyrightPath (pMod, "AutoExportHeads_Trans.txt");
  IfmSetHtmlPage (pMod, "AutoExportHeads_Trans.htm");
  IfmSetPrimarySource (pMod, "AutoExportHeads_Trans.cpp");
  IfmRegisterProc (pMod, "OnBeginDocument", 1, (IfmProc)OnBeginDocument);
  IfmRegisterProc (pMod, "OnEndDocument", 1, (IfmProc)OnEndDocument);
  IfmRegisterProc (pMod, "OnActivate", 1, (IfmProc)OnActivate);
  IfmRegisterProc (pMod, "OnEditDocument", 1, (IfmProc)OnEditDocument);
  IfmRegisterProc (pMod, "PreSimulation", 1, (IfmProc)PreSimulation);
  IfmRegisterProc (pMod, "PostSimulation", 1, (IfmProc)PostSimulation);
  IfmRegisterProc (pMod, "PostTimeStep", 1, (IfmProc)PostTimeStep);
  IfmRegisterProc (pMod, "PreFlowSimulation", 1, (IfmProc)PreFlowSimulation);
  IfmRegisterProc (pMod, "PostFlowSimulation", 1, (IfmProc)PostFlowSimulation);
  return True;
}

static void OnActivate (IfmDocument pDoc, Widget button)
{
  CAutoexportheadsTrans::FromHandle(pDoc)->OnActivate (pDoc, button);
}
static void OnEditDocument (IfmDocument pDoc, Widget wParent)
{
  CAutoexportheadsTrans::FromHandle(pDoc)->OnEditDocument (pDoc, wParent);
}
static void PreSimulation (IfmDocument pDoc)
{
  CAutoexportheadsTrans::FromHandle(pDoc)->PreSimulation (pDoc);
}
static void PostSimulation (IfmDocument pDoc)
{
  CAutoexportheadsTrans::FromHandle(pDoc)->PostSimulation (pDoc);
}
static void PostTimeStep (IfmDocument pDoc)
{
  CAutoexportheadsTrans::FromHandle(pDoc)->PostTimeStep (pDoc);
}
static void PreFlowSimulation (IfmDocument pDoc)
{
  CAutoexportheadsTrans::FromHandle(pDoc)->PreFlowSimulation (pDoc);
}
static void PostFlowSimulation (IfmDocument pDoc)
{
  CAutoexportheadsTrans::FromHandle(pDoc)->PostFlowSimulation (pDoc);
}

/* --- IFMREG_END --- */
#pragma endregion


static IfmResult OnBeginDocument (IfmDocument pDoc)
{
  if (IfmDocumentVersion (pDoc) < IFM_CURRENT_DOCUMENT_VERSION)
    return false;

  try {
    IfmDocumentSetUserData(pDoc, new CAutoexportheadsTrans(pDoc));
  }
  catch (...) {
    return false;
  }

  return true;
}

static void OnEndDocument (IfmDocument pDoc)
{
  delete CAutoexportheadsTrans::FromHandle(pDoc);
}

///////////////////////////////////////////////////////////////////////////
// Implementation of CAutoexportheadsTrans

// Constructor
CAutoexportheadsTrans::CAutoexportheadsTrans (IfmDocument pDoc)
  : m_pDoc(pDoc)
{
  /*
   * TODO: Add your own code here ...
   */
}

// Destructor
CAutoexportheadsTrans::~CAutoexportheadsTrans ()
{
  /*
   * TODO: Add your own code here ...
   */
}

// Obtaining class instance from document handle
CAutoexportheadsTrans* CAutoexportheadsTrans::FromHandle (IfmDocument pDoc)
{
  return reinterpret_cast<CAutoexportheadsTrans*>(IfmDocumentGetUserData(pDoc));
}

// Callbacks
void CAutoexportheadsTrans::OnActivate (IfmDocument pDoc, Widget button)
{
  /* 
   * TODO: Add your own code here ...
   */
	using namespace std;
	double whead(0.0);
	double currenttime;
	
	double x, y, z;
	double ori_x = IfmGetOriginX(pDoc);
	double ori_y = IfmGetOriginY(pDoc);
	double ori_z = IfmGetOriginZ(pDoc);

	//int node;
	//double distance;
	//double whead1(0.0);
	//distance = 2;

	string BH_ID;
	IfmBool inDomain(false);
	int nn = 0;
	int i = 0; int j = 0;

	//change the work directory 
	string problemPath(IfmGetProblemPath(pDoc));
	string problemName(IfmGetProblemTitle(pDoc));
	problemPath.resize(problemPath.length() - problemName.length(), '+');
	_chdir(problemPath.c_str());

	//open the output file and set flags
	ofstream  fout;
	fout.open("AutoExportHeads_Trans.out", ios_base::out);
	fout.setf(ios_base::fixed, ios_base::floatfield);
	fout << setiosflags(ios::fixed | ios::right);
	//write the file head
	fout << "ObsName" << '\t' << "Easting" << '\t' << "Northing" << '\t' << "Elevation"
		<< '\t' << "Time" << '\t' << "Head" << '\t' << "Saturation" << endl;

	//write the intial head
	struct stat buffer;
	if (stat("AutoExportHeads_Trans.inp", &buffer)) IfmInfo(pDoc, "found the file! \n");
	ifstream  fin;
	fin.open("AutoExportHeads_Trans.inp", ios_base::in);

	//Check if the input file is opened properly, if so retrive the head at selected locations.
	if (fin.is_open())
	{
		IfmInfo(pDoc, "Running AutoExportHeads_Trans... for time %e days. \n", IfmGetAbsoluteSimulationTime(pDoc));
		fin >> int(nn);
		//fin>>filehead;
		//fin>>filehead;

		for (i = 0; i<nn; i++)
		{
			fin >> BH_ID >> x >> y >> z;
			fout << setiosflags(ios::fixed | ios::left);
			fout << BH_ID << '\t' << setprecision(6) << x <<
				'\t' << setprecision(6) << y << '\t' << setprecision(2) << z;
			
//			if (BH_ID == "TW16-1") {
//				node = IfmFindNodeAtXY(pDoc, x - ori_x, y - ori_y, &distance);
//				whead1 = IfmGetResultsFlowHeadValue(pDoc, node+182839*7);
//				whead1 = IfmGetResultsFlowHeadValue(pDoc, 1462710-1);
//				whead1 = IfmGetResultsFlowHeadValueAtXYSlice(pDoc, x-ori_x, y - ori_y, 8, &inDomain);
//			}
			
			whead = IfmGetResultsFlowHeadValueAtXYZ(pDoc, x - ori_x, y - ori_y, z - ori_z, &inDomain);
			currenttime = IfmGetAbsoluteSimulationTime(pDoc);
			if (inDomain) {
				fout << '\t' << scientific << setprecision(6) << currenttime << '\t' << fixed << setprecision(2) << whead <<
					'\t' << fixed << setprecision(2) << IfmGetResultsFlowSaturationValueAtXYZ(pDoc, x - ori_x, y - ori_y, z - ori_z, &inDomain) << endl;
			}
			else
			{
				fout << '\t' << scientific << currenttime << '\t' << "out_of_model_domain" << endl;
				BH_ID.append(" Out of model domain");
				IfmInfo(pDoc, BH_ID.c_str());
			}
		}
		fin.close();
		fout.flush();
		fout.close();
	}
	else
	{
		IfmInfo(pDoc, "can not open the input file");
	}
}

void CAutoexportheadsTrans::OnEditDocument (IfmDocument pDoc, Widget wParent)
{
  /* 
   * TODO: Add your own code here ...
   */
}

void CAutoexportheadsTrans::PreSimulation (IfmDocument pDoc)
{
  /* 
   * TODO: Add your own code here ...
   */
	using namespace std;
	double whead(0.0);

	
	double currenttime;
	double x, y, z;
	double ori_x = IfmGetOriginX(pDoc);
	double ori_y = IfmGetOriginY(pDoc);
	double ori_z = IfmGetOriginZ(pDoc);



	string BH_ID;
	IfmBool inDomain(false);
	int nn = 0;
	int i = 0; int j = 0;

	//change the work directory 
	string problemPath(IfmGetProblemPath(pDoc));
	string problemName(IfmGetProblemTitle(pDoc));
	problemPath.resize(problemPath.length() - problemName.length(), '+');
	_chdir(problemPath.c_str());

	//open the output file and set flags
	ofstream  fout;
	fout.open("AutoExportHeads_Trans.out", ios_base::out);
	fout.setf(ios_base::fixed, ios_base::floatfield);
	fout << setiosflags(ios::fixed | ios::right);
	//write the file head
	fout << "ObsName" << '\t' << "Easting" << '\t' << "Northing" << '\t' << "Elevation"
		<< '\t' << "Time" << '\t' << "Head" << '\t' << "Saturation" << endl;

	//write the intial head
	struct stat buffer;
	if (stat("AutoExportHeads_Trans.inp", &buffer)) IfmInfo(pDoc, "found the file! \n");
	ifstream  fin;
	fin.open("AutoExportHeads_Trans.inp", ios_base::in);

	//Check if the input file is opened properly, if so retrive the head at selected locations.
	if (fin.is_open())
	{
		IfmInfo(pDoc, "Running AutoExportHeads_Trans... for time %e days. \n", IfmGetAbsoluteSimulationTime(pDoc));
		fin >> int(nn);
		//fin>>filehead;
		//fin>>filehead;

		for (i = 0; i<nn; i++)
		{
			fin >> BH_ID >> x >> y >> z;
			fout << setiosflags(ios::fixed | ios::left);
			fout << BH_ID << '\t' << setprecision(6) << x <<
				'\t' << setprecision(6) << y << '\t' << setprecision(2) << z;



			whead = IfmGetResultsFlowHeadValueAtXYZ(pDoc, x - ori_x, y - ori_y, z - ori_z, &inDomain);
			currenttime = IfmGetAbsoluteSimulationTime(pDoc);
			if (inDomain) {
				fout << '\t' << scientific << setprecision(6) << currenttime << '\t' << fixed << setprecision(2) << whead <<
					'\t' << fixed << setprecision(2)<<IfmGetResultsFlowSaturationValueAtXYZ(pDoc, x-ori_x, y-ori_y, z-ori_z,&inDomain)<<endl;
			}
			else
			{
				fout << '\t' << scientific << currenttime << '\t' << "out_of_model_domain" << endl;
				BH_ID.append(" Out of model domain");
				IfmInfo(pDoc, BH_ID.c_str());
			}
		}
		fin.close();
		fout.flush();
		fout.close();
	}
	else
	{
		IfmInfo(pDoc, "can not open the input file");
	}
}

void CAutoexportheadsTrans::PostSimulation (IfmDocument pDoc)
{
  /* 
   * TODO: Add your own code here ...
   */
}

void CAutoexportheadsTrans::PostTimeStep (IfmDocument pDoc)
{
  /* 
   * TODO: Add your own code here ...
   */
	using namespace std;
	int i, nn;
	double whead(0.0);
	double currenttime;
	double x, y, z;
	double ori_x = IfmGetOriginX(pDoc);
	double ori_y = IfmGetOriginY(pDoc);
	double ori_z = IfmGetOriginZ(pDoc);
	//char *filehead;
	string filehead;
	string BH_ID;
	IfmBool inDomain(false);


	//Change the working directory to the model direstory
	string problemPath(IfmGetProblemPath(pDoc));
	string problemName(IfmGetProblemTitle(pDoc));
	problemPath.resize(problemPath.length() - problemName.length(), '+');
	_chdir(problemPath.c_str());

	nn = 0;
	;

	//open the input file
	struct stat buffer;
	if (stat("AutoExportHeads_Trans.inp", &buffer)) IfmInfo(pDoc, "found the file! \n");

	ifstream  fin;
	fin.open("AutoExportHeads_Trans.inp", ios_base::in);

	//open the output file and set output flags
	ofstream fout;
	fout.open("AutoExportHeads_Trans.out", ios_base::app);
	fout << setiosflags(ios::fixed | ios::left);
	fout << setprecision(4);

	//Check if the input file is opened properly, if so retrive the head at selected locations.
	if (fin.is_open())
	{
		IfmInfo(pDoc, "Running AutoExportHeads_Trans... for time %e days.\n", IfmGetAbsoluteSimulationTime(pDoc));
		fin >> int(nn);
		//fin>>filehead;
		//fin>>filehead;

		for (i = 0; i<nn; i++)
		{
			fin >> BH_ID >> x >> y >> z;
			fout << setiosflags(ios::fixed | ios::left);
			fout << BH_ID << '\t' << setprecision(6) << x <<
				'\t' << setprecision(6) << y << '\t' << setprecision(2) << z;
			whead = IfmGetResultsFlowHeadValueAtXYZ(pDoc, x - ori_x, y - ori_y, z - ori_z, &inDomain) + ori_z;
			currenttime = IfmGetAbsoluteSimulationTime(pDoc);
			if (inDomain) {
				fout << '\t' << scientific << setprecision(6) << currenttime << '\t' << fixed << setprecision(2) << whead <<
					'\t' << fixed << setprecision(2) <<IfmGetResultsFlowSaturationValueAtXYZ(pDoc, x-ori_x, y-ori_y, z-ori_z,&inDomain)<<endl;
			}
			else
			{
				fout << '\t' << scientific << currenttime << '\t' << "out_of_model_domain" << endl;
				BH_ID.append(" Out of model domain");
				IfmInfo(pDoc, BH_ID.c_str());
			}
		}
		fin.close();
		fout.flush();
		fout.close();

	}
	else
	{
		IfmInfo(pDoc, "can not open the input file");
	}

}

void CAutoexportheadsTrans::PreFlowSimulation (IfmDocument pDoc)
{
  /* 
   * TODO: Add your own code here ...
   */
}

void CAutoexportheadsTrans::PostFlowSimulation (IfmDocument pDoc)
{
  /* 
   * TODO: Add your own code here ...
   */
}

