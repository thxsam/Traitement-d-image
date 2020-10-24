#include "camera.h"
#include <iostream>
#include <sstream>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#define M_PI 3.14159265358979323846
using namespace std;
using namespace cv;
RNG rng(12345);

int cptg = 0; // Compteur de voiture voie de gauche
int cptd = 0;// Compteur de voiture voie de droite
int seuil = 5; // numéro de l'image qui va nous servir d'image de référence pour tracer les lignes
int i = 0; // Compteur pour compter les images
Mat m_framecpy,gray,gray2, imgout2, imgout3,imgout,drawing, m_frame2; // Image qui nous serviront par la suite

Camera::Camera()
{
	m_fps = 30;
}

bool Camera::open(std::string filename)
{
	m_fileName = filename;

    bool isOpen;
    isOpen = m_cap.open(filename);

	if(!isOpen)
	{
		std::cerr << "Unable to open video file." << std::endl;
		return false;
	}
	
	// set framerate, if unable to read framerate, set it to 30
    m_fps = m_cap.get(cv::CAP_PROP_FPS);
	if(m_fps == 0)
		m_fps = 30;

    return true;
}

void Camera::play()
{
	// Create main window
	namedWindow("Video", WINDOW_AUTOSIZE);
	bool isReading = true;
	// Compute time to wait to obtain wanted framerate
	int timeToWait = 1000/m_fps;
	

	while(isReading)
	{
		// Get frame from stream
		isReading = m_cap.read(m_frame);
		if(isReading)
		{
			i++; // On incrémente le compteur d'image à chaque tour de boucle
			if ( i < seuil ) {  // Avant l'image de référence, on applique Hough pour tarcer les lignes en temps réel sur chaque image
				Mat gray;
				cvtColor(m_frame, gray, COLOR_BGR2GRAY);
				Mat gray_blured;
				blur(gray, gray_blured, Size(4,4));
				Mat gray_canny;
				Canny(gray_blured, gray_canny, 75, 200, 3, false);
				vector<Vec2f> lines;
				HoughLines(gray_canny, lines, 1, M_PI / 180, 130, 0, 0);
				for (size_t i = 0; i < lines.size(); i++) {
					float rho = lines[i][0], theta = lines[i][1];
					Point pt1, pt2;
					double a = cos(theta), b = sin(theta);
					double x0 = a * rho, y0 = b * rho;
					pt1.x = cvRound(x0 + 1000 * (-b));
					pt1.y = cvRound(y0 + 1000 * (a));
					pt2.x = cvRound(x0 - 1000 * (-b));
					pt2.y = cvRound(y0 - 1000 * (a));
					line(m_frame, pt1, pt2, Scalar(255, 0, 0), 3, LINE_AA);
				}
			}
			if (i == seuil){ // On crée une image de référence m_framecpy 
				m_framecpy = m_frame.clone();
				
			}
			if (i > seuil){ // On utilisera l'image de référence m_framecpy pour appliquer Hough et donc dessiner les lignes à partir de l'image de référence
			
				absdiff(m_framecpy,m_frame,imgout); // On extrait le background
				cvtColor(imgout, imgout, COLOR_BGR2GRAY); // On met l'image en nuance de gris
				threshold(imgout, imgout, 20, 255,THRESH_BINARY); // On binarise l'image
				// Ouverture
				dilate(imgout, imgout,getStructuringElement(MORPH_RECT, Size(5,5) ));
				erode(imgout, imgout, getStructuringElement(MORPH_RECT, Size(5, 5)));
				// Fermeture
				erode(imgout, imgout, getStructuringElement(MORPH_ELLIPSE, Size(15, 15)));
				dilate(imgout, imgout,getStructuringElement(MORPH_ELLIPSE, Size(20, 20) ));
				
				Canny(imgout, gray2, 100, 200); // Canny edge detection
				vector<vector<Point> > contours;
				vector<Vec4i> hierarchy;

				findContours( gray2, contours,hierarchy, CV_RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0,0) );// On récupère les contours des véhicules
				
				vector<vector<Point> > contours_poly( contours.size() );
				vector<Rect> boundRect( contours.size() );
				vector<Point2f>centers( contours.size() );
				vector<float>radius( contours.size() );
				
				int copy = 0;
				int copy2 =0;
				
				// On créé 2 rectangles sur les voies pour compter les voitures
				
				Scalar color = Scalar(0,0,255);
				Rect A(0,90,400,6);
				rectangle( m_frame,A, color, 2,8,0 );  // rectangle A, pour la voie de gauche
				Rect B(400,50,800,5);
				rectangle( m_frame,B, color, 2,8,0 );  // rectangle B , pour la voie de droite
				
				for( int i = 0; i < contours.size(); i++ ){
					approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true );
					boundRect[i] = boundingRect( contours_poly[i] );	

				}

				for( int i = 0; i< contours.size(); i++ ){
					
                    			// On vérifie que la taille du rectangle est suffisament grande pour ne pas comptabiliser les rectangles dues au bruit
				    	if (abs(boundRect[i].tl().x- boundRect[i].br().x) > 28 && abs(boundRect[i].tl().y- boundRect[i].br().y) > 28){
				        rectangle(m_frame, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 0, 255), 2);    //dessine un rectangle sur le véhicule 
				        
				       
					if (boundRect[i].br().x <400 && boundRect[i].br().y >92 && boundRect[i].br().y < 98){ // Zone de gauche
					    
					    if (boundRect[i].br().y != copy) // Vérifie que la position du rectangle à bien changer pour ne pas compter deux fois un même véhicule
					    	cptg++; // Incrémente le compteur
					    
					}
				        else if ( boundRect[i].tl().x > 400 && boundRect[i].tl().y >50 && boundRect[i].tl().y < 53) { // Zone droite
				        
				            if (boundRect[i].tl().y != copy2) // On vérifie que la position du rectangle à bien changer
				           	 cptd++; // On incrémente le compteur
				        }
				    }	
					copy = boundRect[i].br().y;
					copy2 = boundRect[i].tl().y;
				}

				// On applique Hough sur l'image de référence m_framecpy pour détecter les voies
				cvtColor(m_framecpy, gray, COLOR_BGR2GRAY);	
				Mat gray_blured;
				blur(gray, gray_blured, Size(4,4)); // On floute l'image pour enlever le bruit
				Mat gray_canny;
				Canny(gray_blured, gray_canny, 75, 200, 3, false); // On applique Canny
				vector<Vec2f> lines;
				HoughLines(gray_canny, lines, 1, M_PI / 180, 130, 0, 0); // On applique Hough
					for (size_t i = 0; i < lines.size(); i++) {
						float rho = lines[i][0], theta = lines[i][1];
						Point pt1, pt2;
						double a = cos(theta), b = sin(theta);
						double x0 = a * rho, y0 = b * rho;
						pt1.x = cvRound(x0 + 1000 * (-b));
						pt1.y = cvRound(y0 + 1000 * (a));
						pt2.x = cvRound(x0 - 1000 * (-b));
						pt2.y = cvRound(y0 - 1000 * (a));
						line(m_frame, pt1, pt2, Scalar(255, 0, 0), 3, LINE_AA); // On trace les lignes 
						
					}
			}
			
			// Affichage et comptage en direct
			cout << " Véhicules voie de gauche : " << cptg << endl;
			cout << " Véhicules voie de droite : " << cptd << endl;	
			imshow(" Détection de véhicules ", m_frame);
			
			
		}
		
		else
		{
			std::cerr << "Unable to read device" << std::endl;
		}
		
		// If escape key is pressed, quit the program
		if(waitKey(timeToWait)%256 == 27)
		{
			std::cerr << "Stopped by user" << std::endl;
			isReading = false;
		}	
	}	
	// Résultat du comptage
	cout << " Nombre total de véhicules voie de gauche :  " << cptg << endl;
	cout << " Nombre total de véhicules voie de droite : " << cptd << endl;
	cout << " Nombre total de véhicules détectées : " << cptd+cptg << endl;
}

bool Camera::close()
{
	// Close the stream
	m_cap.release();
	
	// Close all the windows
    destroyAllWindows();

    return true;
}























	
				
