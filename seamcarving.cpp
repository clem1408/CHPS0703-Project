#include <algorithm>
#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

#include "seamcarving.h"

using namespace cv;
using namespace std;

/**
 * Algorithme gaussien
 *
 * Lissage modéré dépendament de la fenêtre utilisé
 * On perd moins de détaille que la moyenne
 *
 * On applique une fenêtre (kernel) sur chaque pixel de l'image.
 * Dans le gaussien on applique sur notre pixel, une fenêtre avec des valeurs
 * qui peuvent être différentes. Ici on fait une moyenne en divisant pas la
 * somme des valeurs de la fenêtre.
 *
 * @param image Image chargé dans le main
 * @param nomImage Nom de l'image avec l'extension
 * @param repertoire Répertoire d'export de l'image générée
 * @return Clone de la nouvelle image générée
 */
Mat filtreGaussien(const Mat &image) {

  int kernel_size = 3;
  int half_size = kernel_size / 2;

  int k = 0, l = 0;

  int fenetre[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};

  Mat gaussien_img = image.clone();

  // Parcourir chaque pixel de l'image
  for (int i = 0; i < image.rows; i++) {
    for (int j = 0; j < image.cols; j++) {

      uchar &pixel = gaussien_img.at<uchar>(i, j);

      int som = 1;
      uchar mean_value = 0;

      // Appliquer le kernel
      k = 0;
      for (int m = -half_size; m <= half_size; m++) {
        l = 0;
        for (int n = -half_size; n <= half_size; n++) {
          // Coordonnées pour l'image, adaptées celon la position de la fenêtre
          // dans l'image
          int x = j + n;
          int y = i + m;

          // Vérifier les limites de l'image
          if (x >= 0 && x < image.cols && y >= 0 && y < image.rows) {
            uchar pixel_value = image.at<uchar>(y, x);

            som += fenetre[k][l] * pixel_value;
          }
          l++;
        }
        k++;
      }

      mean_value = som / 16;

      // Stocker la valeur
      pixel = mean_value;
    }
  }

  return gaussien_img;
} // fin filtreGaussien

/**
 * Fonction permettant de calculer les gradients horizontaux
 * et verticaux d'une image en niveaux de gris à l'aide de masques
 * de dérivation. Ensuite, elle combine ces gradients pour détecter
 * les bords, les normalise et retourne une image où les bords sont
 * mis en évidence. C'est un procédé de détection des bords
 *
 * @param image L'image en question
 * @param nomImage Nom de l'image
 * @return Un clone de l'image ayant subie un un gradient
 */
Mat filtreGradient(
    const Mat &image) { // Vérifier si l'image est en niveaux de gris
  if (image.channels() != 1) {
    cvtColor(image, image, COLOR_BGR2GRAY);
  }

  // Matrices pour les gradients
  Mat grad_x = Mat::zeros(image.size(), CV_32F);
  Mat grad_y = Mat::zeros(image.size(), CV_32F);

  // Masques de dérivation (noyaux de convolution)
  int dx_mask[3] = {-1, 0, 1};
  int dy_mask[3] = {-1, 0, 1};

  // Calculer les gradients horizontaux et verticaux
  for (int y = 1; y < image.rows - 1; y++) {
    for (int x = 1; x < image.cols - 1; x++) {
      // Calculer le gradient horizontal
      float sum_x = 0;
      for (int j = -1; j <= 1; j++) {
        sum_x += dx_mask[j + 1] * (float)image.at<uchar>(y, x + j);
      }

      grad_x.at<float>(y, x) = sum_x;

      // Calculer le gradient vertical
      float sum_y = 0;
      for (int i = -1; i <= 1; i++) {
        sum_y += dy_mask[i + 1] * (float)image.at<uchar>(y + i, x);
      }
      grad_y.at<float>(y, x) = sum_y;
    }
  }

  // Combiner les gradients pour obtenir l'intensité des bords
  // Magnitude = sqrt((gradx)² + (grady)²)
  Mat edges = Mat::zeros(image.size(), CV_32F);
  for (int y = 0; y < image.rows; y++) {
    for (int x = 0; x < image.cols; x++) {
      edges.at<float>(y, x) =
          sqrt(grad_x.at<float>(y, x) * grad_x.at<float>(y, x) +
               grad_y.at<float>(y, x) * grad_y.at<float>(y, x));
    }
  }

  // Normaliser les bords [0,255]
  normalize(edges, edges, 0, 255, NORM_MINMAX);
  edges.convertTo(edges, CV_8U);

  return edges.clone();
} // fin filtreGradient

/**
 * Crée la matrice cumulative en partant du haut vers le bas.
 * Adapté à un seam colonnes.
 *
 * @param image Image d'entrée
 * @return Matrice cumulative
 */
int **matriceCumulativeCols(const Mat &image) {
  int rows = image.rows;
  int cols = image.cols;

  // Allocation de la matrice cumulative
  int **m_cumul = (int **)malloc(rows * sizeof(int *));
  if (!m_cumul) {
    perror("Erreur d'allocation mémoire pour la matrice cumulative");
    return nullptr;
  }
  for (int i = 0; i < rows; ++i) {
    m_cumul[i] = (int *)malloc(cols * sizeof(int));
    if (!m_cumul[i]) {
      perror("Erreur d'allocation mémoire pour une ligne");
      for (int k = 0; k < i; ++k)
        free(m_cumul[k]);
      free(m_cumul);
      return nullptr;
    }
  }

  // Initialisation de la première ligne
  for (int j = 0; j < cols; ++j) {
    m_cumul[0][j] = image.at<uchar>(0, j);
  }

  // Calcul de la matrice cumulative
  for (int i = 1; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      int current_pixel = image.at<uchar>(i, j);
      int min_prev = m_cumul[i - 1][j];

      // Vérifier les pixels voisins en respectant les limites
      if (j > 0) {
        min_prev = min(min_prev, m_cumul[i - 1][j - 1]);
      }
      if (j < cols - 1) {
        min_prev = min(min_prev, m_cumul[i - 1][j + 1]);
      }

      m_cumul[i][j] = current_pixel + min_prev;
    }
  }

  return m_cumul;
} // Fin matriceCumulativeCols

/**
 * Crée la matrice cumulative en partant de gauche vers la droite.
 * Adapté à un seam ligne.
 *
 * @param image Image d'entrée
 * @return Matrice cumulative
 */
int **matriceCumulativeRows(const Mat &image) {
  int rows = image.rows;
  int cols = image.cols;

  // Allocation de la matrice cumulative
  int **m_cumul = (int **)malloc(rows * sizeof(int *));
  if (!m_cumul) {
    perror("Erreur d'allocation mémoire pour la matrice cumulative");
    return nullptr;
  }
  for (int i = 0; i < rows; ++i) {
    m_cumul[i] = (int *)malloc(cols * sizeof(int));
    if (!m_cumul[i]) {
      perror("Erreur d'allocation mémoire pour une ligne");
      for (int k = 0; k < i; ++k)
        free(m_cumul[k]);
      free(m_cumul);
      return nullptr;
    }
  }

  // Initialisation de la première colonne
  for (int i = 0; i < rows; ++i) {
    m_cumul[i][0] = image.at<uchar>(i, 0);
  }

  // Calcul de la matrice cumulative
  for (int j = 1; j < cols; ++j) {
    for (int i = 0; i < rows; ++i) {
      int current_pixel = image.at<uchar>(i, j);
      int min_prev = m_cumul[i][j - 1];

      // Vérifier les pixels voisins en respectant les limites
      if (i > 0) {
        min_prev = min(min_prev, m_cumul[i - 1][j - 1]);
      }
      if (i < rows - 1) {
        min_prev = min(min_prev, m_cumul[i + 1][j - 1]);
      }

      m_cumul[i][j] = current_pixel + min_prev;
    }
  }

  return m_cumul;
} // Fin matriceCumulativeRows

/**
 * Recherche du chemin minium du bas vers le haut
 *
 * @param m_cumul Matrice cumulative adaptée à la suppression de colonnes
 * @return Tableau de taille rows qui contient l'indice de la colonne à
 * supprimer sur chaque ligne
 */
int *findWayCols(const Mat &image, int **m_cumul) {
  int rows = image.rows;
  int cols = image.cols;

  // Tableau pour stocker le chemin
  int *way = (int *)malloc(sizeof(int) * rows);
  if (!way) {
    perror("Erreur d'allocation mémoire");
    return nullptr;
  }

  // Étape 1 : Trouver le minimum dans la dernière ligne
  int min_col = 0;
  int min_val = INT_MAX;

  for (int j = 0; j < cols; ++j) {
    if (m_cumul[rows - 1][j] < min_val) {
      min_val = m_cumul[rows - 1][j];
      min_col = j;
    }
  }

  way[0] = min_col;

  // Étape 2 : Remonter ligne par ligne
  for (int i = rows - 2, k = 1; i >= 0; --i, ++k) {
    int prev_col = way[k - 1];
    min_col = prev_col; // Par défaut, la colonne actuelle est le minimum
    min_val = m_cumul[i][prev_col];

    // Vérifier les colonnes adjacentes
    for (int offset = -1; offset <= 1; ++offset) {
      int adj_col = prev_col + offset;
      if (adj_col >= 0 && adj_col < cols && m_cumul[i][adj_col] < min_val) {
        min_val = m_cumul[i][adj_col];
        min_col = adj_col;
      }
    }

    way[k] = min_col; // Enregistrer la colonne trouvée
  }

  return way;
} // Fin findWayCols

/**
 * Recherche du chemin minimum de droite vers la gauche
 *
 * @param m_cumul Matrice cumulative adaptée à la suppression de lignes
 * @return Tableau de taille cols qui contient l'indice de la ligne à supprimer
 * sur chaque colonne
 */
int *findWayRows(const Mat &image, int **m_cumul) {
  int rows = image.rows;
  int cols = image.cols;

  // Tableau pour stocker le chemin
  int *way = (int *)malloc(sizeof(int) * cols);
  if (!way) {
    perror("Erreur d'allocation mémoire");
    return nullptr;
  }

  // Étape 1 : Trouver le minimum dans la dernière colonne
  int min_row = 0;
  int min_val = INT_MAX;

  for (int i = 0; i < rows; ++i) {
    if (m_cumul[i][cols - 1] < min_val) {
      min_val = m_cumul[i][cols - 1];
      min_row = i;
    }
  }

  way[0] = min_row;

  // Étape 2 : Revenir colonne par colonne
  for (int j = cols - 2, k = 1; j >= 0; --j, ++k) {
    int prev_row = way[k - 1];
    min_row = prev_row; // Par défaut, la ligne actuelle est le minimum
    min_val = m_cumul[prev_row][j];

    // Vérifier les lignes adjacentes
    for (int offset = -1; offset <= 1; ++offset) {
      int adj_row = prev_row + offset;
      if (adj_row >= 0 && adj_row < rows && m_cumul[adj_row][j] < min_val) {
        min_val = m_cumul[adj_row][j];
        min_row = adj_row;
      }
    }

    way[k] = min_row; // Enregistrer la ligne trouvée
  }

  return way;
} // Fin findWayRows

/**
 * Supprime un pixel d'une image en niveaux de gris, décale les pixels de la
 * ligne vers la gauche, et remplace le dernier pixel par un pixel blanc (255).
 *
 * @param image Image en niveaux de gris (type CV_8UC1)
 * @param row Ligne où effectuer l'opération
 * @param col Colonne du pixel à supprimer
 */
inline void removePixelAndShiftLeftGray(Mat &image, int row, int col) {
  // Vérification des dimensions
  if (row < 0 || row >= image.rows || col < 0 || col >= image.cols) {
    cerr << "Erreur : Indices hors limites (row=" << row << ", col=" << col
         << ")." << endl;
    return;
  }

  // Décalage des pixels vers la gauche
  uchar *row_ptr = image.ptr<uchar>(row);
  for (int c = col; c < image.cols - 1; ++c) {
    row_ptr[c] = row_ptr[c + 1];
  }

  // Remplacement du dernier pixel par blanc (255)
  row_ptr[image.cols - 1] = 255;
} // Fin removePixelAndShiftLeftGray

/**
 * Supprime un pixel d'une image en niveaux de gris, décale les pixels de la
 * colonne vers le haut, et remplace le dernier pixel par un pixel blanc (255).
 *
 * @param image Image en niveaux de gris (type CV_8UC1)
 * @param row Ligne où effectuer l'opération
 * @param col Colonne du pixel à supprimer
 */
inline void removePixelAndShiftUpGray(Mat &image, int row, int col) {
  // Vérification des dimensions
  if (row < 0 || row >= image.rows || col < 0 || col >= image.cols) {
    cerr << "Erreur : Indices hors limites (row=" << row << ", col=" << col
         << ")." << endl;
    return;
  }

  // Décalage des pixels vers le haut
  for (int r = row; r < image.rows - 1; ++r) {
    image.at<uchar>(r, col) = image.at<uchar>(r + 1, col);
  }

  // Remplacement du dernier pixel par blanc (255)
  image.at<uchar>(image.rows - 1, col) = 255;
} // Fin removePixelAndShiftUpGray

/**
 * Suppression du chemin sur l'image.
 *
 * @param image ImaseamCarvinge Type de seam (SEAM_ROWS ou SEAM_COLS).
 * @return Image réduite après suppression du chemin.
 */
inline Mat suppressionSeamGray(const Mat &image, const int *way,
                               int seam_type) {
  // Cloner l'image pour préserver l'originale
  Mat reduced_img = image.clone();

  // Initialisation de l'indice pour parcourir le tableau `way`
  int k = 0;

  // Suppression du chemin pour chaque type de seam
  if (seam_type == SEAM_COLS) {
    // Suppression colonne par colonne en partant de la dernière ligne
    for (int i = reduced_img.rows - 1; i >= 0; --i) {
      removePixelAndShiftLeftGray(reduced_img, i, way[k]);
      ++k;
    }
  } else if (seam_type == SEAM_ROWS) {
    // Suppression ligne par ligne en partant de la dernière colonne
    for (int i = reduced_img.cols - 1; i >= 0; --i) {
      removePixelAndShiftUpGray(reduced_img, way[k], i);
      ++k;
    }
  } else {
    // Gestion des erreurs : type de seam invalide
    cerr << "Erreur : Type de seam non valide." << endl;
  }

  return reduced_img;
} // fin suppressionSeam

/**
 * Supprime un pixel d'une image couleur, décale les pixels de la
 * ligne vers la gauche, et remplace le dernier pixel par un pixel blanc (255).
 *
 * @param image Image en couleur
 * @param row Ligne où effectuer l'opération
 * @param col Colonne du pixel à supprimer
 */
inline void removePixelAndShiftLeftColor(Mat &image, int row, int col) {
  if (row < 0 || row >= image.rows || col < 0 || col >= image.cols) {
    cerr << "Erreur : Indices hors limites (row=" << row << ", col=" << col
         << ")." << endl;
    return;
  }

  for (int c = col; c < image.cols - 1; ++c) {
    image.at<Vec3b>(row, c) = image.at<Vec3b>(row, c + 1);
  }

  image.at<Vec3b>(row, image.cols - 1) = Vec3b(255, 255, 255);
} // Fin removePixelAndShiftLeftColor

/**
 * Supprime un pixel d'une image en couleur, décale les pixels de la
 * colonne vers le haut, et remplace le dernier pixel par un pixel blanc (255).
 *
 * @param image Image en couleur
 * @param row Ligne où effectuer l'opération
 * @param col Colonne du pixel à supprimer
 */
inline void removePixelAndShiftUpColor(Mat &image, int row, int col) {
  if (row < 0 || row >= image.rows || col < 0 || col >= image.cols) {
    cerr << "Erreur : Indices hors limites (row=" << row << ", col=" << col
         << ")." << endl;
    return;
  }

  for (int r = row; r < image.rows - 1; ++r) {
    image.at<Vec3b>(r, col) = image.at<Vec3b>(r + 1, col);
  }

  image.at<Vec3b>(image.rows - 1, col) = Vec3b(255, 255, 255);
} // Fin removePixelAndShiftUpColor

/**
 * Suppression du chemin sur l'image couleur.
 *
 * @param image ImaseamCarvinge Type de seam (SEAM_ROWS ou SEAM_COLS).
 * @return Image réduite après suppression du chemin.
 */
inline Mat suppressionSeamColor(const Mat &image, const int *way,
                                int seam_type) {
  // Cloner l'image pour préserver l'originale
  Mat reduced_img = image.clone();

  // Initialisation de l'indice pour parcourir le tableau `way`
  int k = 0;

  // Suppression du chemin pour chaque type de seam
  if (seam_type == SEAM_COLS) {
    // Suppression colonne par colonne en partant de la dernière ligne
    for (int i = reduced_img.rows - 1; i >= 0; --i) {
      removePixelAndShiftLeftColor(reduced_img, i, way[k]);
      ++k;
    }
  } else if (seam_type == SEAM_ROWS) {
    // Suppression ligne par ligne en partant de la dernière colonne
    for (int i = reduced_img.cols - 1; i >= 0; --i) {
      removePixelAndShiftUpColor(reduced_img, way[k], i);
      ++k;
    }
  } else {
    // Gestion des erreurs : type de seam invalide
    cerr << "Erreur : Type de seam non valide." << endl;
  }

  return reduced_img;
} // fin suppressionSeamColor

/**
 * Traçage du chemin sur l'image.
 *
 * @param image Image en couleurs à modifier.
 * @param way Tableau contenant les indices du chemin minimum.
 * @param nomImage Nom de l'image.
 * @param repertoire Répertoire.
 * @param seam_type Type de seam (SEAM_ROWS ou SEAM_COLS).
 * @return Image contenant le tracé du chemin minimum.
 */
inline Mat imageSeamed(const Mat &image, const int *way, int seam_type) {
  // Cloner l'image pour préserver l'originale
  Mat seamed_img = image.clone();

  // Initialisation de l'indice pour parcourir le tableau `way`
  int k = 0;

  // Traçage du chemin pour chaque type de seam
  if (seam_type == SEAM_COLS) {
    // Traçage colonne par colonne en partant de la dernière ligne
    for (int i = seamed_img.rows - 1; i >= 0; --i) {
      if (way[k] >= 0 && way[k] < seamed_img.cols) { // Validation des indices
        seamed_img.at<Vec3b>(i, way[k]) =
            Vec3b(0, 0, 255); // Marqueur rouge (0, 0, 255)
      }
      ++k;
    }
  } else if (seam_type == SEAM_ROWS) {
    // Traçage ligne par ligne en partant de la dernière colonne
    for (int i = seamed_img.cols - 1; i >= 0; --i) {
      if (way[k] >= 0 && way[k] < seamed_img.rows) { // Validation des indices
        seamed_img.at<Vec3b>(way[k], i) =
            Vec3b(0, 0, 255); // Marqueur rouge (0, 0, 255)
      }
      ++k;
    }
  } else {
    // Gestion des erreurs : type de seam invalide
    cerr << "Erreur : Type de seam non valide." << endl;
  }

  return seamed_img;
} // Fin imageSeamed

/**
 * Fonction principale qui contient tout l'algorithme du seam carving.
 *
 * @param NB_TOUR Nombre de pixels à retirer.
 * @param seam_type SEAM_ROWS ou SEAM_COLS, pour suppression sur lignes ou
 * colonnes.
 * @return Image redimensionnée.
 */
Mat seamCarving(Mat image, Mat image_gray, int NB_TOUR, const string &nomImage,
                const string &repertoire, int seam_type) {

  // Vérification de la validité de l'image d'entrée
  if (image.empty() || image_gray.empty()) {
    cerr << "Erreur : Les images d'entrée sont vident." << endl;
    return Mat();
  }

  Mat image_reduce = image.clone();
  Mat img_seamed = image.clone();

  // Pré-traitement
  Mat img_gausse = filtreGaussien(image_gray.clone());
  Mat image_gradient = filtreGradient(img_gausse.clone());

  Mat resized_image;

  // Suppression des seams
  if (seam_type == SEAM_COLS) {
    for (int tour = 0; tour < NB_TOUR; ++tour) {
      // Calcul de la matrice cumulative et du chemin minimal
      int **m = matriceCumulativeCols(image_gradient);
      int *way = findWayCols(image_gradient, m);

      // Suppression du chemin dans les différentes images
      image_gradient =
          suppressionSeamGray(image_gradient.clone(), way, SEAM_COLS);
      image_reduce = suppressionSeamColor(image_reduce.clone(), way, SEAM_COLS);
      img_seamed = imageSeamed(img_seamed.clone(), way, SEAM_COLS);

      // Libération de la mémoire
      for (int i = 0; i < image.rows; ++i) {
        free(m[i]);
      }
      free(m);
      free(way);
    }

    // Redimensionnement et enregistrement
    int new_height = image_reduce.rows;
    int new_width = image_reduce.cols - NB_TOUR;
    resized_image = image_reduce(Rect(0, 0, new_width, new_height));

    // Sauvegarde des images
    string fichier_modifie = repertoire + "resized_cols-" + nomImage;
    imwrite(fichier_modifie, resized_image);
    cout << "Image resized et enregistrée: " << fichier_modifie << endl;

    fichier_modifie = repertoire + "seamed_cols-" + nomImage;
    imwrite(fichier_modifie, img_seamed);
    cout << "Image seamed et enregistrée: " << fichier_modifie << endl;

  } else if (seam_type == SEAM_ROWS) {
    for (int tour = 0; tour < NB_TOUR; ++tour) {
      // Calcul de la matrice cumulative et du chemin minimal
      int **m = matriceCumulativeRows(image_gradient);
      int *way = findWayRows(image_gradient, m);

      // Suppression du chemin dans les différentes images
      image_gradient =
          suppressionSeamGray(image_gradient.clone(), way, SEAM_ROWS);
      image_reduce = suppressionSeamColor(image_reduce.clone(), way, SEAM_ROWS);
      img_seamed = imageSeamed(img_seamed.clone(), way, SEAM_ROWS);

      // Libération de la mémoire
      for (int i = 0; i < image.rows; ++i) {
        free(m[i]);
      }
      free(m);
      free(way);
    }

    // Redimensionnement et enregistrement
    int new_height = image_reduce.rows - NB_TOUR;
    int new_width = image_reduce.cols;
    resized_image = image_reduce(Rect(0, 0, new_width, new_height));

    // Sauvegarde des images
    string fichier_modifie = repertoire + "resized_rows-" + nomImage;
    imwrite(fichier_modifie, resized_image);
    cout << "Image resized et enregistrée: " << fichier_modifie << endl;

    fichier_modifie = repertoire + "seamed_rows-" + nomImage;
    imwrite(fichier_modifie, img_seamed);
    cout << "Image seamed et enregistrée: " << fichier_modifie << endl;
  } else {
    cerr << "Erreur : Type de seam invalide." << endl;
  }

  return resized_image;
}
// fin seamCarving
