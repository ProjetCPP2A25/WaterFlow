#include "commande.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <QVariant>
#include <QDebug>

Commande::Commande(int id, QString client, QString produits, int quantite, QDate dateCommande,
                   QString adresse, QDate datePrevue, QString statut, double montant,
                   QString transporteur)
    : id(id), client(client.trimmed()), produits(produits.trimmed()), adresse(adresse.trimmed()),
      statut(statut), transporteur(transporteur.trimmed()), quantite(quantite),
      dateCommande(dateCommande), datePrevue(datePrevue), montant(montant)
{
}

int Commande::prochainId()
{
    QSqlQuery q;
    if (q.exec("SELECT NVL(MAX(ID_COMMANDE), 0) + 1 FROM COMMANDE") && q.next())
        return q.value(0).toInt();
    return -1;
}

// ---------------------------------------------------------------- CREATE
bool Commande::ajouter()
{
    m_erreur.clear();
    if (id <= 0) {
        id = prochainId();
        if (id <= 0) { m_erreur = "Impossible de générer l'identifiant."; return false; }
    }

    QSqlQuery q;
    q.prepare("INSERT INTO COMMANDE (ID_COMMANDE, CLIENT, DATE_COMMANDE, PRODUITS_COMMANDES, QUANTITE, "
              "ADRESSE_LIVRAISON, DATE_LIVRAISON_PREVUE, STATUT, TRANSPORTEUR_ASSIGNE, MONTANT) "
              "VALUES (:id, :client, :dcmd, :produits, :qte, :adresse, :dprev, :statut, :transp, :montant)");
    q.bindValue(":id", id);
    q.bindValue(":client", client);
    q.bindValue(":dcmd", QDateTime(dateCommande, QTime(0, 0)));
    q.bindValue(":produits", produits);
    q.bindValue(":qte", quantite);
    q.bindValue(":adresse", adresse);
    q.bindValue(":dprev", QDateTime(datePrevue, QTime(0, 0)));
    q.bindValue(":statut", statut);
    q.bindValue(":transp", transporteur);
    q.bindValue(":montant", montant);

    if (!q.exec()) { m_erreur = q.lastError().text(); return false; }
    return true;
}

// ---------------------------------------------------------------- UPDATE
bool Commande::modifier()
{
    m_erreur.clear();
    QSqlQuery q;
    q.prepare("UPDATE COMMANDE SET CLIENT=:client, DATE_COMMANDE=:dcmd, PRODUITS_COMMANDES=:produits, "
              "QUANTITE=:qte, ADRESSE_LIVRAISON=:adresse, DATE_LIVRAISON_PREVUE=:dprev, STATUT=:statut, "
              "TRANSPORTEUR_ASSIGNE=:transp, MONTANT=:montant WHERE ID_COMMANDE=:id");
    q.bindValue(":client", client);
    q.bindValue(":dcmd", QDateTime(dateCommande, QTime(0, 0)));
    q.bindValue(":produits", produits);
    q.bindValue(":qte", quantite);
    q.bindValue(":adresse", adresse);
    q.bindValue(":dprev", QDateTime(datePrevue, QTime(0, 0)));
    q.bindValue(":statut", statut);
    q.bindValue(":transp", transporteur);
    q.bindValue(":montant", montant);
    q.bindValue(":id", id);

    if (!q.exec()) { m_erreur = q.lastError().text(); return false; }
    if (q.numRowsAffected() == 0) { m_erreur = "Commande introuvable."; return false; }
    return true;
}

// ---------------------------------------------------------------- DELETE
bool Commande::supprimer(int idCmd)
{
    m_erreur.clear();
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM REGROUPE R JOIN LIVRAISON L ON L.ID_LIVRAISON = R.ID_LIVRAISON "
              "WHERE R.ID_COMMANDE = :id");
    q.bindValue(":id", idCmd);
    if (q.exec() && q.next() && q.value(0).toInt() > 0) {
        m_erreur = "Cette commande est liée à une livraison : supprimez d'abord la livraison.";
        return false;
    }

    q.prepare("DELETE FROM REGROUPE WHERE ID_COMMANDE = :id");   // liens orphelins éventuels
    q.bindValue(":id", idCmd);
    q.exec();

    q.prepare("DELETE FROM COMMANDE WHERE ID_COMMANDE = :id");
    q.bindValue(":id", idCmd);
    if (!q.exec()) { m_erreur = q.lastError().text(); return false; }
    if (q.numRowsAffected() == 0) { m_erreur = "Commande introuvable."; return false; }
    return true;
}

// ---------------------------------------------------------------- READ
QSqlQueryModel *Commande::afficher(const QString &texte, const QString &statut, bool triDateDesc)
{
    QString sql =
        "SELECT ID_COMMANDE, CLIENT, PRODUITS_COMMANDES, QUANTITE, "
        "TO_CHAR(DATE_COMMANDE, 'YYYY-MM-DD'), ADRESSE_LIVRAISON, "
        "TO_CHAR(DATE_LIVRAISON_PREVUE, 'YYYY-MM-DD'), STATUT, MONTANT, TRANSPORTEUR_ASSIGNE "
        "FROM COMMANDE";

    const QString t = texte.trimmed();
    const bool filtreStatut = !statut.isEmpty() && statut != "Tous les statuts";
    QStringList cond;
    if (!t.isEmpty())
        cond << "(LOWER(CLIENT) LIKE :t1 OR LOWER(PRODUITS_COMMANDES) LIKE :t2 "
                "OR LOWER(ADRESSE_LIVRAISON) LIKE :t3 OR TO_CHAR(ID_COMMANDE) LIKE :t4)";
    if (filtreStatut)
        cond << "STATUT = :statut";
    if (!cond.isEmpty())
        sql += " WHERE " + cond.join(" AND ");
    sql += triDateDesc ? " ORDER BY DATE_COMMANDE DESC, ID_COMMANDE" : " ORDER BY ID_COMMANDE";

    QSqlQuery q;
    q.prepare(sql);
    if (!t.isEmpty()) {
        const QString motif = "%" + t.toLower() + "%";
        q.bindValue(":t1", motif); q.bindValue(":t2", motif);
        q.bindValue(":t3", motif); q.bindValue(":t4", motif);
    }
    if (filtreStatut)
        q.bindValue(":statut", statut);
    if (!q.exec())
        qDebug() << "Commande::afficher :" << q.lastError().text();

    auto *model = new QSqlQueryModel();
    model->setQuery(std::move(q));
    while (model->canFetchMore()) model->fetchMore();

    const QStringList titres = {"ID", "Client", "Produits", "Qté", "Date Cmd", "Adresse",
                                "Date Prév", "Statut", "Montant", "Transporteur"};
    for (int i = 0; i < titres.size(); ++i)
        model->setHeaderData(i, Qt::Horizontal, titres[i]);
    return model;
}

// Appelée quand une livraison change : statut de la commande + transporteur (livreur).
bool Commande::mettreAJourLivraison(int idCommande, const QString &statutCmd, const QString &transp)
{
    QSqlQuery q;
    if (statutCmd.isEmpty()) {
        q.prepare("UPDATE COMMANDE SET TRANSPORTEUR_ASSIGNE = :t WHERE ID_COMMANDE = :id");
    } else {
        q.prepare("UPDATE COMMANDE SET TRANSPORTEUR_ASSIGNE = :t, STATUT = :s WHERE ID_COMMANDE = :id");
        q.bindValue(":s", statutCmd);
    }
    q.bindValue(":t", transp);
    q.bindValue(":id", idCommande);
    if (!q.exec()) {
        qDebug() << "Commande::mettreAJourLivraison :" << q.lastError().text();
        return false;
    }
    return true;
}
