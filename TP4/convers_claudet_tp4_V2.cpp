/*
    Exemples de transformations en OpenCV, avec zoom, seuil et affichage en
    couleurs. L'image de niveau est en CV_32SC1.

    g++ -Wall --std=c++14 $(pkg-config opencv --cflags)  ex01-transfos.cpp \
                          $(pkg-config opencv --libs) -o ex01-transfos
    ./ex01-transfos [-mag width height] [-thr seuil] image_in [image_out]

    CC-BY Edouard.Thiel@univ-amu.fr - 23/09/2019
    Version 2 : 14/10/2019
*/

#include <iostream>
#include <cstring>
#include <opencv2/opencv.hpp>
#include <vector>
#include <climits>
#define CHECK_MAT_TYPE(mat, format_type) \
    if (mat.type() != int(format_type)) \
        throw std::runtime_error(std::string(__func__) +\
            ": format non géré '" + std::to_string(mat.type()) +\
            "' pour la matrice '" # mat "'");


//--------------------------------- L O U P E ---------------------------------

//int dir_x[] = {1,1,0,-1,-1,-1,0,1};
//int dir_y[] = {0,1,1,1,0,-1,-1,-1};




class Loupe {
  public:
    int zoom = 5;
    int zoom_max = 20;
    int zoom_x0 = 0;
    int zoom_y0 = 0;
    int zoom_x1 = 100;
    int zoom_y1 = 100;

    void reborner (cv::Mat &res1, cv::Mat &res2)
    {
        int bon_zoom = zoom >= 1 ? zoom : 1;

        int h = res2.rows / bon_zoom;
        int w = res2.cols / bon_zoom;

        if (zoom_x0 < 0) zoom_x0 = 0;
        zoom_x1 = zoom_x0 + w;
        if (zoom_x1 > res1.cols) {
            zoom_x1 = res1.cols;
            zoom_x0 = zoom_x1 - w;
            if (zoom_x0 < 0) zoom_x0 = 0;
        }

        if (zoom_y0 < 0) zoom_y0 = 0;
        zoom_y1 = zoom_y0 + h;
        if (zoom_y1 > res1.rows) {
            zoom_y1 = res1.rows;
            zoom_y0 = zoom_y1 - h;
            if (zoom_y0 < 0) zoom_y0 = 0;
        }
    }

    void deplacer (cv::Mat &res1, cv::Mat &res2, int dx, int dy)
    {
        zoom_x0 += dx; zoom_y0 += dy;
        zoom_x1 += dx; zoom_y1 += dy;
        reborner (res1, res2);
    }

    void dessiner_rect (cv::Mat &src, cv::Mat &dest)
    {
        dest = src.clone();
        if (zoom == 0) return;
        cv::Point p0 = cv::Point(zoom_x0, zoom_y0),
                  p1 = cv::Point(zoom_x1, zoom_y1);
        cv::rectangle(dest, p0, p1, cv::Scalar (255, 255, 255), 3, 4);
        cv::rectangle(dest, p0, p1, cv::Scalar (  0,   0, 255), 1, 4);
    }

    void dessiner_portion (cv::Mat &src, cv::Mat &dest)
    {
        CHECK_MAT_TYPE(src, CV_8UC3)

        int bon_zoom = zoom >= 1 ? zoom : 1;

        for (int y = 0; y < dest.rows; y++)
        for (int x = 0; x < dest.cols; x++)
        {
            int x0 = zoom_x0 + x / bon_zoom;
            int y0 = zoom_y0 + y / bon_zoom;

            if (x0 < 0 || x0 >= src.cols || y0 < 0 || y0 >= src.rows) {
                dest.at<cv::Vec3b>(y,x)[0] = 64;
                dest.at<cv::Vec3b>(y,x)[1] = 64;
                dest.at<cv::Vec3b>(y,x)[2] = 64;
                continue;
            }
            dest.at<cv::Vec3b>(y,x)[0] = src.at<cv::Vec3b>(y0,x0)[0];
            dest.at<cv::Vec3b>(y,x)[1] = src.at<cv::Vec3b>(y0,x0)[1];
            dest.at<cv::Vec3b>(y,x)[2] = src.at<cv::Vec3b>(y0,x0)[2];
        }
    }
};


//----------------------- C O U L E U R S   V G A -----------------------------

void representer_en_couleurs_vga (cv::Mat img_niv, cv::Mat img_coul)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)
    CHECK_MAT_TYPE(img_coul, CV_8UC3)

    unsigned char couls[16][3] = {  // R, G, B
        {   0,   0,   0 },   //  0  black           ->  0 uniquement
        {  20,  20, 190 },   //  1  blue            ->  1, 15, 29, ...
        {  30, 200,  30 },   //  2  green           ->  2, 16, 30, ...
        {  30, 200, 200 },   //  3  cyan            ->  3, 17, 31, ...
        { 200,  30,  30 },   //  4  red             ->  4, 18, 32, ...
        { 200,  30, 200 },   //  5  magenta         ->  5, 19, 33, ...
        { 200, 130,  50 },   //  6  brown           ->  6, 20, 34, ...
        { 200, 200, 200 },   //  7  light gray      ->  7, 21, 35, ...
        { 110, 110, 140 },   //  8  dark gray       ->  8, 22, 36, ...
        {  84, 130, 252 },   //  9  light blue      ->  9, 23, 37, ...
        {  84, 252,  84 },   // 10  light green     -> 10, 24, 38, ...
        {  84, 252, 252 },   // 11  light cyan      -> 11, 25, 39, ...
        { 252,  84,  84 },   // 12  light red       -> 12, 26, 40, ...
        { 252,  84, 252 },   // 13  light magenta   -> 13, 27, 41, ...
        { 252, 252,  84 },   // 14  yellow          -> 14, 28, 42, ...
        { 252, 252, 252 },   // 15  white           -> 255 uniquement
    };

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x), c = 0;
        if (g == 255) c = 15;                      // seul 255 est blanc
        else if (g != 0) c = 1 + abs(g-1) % 14;    // seul 0 est noir
        // Attention img_coul est en B, G, R -> inverser les canaux
        img_coul.at<cv::Vec3b>(y,x)[0] = couls[c][2];
        img_coul.at<cv::Vec3b>(y,x)[1] = couls[c][1];
        img_coul.at<cv::Vec3b>(y,x)[2] = couls[c][0];
    }
}


//----------------------------------- M Y -------------------------------------

class My {
  public:
    cv::Mat img_src, img_res1, img_res2, img_niv, img_coul;
    Loupe loupe;
    int seuil = 127;
    int seuil_pol = 600;
    int clic_x = 0;
    int clic_y = 0;
    int clic_n = 0;

    enum Recalc { R_RIEN, R_LOUPE, R_TRANSFOS, R_SEUIL };
    Recalc recalc = R_SEUIL;

    void reset_recalc ()             { recalc = R_RIEN; }
    void set_recalc   (Recalc level) { if (level > recalc) recalc = level; }
    int  need_recalc  (Recalc level) { return level <= recalc; }

    // Rajoutez ici des codes A_TRANSx pour le calcul et l'affichage
    enum Affi { A_ORIG, A_SEUIL, A_TRANS1, A_TRANS2, A_TRANS3, A_TRANS4, A_TRANS5, A_TRANS6, A_TRANS7,A_TRANS8,A_TRANS9 };
    Affi affi = A_ORIG;
};


//----------------------- T R A N S F O R M A T I O N S -----------------------

void inverser_couleurs (cv::Mat img)
{
    CHECK_MAT_TYPE(img, CV_8UC3)

    for (int y = 0; y < img.rows; y++)
    for (int x = 0; x < img.cols; x++)
    {
        img.at<cv::Vec3b>(y,x)[0] = 255 - img.at<cv::Vec3b>(y,x)[0];
        img.at<cv::Vec3b>(y,x)[1] = 255 - img.at<cv::Vec3b>(y,x)[1];
        img.at<cv::Vec3b>(y,x)[2] = 255 - img.at<cv::Vec3b>(y,x)[2];
    }
}


// Placez ici vos fonctions de transformations à la place de ces exemples

void marquer_contours_c4(cv::Mat img_niv)
{
	//PLOP
	//vector<int> marquage;
	int check[img_niv.rows][img_niv.cols];

    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
		//marquage.append(-1);
		check[y][x] = -1;
        int g = img_niv.at<int>(y,x);
        if (g > 0)
        {
            img_niv.at<int>(y,x) = y;
        }
    }
    int valeur_affectation = 1;
    for (int y = 0; y < img_niv.rows-1; y++)
    for (int x = 0; x < img_niv.cols-1; x++)
    {

		if(img_niv.at<int>(y,x) > 0)
		{

			if(x==0
			|| y ==0
			|| img_niv.at<int>(y-1,x) == 0
			|| img_niv.at<int>(y,x-1) == 0
			|| img_niv.at<int>(y,x+1) == 0
			|| img_niv.at<int>(y+1,x) == 0
			|| img_niv.at<int>(y+1,x+1) == 0
			|| img_niv.at<int>(y-1,x-1) == 0
			|| img_niv.at<int>(y-1,x+1) == 0
			|| img_niv.at<int>(y+1,x-1) == 0)
			{
				check[y][x] = valeur_affectation;
			}
			else
			{
				valeur_affectation++;
				check[y][x] = valeur_affectation;
			}
			//valeur_affectation++;
				//check[y][x] = valeur_affectation;
		}

    }
    int check2 [img_niv.rows][img_niv.cols];
    for (int y = 1; y < img_niv.rows-1; y++)
    for (int x = 1; x < img_niv.cols-1; x++)
		check2[y][x] = check[y][x];
    for (int y = 1; y < img_niv.rows-1; y++)
    for (int x = 1; x < img_niv.cols-1; x++)
    {
		if(check2[y][x] > 0)
		{

			if(check2[y-1][x] < 0 ||
			check2[y][x-1] < 0 ||
			check2[y][x+1] < 0 ||
			check2[y+1][x] < 0)
			{
				check[y][x] = 1;
			}
			else
				check[y][x] = -1;
		}
	}
	bool color_change = false;
	for (int y = 0; y < img_niv.rows; y++)
	{
    for (int x = 0; x < img_niv.cols; x++)
    {
		if(check[y][x] > 0)
		{
			img_niv.at<int>(y,x) = 255;
			if(color_change && check[y][x+1] < 0)
				color_change = false;
			else if(check[y][x+1] < 0 && !color_change)
				color_change = true;
		}
		else
		{
			//img_niv.at<int>(y,x) = 1;
		}

		if(check[y][x] < 0 && !color_change)
		{
			//img_niv.at<int>(y,x) = 0;
		}
		if(check[y][x] < 0 && color_change)
		{
			//img_niv.at<int>(y,x) = 1;
		}
	}
		color_change = false;
	}
	for (int y = 0; y < img_niv.rows; y++)
	{
    for (int x = 0; x < img_niv.cols; x++)
    {
		if(img_niv.at<int>(y,x) !=255 && img_niv.at<int>(y,x) > 0)
		{
			img_niv.at<int>(y,x) = 1;
		}
	}
	}
}

void marquer_contours_c8(cv::Mat img_niv)
{
	//PLOP
	//vector<int> marquage;
	int check[img_niv.rows][img_niv.cols];

    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
		//marquage.append(-1);
		check[y][x] = -1;
        int g = img_niv.at<int>(y,x);
        if (g > 0)
        {
            img_niv.at<int>(y,x) = y;
        }
    }
    int valeur_affectation = 1;
    for (int y = 0; y < img_niv.rows-1; y++)
    for (int x = 0; x < img_niv.cols-1; x++)
    {

		if(img_niv.at<int>(y,x) > 0)
		{

			if(x==0
			|| y ==0
			|| img_niv.at<int>(y-1,x) == 0
			|| img_niv.at<int>(y,x-1) == 0
			|| img_niv.at<int>(y,x+1) == 0
			|| img_niv.at<int>(y+1,x) == 0)
			{
				check[y][x] = valeur_affectation;
			}
			else
			{
				valeur_affectation++;
				check[y][x] = valeur_affectation;
			}

		}

    }
    int check2 [img_niv.rows][img_niv.cols];
    for (int y = 1; y < img_niv.rows-1; y++)
    for (int x = 1; x < img_niv.cols-1; x++)
		check2[y][x] = check[y][x];
    for (int y = 1; y < img_niv.rows-1; y++)
    for (int x = 1; x < img_niv.cols-1; x++)
    {
		if(check2[y][x] > 0)
		{


			if(check2[y-1][x] < 0 ||
			check2[y][x-1] < 0 ||
			check2[y][x+1] < 0 ||
			check2[y+1][x] < 0 ||
			check2[y-1][x-1] < 0 ||
			check2[y+1][x+1] < 0 ||
			check2[y+1][x+1] < 0 ||
			check2[y+1][x-1] < 0 ||
			check2[y-1][x+1] < 0)
			{
				check[y][x] = 1;
			}
			else
				check[y][x] = -1;
		}
	}
	bool color_change = false;
	for (int y = 0; y < img_niv.rows; y++)
	{
    for (int x = 0; x < img_niv.cols; x++)
    {
		if(check[y][x] > 0)
		{
			img_niv.at<int>(y,x) = 255;
			if(color_change)
				color_change = false;
			else
				color_change = true;
		}

		if(check[y][x] < 0 && !color_change)
		{
			//img_niv.at<int>(y,x) = 0;
		}
		if(check[y][x] < 0 && color_change)
		{
			//img_niv.at<int>(y,x) = 1;
		}
	}
		color_change = false;
	}
	for (int y = 0; y < img_niv.rows; y++)
	{
    for (int x = 0; x < img_niv.cols; x++)
    {
		if(img_niv.at<int>(y,x) !=255 && img_niv.at<int>(y,x) > 0)
		{
			img_niv.at<int>(y,x) = 1;
		}
	}
	}
}

void numeroter_contours_c8(cv::Mat img_niv)
{
	//PLOP
	//vector<int> marquage;
	int check[img_niv.rows][img_niv.cols];

    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
		//marquage.append(-1);
		check[y][x] = -1;
        int g = img_niv.at<int>(y,x);
        if (g > 0)
        {
            img_niv.at<int>(y,x) = y;
        }
    }
    int size = img_niv.rows * img_niv.cols;
	int list_eq[size];
    int valeur_affectation = 1;

    for(int i=0;i<size;i++)
		list_eq[i]=-1;

    for (int y = 0; y < img_niv.rows-1; y++)
    for (int x = 0; x < img_niv.cols-1; x++)
    {


		if(img_niv.at<int>(y,x) > 0)
		{

			if(x==0
			|| y ==0
			|| img_niv.at<int>(y-1,x) == 0
			|| img_niv.at<int>(y,x-1) == 0
			|| img_niv.at<int>(y,x+1) == 0
			|| img_niv.at<int>(y+1,x) == 0)
			{

				//int k = check[y][x];
				if(check[y][x-1] > 0)
				{
					check[y][x] = valeur_affectation;
				}
				else
				{
					valeur_affectation ++;
					check[y][x] = valeur_affectation;
				}
				if(check[y-1][x] < check[y][x] && check[y-1][x] > 0)
				{
					int j = check[y-1][x];
					list_eq[j] = check[y][x];
					//list_eq[k] = j;

				}
				if(check[y][x-1] < check[y][x] && check[y][x-1] > 0)
				{
					int j = check[y][x-1];
					list_eq[j] = check[y][x];
					//list_eq[k] = j;
				}
				if(check[y][x+1] < check[y][x] && check[y][x+1] > 0)
				{
					int j = check[y][x+1];
					list_eq[j] = check[y][x];
					//list_eq[k] = j;
				}
				if(check[y+1][x] < check[y][x] && check[y+1][x] > 0)
				{
					int j = check[y+1][x];
					list_eq[j] = check[y][x];
					//list_eq[k] = j;
				}
				if(check[y+1][x+1] < check[y][x] && check[y+1][x+1] > 0)
				{
					int j = check[y+1][x+1];
					list_eq[j] = check[y][x];
					//list_eq[k] = j;
				}
				if(check[y+1][x-1] < check[y][x] && check[y+1][x-1] > 0)
				{
					int j = check[y+1][x-1];
					list_eq[j] = check[y][x];
					//list_eq[k] = j;
				}
				//oooxxxoooo
				//xxxxoooooo
				//ooooxxxxxx
				//
				if(check[y-1][x+1] < check[y][x] && check[y-1][x+1] > 0)
				{
					int j = check[y-1][x+1];
					list_eq[j] = check[y][x];
					//list_eq[k] = j;
				}
				if(check[y-1][x-1] < check[y][x] && check[y-1][x-1] > 0)
				{
					int j = check[y-1][x-1];
					list_eq[j] = check[y][x];
					//list_eq[k] = j;
				}
			}
			else
			{
				//valeur_affectation++;
				//check[y][x] = valeur_affectation;
			}
			//valeur_affectation++;
				//check[y][x] = valeur_affectation;

		}

    }
    //int check2 [img_niv.rows][img_niv.cols];
    int i=1;
    int nb_elem = 1;

    int list_eq2[size];


    for(int i=0;i<size;i++)
		list_eq2[i]=-1;

    for (int y = 1; y < img_niv.rows-1; y++)
    for (int x = 1; x < img_niv.cols-1; x++)
	{
		if(check[y][x]>0)
		{

			i = check[y][x];
			while(list_eq[i] != -1 && i< size)
			{
				if(list_eq2[i] < 0)
				{
					list_eq2[i] = nb_elem;
					//list_eq[i] = -1;
					i = list_eq[i];
				}
				else
				{
					break;
				}
			}

			nb_elem++;
		}

	}

	bool color_change = false;
	//int temp = 1;
	for (int y = 0; y < img_niv.rows; y++)
	{
    for (int x = 0; x < img_niv.cols; x++)
    {
		if(check[y][x] > 0)
		{
			int j = check[y][x];
			img_niv.at<int>(y,x) = list_eq2[j] %255;
			//std::cout<<" "<<list_eq2[j];
			std::cout<<" j : "<<j<<" "<<list_eq[j];

		}


		if(check[y][x] < 0 && !color_change)
		{
			img_niv.at<int>(y,x) = 0;
		}
		if(check[y][x] < 0 && color_change)
		{
			//img_niv.at<int>(y,x) = 1;
		}
	}
		color_change = false;
	}
}

void transformer_bandes_verticales (cv::Mat img_niv)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        if (g > 0) {
            img_niv.at<int>(y,x) = x;
        }
    }
}


void transformer_bandes_diagonales (cv::Mat img_niv)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        if (g > 0) {
            img_niv.at<int>(y,x) = x+y;
        }
    }
}

int dir_x[] = {1,1,0,-1,-1,-1,0,1};
int dir_y[] = {0,1,1,1,0,-1,-1,-1};

struct ContourF8
{
  int xPointDepart;
  int yPointDepart;
  //TP4
  int dir_init;
  //----
  std::vector<int> chaineFreeman;
  int taillchaineFreeman;
};
struct point_img
{
	int x,y;
};
struct ContourPol
{
	point_img p;
	bool estSommetApproxPoly = false;
};

//std::vector<int *> freeman_chains;
//------TP3------
double seuil_recalc = 0.6;
//double seuil_pol = 4.0;
std::vector<ContourPol> suivit_chaine_freeman(ContourF8 cfc,cv::Mat img)
{
	std::vector<ContourPol> vect_contour_pol;
	point_img pts;
	pts.x=cfc.xPointDepart;
	pts.y=cfc.yPointDepart;
	ContourPol cp;
	cp.p = pts;
	//pts.x = 0;
	//pts.y = 0;
	cp.estSommetApproxPoly = true;
	//vect_contour_pol.push_back(cp);
	for(unsigned int i = 0; i< cfc.chaineFreeman.size();i++)
	{
		//if(pts.x>0 && pts.y>0 && pts.x<img.rows && pts.y<img.cols)
		//{
			//pts.x=pts.x +dir_x[cfc.chaineFreeman[i]];
			//pts.y=pts.y +dir_y[cfc.chaineFreeman[i]];
      pts.x=pts.x +dir_x[cfc.chaineFreeman[i]];
			pts.y=pts.y +dir_y[cfc.chaineFreeman[i]];
		//}


		cp.p = pts;
    std::cout<<"____CP____ "<<std::endl;
    std::cout<<"x : "<<cp.p.x<<" y : "<<cp.p.y<<std::endl;
    std::cout<<"___CPEND"<<std::endl;
		vect_contour_pol.push_back(cp);

	}
	return vect_contour_pol;
}
double PerpendicularDistance(const point_img pt, const point_img lineStart, const point_img lineEnd)
{
	double dx = lineEnd.x - lineStart.x;
	double dy = lineEnd.y - lineStart.y;

	//Normalise
	double mag = pow(pow(dx,2.0)+pow(dy,2.0),0.5);
	if(mag > 0.0)
	{
		dx /= mag; dy /= mag;
	}

	double pvx = pt.x - lineStart.x;
	double pvy = pt.y - lineStart.y;

	//Get dot product (project pv onto normalized direction)
	double pvdot = dx * pvx + dy * pvy;

	//Scale line direction vector
	double dsx = pvdot * dx;
	double dsy = pvdot * dy;

	//Subtract this from pv
	double ax = pvx - dsx;
	double ay = pvy - dsy;

	return pow(pow(ax,2.0)+pow(ay,2.0),0.5);
	//double a = pt.x - lineStart.x;    double b = pt.y - pt.x;
    //return sqrt (a*a + b*b);
}
struct Distance
{
  float distance = 0;
  int idx = 0;
};
Distance distance(unsigned int idx1, unsigned int idx2, std::vector<ContourPol>  * vect_cp)
{
  Distance dis;
  int x1 = vect_cp->at(idx1).p.x;
  int y1 = vect_cp->at(idx1).p.y;

  int x2 =  vect_cp->at(idx2).p.x;
  int y2 =  vect_cp->at(idx2).p.y;
  dis.distance = 0;

  int xc;
  int yc;
  for(unsigned int i=idx1 ; i<idx2 ; i++)
  {
    xc = vect_cp->at(i).p.x;
    yc = vect_cp->at(i).p.y;
    float dist = abs(((x2-x1)*(yc-y1) - (y2-y1)*(xc-x1)))
    / sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
    if (dist > dis.distance){
      dis.distance = dist;
      dis.idx = i;
    }
  }
  //abs(((x2-x1)*(yc-y1)-(y2-y1)*(xc-x1)))/sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
  return dis;
}
double distance_max_poly(std::vector<ContourPol> vect_cp,ContourPol p1,int idx_cp1, ContourPol p2,int idx_cp2)
{
	double dmax = 0;
	int index = 0;
	for(int i = idx_cp1; i <= idx_cp2; i++)
	{
		double d = PerpendicularDistance(vect_cp.at(i).p,
		vect_cp.at(idx_cp1).p, vect_cp.at(idx_cp2).p);
		if (d > dmax)
		{
			index = i;
			dmax = d;
		}
	}
	return dmax;
}
/*void homcon(std::vector<ContourPol> * vect_cp, ContourPol cp1, int idx_cp1,
 ContourPol cp2, int idx_cp2, double distance, double seuil)
{
	if(abs(vect_cp->at(idx_cp1).p.x-vect_cp->at(idx_cp2).p.x<2)&& abs(vect_cp->at(idx_cp1).p.y-vect_cp->at(idx_cp2).p.y<2))
	{
		return vect_cp;
	}
	double dmax = 0;
	int index = 0;
	for(int i = idx_cp1; i < idx_cp2; i++)
	{
		double d = Perpendicularance(vect_cp->at(i).p, vect_cp->at(idx_cp1).p, vect_cp->at(idx_cp2).p);
		if (d > dmax)
		{
			index = i;
			dmax = d;
		}
	}
	if(dmax<seuil)
	{
		for(int i = idx_cp1; i < idx_cp2; i++)
		{
			vect_cp->at(i).estSommetApproxPoly = false;
			return vect_cp;
		}
	}
	else
	{
		homcon(vect_cp, cp1, idx_cp1,vect_cp->at(index), index, dmax, seuil);
		homcon(vect_cp, vect_cp->at(index), index,cp2, idx_cp2, dmax, seuil);
	}
}*/

void homcon2(std::vector<ContourPol> * vect_cp, int idx_cp1, int idx_cp2, double seuil)
 {
   if(abs(vect_cp->at(idx_cp1).p.x - vect_cp->at(idx_cp2).p.x) < 2
    && abs(vect_cp->at(idx_cp1).p.y - vect_cp->at(idx_cp2).p.y) < 2) return;
   std::cout<<"call homcon2 "<<std::endl;
   Distance dis = distance(idx_cp1,idx_cp2, vect_cp);
   std::cout<<"HAMCON DIST : "<<dis.distance<<std::endl;
   if(dis.distance<seuil)
   {
     for(int i = idx_cp1+1; i<=idx_cp2;i++)
     {
       vect_cp->at(i).estSommetApproxPoly = false;
     }
     return;
   }
   homcon2(vect_cp,idx_cp1,dis.idx,seuil);
   homcon2(vect_cp,dis.idx,idx_cp2,seuil);
 }

void bresmhan(cv::Mat img, int x0, int y0, int x1, int y1)
{
	int dx = x1-x0;
	int dy = y1-y0;
	int k  = 2*dy-dx;
	int y=y0;
	for(int x = x0; x < (x1+1); x++)
	{
		std::cout<<"bresmhan x : "<<x<<"bresmhan y : "<<y<<"bresmhan k : "<< k << std::endl;
		if(k>0)
		{
			y += 1;
			k -= 2*dx;
		}
		k += 2*dy;
		if(x>=0 && y>=0 && x<=img.rows && y<=img.cols)
			img.at<int>(y, x) = 2;

      std::cout<<"x img : "<<x<<" y img : "<<y << std::endl;
	}
}

void colorier_morceaux(std::vector<ContourPol> v,cv::Mat img)
{
  int cpt = 1;
  for(unsigned int i = 0; i<v.size();i++)
  {
      int x = v.at(i).p.x;
      int y = v.at(i).p.y;
      if(x>=0 && y>=0 && y<img.rows && x<img.cols)
        img.at<int>(y,x) = cpt;
      else
        std::cout<<"x img : "<<x<<" y img : "<<y << std::endl;
      if(v.at(i).estSommetApproxPoly)
      {
        std::cout<<"COLOR CHANGE"<<std::endl;
        cpt++;
        if(cpt==255)
          cpt++;
      }

      std::cout<<"CPT COLOR : "<<cpt<<std::endl;
  }
}

std::vector<ContourPol> approximer_contour_c8(ContourF8 cfc, cv::Mat img)
{
  std::vector<ContourPol> vect_contour_pol;
	point_img pts;
	pts.x=cfc.xPointDepart;
	pts.y=cfc.yPointDepart;
	ContourPol cp;
	cp.p = pts;


  std::cout<<"pts de départ : "<< cfc.xPointDepart<<" x "<<cfc.yPointDepart<<" y "<<std::endl;
	for(unsigned int i = 0; i< cfc.chaineFreeman.size();i++)
	{
      cp.estSommetApproxPoly = true;
      pts.x=pts.x +dir_x[cfc.chaineFreeman[i]];
			pts.y=pts.y +dir_y[cfc.chaineFreeman[i]];

      cp.p = pts;
      std::cout<<"____CP____ "<<std::endl;
      std::cout<<"x : "<<cp.p.x<<" y : "<<cp.p.y<<std::endl;
      std::cout<<"___CPEND"<<std::endl;
		  vect_contour_pol.push_back(cp);
	}
	//std::vector<ContourPol> vect_contour_pol = suivit_chaine_freeman(cfc,img);

	//int max_iter = 0;
	//if(max_iter>1)return vect_contour_pol;
	double dmax = 0;
	int index = 0;
	double seuil = seuil_recalc;
  //double seuil = 0.6;
	for(unsigned int i = 1; i < vect_contour_pol.size(); i++)
	{
		double d = PerpendicularDistance(vect_contour_pol.at(i).p, vect_contour_pol.at(0).p, vect_contour_pol.at(vect_contour_pol.size()-1).p);
		if (d > dmax)
		{
			index = i;
			dmax = d;
		}
	}
  Distance dis = distance(0,vect_contour_pol.size()-1,&vect_contour_pol);
	std::cout<<"distance max : "<< dmax<< " at idx : "<< index <<std::endl;
	//homcon(&vect_contour_pol, vect_contour_pol.at(0),0,vect_contour_pol.at(index),index,dmax,seuil);
	//homcon(&vect_contour_pol, vect_contour_pol.at(index),index,vect_contour_pol.at(vect_contour_pol.size()-1),vect_contour_pol.size()-1,dmax,seuil);
  std::cout<<" hamcon : "<<std::endl;
  homcon2(&vect_contour_pol,0,dis.idx,seuil);
  homcon2(&vect_contour_pol,dis.idx,vect_contour_pol.size()-1,seuil);

  std::vector<unsigned int> tab_indice;
  for (unsigned i = 0 ; i < vect_contour_pol.size() ; i++){
    if(vect_contour_pol.at(i).estSommetApproxPoly) tab_indice.push_back(i);
  }
  int tab_size = tab_indice.size();
  for(int i = 0 ; i < tab_size -2 ; i++){

    int x_D = vect_contour_pol.at(tab_indice[i]).p.x;
    int y_D = vect_contour_pol.at(tab_indice[i]).p.y;
    int x_E = vect_contour_pol.at(tab_indice[i+1]).p.x;
    int y_E = vect_contour_pol.at(tab_indice[i+1]).p.y;
    int x_F = vect_contour_pol.at(tab_indice[i+2]).p.x;
    int y_F = vect_contour_pol.at(tab_indice[i+2]).p.y;

    float distance = abs(((x_F-x_D)*(y_E-y_D) - (y_F-y_D)*(x_E-x_D))) /
                      sqrt((x_F - x_D)*(x_F - x_D) + (y_F - y_D)*(y_F - y_D));
    if(distance < seuil){
      vect_contour_pol.at(tab_indice[i+1]).estSommetApproxPoly = false;
      std::cout<<vect_contour_pol.at(tab_indice[i]).estSommetApproxPoly<<std::endl;
      tab_indice.erase(tab_indice.begin()+i+1);
    }
  }
  //colorier_morceaux(vect_contour_pol,img)
  return vect_contour_pol;

}
//-----_TP3_-----

//------TP4------

int glob_connex = 4;
void remplir_polyg(cv::Mat img,std::vector<ContourPol> vec_pol,int color)
{
  std::vector<cv::Point> vec_pts;
  for(unsigned int i=0;i<vec_pol.size();i++)
  {
    if(vec_pol.at(i).estSommetApproxPoly)
    {
      cv::Point point(vec_pol.at(i).p.x,vec_pol.at(i).p.y);
      vec_pts.push_back(point);
    }
  }
  const cv::Point* elementPoints[1] = { &vec_pts[0] };
  int nb_point = (int)vec_pts.size();
  cv::fillPoly(img, elementPoints, &nb_point, 1, cv::Scalar(color));
}

void approximer_et_remplir_contour_c8(cv::Mat img,std::vector<ContourF8> vec,double seuil)
{
  //nettoyage de l'image
  img.setTo(0);
  for(unsigned int i = 0; i< vec.size();i++)
  {
    std::vector<ContourPol> vec_pol = approximer_contour_c8(vec.at(i), img);
    int color = 255;
    if(vec.at(i).dir_init == 2 || vec.at(i).dir_init == 0)
    {
      color = 0;
    }
    remplir_polyg(img,vec_pol,color);
  }
}

void effectuer_pelage_DT(cv::Mat img, int connexite)
{
  for(int y = 0; y< img.rows; y++)
	{
		for (int x = 0; x < img.cols; x++)
		{
      if(img.at<int>(y,x)>0)
      {
        img.at<int>(y,x) = INT_MAX;
      }
    }
  }

  int cpt = 0;
  bool out = false;
  while(!out)
  {
    out = true;
    for(int y = 0; y< img.rows; y++)
  	{
  		for (int x = 0; x < img.cols; x++)
  		{

        if(img.at<int>(y,x) > cpt)
        {
          if(x == 0 || y == 0
          || x == img.cols-1 || y == img.rows-1
          || img.at<int>(y,x-1) == cpt || img.at<int>(y-1,x)== cpt
          || img.at<int>(y+1,x) == cpt || img.at<int>(y,x+1) == cpt
          || (connexite == 4 &&
            (img.at<int>(y-1,x-1) || img.at<int>(y+1,x+1) == cpt
            || img.at<int>(y-1,x+1) == cpt || img.at<int>(y+1,x-1) == cpt))
          )
          {
            img.at<int>(y,x) = cpt +1;
            out = false;
          }
        }
      }
    }
    cpt++;
  }
}

void detecter_maximum_locaux(cv::Mat img,int connexite)
{
  cv::Mat img2 = img.clone();
  //img2.clone(img);
  bool est_max_loc = true;
  for(int y = 0; y< img2.rows; y++)
  {
    for (int x = 0; x < img2.cols; x++)
    {
      est_max_loc = true;
      int pix = img.at<int>(y,x);
      //cherche si max local

      unsigned int nv = 8;
      if(connexite == 8)
      {
        nv = 4;
      }
      for(unsigned i = 0 ; i < nv ; i++)
      {
        int x_temp = x + dir_x[i];
        int y_temp = y + dir_y[i];
        if( x_temp > img.cols-1 || y_temp > img.rows-1||x_temp < 0 || y_temp < 0 )
        {
          continue;
        }
        if(img.at<int>(y_temp,x_temp) > pix)
        {
          est_max_loc = false;
          break;
        }
      }
    }
  }
}

void effectuer_pelage_RDT(cv::Mat img, int connexite)
{
  int max = 0;
  for (int y = 0; y < img.rows; y++)
  for (int x = 0; x < img.cols; x++)
  {
    if (img.at<int>(y,x) > max)
    {
      max = img.at<int>(y,x);
    }
  }

  for (int m = max-1; m > 0; m--)
  {
    for (int y = 0; y < img.rows; y++)// +1 -1
    for (int x = 0; x < img.cols; x++)// +1 -1
    {
      if (img.at<int>(y,x) < m){
        unsigned int nv = 8;
        if(connexite == 8)
        {
          nv = 4;
        }
        for(unsigned i = 0 ; i < nv ; i++)
        {
          int x_temp = x + dir_x[i];
          int y_temp = y + dir_y[i];
          if( x_temp > img.cols-1 || y_temp > img.rows-1 ||x_temp < 0 || y_temp < 0)
          {
            continue;
          }
          if(img.at<int>(y_temp,x_temp) > m)
          {
            img.at<int>(y,x) = m;
            break;
          }
        }
      }
    }
  }
}


//-----_TP4_-----
int next(cv::Mat img,int x,int y,int d)
{
	int next = img.at<int>(y+dir_y[d],x+dir_x[d]);
	return next;
}
//variable stockant chaque chaine de freeman;
//ContourF8 cdf;

void marquer_un_contour_c8(cv::Mat img,int xa,int ya,int dira,int num_contour,ContourF8 * cdf)
{
	std::cout << "suivre contour " << xa << " , " << ya <<std::endl;
	int dir_finale ;
	for(int i = 0; i<8; i++)
	{
		int d= (dira+i)%8;
		int xb = xa + dir_x[d];
		int yb = ya + dir_y[d];
		int q = img.at<int>(yb,xb);
		if(q > 0 && (yb)>=0 && (xb)>=0
			&& (yb)<=img.rows-1 && (xb)<=img.cols-1)
		{
			dir_finale = (d+4)%8;
			break;
		}
	}

	int x = xa;
	int y = ya;
	int dir = dir_finale;
	int cpt = 0;
	int q;
	do
	{
		cpt++;
		//if (cpt > 300)break;
		std::cout << "do"<<std::endl;
		img.at<int>(y,x) =  num_contour;
		dir = (dir + 4 -1)%8;
		int i = 0;

		for(;i<8;i++)
		{
			int d = (dir+8-i)%8;


			int xb = x + dir_x[d];
			int yb = y + dir_y[d];
			q = img.at<int>(yb,xb);

			if (xb < 0 || yb < 0 || xb >= img.cols || yb >= img.rows) continue;
				q = img.at<int>(yb, xb);
			if(q>0 && (xb)>=0 && (yb)>=0
			&& (yb)<=img.rows-1 && (xb)<=img.cols-1)
			{
				x = xb;
				y = yb;
				dir = d;

				cdf->chaineFreeman.push_back(dir);
				//std::cout<<"if dir"<<dir<<std::endl;;
				break;
			}
			//std::cout << "   after if"<<std::endl;
		}
		if(i == 8)//8?
		{
			return;
		}
	}
	while(!(x==xa && y==ya && dir == dir_finale));
	std::cout<<"dir : "<<dir<<std::endl;

}

void suivre_un_contour_c8(cv::Mat img,int xa,int ya,int dira,int num_contour,ContourF8 * cdf)
{

	marquer_un_contour_c8(img, xa,ya,dira,num_contour,cdf);
	//return img;
}
std::vector<ContourF8> effectuer_suivi_contours_c8(cv::Mat img_niv)
{
	//------TP3------
	//
	int max_iter = 0;

	std::vector<ContourF8 > contours;
	bool estCoordDepart = true;
	//int tailleChaineCourante = 0;
	ContourF8 contourCourrant;
	//------_TP3_------
	int new_contour = 1;
	int dir;
	for(int y = 0; y< img_niv.rows; y++)
	{
		for (int x = 0; x < img_niv.cols; x++)
		{

			ContourF8 cdf ;
			//std::cout<< "pixel : " << x << ", "<< y << std::endl;
			if(img_niv.at<int>(y,x) == 255)
			{
				//------TP3------
				if(estCoordDepart)
				{
				  contourCourrant.xPointDepart = x;
				  contourCourrant.yPointDepart = y;
				  cdf.xPointDepart = x;
				  cdf.yPointDepart = y;
          cdf.dir_init = 0; // WARNING
				  estCoordDepart = false;
				}
				//-----_TP3_-----
				dir = -1;
				if(x==img_niv.cols-1 || img_niv.at<int>(y,x+1) == 0)
				{
					dir = 0;
				}
				else if(y==img_niv.rows-1 || img_niv.at<int>(y+1,x) == 0)
				{
					dir = 2;
				}
				else if(x==0 || img_niv.at<int>(y,x-1) == 0)
				{
					dir = 4;
				}
				else if(y==0 || img_niv.at<int>(y-1,x) == 0)
				{
					dir = 6;
				}
				//------TP3------
				//contourCourrant.chaineFreeman.push_back(dir);
				//tailleChaineCourante ++;
				//------_TP3_------
				if(dir>=0)
				{
          cdf.xPointDepart = x;
				  cdf.yPointDepart = y;
          cdf.dir_init = dir;
					suivre_un_contour_c8(img_niv,x,y,dir,new_contour,&cdf);
					new_contour++;
					if(new_contour == 255)
					{
						new_contour++;
					}
					//------TP3------
					estCoordDepart = true;
					//contourCourrant.taillchaineFreeman = tailleChaineCourante;
					cdf.taillchaineFreeman = cdf.chaineFreeman.size();
					//contours.push_back(contourCourrant);
					contours.push_back(cdf);
					std::cout <<"contour : "<<new_contour<< " premier point : "<<cdf.xPointDepart<<" "
					<<cdf.yPointDepart<<" taille chaine freeman"<<cdf.taillchaineFreeman<<"\n";
					std::cout <<" chaine de Freeman :\n";

					for(unsigned int i=0; i<cdf.chaineFreeman.size(); i++)
					{
						std::cout <<" "<< cdf.chaineFreeman.at(i);
					}

					std::cout <<"\n";
					//tailleChaineCourante = 0;
					//if(max_iter<3)approximer_contour_c8(cdf, img_niv);
					max_iter++;
					//cdf.chaineFreeman.clear();
					ContourF8 contourCourrant;
					//-----_TP3_-----
				}
			}
		}
	}
	return contours;
}

void dessiner_contours_poly(cv::Mat img)
{
  std::vector<ContourF8> contours = effectuer_suivi_contours_c8(img);
	for(unsigned int i = 0;i<contours.size();i++)
	{
		std::cout<<"step : "<< i <<std::endl;
		std::vector<ContourPol> vect_contour_pol = approximer_contour_c8(contours.at(i), img);
    colorier_morceaux(vect_contour_pol,img);
		std::cout<<"pomdeter "<<std::endl;
	}
}

void dessiner_approx_poly(cv::Mat img)
{
	std::vector<ContourF8> contours = effectuer_suivi_contours_c8(img);
  //int cpt = 1;
	for(unsigned int i = 0;i<contours.size();i++)
	{
		std::cout<<"step : "<< i <<std::endl;
		std::vector<ContourPol> vect_contour_pol = approximer_contour_c8(contours.at(i), img);
    colorier_morceaux(vect_contour_pol,img);

    //remplir_polyg(img,vect_contour_pol,cpt);
    //cpt++;
    //if(cpt == 255)cpt++;
		std::cout<<"pomdeter "<<std::endl;
	}
  approximer_et_remplir_contour_c8(img,contours,seuil_recalc);

}

void pelage(cv::Mat img)
{
  std::vector<ContourF8> contours = effectuer_suivi_contours_c8(img);
  approximer_et_remplir_contour_c8(img,contours,seuil_recalc);
  effectuer_pelage_DT(img, glob_connex);

}
// Appelez ici vos transformations selon affi
void effectuer_transformations (My::Affi affi, cv::Mat img_niv,int s_pol)
{
    switch (affi) {
        case My::A_TRANS1 :
            marquer_contours_c8(img_niv);
            break;
        case My::A_TRANS2 :
            marquer_contours_c4(img_niv);
            break;
        case My::A_TRANS3 :
            numeroter_contours_c8(img_niv);
            break;
        case My::A_TRANS4 :
			effectuer_suivi_contours_c8(img_niv);
			break;
		case My::A_TRANS5 :
    {
      if(s_pol == 0) s_pol++;
      seuil_recalc = s_pol / 1000.0f;
      dessiner_contours_poly(img_niv);

			break;
    }
    case My::A_TRANS6 :
    {
      if(s_pol == 0) s_pol++;
      seuil_recalc = s_pol / 1000.0f;
      dessiner_approx_poly(img_niv);

			break;
    }
    case My::A_TRANS7 :
    {
      if(s_pol == 0) s_pol++;
      seuil_recalc = s_pol / 1000.0f;
      pelage(img_niv);

			break;
    }
    case My::A_TRANS8 :
    {
      if(s_pol == 0) s_pol++;
      seuil_recalc = s_pol / 1000.0f;
      //effectuer_suivi_contours_c8(img_niv);
      pelage(img_niv);
      detecter_maximum_locaux(img_niv,glob_connex);
      break;
    }
    case My::A_TRANS9 :
    {
      if(s_pol == 0) s_pol++;
      seuil_recalc = s_pol / 1000.0f;
      pelage(img_niv);
      detecter_maximum_locaux(img_niv,glob_connex);
      effectuer_pelage_RDT(img_niv,glob_connex);
			break;
    }


    default :
      std::cout<<"touche non valide"<<std::endl;
    }
}


//---------------------------- C A L L B A C K S ------------------------------

// Callback des sliders
void onZoomSlide (int pos, void *data)
{
    My *my = (My*) data;
    my->loupe.reborner (my->img_res1, my->img_res2);
    my->set_recalc(My::R_LOUPE);
}

void onSeuilSlide (int pos, void *data)
{
    My *my = (My*) data;
    my->set_recalc(My::R_SEUIL);
}

void onSeuilSlide2 (int pos, void *data)
{
    My *my = (My*) data;
    my->set_recalc(My::R_SEUIL);
    /*My *my = (My*) data;
    //my->seuil_pol = pos;
    //if(my->seuil_pol <= 0){my->seuil_pol =1;}
    //onKeyPressEvent ("5");
    seuil_recalc = my->seuil_pol/1000;
    effectuer_transformations (My::A_TRANS5,my->img_niv);
    std::cout << "Transformation 5" << std::endl;
    my->affi = My::A_TRANS5;
    my->set_recalc(My::R_SEUIL);*/
    //dessiner_approx_poly(my->img_niv);
}

// Callback pour la souris
void onMouseEvent (int event, int x, int y, int flags, void *data)
{
    My *my = (My*) data;

    switch (event) {
        case cv::EVENT_LBUTTONDOWN :
            my->clic_x = x;
            my->clic_y = y;
            my->clic_n = 1;
            break;
        case cv::EVENT_MOUSEMOVE :
            // std::cout << "mouse move " << x << "," << y << std::endl;
            if (my->clic_n == 1) {
                my->loupe.deplacer (my->img_res1, my->img_res2,
                    x - my->clic_x, y - my->clic_y);
                my->clic_x = x;
                my->clic_y = y;
                my->set_recalc(My::R_LOUPE);
            }
            break;
        case cv::EVENT_LBUTTONUP :
            my->clic_n = 0;
            break;
    }
}


void afficher_aide() {
    // Indiquez les transformations ici
    std::cout <<
        "Touches du clavier:\n"
        "   a    affiche cette aide\n"
        " hHlL   change la taille de la loupe\n"
        "   i    inverse les couleurs de src\n"
        "   o    affiche l'image src originale\n"
        "   s    affiche l'image src seuillée\n"
        "   1    affiche la transformation 1\n"
        "   2    affiche la transformation 2\n"
        "   3    affiche la transformation 3\n"
        "  esc   quitte\n"
    << std::endl;
}

// Callback "maison" pour le clavier
int onKeyPressEvent (int key, void *data)
{
    My *my = (My*) data;

    if (key < 0) return 0;        // aucune touche pressée
    key &= 255;                   // pour comparer avec un char
    if (key == 27) return -1;     // ESC pour quitter

    switch (key) {
        case 'a' :
            afficher_aide();
            break;
        case 'h' :
        case 'H' :
        case 'l' :
        case 'L' : {
            std::cout << "Taille loupe" << std::endl;
            int h = my->img_res2.rows, w = my->img_res2.cols;
            if      (key == 'h') h = h >=  200+100 ? h-100 :  200;
            else if (key == 'H') h = h <= 2000-100 ? h+100 : 2000;
            else if (key == 'l') w = w >=  200+100 ? w-100 :  200;
            else if (key == 'L') w = w <= 2000-100 ? w+100 : 2000;
            my->img_res2 = cv::Mat(h, w, CV_8UC3);
            my->loupe.reborner(my->img_res1, my->img_res2);
            my->set_recalc(My::R_LOUPE);
          } break;
        case 'i' :
            std::cout << "Couleurs inversées" << std::endl;
            inverser_couleurs(my->img_src);
            my->set_recalc(My::R_SEUIL);
            break;
        case 'o' :
            std::cout << "Image originale" << std::endl;
            my->affi = My::A_ORIG;
            my->set_recalc(My::R_TRANSFOS);
            break;
        case 's' :
            std::cout << "Image seuillée" << std::endl;
            my->affi = My::A_SEUIL;
            my->set_recalc(My::R_SEUIL);
            break;
        case 'c' :
            std::cout << "Changement connex" << std::endl;
            if(glob_connex == 4) glob_connex = 8;
            else glob_connex = 4;

            my->affi = My::A_TRANS7;
            my->set_recalc(My::R_SEUIL);
            break;

        // Rajoutez ici des touches pour les transformations
        case '1' :
            std::cout << "Transformation 1" << std::endl;
            my->affi = My::A_TRANS1;
            my->set_recalc(My::R_SEUIL);
            break;
        case '2' :
            std::cout << "Transformation 2" << std::endl;
            my->affi = My::A_TRANS2;
            my->set_recalc(My::R_SEUIL);
            break;
        case '3' :
            std::cout << "Transformation 3" << std::endl;
            my->affi = My::A_TRANS3;
            my->set_recalc(My::R_SEUIL);
            break;
        case '4' :
            std::cout << "Transformation 4" << std::endl;
            my->affi = My::A_TRANS4;
            my->set_recalc(My::R_SEUIL);
            break;
        case '5' :
            std::cout << "Transformation 5" << std::endl;
            my->affi = My::A_TRANS5;
            my->set_recalc(My::R_SEUIL);
            break;
        case '6' :
            std::cout << "Transformation 6" << std::endl;
            my->affi = My::A_TRANS6;
            my->set_recalc(My::R_SEUIL);
            break;
        case '7' :
            std::cout << "Transformation 7" << std::endl;
            my->affi = My::A_TRANS7;
            my->set_recalc(My::R_SEUIL);
            break;
        case '8' :
            std::cout << "Transformation 8" << std::endl;
            my->affi = My::A_TRANS8;
            my->set_recalc(My::R_SEUIL);
            break;
        case '9' :
            std::cout << "Transformation 9" << std::endl;
            my->affi = My::A_TRANS9;
            my->set_recalc(My::R_SEUIL);
            break;

        default :
            //std::cout << "Touche '" << char(key) << "'" << std::endl;
            break;
    }
    return 1;
}


//---------------------------------- M A I N ----------------------------------

void afficher_usage (char *nom_prog) {
    std::cout << "Usage: " << nom_prog
              << "[-mag width height] [-thr seuil] in1 [out2]"
              << std::endl;
}

int main (int argc, char**argv)
{
    My my;
    char *nom_in1, *nom_out2, *nom_prog = argv[0];
    int zoom_w = 600, zoom_h = 500;

    while (argc-1 > 0) {
        if (!strcmp(argv[1], "-mag")) {
            if (argc-1 < 3) { afficher_usage(nom_prog); return 1; }
            zoom_w = atoi(argv[2]);
            zoom_h = atoi(argv[3]);
            argc -= 3; argv += 3;
        } else if (!strcmp(argv[1], "-thr")) {
            if (argc-1 < 2) { afficher_usage(nom_prog); return 1; }
            my.seuil = atoi(argv[2]);
            argc -= 2; argv += 2;
        } else break;
    }
    if (argc-1 < 1 or argc-1 > 2) { afficher_usage(nom_prog); return 1; }
    nom_in1  = argv[1];
    nom_out2 = (argc-1 == 2) ? argv[2] : NULL;

    // Lecture image
    my.img_src = cv::imread (nom_in1, cv::IMREAD_COLOR);  // produit du 8UC3
    if (my.img_src.empty()) {
        std::cout << "Erreur de lecture" << std::endl;
        return 1;
    }

    // Création résultats
    my.img_res1 = cv::Mat(my.img_src.rows, my.img_src.cols, CV_8UC3);
    my.img_res2 = cv::Mat(zoom_h, zoom_w, CV_8UC3);
    my.img_niv  = cv::Mat(my.img_src.rows, my.img_src.cols, CV_32SC1);
    my.img_coul = cv::Mat(my.img_src.rows, my.img_src.cols, CV_8UC3);
    my.loupe.reborner(my.img_res1, my.img_res2);

    // Création fenêtre
    cv::namedWindow ("ImageSrc", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar ("Zoom", "ImageSrc", &my.loupe.zoom, my.loupe.zoom_max,
        onZoomSlide, &my);
    cv::createTrackbar ("Seuil", "ImageSrc", &my.seuil, 255,
        onSeuilSlide, &my);
    cv::createTrackbar ("Seuil2", "ImageSrc", &my.seuil_pol, 1000,
            onSeuilSlide2, &my);
    cv::setMouseCallback ("ImageSrc", onMouseEvent, &my);

    cv::namedWindow ("Loupe", cv::WINDOW_AUTOSIZE);
    afficher_aide();

    // Boucle d'événements
    for (;;) {

        if (my.need_recalc(My::R_SEUIL))
        {
            // std::cout << "Calcul seuil" << std::endl;
            cv::Mat img_gry;
            cv::cvtColor (my.img_src, img_gry, cv::COLOR_BGR2GRAY);
            cv::threshold (img_gry, img_gry, my.seuil, 255, cv::THRESH_BINARY);
            img_gry.convertTo (my.img_niv, CV_32SC1,1., 0.);
        }

        if (my.need_recalc(My::R_TRANSFOS))
        {
            // std::cout << "Calcul transfos" << std::endl;
            if (my.affi != My::A_ORIG) {
                effectuer_transformations (my.affi, my.img_niv,my.seuil_pol);
                representer_en_couleurs_vga (my.img_niv, my.img_coul);
            } else my.img_coul = my.img_src.clone();
        }

        if (my.need_recalc(My::R_LOUPE)) {
            // std::cout << "Calcul loupe puis affichage" << std::endl;
            my.loupe.dessiner_rect    (my.img_coul, my.img_res1);
            my.loupe.dessiner_portion (my.img_coul, my.img_res2);
            cv::imshow ("ImageSrc", my.img_res1);
            cv::imshow ("Loupe"   , my.img_res2);
        }
        my.reset_recalc();

        // Attente du prochain événement sur toutes les fenêtres, avec un
        // timeout de 15ms pour détecter les changements de flags
        int key = cv::waitKey (15);

        // Gestion des événements clavier avec une callback "maison" que l'on
        // appelle nous-même. Les Callbacks souris et slider sont directement
        // appelées par waitKey lors de l'attente.
        if (onKeyPressEvent (key, &my) < 0) break;
    }

    // Enregistrement résultat
    if (nom_out2) {
        if (! cv::imwrite (nom_out2, my.img_coul))
             std::cout << "Erreur d'enregistrement" << std::endl;
        else std::cout << "Enregistrement effectué" << std::endl;
     }
    return 0;
}
