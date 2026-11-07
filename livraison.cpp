#include "livraison.h"
#include "commande.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QStringList>
#include <QDebug>
#include <utility>

const QString Livraison::POSTE_LIVREUR = "Livreur";

// Colonnes renvoyées par les SELECT
// 0 ID | 1 Commande | 2 Client | 3 Livreur | 4 Zone | 5 Départ | 6 Statut      (visibles)
// 7 Véhicule | 8 Adr départ | 9 Adr livraison | 10 Arrivée | 11 ID_EMPLOYE  (cachées)
static const QString SELECT_BASE =
    "SELECT L.ID_LIVRAISON, R.ID_COMMANDE, C.CLIENT, "
    "NVL(E.PRENOM || ' ' || E.NOM, L.TRANSPORTEUR_CHAUFFEUR), L.ZONE_GEOGRAPHIQUE, "
    "TO_CHAR(L.DATE_DEPART_PREVUE, 'DD/MM/YYYY HH24:MI'), L.STATUT, "
    "L.VEHICULE, L.ADRESSE_DEPART, L.ADRESSE_LIVRAISON, "
    "TO_CHAR(L.DATE_LIVRAISON_PREVUE, 'DD/MM/YYYY HH24:MI'), L.ID_EMPLOYE "
    "FROM LIVRAISON L "
    "LEFT JOIN EMPLOYE E ON E.ID_EMPLOYE = L.ID_EMPLOYE "
    "LEFT JOIN REGROUPE R ON R.ID_LIVRAISON = L.ID_LIVRAISON "
    "LEFT JOIN COMMANDE C ON C.ID_COMMANDE = R.ID_COMMANDE";

static QSqlQueryModel *creerModele(QSqlQuery &&query)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(std::move(query));
    while (model->canFetchMore()) model->fetchMore();
    const QStringList titres = {"ID", "Commande", "Client", "Livreur", "Zone", "Départ", "Statut",
                                "Véhicule", "Adresse départ", "Adresse livraison", "Livraison prévue",
                                "ID livreur"};
    for (int i = 0; i < titres.size(); ++i)
        model->setHeaderData(i, Qt::Horizontal, titres[i]);
    return model;
}

Livraison::Livraison(int id, int idCommande, int idEmploye, const QString &livreur, const QString &vehicule,
                     const QString &zone, const QString &adrDepart, const QString &adrLivraison,
                     const QDateTime &dateDepart, const QDateTime &dateArrivee, const QString &statut)
    : id(id), idCommande(idCommande), idEmploye(idEmploye), livreur(livreur),
      vehicule(vehicule.trimmed()), zone(zone), adrDepart(adrDepart.trimmed()),
      adrLivraison(adrLivraison.trimmed()), statut(statut),
      dateDepart(dateDepart), dateArrivee(dateArrivee)
{
}

// Statut de la commande qui découle du statut de sa livraison
QString Livraison::statutCommandePour(const QString &s)
{
    if (s == "Planifiée") return "En préparation";
    if (s == "En cours" || s == "Retardée") return "Expédiée";
    if (s == "Livrée") return "Livrée";
    return "En attente";                                   // Échouée : la commande est à relivrer
}

// ---------- CONTRÔLE DE SAISIE ----------
QString Livraison::verifier() const
{
    if (idEmploye <= 0)
        return "Choisissez un livreur (employé au poste « " + POSTE_LIVREUR + " » dans Personnel) !";
    if (idCommande <= 0)
        return "Choisissez une commande à livrer !";
    if (vehicule.isEmpty() || vehicule.size() > 100)
        return "Entrez l'immatriculation du véhicule (100 caractères max) !";
    if (adrDepart.isEmpty() || adrDepart.size() > 200)
        return "Entrez l'adresse de départ (200 caractères max) !";
    if (adrLivraison.isEmpty() || adrLivraison.size() > 200)
        return "Entrez l'adresse de livraison (200 caractères max) !";
    if (!dateDepart.isValid() || !dateArrivee.isValid() || dateArrivee <= dateDepart)
        return "La livraison prévue doit être après le départ !";
    return QString();
}

// Un livreur ne peut pas avoir deux livraisons actives qui se chevauchent
bool Livraison::livreurOccupe()
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM LIVRAISON WHERE ID_EMPLOYE = :e AND ID_LIVRAISON <> :id "
              "AND STATUT IN ('Planifiée', 'En cours', 'Retardée') "
              "AND DATE_DEPART_PREVUE < :fin AND DATE_LIVRAISON_PREVUE > :debut");
    q.bindValue(":e", idEmploye);
    q.bindValue(":id", id);
    q.bindValue(":fin", dateArrivee);
    q.bindValue(":debut", dateDepart);
    return q.exec() && q.next() && q.value(0).toInt() > 0;
}

// ---------- CREATE / UPDATE (transaction : LIVRAISON + REGROUPE + COMMANDE) ----------
bool Livraison::enregistrer(bool creation)
{
    erreurSql.clear();
    if (livreurOccupe()) {
        erreurSql = livreur + " a déjà une livraison active sur ce créneau.";
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    QSqlQuery q;
    auto echec = [&](const QString &etape) {
        erreurSql = etape + " : " + q.lastError().text();
        qDebug() << "Livraison::enregistrer" << erreurSql;
        db.rollback();
        if (creation) id = 0;
        return false;
    };

    int ancienneCommande = 0;
    if (creation) {
        if (!q.exec("SELECT GREATEST(NVL((SELECT MAX(ID_LIVRAISON) FROM LIVRAISON), 0), "
                    "NVL((SELECT MAX(ID_LIVRAISON) FROM REGROUPE), 0)) + 1 FROM DUAL") || !q.next())
            return echec("Génération de l'identifiant");
        id = q.value(0).toInt();
        q.prepare("INSERT INTO LIVRAISON (ID_LIVRAISON, ID_EMPLOYE, TRANSPORTEUR_CHAUFFEUR, VEHICULE, "
                  "ZONE_GEOGRAPHIQUE, ADRESSE_DEPART, ADRESSE_LIVRAISON, DATE_DEPART_PREVUE, "
                  "DATE_LIVRAISON_PREVUE, STATUT, DATE_LIVRAISON_REELLE) "
                  "VALUES (:id, :emp, :livreur, :vehicule, :zone, :adrdep, :adrliv, :ddep, :darr, :statut, "
                  "CASE WHEN :st2 = 'Livrée' THEN SYSTIMESTAMP END)");
    } else {
        q.prepare("SELECT ID_COMMANDE FROM REGROUPE WHERE ID_LIVRAISON = :id");
        q.bindValue(":id", id);
        if (q.exec() && q.next()) ancienneCommande = q.value(0).toInt();

        q.prepare("UPDATE LIVRAISON SET ID_EMPLOYE = :emp, TRANSPORTEUR_CHAUFFEUR = :livreur, "
                  "VEHICULE = :vehicule, ZONE_GEOGRAPHIQUE = :zone, ADRESSE_DEPART = :adrdep, "
                  "ADRESSE_LIVRAISON = :adrliv, DATE_DEPART_PREVUE = :ddep, "
                  "DATE_LIVRAISON_PREVUE = :darr, STATUT = :statut, "
                  "DATE_LIVRAISON_REELLE = CASE WHEN :st2 = 'Livrée' "
                  "THEN NVL(DATE_LIVRAISON_REELLE, SYSTIMESTAMP) END "
                  "WHERE ID_LIVRAISON = :id");
    }
    q.bindValue(":id", id);
    q.bindValue(":emp", idEmploye);
    q.bindValue(":livreur", livreur);
    q.bindValue(":vehicule", vehicule);
    q.bindValue(":zone", zone);
    q.bindValue(":adrdep", adrDepart);
    q.bindValue(":adrliv", adrLivraison);
    q.bindValue(":ddep", dateDepart);
    q.bindValue(":darr", dateArrivee);
    q.bindValue(":statut", statut);
    q.bindValue(":st2", statut);
    if (!q.exec())
        return echec(creation ? "Ajout de la livraison" : "Modification de la livraison");
    if (!creation && q.numRowsAffected() == 0) {
        erreurSql = "Livraison introuvable.";
        db.rollback();
        return false;
    }

    // Lien avec la commande (table d'association REGROUPE)
    if (ancienneCommande != idCommande) {
        if (ancienneCommande > 0) {
            q.prepare("DELETE FROM REGROUPE WHERE ID_LIVRAISON = :id");
            q.bindValue(":id", id);
            if (!q.exec()) return echec("Lien commande");
            Commande::mettreAJourLivraison(ancienneCommande, "En attente", QString());
        }
        q.prepare("INSERT INTO REGROUPE (ID_LIVRAISON, ID_COMMANDE) VALUES (:id, :cmd)");
        q.bindValue(":id", id);
        q.bindValue(":cmd", idCommande);
        if (!q.exec()) return echec("Lien commande");
    }

    // La commande suit sa livraison : statut + transporteur assigné
    if (!Commande::mettreAJourLivraison(idCommande, statutCommandePour(statut), livreur)) {
        erreurSql = "Mise à jour de la commande impossible.";
        db.rollback();
        return false;
    }

    if (!db.commit()) {
        erreurSql = db.lastError().text();
        db.rollback();
        return false;
    }
    return true;
}

bool Livraison::ajouter()  { return enregistrer(true); }
bool Livraison::modifier() { return enregistrer(false); }

// ---------- READ ----------
QSqlQueryModel *Livraison::afficher()
{
    return rechercher(QString(), QString());
}

// ---------- DELETE ----------
bool Livraison::supprimer(int idLiv)
{
    erreurSql.clear();
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery q;

    int idCmd = 0;
    QString st;
    q.prepare("SELECT R.ID_COMMANDE, L.STATUT FROM LIVRAISON L "
              "LEFT JOIN REGROUPE R ON R.ID_LIVRAISON = L.ID_LIVRAISON WHERE L.ID_LIVRAISON = :id");
    q.bindValue(":id", idLiv);
    if (q.exec() && q.next()) { idCmd = q.value(0).toInt(); st = q.value(1).toString(); }

    db.transaction();
    q.prepare("DELETE FROM REGROUPE WHERE ID_LIVRAISON = :id");
    q.bindValue(":id", idLiv);
    bool ok = q.exec();
    if (ok) {
        q.prepare("DELETE FROM LIVRAISON WHERE ID_LIVRAISON = :id");
        q.bindValue(":id", idLiv);
        ok = q.exec() && q.numRowsAffected() > 0;
    }
    // Commande non livrée : elle redevient « En attente », sans transporteur
    if (ok && idCmd > 0 && st != "Livrée")
        ok = Commande::mettreAJourLivraison(idCmd, "En attente", QString());

    if (!ok) {
        erreurSql = q.lastError().text();
        db.rollback();
        return false;
    }
    return db.commit();
}

// ---------- RECHERCHE (livreur / zone / client + filtre statut) ----------
QSqlQueryModel *Livraison::rechercher(const QString &texte, const QString &statutFiltre)
{
    const QString t = texte.trimmed();
    const bool filtreStatut = !statutFiltre.isEmpty() && statutFiltre != "Tous les statuts";

    QStringList conditions;
    if (!t.isEmpty())
        conditions << "(LOWER(E.PRENOM || ' ' || E.NOM) LIKE :t1 OR LOWER(L.ZONE_GEOGRAPHIQUE) LIKE :t2 "
                      "OR LOWER(C.CLIENT) LIKE :t3)";
    if (filtreStatut)
        conditions << "L.STATUT = :statut";

    QString sql = SELECT_BASE;
    if (!conditions.isEmpty())
        sql += " WHERE " + conditions.join(" AND ");
    sql += " ORDER BY L.ID_LIVRAISON";

    QSqlQuery query;
    query.prepare(sql);
    if (!t.isEmpty()) {
        const QString motif = "%" + t.toLower() + "%";
        query.bindValue(":t1", motif);
        query.bindValue(":t2", motif);
        query.bindValue(":t3", motif);
    }
    if (filtreStatut)
        query.bindValue(":statut", statutFiltre);

    if (!query.exec())
        qDebug() << "rechercher :" << query.lastError().text();
    return creerModele(std::move(query));
}

// ---------- Données venant du module Personnel ----------
QList<ChoixListe> Livraison::listeLivreurs()
{
    QList<ChoixListe> l;
    QSqlQuery q;
    q.prepare("SELECT ID_EMPLOYE, PRENOM || ' ' || NOM, TELEPHONE FROM EMPLOYE "
              "WHERE LOWER(TRIM(POSTE)) = LOWER(:poste) AND STATUT = 'Actif' ORDER BY NOM, PRENOM");
    q.bindValue(":poste", POSTE_LIVREUR);
    if (!q.exec()) qDebug() << "listeLivreurs :" << q.lastError().text();
    while (q.next())
        l.append({q.value(0).toInt(), q.value(1).toString(), q.value(2).toString()});
    return l;
}

// ---------- Données venant du module Commandes ----------
// Commandes pas encore livrées/annulées et sans livraison, + celle de la livraison en cours d'édition
QList<ChoixListe> Livraison::listeCommandes(int idCommandeEnCours)
{
    QList<ChoixListe> l;
    QSqlQuery q;
    q.prepare("SELECT C.ID_COMMANDE, C.CLIENT, C.ADRESSE_LIVRAISON FROM COMMANDE C "
              "WHERE (C.STATUT NOT IN ('Livrée', 'Annulée') "
              "       AND NOT EXISTS (SELECT 1 FROM REGROUPE R JOIN LIVRAISON L "
              "                       ON L.ID_LIVRAISON = R.ID_LIVRAISON "
              "                       WHERE R.ID_COMMANDE = C.ID_COMMANDE)) "
              "   OR C.ID_COMMANDE = :enCours "
              "ORDER BY C.ID_COMMANDE");
    q.bindValue(":enCours", idCommandeEnCours);
    if (!q.exec()) qDebug() << "listeCommandes :" << q.lastError().text();
    while (q.next())
        l.append({q.value(0).toInt(),
                  QString("n° %1 — %2").arg(q.value(0).toString(), q.value(1).toString()),
                  q.value(2).toString()});
    return l;
}
