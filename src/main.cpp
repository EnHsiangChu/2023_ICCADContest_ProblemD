// hmetis_test.cpp : wqDx3ε{iJ�C
//

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <climits>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <string>
using namespace std;

//corner stitching include
#include <vector>
#include "structure.h"
#include "update.h"
#include "search.h"
#include "multithread.h"
#include "dddetail.h"
#include <math.h>

//#define thread_num 2
int thread_num = 5;
int thread_run_time = 10*60;//sec
string case_file_name = "wow";

//corner stitching end

#include "util.h"
#include "legal.h"
#include "DiamondSearch.h"
#include "DiamondLegalizer.h"
#include "Prelegalizer.h"
#include "detail.h"
#include "lefdef.h"
#include "mincutplacer.h"
#include "ParserIBMMix.h"
#include "placedb.h"
#include "randomplace.h"
#include "placeutil.h"

//Added by Jin 20060223
#include "DPlace.h"
CDetailPlacerParam dpParam;
//Added by Jin 20060224
#include "TetrisLegal.h"

#include "MyNLP.h"
#include "mincutcluster.h"
#include "mlnlp.h"	// multilevel nlp global placement

#include "PlaceDBQP.h"
#include "fccluster.h" 

#include "macrolegal.h"
#include "placebin.h"
#include "cellmoving.h" //by tchsu 2006.03.01

//test 2023-05-01
#include "Parsertxt.h"

// 2005-12-05
string outBookshelf;
string outBookshelfNoBlockSite;

// 2005-12-18
bool bPlotOnly = false;

double programStartTime = 0;

double timePacking = 0.0;
double timeDataPrepare = 0.0;
double timePartition = 0.0;

//w
//==============================================
bool bRunGlobal = true;
bool bRunDetail = true;
bool bRunLegal  = true;
bool bOutInterPL = false;
bool bCellRedistribution = true; //by tchsu 2006.03.10
double gCellRedistributionTarget = 1.0; //by tchsu 2006.03.12
//============================================== by indark
int nMacroLegalRun = 40;
//==============================================
bool bRunMincut = false;
bool bRunMacroLegal = false;
bool bBuildInfo = false;

CParserLEFDEF parserLEFDEF;	// global var for WriteDEF

////////////////////////////////////////////////
// parameters for mlnlp
double gClusterRatio = 5;
int    gClusterTargetSize = 6000;
double gTargetNNB = 0.8;
double gTargetDensity = 1.05;
double gWireWeight = 1;	
double gIncFactor = 2.0;
bool   gRunWCycle = false;
int    gMaxLevel = INT_MAX;
double gWeightLevelDecreaingRate = 1.0;
double gTargetUtil = -1;
////////////////////////////////////////////////

bool allMacroMove = false;
bool bShrinkCore = false;
int gCType = 4;

// void *ThreadStartup(void *_tgtObject) {
//   MyNLP *tgtObject = ((eval_grad_f_arg *)_tgtObject)->pNLP;
// //   printf("Running thread object in a new thread\n");
//   void *threadResult = tgtObject->P_eval_grad_f( _tgtObject);
// //   printf("Deleting object\n");
//   
//   return threadResult;
// }

bool handleArgument( const int& argc, char* argv[], CParamPlacement& param )
{
	//2023/6/14
	bRunLegal = false;
	bRunDetail = false;
	param.bPlot = true;
	//param.bShow = true;
	//param.bLog = true;
    //bOutInterPL = true;
    //bBuildInfo = true;
    /*int i; //2023/6/14
    if( strcmp( argv[1], "-aux" ) == 0 )
	i = 3;
    else
	i = 4;
    while( i < argc )
    {
	// 2006-03-19 (donnie) remove checking "-"
	if( strlen( argv[i] ) <= 1 )	// bad argument?
	{
	    i++;
	    continue;
	}


	if( strcmp( argv[i]+1, "seed" ) == 0 )
	    param.seed = (unsigned long)atol( argv[++i] );
	else if( strcmp( argv[i]+1, "noglobal" ) == 0 )
	    bRunGlobal = false;
	else if( strcmp( argv[i]+1, "nolegal" ) == 0 )
	    bRunLegal = false;
	else if( strcmp( argv[i]+1, "detail" ) == 0 )
	    bRunDetail = true;
	else if( strcmp( argv[i]+1, "nodetail" ) == 0 )
	    bRunDetail = false;
	else if( strcmp( argv[i]+1, "log" ) == 0 )
	    param.bLog = true;
	//else if( strcmp( argv[i]+1, "show" ) == 0 )	// use -devdev
	//    param.bShow = true;
	else if( strcmp( argv[i]+1, "plot" ) == 0 )
	    param.bPlot = true;
	//else if( strncmp( argv[i]+1, "dev", 3 ) == 0 && strncmp( argv[i]+4, "dev", 3 ) == 0 )
	else if( argv[i][1] == 'd' && argv[i][2] == 'e' && argv[i][3] == 'v' &&
	 	 argv[i][4] == 'd' && argv[i][5] == 'e' && argv[i][6] == 'v' )
	{
	    param.bPlot = true;
	    param.bShow = true;
	    param.bLog = true;
	    bOutInterPL = true;
	    bBuildInfo = true;
	}
	else if( strcmp( argv[i]+1, "loadpl" ) == 0 )
	    param.plFilename = string( argv[++i] );
	else if( strcmp( argv[i]+1, "loadnodes" ) == 0 )
	    param.nodesFilename = string( argv[++i] );
	else if( strcmp( argv[i]+1, "setoutorientn" ) == 0 )
	    param.setOutOrientN = true;
	else if( strcmp( argv[i]+1, "outbookshelf" ) == 0 )
	{
	    printf( "<< Bookshelf Converter >>\n" );
	    outBookshelf = string( argv[++i] );
	}
	else if( strcmp( argv[i]+1, "outbookshelfnoblocksite" ) == 0 )
	{
	    printf( "<<< Bookshelf Converter (remove preplaced block site) >>\n" );
	    outBookshelfNoBlockSite = string( argv[++i] );
	}
	else if( strcmp( argv[i]+1, "plt" ) == 0 )
	{
	    printf( "<<< Plotter >>>\n" );
	    bPlotOnly = true;
	    bOutInterPL = true;
	}


	// min-cut placer	
	else if( strcmp(argv[i]+1, "mincut") == 0)
	{
	    printf( "<< NTU Min-Cut Placer >>\n" );
	    bRunMincut = true;
	}
	else if( strcmp( argv[i]+1, "run" ) == 0 )
	    param.hmetis_run = atol( argv[++i] );
	else if( strcmp( argv[i]+1, "rtype" ) == 0 )
	    param.hmetis_rtype = atol( argv[++i] );
	else if( strcmp( argv[i]+1, "vcycle" ) == 0 )
	    param.hmetis_vcycle = atol( argv[++i] );
	else if( strcmp( argv[i]+1, "ubfactor" ) == 0 )
	    param.ubfactor = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "ub" ) == 0 )
	    param.hmetis_ubfactor = atoi( argv[++i] );
	else if( strcmp( argv[i]+1, "prepart" ) == 0 )	
	    param.bPrePartitioning = true;
	else if( strcmp( argv[i]+1, "refine" ) == 0 )
	    param.bRefineParts = true;
	else if( strcmp( argv[i]+1, "refineAll" ) == 0 )   
	    param.bRefinePartsAll = true;
	else if( strcmp( argv[i]+1, "ar" ) == 0 )  
	    param.aspectRatio = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "nofrac" ) == 0 )
	    param.bFractionalCut = false;
	else if( strcmp( argv[i]+1, "shrink" ) == 0 )
	    param.coreShrinkFactor = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "shrinkWidth" ) == 0 )
	    param.coreShrinkWidthFactor = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "repart" ) == 0 )
	    param.n_repart = atoi( argv[++i] );
	else if( strcmp( argv[i]+1, "out" ) == 0 )
	    param.outFilePrefix = argv[++i];

	else if( strcmp( argv[i]+1, "ctype" ) == 0 )	// used in both min-cut and nlp
	{
	    i++;
	    param.hmetis_ctype = atol( argv[i] );
	    gCType = atol( argv[i] );
	}
	else if( strcmp( argv[i]+1, "util" ) == 0 )	// used in both min-cut and nlp
	{
	    i++;
	    param.coreUtil = atof( argv[i] );           // legalization
	    gTargetUtil = param.coreUtil;               // for nlp
	    gCellRedistributionTarget = param.coreUtil; // cell shifting
	}


	// tellux diamond legalizer	
	else if( strcmp( argv[i]+1, "preLegal" ) == 0 )
	    param.preLegalFactor = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "scaleType" ) == 0 )
	    param.scaleType = (SCALE_TYPE)atoi( argv[++i] );

	
	// tellux detailed placer
	else if( strcmp( argv[i]+1, "de_MW" ) == 0 )	
	    param.de_MW = (unsigned long)atol( argv[++i] );
	else if( strcmp( argv[i]+1, "de_MM" ) == 0 )
	    param.de_MM = (unsigned long)atol( argv[++i] );
	else if( strcmp( argv[i]+1, "de_window" ) == 0 )    
	    param.de_window = atoi( argv[++i] );
	else if( strcmp( argv[i]+1, "de_time" ) == 0 )	
	{
	    param.de_btime=true;
	    param.de_time_constrain = atoi( argv[++i] );
	}
       
	
	// indark macro legalizer
	else if( strcmp(argv[i]+1, "macrolegal") == 0)	    
	{
	    printf( "<< Macro Legalizer Enabled >>\n" );
	    bRunMacroLegal = true;
	}
	else if ( strcmp(argv[i]+1, "macroheight") == 0)
	    param.n_MacroRowHeight = atoi( argv[++i] );
	else if ( strcmp(argv[i]+1, "macrolegalrun") == 0)   
	    nMacroLegalRun = atoi( argv[++i] );

	
	// donnie.  NLP placer (2006-03-19)
	else if( strcmp( argv[i]+1, "nlp" ) == 0 )
	{
	    printf( "<< NTU NLP Placer >>\n" );
	    bRunMincut = false;
	}
	else if( strcmp( argv[i]+1, "step" ) == 0 )	 // 2006-03-24
	    param.step = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "precision" ) == 0 ) // 2006-03-24
	    param.precision = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "topdblstep" ) == 0 )
	    param.topDblStep = true;
	else if( strcmp( argv[i]+1, "nlpR" ) == 0 )	 
	    gClusterRatio = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "nlpN" ) == 0 )	
	    gClusterTargetSize = atoi( argv[++i] );
	else if( strcmp( argv[i]+1, "nlpn" ) == 0 )	// (UNUSE)
	    gTargetNNB = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "nlpd" ) == 0 )	
	    gTargetDensity = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "nlpw" ) == 0 )	
	    gWireWeight = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "nlpi" ) == 0 )	
	    gIncFactor = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "nlpw" ) == 0 )	
	    gRunWCycle = true;
	else if( strcmp( argv[i]+1, "maxLevel" ) == 0 )	
	    gMaxLevel = atoi( argv[++i] );
	else if( strcmp( argv[i]+1, "nlpweightDe" ) == 0 )  
	    gWeightLevelDecreaingRate = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "UTIL" ) == 0 )	     
	    gTargetUtil = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "allmacromove" ) == 0 )  
	    allMacroMove = true;
	else if( strcmp( argv[i]+1, "shrinkCore" ) == 0 )
	    bShrinkCore = true;
	
	
	//Added by Jin 20060223
	else if( strcmp( argv[i]+1, "bbcellswap" ) == 0 )
		dpParam.SetRunBBCellSwap();
	else if( strcmp( argv[i]+1, "bbwindowsize" ) == 0 )
		dpParam.SetBBWindowSize( atoi(argv[++i]) );
	else if( strcmp( argv[i]+1, "bboverlapsize" ) == 0 )
		dpParam.SetBBOverlapSize( atoi(argv[++i]) );
	else if( strcmp( argv[i]+1, "bbiteration" ) == 0 )
		dpParam.SetBBIteration( atoi(argv[++i]) );

	
	// tellux cell redistributor (density constraint)
	else if( strcmp( argv[i]+1, "cr" ) == 0 ) //by tchsu
	{
	    i++;
	    if( atof(argv[i]) < 0 )
		bCellRedistribution = false;
	    else
	    {
		bCellRedistribution = true; 
		gCellRedistributionTarget= atof( argv[i] );
	    }
	}


	else if( strcmp( argv[i]+1, "lpnorm_p" ) == 0 || strcmp( argv[i]+1, "alpha" ) == 0 || strcmp( argv[i] +1, "p" ) == 0 )
	{	
	    param.dLpNorm_P = atof( argv[++i] );
	}

	else 
	{
	    cout << "Unknown argument: " << argv[i] << endl;
	    return false;
	}
	i++;
    }*/
    return true;
}



///////////////////////////////////////////////////////////////////////////

void printUsage()
{
    printf( "\n" );
    printf( "Usage: (1) place.exe -aux circuit.aux [-util float]\n" );
    printf( "       (2) place.exe -lefdef circuit.lef circuit.def [-util float]\n" );
    printf( "Ex: place.exe -aux adaptec1.aux -util 0.8\n" );

}

///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	int main_start_time = time(NULL);

    programStartTime = seconds();

    /*cout << "\n";
    cout << "==============================================================\n\n";
    cout << "                    NTUplace " << __NTUPLACE_VERSION__ << endl <<endl;
    cout << __BUILD_USER__ <<"@" <<__BUILD_HOST__ << "  " << __BUILD_ARCH__<<  "  " << __BUILD_DATE__ << endl;
    cout << "==============================================================\n";
    cout << "\n";*/ //comment by chu 2023

    //ShowSystemInfo();  // 2005-12-26 //comment by chu 2023
   
	//2023-04-30 Chu
	//parse txt to aux, argv[1]=="input.txt"
	case_file_name = argv[1];

	Parsertxt p;
	p.parsetxt(argv[1]);
	//parse txt to aux finish //2023-04-30 Chu


    /*if( argc < 2 )
    {
	printUsage();
	return 0;
    }*/ //comment by chu 2023

    CPlaceDB placedb;
    
    //param.seed = (unsigned)time(NULL);
    param.seed = 1;
    
    if( !handleArgument( argc, argv, param ) )
	return -1;
    


    if (bBuildInfo)
    {
	cout << "CXX=" << __CXX_VERSION__ << endl;
	cout << "CXXFLAGS=" << __CXXFLAGS__ <<endl;
	cout << "LDFLAGS=" << __LDFLAGS__ <<endl;
    }

    if( param.bShow )
	param.Print();

    srand( param.seed );
    

    ////////////////////////////////////////////////////////////////
    // Read files
    ////////////////////////////////////////////////////////////////
    double read_time_start = clock();
    /*if( strcmp( argv[1], "-aux" ) == 0 )//2023/6/14
    {
	// bookshelf 
	printf( "Use BOOKSHELF format\n" );*/
	CParserIBMMix parserIBM;
	/*if( param.nodesFilename.length() > 0 )
	    parserIBM.ReadFile( argv[2], placedb, param.nodesFilename.c_str() );
	else*/
	    //parserIBM.ReadFile( argv[2], placedb );
		parserIBM.ReadFile( "circuit.aux", placedb );//2023/6/14
	placedb.CheckStdCellOrient();
    //}
    /*else if( strcmp( argv[1], "-lefdef" ) == 0 ) //2023/6/14
    {
	// lef/def
	printf( "Use LEF/DEF format\n" );
	parserLEFDEF.ReadLEF( argv[2], placedb  );
	
	//parserLEFDEF.PrintMacros( false );  // no pins
	parserLEFDEF.ReadDEF( argv[3], placedb  );
	
	placedb.SetCoreRegion();
	//placedb.PrintModules();
	//placedb.PrintNets();
    }
    else
    {
	printUsage();
	return 0;
    }*/
    printf( "Circuit imported (CPU time: %.0f sec)\n\n", (clock()-read_time_start)/CLOCKS_PER_SEC );
    

    ///////////////////////////////////////////////////////
    // Load PL from file (2005-12-06 by donnie)
    ///////////////////////////////////////////////////////
    if( param.plFilename != "" )
    {
	CParserIBMMix parserIBM;
	placedb.CreateModuleNameMap();
	parserIBM.ReadPLFile( param.plFilename.c_str(), placedb );
	placedb.ClearModuleNameMap();
    }
    
    
    placedb.Init(); // set member variables, module member variables, "isOutCore"
    
    if( allMacroMove )
	placedb.SetAllBlockMovable();
    
    // 2005-12-17
    placedb.ShowDBInfo();
    

    //////// PLOTTER ////////////////////////
    
    if( bOutInterPL )
    {
	placedb.OutputGnuplotFigure( "init.plt", false );
	placedb.OutputGnuplotFigureWithZoom("init",false,true,true);
	placedb.OutputGnuplotFigureWithMacroPin( "init_with_pin.plt", false );
    
	if( bPlotOnly )
	{
	//Start:=====================(indark)==========================
// 		for(unsigned int i = 0 ; i < placedb.m_modules.size() ; i++){
// 			if (!placedb.m_modules[i].m_isOutCore){
// 				placedb.m_modules[i].m_isFixed = false;
// 			}
// 		}
// 		string file = param.outFilePrefix + ".mac.nodes";
// 		placedb.OutputNodes(file.c_str(),true);
	//End:=====================(indark)==========================
	    return 0;
	}
	
	if( strcmp( argv[1], "-lefdef" ) == 0 )
	    parserLEFDEF.WriteDEF( argv[3], "init.def", placedb );
    }

    //////// Bookshelf Converter ////////////

    if( outBookshelf != "" )
    {
	placedb.OutputBookshelf( outBookshelf.c_str(), param.setOutOrientN );
	return 0;
    }
    if( outBookshelfNoBlockSite != "" )
    {
	placedb.RemoveFixedBlockSite();
	placedb.OutputBookshelf( outBookshelfNoBlockSite.c_str(), param.setOutOrientN );
	return 0;
    }


    //===== debugging ======
     //placedb.PrintNets();
     //placedb.PrintModules();
    //======================

     //placedb.AdjustCoordinate();

     //placedb.SetCoreRegion();
     //placedb.ShowRows();
     //cout << "SITE INFO:WIDTH=" << parserLEFDEF.m_coreSiteWidth * parserLEFDEF.m_defUnit
     //		    << "'" << parserLEFDEF.m_coreSiteHeight  * parserLEFDEF.m_defUnit << endl;
     //placedb.CheckRowHeight(parserLEFDEF.m_coreSiteHeight  * parserLEFDEF.m_defUnit );


    ////////////////////////////////////////////////////////////////
    // Global Placement
    ////////////////////////////////////////////////////////////////
    
    double part_time_start;    
    double total_part_time = 0;
    bool isLegal = false; 
    if( bRunGlobal )
    {
	part_time_start = seconds();
	
	cout << "\n[STAGE 1]: Global placement...\n";

	int number = placedb.CreateDummyFixedBlock();
	printf( "Add %d fixed blocks\n", number );
	placedb.RemoveFixedBlockSite();
	
	if (bRunMincut)
	{
	    CMinCutPlacer  *pPlacer = new CMinCutPlacer(placedb);
	    CMinCutPlacer  &placer = *pPlacer;
	    //CMinCutPlacer  placer(placedb);
	    placer.param = param;
	    
	    // min-cut placer
	    placer.ShrinkCore(); // shrink core region according to the placer.parameters
	    placer.Init();

	    cout << "\n%%% INIT %%%\n" ;
	    cout << "INIT: Pin-to-pin HPWL= " << placedb.CalcHPWL() << "\n";

	    if( bOutInterPL )
	    {
		string file = placer.param.outFilePrefix + "_global_init.plt";
		placedb.OutputGnuplotFigure( file.c_str(), false );
	    }

	    placer.RecursivePartition();	    // global placement

	    if( param.bShow )
	    {
		cout << "\n ctype1: " << param.n_ctype_count[1] << endl;
		cout << " ctype2: " << param.n_ctype_count[2] << endl;
		cout << " ctype3: " << param.n_ctype_count[3] << endl;
		cout << " ctype4: " << param.n_ctype_count[4] << endl;
		cout << " ctype5: " << param.n_ctype_count[5] << endl;
	    }

	    param = placer.param;   // record param.n_ctype_count[]

	    // TODO: restore the old core region
	    
	    delete pPlacer; 
	    pPlacer = NULL; 
	}
	else
	{
	    /*if( bShrinkCore )
	    {
		placer.ShrinkCore(); // shrink core region according to the placer.parameters
		// since core shrinked, cells must be distributed evenly
		gTargetUtil = -1;
	    }*/

	    double xMin = placedb.m_coreRgn.left;
	    double yMin = placedb.m_coreRgn.bottom;

	    for( unsigned int i=0; i<placedb.m_modules.size(); i++ )
                if( placedb.m_modules[i].m_isFixed )
                {
                    if( placedb.m_modules[i].m_cx < xMin )
                        xMin = placedb.m_modules[i].m_cx;
                    if( placedb.m_modules[i].m_cy < yMin )
                        yMin = placedb.m_modules[i].m_cy;
                }
            for( unsigned int i=0; i<placedb.m_pins.size(); i++ )
            {
                int modId = placedb.m_pins[i].moduleId;
                if( placedb.m_modules[modId].m_isFixed )
                {
                    if( placedb.m_pins[i].absX < xMin )
                        xMin = placedb.m_pins[i].absX;
                    if( placedb.m_pins[i].absY < yMin )
                        yMin = placedb.m_pins[i].absY;
                }
            }

	    xMin -= (placedb.m_coreRgn.right - placedb.m_coreRgn.left) * 0.02;
	    yMin -= (placedb.m_coreRgn.top - placedb.m_coreRgn.bottom) * 0.02;

	    xMin = floor( xMin );
            yMin = floor( yMin );
            double xShift = -xMin;
            double yShift = -yMin;
            if( param.bShow )
            {
                printf( " shift (%.0f %.0f)\n", xShift, yShift );
            }

            CPlaceDBScaling::XShift( placedb, xShift );
            CPlaceDBScaling::YShift( placedb, yShift );
	    
	    if( gRunWCycle == true )
	    {
		// w-cycle is not tuned (2006-03-22) donnie
		multilevel_nlp( placedb, param.outFilePrefix, 1,
			gClusterTargetSize, gClusterRatio, gTargetNNB, gTargetDensity,
			gIncFactor, gWireWeight, gMaxLevel, gWeightLevelDecreaingRate  );  // donnie

		if( bOutInterPL )
		{
		    string file = param.outFilePrefix + "_global_mid.plt";
		    placedb.OutputGnuplotFigure( file.c_str(), false );
		    file = param.outFilePrefix + "_global_mid.pl";
		    placedb.OutputPL( file.c_str() );
		}

		isLegal = multilevel_nlp( placedb, param.outFilePrefix, 2,
			gClusterTargetSize, gClusterRatio, gTargetNNB, gTargetDensity,
			gIncFactor, gWireWeight, gMaxLevel, gWeightLevelDecreaingRate,
		        gTargetUtil );  // donnie
	    }
	    else
	    {
		isLegal = multilevel_nlp( placedb, param.outFilePrefix, gCType,   // 2006-03-18, test new clustering
			gClusterTargetSize, gClusterRatio, gTargetNNB, gTargetDensity,
			gIncFactor, gWireWeight, gMaxLevel, gWeightLevelDecreaingRate,
		        gTargetUtil );  // donnie
	    }

	    CPlaceDBScaling::YShift( placedb, -yShift );
            CPlaceDBScaling::XShift( placedb, -xShift );
	}
	total_part_time = seconds() - part_time_start;	    // end global place
	

	if( bOutInterPL )
	{
	    string file = param.outFilePrefix + "_global.pl";
	    placedb.OutputPL( file.c_str() );
	    if( strcmp( argv[1], "-lefdef" ) == 0 )
		parserLEFDEF.WriteDEF( argv[3], "out_global.def", placedb );
	}
    
	if( param.bPlot )
	{
	    string file = param.outFilePrefix + "_global.plt";
	    placedb.OutputGnuplotFigure( file.c_str(), false );
	}

	if( param.bShow )
	    placedb.ShowDensityInfo();

	printf( "\nGLOBAL: CPU = %.0f sec = %.1f min\n", total_part_time, total_part_time/60.0 );
	printf( "GLOBAL: Pin-to-pin HPWL = %.0f\n", placedb.CalcHPWL() );
    }
    double wl1 = placedb.CalcHPWL();
   

   /*if( true )//2023/6/18
    {
	if( strcmp( argv[1], "-lefdef" ) == 0 )
	{
	    string file = param.outFilePrefix + ".ntup.def";
	    parserLEFDEF.WriteDEF( argv[3], file.c_str(), placedb );	    
	}
	else
	{
	    string file = param.outFilePrefix + ".ntup.pl";
	    placedb.OutputPL( file.c_str() );
	}
    }
   cin.get();*/
    
    ////////////////////////////////////////////////////////////////////////
    //Macro Lefalization
    ////////////////////////////////////////////////////////////////////////
    //double time_LPML = seconds();
	
#if 0
    bool bMLFail = false;
    if (bRunMacroLegal )
    {  
	placedb.m_modules_bak = placedb.m_modules;
	vector<int> legaled_moduleID;
	CMacroLegal* mlegal[10];
	mlegal[0]  = new CMacroLegal(placedb, param.n_MacroRowHeight,nMacroLegalRun);
	bMLFail = mlegal[0]->Legalize(legaled_moduleID);
	printf("LP-Macro Legalization Time : %6.1f s\n",seconds() - time_LPML);
	// 	    mlegal[0]->ApplyPlaceDB(true);
	// 	    placedb.RemoveFixedBlockSite();
	// 	for(unsigned int i = 0 ; i < 6 ; i++){
	// 		
	// 		mlegal[i]->Legalize();
	// 		delete mlegal[i];
	// 	}

	//placer.ShrinkCore(); // shrink core region according to the placer.parameters
	if (bMLFail)
	{
	    placedb.m_modules = placedb.m_modules_bak;
	}
	else
	{
	    if( bOutInterPL )
	    {
		string file = param.outFilePrefix + "_macrolegal.pl";
		string file2 = param.outFilePrefix + "_macrolegal.nodes";
		placedb.OutputPL( file.c_str() );
		//placedb.OutputNodes(file2.c_str(),false);
	    }
	    if( param.bPlot )
	    {
		string file = param.outFilePrefix + "_macrolegal.plt";
		placedb.OutputGnuplotFigure( file.c_str(), true );
	    }
	}


	//re initialize
	//placedb.RestoreCoreRgn();
	// 	delete pPlacer;
	// 	pPlacer = new CMinCutPlacer(placedb);
	// 	placer = *pPlacer;
	// 	
	// 	// min-cut placer
	// 	 placer.Init();
	// 
	// 	    cout << "\n%%% INIT %%%\n" ;
	// 	    cout << "INIT: Pin-to-pin HPWL= " << placedb.CalcHPWL() << "\n";
	// 
	// 	    if( bOutInterPL )
	// 	    {
	// 		string file = placer.param.outFilePrefix + "_global_init.plt";
	// 		placedb.OutputGnuplotFigure( file.c_str(), false );
	// 	    }
	// 
	// 	    placer.RecursivePartition();	    // global placement
	// 	
	// 	    file = placer.param.outFilePrefix + "_global-ml.plt";
	// 	    placedb.OutputGnuplotFigure( file.c_str(), false );
	// 	

    }
#endif


    ////////////////////////////////////////////////////////////////
    // Legalization
    ////////////////////////////////////////////////////////////////

    double legal_time_start = seconds();
    double total_legal_time = 0;
    double wl2 = 1e20;
    if( bRunLegal )	// add "!isLegal" (donnie, 2006-03-06)
    {
	printf( "\n[STAGE 2]: Legalization (circuit = %s)\n\n", argv[2] );
	flush( cout );
	placedb.RemoveFixedBlockSite();

	if( isLegal )
	{
	    // fake!
	    for( int i=0; i<=10; i++ )
		printf( "[%d] ", i );
	    printf( "done\n" );
	}
	else
	{

	    //Start:=====================(indark)==========================
#if 1 
	    CTetrisLegal legal(placedb);
	    //test code
	    //bool bMacroShifter = legal.MacroShifter(2, false);
	    //if(bMacroShifter)
	    //{
	    //	cout << "MacroShifter success" << endl;
	    //}
	    //else
	    //{
	    //	cout << "MacroShifter fail" << endl;
	    //}
	    //legal.RestoreFreeSite();
	    //@test code

	    //bool bLegal = legal.Solve( gTargetUtil, bRunMacroLegal, true );
	    bool bLegal = legal.Solve( gTargetUtil, bRunMacroLegal, true );

    
	    /*if( !bLegal )//2023/6/18
	    {
		cout << "Legalization failed" << endl;
		cout << "Unlegal count: " << legal.GetUnlegalCount() << endl;
		flush(cout);
		exit( 0 );  // fail to legalize, stop
	    }*/
		
#else
	    PlaceLegalize( placedb, param, wl1, wl2 );
#endif
	    //End:=====================(indark)==========================
	    total_legal_time = seconds() - legal_time_start; 
	    wl2 = placedb.CalcHPWL();
	    printf( "LEGAL: Pin-to-pin HPWL = %.0f (%.2f%%)\n", wl2, 100.0*(wl2/wl1-1.0) );
	    fflush( stdout );

	    if( bOutInterPL )
	    {
		if( strcmp( argv[1], "-lefdef" ) == 0 )
		{
		    string file = param.outFilePrefix + "_legal.def";
		    parserLEFDEF.WriteDEF( argv[3], file.c_str(), placedb );
		}
		else
		{
		    string file = param.outFilePrefix + "_legal.pl";
		    placedb.OutputPL( file.c_str() );
		}
	    }

	    if( param.bPlot )
	    {
		string file = param.outFilePrefix + "_legal.plt";
		//placedb.OutputGnuplotFigure( file.c_str(), false );
		placedb.OutputGnuplotFigure( file.c_str(), true );
	    }

	    if( param.bShow )	
		placedb.ShowDensityInfo();   // donnie

	}
    }

	//Alignment after legalization
//	placedb.Align();

	if( param.bLog )
    {
	printf("Checking after legalization");	
	//Check for overlap
	CCheckLegal clegal( placedb );
	clegal.check();
	}


	
    ////////////////////////////////////////////////////////////////
    // Detailed Placement
    ////////////////////////////////////////////////////////////////

    double detail_time_start = seconds();
    double wl3 = 0;
    double total_detail_time = 0;
    if( bRunDetail == true )
    {
	printf( "\n[STAGE 3]: Deatiled Placement (circuit = %s)\n\n", argv[2] );
	placedb.RemoveMacroSite();
	CDetailPlacer dplacer( placedb, param, dpParam );
	dplacer.DetailPlace();
	if( param.bLog )
	    placedb.ShowDensityInfo();
    }
    wl3 = placedb.GetHPWLp2p();
    total_detail_time = seconds() - detail_time_start;
    if( bOutInterPL && strcmp( argv[1], "-lefdef" ) == 0 )
    {
	string file = param.outFilePrefix + "_detail.def";
	parserLEFDEF.WriteDEF( argv[3], file.c_str(), placedb );	
    }

    ////////////////////////////////////////////////////////////////
    // cell redistribution by tchsu
    ////////////////////////////////////////////////////////////////
    
    if( bCellRedistribution==true && gCellRedistributionTarget>0 && gCellRedistributionTarget<1.0 )
    {
	placedb.RemoveMacroSite();
	CCellmoving cm( placedb );
	cm.redistributeCell(gCellRedistributionTarget);
	if( param.bLog )
	{	
	    printf("Checking after cell redistribution\n");
	    fflush( stdout );
	    CCheckLegal clegal( placedb );
	    clegal.check();
	}	
    }

    
    //////////////////////////////////////////////////////////////
    // align cells to the site (by indark)
    //////////////////////////////////////////////////////////////
   
    if( strcmp( argv[1], "-lefdef" ) == 0 )
    {	
	placedb.Align();
	printf( " Aligned HPWL: %.0f %.0f %.0f\n", 
		placedb.CalcHPWL(), placedb.GetHPWLp2p(), placedb.GetHPWLdensity(gTargetUtil) );

	if( param.bLog )
	{	
	    printf("Checking after alignment\n");
	    fflush( stdout );
	    CCheckLegal clegal( placedb );
	    clegal.check();
	}

	if( param.bPlot )
	{
	    string file = param.outFilePrefix + "_align.plt";	
	    placedb.OutputGnuplotFigure( file.c_str(), true );
	}
    }
    
    
    ////////////////////////////////////////////////////////////////
    // overlap checking
    ////////////////////////////////////////////////////////////////

    if( param.bLog )
    {	
	printf("Check overlap after detailed placement");
	fflush( stdout );
	CCheckLegal clegal( placedb );
	clegal.check();
    }

    
    //////////////////////////////////////////////////////////////
    // output placement result
    //////////////////////////////////////////////////////////////
    
    if( true )
    {
	if( strcmp( argv[1], "-lefdef" ) == 0 )
	{
	    string file = param.outFilePrefix + ".ntup.def";
	    parserLEFDEF.WriteDEF( argv[3], file.c_str(), placedb );	    
	}
	else
	{
	    string file = param.outFilePrefix + ".ntup.pl";
	    placedb.OutputPL( file.c_str() );
	}
    }

    
    ///////////////////////////////////////////////////
    // Show results (donnie, 2006-03-16) 
    ///////////////////////////////////////////////////
    /*double total_time = seconds() - programStartTime;//comment 2023/6/19
    printf( "\nCircuit: %s\n", argv[2] );
    printf( "  Global Time: %6.0f sec (%.1f min) wire= %.0f\n", total_part_time, total_part_time / 60.0, wl1 );
    if( bRunLegal && param.bShow && !isLegal )
    printf( "   Legal Time: %6.0f sec (%.1f min) wire= %.0f\n", total_legal_time, total_legal_time / 60.0, wl2 );
    if( bRunDetail )
    printf( "  Detail Time: %6.0f sec (%.1f min) wire= %.0f\n", total_detail_time, total_detail_time / 60.0, wl3 );
    printf( " ===================================================================\n" );
    printf( "   Total Time: %6.0f sec (%.1f min) wire= %.0f\n", total_time, total_time / 60.0, placedb.GetHPWLp2p() );
    if( gTargetUtil > 0 )
    printf( "                                     wire= %.0f (util %.2f)\n", 
	    placedb.GetHPWLdensity(gTargetUtil), gTargetUtil );

    if( param.bLog )
    {	
	fprintf( stderr, "\nCircuit: %s\n", argv[2] );
	fprintf( stderr, "HPWL: %.0f (ctype %d)\n", placedb.GetHPWLp2p(), gCType ); 
	if( gTargetUtil > 0 )
	    fprintf( stderr, "dHPWL: %.0f (util=%.2f)\n", placedb.GetHPWLdensity(gTargetUtil), gTargetUtil ); 
	fprintf( stderr, "Total CPU: %.0f sec = %.1f min (gCPU = %.0f sec)\n\n", 
		seconds() - programStartTime, (seconds() - programStartTime)/60.0, total_part_time ); 
    }*/

    
    ////////////////////////////////////////////////////////////////
    // Log Result
    ////////////////////////////////////////////////////////////////
	
    /*if( param.bLog ) //comment 2023/6/19
    {
	cout << "Result logged.\n";
	FILE* out_log;
	out_log = fopen( "log_result.txt", "a" );

	struct tm *newtime;
	time_t aclock;
	time( &aclock );                  // Get time in seconds
	newtime = localtime( &aclock );   // Convert time to struct tm form
	char timeStr[100];

	string hostName = GetHostName();
	
	sprintf( timeStr, "%02d/%02d %02d:%02d", 
		newtime->tm_mon+1, newtime->tm_mday,
		newtime->tm_hour, newtime->tm_min );
	fprintf( out_log, "%s,%s, %5.3e, %5.3e, %5.3e, %5.3e, %5.0fs, %5.0fs, %s %s %d %s\n",
		timeStr, argv[2], wl1, wl2, wl3, placedb.GetHPWLdensity( gTargetUtil ),
		total_time, total_part_time, 
		param.GetParamString( bRunMincut ).c_str(), param.outFilePrefix.c_str(), gCType,
		hostName.c_str() );
	fclose( out_log );
    }*/

	


	
	//legalization-corner stitching 2023/6/19
	Plane** plane_best[thread_num];
	thread* th[thread_num];
	//int thread_num = 6;

	for (int i = 0; i < thread_num; i++)
	{
		plane_best[i] = new Plane*;
		th[i] = new thread(legalization, plane_best[i], i, argv[1], thread_run_time);
	}

	for (int i = 0; i < thread_num; i++)
	{
		th[i]->join();
	}

	//system("cls");

	cout << "result: " << endl;
	for (int i = 0; i < thread_num; i++)
	{
		cout << (*plane_best[i])->hpwl << endl;
	}


	/*cout << "result: " << calc_hpwl(plane_result) << endl; cout << endl;

	plot_matlab(plane_result, "out_legal.m");*/


	cout << "=====Legalization Finish=====" << endl; cout << endl;
	
	

	cout << "========Detail Start=========" << endl;
	
	/*cout << "<stage 1> exchange" << endl;
	cout << endl;

	for (int k = 0; k < 2; k++)
	{
		//cout << k << endl;
		for (int i = 0; i < plane_result->soft_tile_list.size(); i++)
		{
			for (int j = 0; j < plane_result->soft_tile_list.size(); j++)
			{
				if (i == j)	continue;

				//cout << i << " " << j << endl;
				//cout << plane_result->soft_tile_list[i]->ti_name << endl;
				//cout << plane_result->soft_tile_list[j]->ti_name << endl;
				int improvement = ExchangeSoftTile(plane_result->soft_tile_list[i], plane_result->soft_tile_list[j], plane_result);
				if (improvement != 0)
				{
					cout << "current hpwl:" << calc_hpwl(plane_result) << endl;
					cout << endl;
				}
			}
		}
	}


	if (plane_result->legal == 0)
		cout << "FUCKFUCKFUCKFUCKFUCKFUCKFUCKFUCKFUCKFUCKFUCKFUCK" << endl;

	plot_matlab(plane_result, "out_detail.m");


	plane_result->hpwl = calc_hpwl(plane_result);
	cout << "result: " << plane_result->hpwl << endl;*/
	

	cout << "<stage 2> transform" << endl;

	
	vector<Trantile> tp_soft_list2[thread_num];

	for (int i = 0; i < thread_num; i++)
	{
		tp_soft_list2[i] = transform_function(*plane_best[i]);
	}

	Plane* plane_result = *plane_best[0];
	vector<Trantile> soft_tile_list2 = tp_soft_list2[0];

	cout << "result:" << endl;
	cout <<"[1]" << (*plane_best[0])->hpwl << endl;
	for (int i = 1; i < thread_num; i++)
	{
		cout<< "[" << i+1 << "]" <<(*plane_best[i])->hpwl << endl;
		if ((*plane_best[i])->hpwl < plane_result->hpwl)
		{
			plane_result = *plane_best[i];
			soft_tile_list2 = tp_soft_list2[i];
		}
	}


	vector<Tile*> white_tile_list;
	Enumerate(plane_result, white_tile_list);
	
	string matlab_file_name(1,argv[1][5]);
	matlab_file_name = "case" + matlab_file_name+".m";

	ofstream newFile; 
	newFile.open(matlab_file_name);
	newFile << "axis equal;\n" << "hold on;\n" << "grid on;\n";
	newFile << "block_x=[0 0 " << plane_result->pl_width << " " << plane_result->pl_width << " 0];" << endl;
	newFile << "block_y=[0 " << plane_result->pl_height << " " << plane_result->pl_height << " 0 0];" << endl;
	newFile << "fill(block_x, block_y, 'c');" << endl;

	for (int i = 0; i < white_tile_list.size(); i++)
	{
		newFile << "block_x=[" << LEFT(white_tile_list[i]) << " " << LEFT(white_tile_list[i]) << " ";
		newFile << RIGHT(white_tile_list[i]) << " " << RIGHT(white_tile_list[i]) << " " << LEFT(white_tile_list[i]) << "];" << endl;
		newFile << "block_y=[" << BOTTOM(white_tile_list[i]) << " " << TOP(white_tile_list[i]) << " ";
		newFile << TOP(white_tile_list[i]) << " " << BOTTOM(white_tile_list[i]) << " " << BOTTOM(white_tile_list[i]) << "];" << endl;
		newFile << "fill(block_x, block_y, 'w');" << endl;
	}

	for (int i = 0; i < plane_result->fixed_tile_list.size(); i++)
	{
		newFile << "block_x=[" << LEFT(plane_result->fixed_tile_list[i]) << " " << LEFT(plane_result->fixed_tile_list[i]) << " ";
		newFile << RIGHT(plane_result->fixed_tile_list[i]) << " " << RIGHT(plane_result->fixed_tile_list[i]) << " " << LEFT(plane_result->fixed_tile_list[i]) << "];" << endl;
		newFile << "block_y=[" << BOTTOM(plane_result->fixed_tile_list[i]) << " " << TOP(plane_result->fixed_tile_list[i]) << " ";
		newFile << TOP(plane_result->fixed_tile_list[i]) << " " << BOTTOM(plane_result->fixed_tile_list[i]) << " " << BOTTOM(plane_result->fixed_tile_list[i]) << "];" << endl;
		newFile << "fill(block_x, block_y, 'y');" << endl;
	}

	if (plane_result->legal)
	{
		for (int i = 0; i < soft_tile_list2.size(); i++)
		{
			if (soft_tile_list2[i].t2 == nullptr)
			{
				newFile << "block_x=[" << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
				newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << "];" << endl;
				newFile << "block_y=[" << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t1) << " ";
				newFile << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;				
			}
			else
			{
				if (soft_tile_list2[i].dir == 1)
				{
					newFile << "block_x=[";
					newFile << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile << LEFT(soft_tile_list2[i].t2) << " " << LEFT(soft_tile_list2[i].t2) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t2) << " " << TOP(soft_tile_list2[i].t2) << " ";
					newFile << TOP(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 2)
				{
					newFile << "block_x=[";
					newFile << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t2) << " " << RIGHT(soft_tile_list2[i].t2) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newFile << TOP(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t2) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 3)
				{
					newFile << "block_x=[";
					newFile << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t2) << " " << RIGHT(soft_tile_list2[i].t2) << " ";
					newFile << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newFile << TOP(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t2) << " ";
					newFile << TOP(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 4)
				{
					newFile << "block_x=[";
					newFile << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile << LEFT(soft_tile_list2[i].t2) << " " << LEFT(soft_tile_list2[i].t2) << " ";
					newFile << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newFile << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t2) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 5)
				{
					newFile << "block_x=[";
					newFile << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t2) << " " << RIGHT(soft_tile_list2[i].t2) << " ";
					newFile << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t2) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newFile << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t2) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t2) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 6)
				{
					newFile << "block_x=[";
					newFile << LEFT(soft_tile_list2[i].t2) << " " << LEFT(soft_tile_list2[i].t2) << " ";
					newFile << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile << LEFT(soft_tile_list2[i].t2) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t2) << " " << TOP(soft_tile_list2[i].t2) << " ";
					newFile << TOP(soft_tile_list2[i].t2) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newFile << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t2) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 7)
				{
					newFile << "block_x=[";
					newFile << LEFT(soft_tile_list2[i].t2) << " " << LEFT(soft_tile_list2[i].t2) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile << LEFT(soft_tile_list2[i].t2) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t2) << " " << TOP(soft_tile_list2[i].t2) << " ";
					newFile << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t2) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t2) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 8)
				{
					newFile << "block_x=[";
					newFile << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t2) << " " << RIGHT(soft_tile_list2[i].t2) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t2) << " ";
					newFile << TOP(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t2) << " ";
					newFile << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
				}
			}
			newFile << "fill(block_x, block_y, 'g');" << endl;
			newFile << "text(" << soft_tile_list2[i].t1->ti_ll.p_x << "," << soft_tile_list2[i].t1->ti_mid.p_y << ",'" << soft_tile_list2[i].ti_name << "')" << endl;
		}
	}

	ofstream newFile3;
	newFile3.open(argv[2]);
	newFile3 << "HPWL " << plane_result->hpwl << endl;
	newFile3 << "SOFTMODULE " << soft_tile_list2.size() << endl;
	for (int i = 0; i < soft_tile_list2.size(); i++)
	{
		newFile3 << soft_tile_list2[i].ti_name << " ";
		if (soft_tile_list2[i].t2 == nullptr)
		{
			newFile3 << "4" << endl;
			newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
			newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
			newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
			newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
		}
		else
		{
			newFile3 << "6" << endl;
			if (soft_tile_list2[i].dir == 1)
			{
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
			}
			else if (soft_tile_list2[i].dir == 2)
			{
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
			}
			else if (soft_tile_list2[i].dir == 3)
			{
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
			}
			else if (soft_tile_list2[i].dir == 4)
			{
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
			}
			else if (soft_tile_list2[i].dir == 5)
			{
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
			}
			else if (soft_tile_list2[i].dir == 6)
			{
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
			}
			else if (soft_tile_list2[i].dir == 7)
			{
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
			}
			else if (soft_tile_list2[i].dir == 8)
			{
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
			}
		}
	}


	//system("pause");

    system("rm circuit.*");
	system("rm out.ntup.pl");
	system("rm out.aux");
	system("rm out_global.plt");
	system("rm gbase1.dat");
	//system("rm output.txt");
	//system("clear");
	cout << "result:" << plane_result->hpwl <<endl;
	cout<<"total run time: "<<(double(time(NULL))-double(main_start_time))/60<<"mins"<<endl;

    return 0;
}

