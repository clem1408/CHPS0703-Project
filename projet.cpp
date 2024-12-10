#include <algorithm>
#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

using namespace cv;
using namespace std;

/**
 * Retourne l'extension d'un fichier passé en paramètre
 *
 * @param str Le nom du fichier
 * @return L'extension du fichier
 */
string getFileExtension(string str) { return str.substr(str.length() - 4); }

/**
 * Retourne le nom d'une image passé en paramètre
 *
 * @param str Le nom du fichier
 * @return Le nom de l'image
 */
string getFileName(string str) {
  string extension = getFileExtension(str);

  if (str.length() > 4) {
    str.erase(str.length() - 4);
  }

  size_t lastSlashPos = str.find_last_of("/");

  string fileName = str.substr(lastSlashPos + 1);

  return fileName;
}

/**
 * Retourne le chemin complet d'une nouvelle image
 *
 * @param str Le nom du fichier
 * @param type Specification de l'image (ex --> Binarisation :
 * saturneBinarisation)
 * @return Le chemin complet de la nouvelle image
 */
string getFilePath(string str, string type) {
  string extension = getFileExtension(str);

  if (str.length() > 4) {
    str.erase(str.length() - 4);
  }

  size_t lastSlashPos = str.find_last_of("/");

  string fileName = str.substr(lastSlashPos + 1);

  return "Images/" + fileName + "_edit/" + fileName + type + extension;
}

/**
 * Fonction permettant de faire le filtre Gaussien d'une image
 *
 * @param image L'image en question
 * @param nomImage Nom de l'image
 * @return Un clone de l'image ayant subie une filtre Gaussien
 */
Mat filtreGaussien(Mat image, char *nomImage) {
  // Matrice pour le filtre Gaussien
  int tab[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};

  for (int y = 1; y < image.rows; y++) {
    for (int x = 1; x < image.cols; x++) {
      uchar &pixel = image.at<uchar>(y, x);

      int sum = 0;

      for (int j = -1; j <= 1; j++) {
        for (int i = -1; i <= 1; i++) {
          sum += tab[j + 1][i + 1] * image.at<uchar>(y + i, x + j);
        }
      }

      // Filtre Gaussien
      pixel = (uchar)(sum / 16 > 255) ? 255 : sum / 16;
    }
  }

  string str(nomImage);

  string fichier_modifie = getFilePath(str, "FiltreGaussien");

  // Enregistrement de l'image
  imwrite(fichier_modifie.c_str(), image);

  cout << "Filtre gaussien appliqué et enregistrée!" << endl;

  return image.clone();
}

/**
 * Fonction permettant de faire le gradient d'une image
 *
 * @param image L'image en question
 * @param nomImage Nom de l'image
 * @return Un clone de l'image ayant subie un un gradient
 */
Mat filtreGradient(Mat image, char *nomImage) {
  // Vérifier si l'image est en niveaux de gris
  if (image.channels() != 1) {
    cvtColor(image, image, COLOR_BGR2GRAY);
  }

  // Matrices pour les gradients
  Mat grad_x = Mat::zeros(image.size(), CV_32F);
  Mat grad_y = Mat::zeros(image.size(), CV_32F);

  // Masques de dérivation
  int dx_mask[3] = {-1, 0, 1};
  int dy_mask[3] = {-1, 0, 1};

  // Calculer les gradients horizontaux et verticaux
  for (int y = 1; y < image.rows - 1; y++) {
    for (int x = 1; x < image.cols - 1; x++) {
      // Calculer le gradient horizontal
      float sum_x = 0;
      for (int j = -1; j <= 1; j++) {
        sum_x += dx_mask[j + 1] * image.at<uchar>(y, x + j);
      }
      grad_x.at<float>(y, x) = sum_x;

      // Calculer le gradient vertical
      float sum_y = 0;
      for (int i = -1; i <= 1; i++) {
        sum_y += dy_mask[i + 1] * image.at<uchar>(y + i, x);
      }
      grad_y.at<float>(y, x) = sum_y;
    }
  }

  // Combiner les gradients pour obtenir l'intensité des bords
  Mat edges = Mat::zeros(image.size(), CV_32F);
  for (int y = 0; y < image.rows; y++) {
    for (int x = 0; x < image.cols; x++) {
      edges.at<float>(y, x) =
          sqrt(grad_x.at<float>(y, x) * grad_x.at<float>(y, x) +
               grad_y.at<float>(y, x) * grad_y.at<float>(y, x));
    }
  }

  // Normaliser les bords
  normalize(edges, edges, 0, 255, NORM_MINMAX);
  edges.convertTo(edges, CV_8U);

  string str(nomImage);
  string fichier_modifie = getFilePath(str, "Gradient");

  // Enregistrement de l'image
  imwrite(fichier_modifie.c_str(), edges);

  cout << "Bords mis en évidence et enregistrés!" << endl;

  return edges.clone();
}

/**
 * Fonction permettant de génerée une energy map
 * @param image L'image en question
 * @param nomImage Nom de l'image
 * @return Un clone de l'energy map
 */
Mat energyMap(Mat image, char *nomImage) {
  int windowSize = 3;
  int halfSize = windowSize / 2;
  Mat gray, gradient, energyMap, energyMapColor;

  // Conversion en niveaux de gris si nécessaire
  if (image.channels() > 1) {
    cvtColor(image, gray, COLOR_BGR2GRAY);
  } else {
    gray = image.clone();
  }

  gray = filtreGaussien(gray.clone(), nomImage);

  // Application du filtre Laplacien
  gradient = filtreGradient(gray.clone(), nomImage);

  energyMap = gradient.clone();

  for (int i = 1; i < energyMap.rows - 1; i++) {
    for (int j = 1; j < energyMap.cols - 1; j++) {

      uchar &pixel = energyMap.at<uchar>(i, j);
      int min = 255;

      // Appliquer le kernel
      for (int m = -halfSize; m <= halfSize; m++) {
        int x = j + m;
        int y = i - 1;

        char pixel_value = energyMap.at<uchar>(y, x);

        min = pixel_value < min ? pixel_value : min;
      }

      // Stocker la valeur
      pixel += min;
      min = 255;
    }
  }

  convertScaleAbs(energyMap, energyMap);

  // Application d'une colormap pour rendre la energy map en couleur
  applyColorMap(energyMap, energyMapColor, COLORMAP_JET);

  // Enregistrement de l'image
  string fichier_modifie = getFilePath(string(nomImage), "EnergyMap");
  imwrite(fichier_modifie.c_str(), energyMapColor);

  cout << "Energy map générée et enregistrée!" << endl;

  return energyMapColor.clone();
}

int main(int argc, char **argv) {
  // Vérification des arguments ./tp2 [arguments]
  if (argc != 2) {
    cerr << "Usage: ./program <image_path>" << endl;
    return -1;
  }

  // Lire l'image en niveau de gris
  Mat image = imread(argv[1], IMREAD_GRAYSCALE);

  // Gestion des erreurs
  if (image.empty()) {
    cerr << "Erreur de lecture de l'image!" << endl;
    return -1;
  }

  // Energy map
  Mat imageEnergyMap = energyMap(image.clone(), argv[1]);

  return EXIT_SUCCESS;
}