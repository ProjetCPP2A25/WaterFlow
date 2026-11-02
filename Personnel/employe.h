#ifndef EMPLOYE_H
#define EMPLOYE_H

#include <QString>
#include <QDate>
#include <QSqlQuery>
#include <QSqlQueryModel>

// Classe purement C++ (Modèle) : toutes les requêtes SQL de l'entité EMPLOYE sont ici.
class Employe
{
    int id = 0;
    QString nom, prenom, poste, departement, telephone, mail, shift, statut;
    QDate dateEmbauche;
    QString m_erreur;                 // dernier message d'erreur SQL

public:
    // Constructeurs
    Employe() {}
    Employe(int id, QString nom, QString prenom, QString poste, QString departement,
            QDate dateEmbauche, QString telephone, QString mail, QString shift, QString statut);

    // Getters
    int getID() const { return id; }
    QString getNom() const { return nom; }
    QString getPrenom() const { return prenom; }
    QString getPoste() const { return poste; }
    QString getDepartement() const { return departement; }
    QDate getDateEmbauche() const { return dateEmbauche; }
    QString getTelephone() const { return telephone; }
    QString getMail() const { return mail; }
    QString getShift() const { return shift; }
    QString getStatut() const { return statut; }
    QString getMatricule() const;     // ex. EMP-001

    // Setters
    void setID(int i) { id = i; }
    void setNom(QString n) { nom = n; }
    void setPrenom(QString p) { prenom = p; }
    void setPoste(QString p) { poste = p; }
    void setDepartement(QString d) { departement = d; }
    void setDateEmbauche(QDate d) { dateEmbauche = d; }
    void setTelephone(QString t) { telephone = t; }
    void setMail(QString m) { mail = m; }
    void setShift(QString s) { shift = s; }
    void setStatut(QString s) { statut = s; }

    // Fonctionnalités de base relatives à l'entité Employe (CRUD)
    bool ajouter();                   // Create
    QSqlQueryModel *afficher();       // Read
    bool modifier();                  // Update
    bool supprimer(int id);           // Delete

    // Utilitaires
    bool existeDeja() const;          // même nom + prénom pour un autre id
    static int prochainId();          // prochain identifiant libre (-1 si erreur)
    QString erreur() const { return m_erreur; }
};

#endif // EMPLOYE_H
