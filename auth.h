#ifndef AUTH_H
#define AUTH_H

#include <QString>

// Gestion des comptes et des droits d'accès par département.
//
// Les numéros de section correspondent aux lignes du menu latéral (personnel.ui) :
//   0 Tableau de bord   1 Personnel   2 Équipements   3 Production & stocks
//   4 Commandes         5 Livraisons  6 Statistiques  7 Alertes
//
// Les comptes sont déclarés dans auth.cpp (un seul endroit à modifier).
class Session
{
public:
    // Vérifie l'e-mail + mot de passe. Retourne true et ouvre la session si valide.
    static bool login(const QString &email, const QString &password);
    static void logout();

    static bool isLoggedIn();
    static bool isAdmin();
    static QString nom();                 // nom affiché (ex. "Dhia")
    static QString email();

    // true si l'utilisateur connecté a le droit d'ouvrir cette section du menu.
    static bool canAccess(int menuRow);

    // Libellé d'une section du menu (ex. 1 -> "Personnel").
    static QString nomSection(int menuRow);
};

#endif // AUTH_H
