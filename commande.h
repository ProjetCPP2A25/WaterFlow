#ifndef COMMANDE_H
#define COMMANDE_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Commande
{
private:
    QString id_commande, client, produit, date_commande, adresse, date_prevue, statut;
    int quantite;
    double montant;

public:
    Commande() {}
    Commande(QString id, QString cli, QString prod, int qte, QString dateC, QString adr, QString dateP, QString stat, double mont);

    bool ajouter();
    bool supprimer(QString id);
    bool modifier(QString id);
    QSqlQueryModel* afficher();
};

#endif // COMMANDE_H