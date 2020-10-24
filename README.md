# Traitement-d-image
Auteur : Samuel Bk

L'objectif est de réaliser un comptage précis de véhicules de tout type à partir d'une vidéo d'une portion d'autoroute
Les véhicules peuvent être de tout type : 2 roues, voitures, camions...
- La vidéo de la portion d'autoroute est contenu dans le fichier CCTV.avi
- Le traitement d'image est réalisé dans le fichier camera.cpp

STRATEGIE MIS EN PLACE

Dans un premier temps, on extrait du flux vidéo les images une par une afin de pouvoir les traiter.
On va ensuite soustraire a chaque image une image de "référence" ne contenant pas de véhicules.
Nous pourrons alors détecter la présence de véhicules et à l'aide de différentes opérations morphologiques et de filtres, réussir à identifier et compter le nombre de véhicules.



