#pragma once
#include <QString>
#include <QDateTime>
#include <QList>
#include <QSqlQueryModel>

// Élément d'une liste déroulante (livreur ou commande)
struct ChoixListe {
    int id;
    QString libelle;
    QString extra;      // livreur : téléphone ; commande : adresse de livraison
};

// Classe purement C++ (Modèle) : gère les données et les requêtes SQL de l'entité Livraison.
// Liens : LIVRAISON.ID_EMPLOYE -> EMPLOYE (le livreur)
//         REGROUPE(ID_LIVRAISON, ID_COMMANDE) -> COMMANDE
class Livraison
{
    int id = 0;
    int idCommande = 0;
    int idEmploye = 0;
    QString livreur;                 // « Prénom Nom », recopié dans TRANSPORTEUR_CHAUFFEUR
    QString vehicule, zone, adrDepart, adrLivraison, statut;
    QDateTime dateDepart, dateArrivee;
    QString erreurSql;

public:
    static const QString POSTE_LIVREUR;          // "Livreur"

    Livraison() {}
    Livraison(int id, int idCommande, int idEmploye, const QString &livreur, const QString &vehicule,
              const QString &zone, const QString &adrDepart, const QString &adrLivraison,
              const QDateTime &dateDepart, const QDateTime &dateArrivee, const QString &statut);

    QString getErreurSql() const       { return erreurSql; }
    int getId() const                  { return id; }
    int getIdCommande() const          { return idCommande; }
    int getIdEmploye() const           { return idEmploye; }
    QString getStatut() const          { return statut; }

    // Contrôle de saisie : renvoie le message d'erreur, ou "" si tout est valide
    QString verifier() const;

    // CRUD
    bool ajouter();
    QSqlQueryModel *afficher();
    bool modifier();
    bool supprimer(int id);
    QSqlQueryModel *rechercher(const QString &texte, const QString &statut);

    // Données des deux autres modules
    static QList<ChoixListe> listeLivreurs();                       // Personnel : livreurs actifs
    static QList<ChoixListe> listeCommandes(int idCommandeEnCours); // Commandes à livrer (+ celle en cours)
    static QString statutCommandePour(const QString &statutLivraison);

private:
    bool livreurOccupe();                     // chevauchement avec une autre livraison active
    bool enregistrer(bool creation);          // INSERT ou UPDATE dans une transaction
};
