#ifndef COMMANDE_H
#define COMMANDE_H

#include <QString>
#include <QDate>
#include <QSqlQueryModel>

// Modèle de l'entité COMMANDE (table Oracle du schéma wf).
// Colonnes renvoyées par afficher() :
//   0 ID | 1 Client | 2 Produits | 3 Qté | 4 Date cmd (yyyy-MM-dd) | 5 Adresse
//   6 Date prévue (yyyy-MM-dd) | 7 Statut | 8 Montant | 9 Transporteur
class Commande
{
    int id = 0;
    QString client, produits, adresse, statut, transporteur;
    int quantite = 0;
    QDate dateCommande, datePrevue;
    double montant = 0.0;
    QString m_erreur;

public:
    Commande() {}
    Commande(int id, QString client, QString produits, int quantite, QDate dateCommande,
             QString adresse, QDate datePrevue, QString statut, double montant,
             QString transporteur = QString());

    int getId() const { return id; }
    QString erreur() const { return m_erreur; }

    bool ajouter();                // id <= 0 -> identifiant généré automatiquement
    bool modifier();
    bool supprimer(int id);

    // Lecture avec recherche (client / produit / adresse) et filtre statut.
    static QSqlQueryModel *afficher(const QString &texte = QString(),
                                    const QString &statut = QString(),
                                    bool triDateDesc = false);
    static int prochainId();

    // Utilisés par le module Livraisons
    static bool mettreAJourLivraison(int idCommande, const QString &statut, const QString &transporteur);
};

#endif // COMMANDE_H
