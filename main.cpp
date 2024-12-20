#include <filesystem>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

#include "seamcarving.h"

#define NB_PARAM 3

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {

  if (argc != NB_PARAM + 1) {
    cerr << "Paramètre : chemin vers l'image, nombre de seams, type (1>cols, "
            "2>rows, 3>both)"
         << endl;
    exit(EXIT_FAILURE);
  }

  string nom_image;       // Nom de l'image original avec son extension (ex :
                          // nom_image.png)
  string nom_image_noExt; // Nom de l'image sans son extension (ex : nom_image)
  string dir_path; // Chemin d'accès vers l'enregistrement des images générées
                   // (ex : images/nom_image/)

  // On récupère un chemin d'accès vers l'image et on ne garde que le nom de
  // l'image avec son extension
  filesystem::path chemin_abs_img(argv[1]);
  nom_image = chemin_abs_img.filename().string();

  // On ne garde que le nom de l'image sans l'extension afin de pouvoir créer un
  // répertoire avec son nom
  filesystem::path chemin_img_ext(nom_image);
  nom_image_noExt = chemin_img_ext.stem().string();

  dir_path = string("Images/") + nom_image_noExt + string("/");

  // Création du répertoire "images"
  if (!filesystem::exists("Images")) {
    if (!filesystem::create_directory("Images")) {
      cerr << "Échec de la création du répertoire 'images'." << endl;
      exit(EXIT_FAILURE);
    }
  }

  // Création du répertoire pourtant le nom de l'image traité
  if (!filesystem::exists(dir_path)) {
    if (!filesystem::create_directory(dir_path)) {
      cerr << "Échec de la création du répertoire d'accueil." << endl;
      exit(EXIT_FAILURE);
    }
  }

  // Lecture de l'image en couleurs (par défaut en BGR)
  Mat image = imread(argv[1], IMREAD_COLOR);

  // Lecture de l'image avec le paramètre "IMREAD_GRAYSCALE" pour uniquement
  // l'avoir en degrés de gris
  Mat image_gray = imread(argv[1], IMREAD_GRAYSCALE);

  if (image.empty()) { // Vérification si l'image existe
    cerr << "Erreur de lecture de l'image (main)!" << endl;
    exit(EXIT_FAILURE);
  }

  // ----------------------- Projet -----------------------

  Mat img;

  int nb_seam = atoi(argv[2]); // nombre de seams

  int type_seam = atoi(argv[3]);

  switch (type_seam) {
  case 1:
    img = seamCarving(image.clone(), image_gray.clone(), nb_seam, nom_image,
                      dir_path, SEAM_COLS);
    break;
  case 2:
    img = seamCarving(image.clone(), image_gray.clone(), nb_seam, nom_image,
                      dir_path, SEAM_ROWS);
    break;
  case 3:
    img = seamCarving(image.clone(), image_gray.clone(), nb_seam, nom_image,
                      dir_path, SEAM_COLS);
    img = seamCarving(image.clone(), image_gray.clone(), nb_seam, nom_image,
                      dir_path, SEAM_ROWS);
    break;
  default:
    break;
  }

  return EXIT_SUCCESS;
} // fin main