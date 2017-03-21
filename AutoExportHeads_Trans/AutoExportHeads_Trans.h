#pragma once
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <sstream>
#include <direct.h> 
#include <string>
#include <cmath>
#include <string>

// Plugin implementation class
class CAutoexportheadsTrans
{
public:
  CAutoexportheadsTrans(IfmDocument pDoc);
  ~CAutoexportheadsTrans();
  static CAutoexportheadsTrans* FromHandle(IfmDocument pDoc);

#pragma region IFM_Definitions
  // Implementation
public:
  void OnActivate (IfmDocument pDoc, Widget wParent);
  void OnEditDocument (IfmDocument pDoc, Widget wParent);
  void PreSimulation (IfmDocument pDoc);
  void PostSimulation (IfmDocument pDoc);
  void PostTimeStep (IfmDocument pDoc);
  void PreFlowSimulation (IfmDocument pDoc);
  void PostFlowSimulation (IfmDocument pDoc);
#pragma endregion

private:
  IfmDocument m_pDoc;
};
