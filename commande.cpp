#include "commande.h"
#include "connection.h"
#include <QSqlRecord>
#include <QSqlDatabase>
#include <QDebug>

Commande::Commande(QString id, QString cli, QString prod, int qte, QString dateC, QString adr, QString dateP, QString stat, double mont)
{
    id_commande = id;
    client = cli;
    produit = prod;
    quantite = qte;
    date_commande = dateC;
    adresse = adr;
    date_prevue = dateP;
    statut = stat;
    montant = mont;
}

bool Commande::ajouter()
{
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("INSERT INTO COMMANDE (ID_COMMANDE, CLIENT, PRODUIT, QUANTITE, DATE_COMMANDE, ADRESSE, DATE_PREVUE, STATUT, MONTANT) "
                  "VALUES (:id, :client, :produit, :qte, :dateCmd, :adresse, :datePrev, :statut, :montant)");
    query.bindValue(":id", id_commande);
    query.bindValue(":client", client);
    query.bindValue(":produit", produit);
    query.bindValue(":qte", quantite);
    query.bindValue(":dateCmd", date_commande);
    query.bindValue(":adresse", adresse);
    query.bindValue(":datePrev", date_prevue);
    query.bindValue(":statut", statut);
    query.bindValue(":montant", montant);

    if(query.exec()) { query.exec("COMMIT"); return true; }
    return false;
}

bool Commande::supprimer(QString id)
{
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("DELETE FROM COMMANDE WHERE ID_COMMANDE = :id");
    query.bindValue(":id", id);
    if(query.exec()) { query.exec("COMMIT"); return true; }
    return false;
}

bool Commande::modifier(QString id)
{
    QSqlQuery query(Connection::createInstance().getDatabase());
    query.prepare("UPDATE COMMANDE SET CLIENT=:client, PRODUIT=:produit, QUANTITE=:qte, "
                  "DATE_COMMANDE=:dateCmd, ADRESSE=:adresse, DATE_PREVUE=:datePrev, STATUT=:statut, MONTANT=:montant "
                  "WHERE ID_COMMANDE=:id");
    query.bindValue(":client", client);
    query.bindValue(":produit", produit);
    query.bindValue(":qte", quantite);
    query.bindValue(":dateCmd", date_commande);
    query.bindValue(":adresse", adresse);
    query.bindValue(":datePrev", date_prevue);
    query.bindValue(":statut", statut);
    query.bindValue(":montant", montant);
    query.bindValue(":id", id);
    if(query.exec()) { query.exec("COMMIT"); return true; }
    return false;
}

// On ne l'utilise plus, mais on la garde pour la compilation
QSqlQueryModel* Commande::afficher()
{
    return nullptr;
}