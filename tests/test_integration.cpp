// Test d'intégration : Personnel -> Commandes -> Livraisons, via les vraies classes et ODBC.
#include <QCoreApplication>
#include <QSqlQuery>
#include <QDebug>
#include "../connection.h"
#include "../commande.h"
#include "../employe.h"
#include "../livraison.h"

static int echecs = 0;
static void verifier(bool ok, const QString &msg) {
    qInfo().noquote() << (ok ? "  OK    " : "  ECHEC ") + msg;
    if (!ok) ++echecs;
}
static QString val(const QString &sql) {
    QSqlQuery q; q.exec(sql); return q.next() ? q.value(0).toString() : QString("<aucun>");
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    verifier(Connection::createInstance().createConnection(), "Connexion Oracle");

    qInfo() << "1) Personnel : nouveau livreur";
    Employe e(0, "TESTEUR", "Livreur", "Livreur", "Logistique", QDate(2025, 1, 1),
              "22 000 000", "testeur@waterflow.tn", "Matin", "Actif");
    verifier(e.ajouter(), "Employe::ajouter (poste Livreur) " + e.erreur());
    bool trouve = false;
    for (const ChoixListe &c : Livraison::listeLivreurs()) if (c.id == e.getID()) trouve = true;
    verifier(trouve, "le livreur apparaît dans la liste des livreurs");
    bool congeVisible = false;
    for (const ChoixListe &c : Livraison::listeLivreurs()) if (c.id == 6) congeVisible = true;
    verifier(!congeVisible, "le livreur en congé (id 6) n'apparaît pas");

    qInfo() << "2) Commandes : nouvelle commande « guiza »";
    Commande c(0, "guiza", "maryoul", 10, QDate::currentDate(), "Tunis, Bardo",
               QDate::currentDate().addDays(2), "En attente", 150.5);
    verifier(c.ajouter(), "Commande::ajouter " + c.erreur());
    trouve = false;
    for (const ChoixListe &x : Livraison::listeCommandes(0)) if (x.id == c.getId()) trouve = true;
    verifier(trouve, "la commande apparaît dans la liste des commandes à livrer");
    Commande ca(0, "Test accents", "Bonbonne 19L", 5, QDate::currentDate(), "Sfax",
                QDate::currentDate().addDays(1), "En préparation", 50);
    verifier(ca.ajouter(), "statut accentué « En préparation » accepté " + ca.erreur());

    qInfo() << "3) Livraisons : création";
    const QDateTime dep(QDate::currentDate().addDays(1), QTime(8, 0));
    Livraison L(0, c.getId(), e.getID(), "Livreur Testeur", "999 TUN 0001", "Tunis Nord",
                "Dépôt WaterFlow", "Tunis, Bardo", dep, dep.addSecs(3 * 3600), "Planifiée");
    verifier(L.verifier().isEmpty(), "contrôle de saisie " + L.verifier());
    verifier(L.ajouter(), "Livraison::ajouter (statut Planifiée) " + L.getErreurSql());
    const QString id = QString::number(L.getId()), idc = QString::number(c.getId());
    verifier(val("SELECT COUNT(*) FROM REGROUPE WHERE ID_LIVRAISON=" + id + " AND ID_COMMANDE=" + idc) == "1",
             "lien REGROUPE créé");
    verifier(val("SELECT STATUT FROM COMMANDE WHERE ID_COMMANDE=" + idc) == "En préparation",
             "commande passée « En préparation »");
    verifier(val("SELECT TRANSPORTEUR_ASSIGNE FROM COMMANDE WHERE ID_COMMANDE=" + idc) == "Livreur Testeur",
             "transporteur de la commande = livreur");
    trouve = false;
    for (const ChoixListe &x : Livraison::listeCommandes(0)) if (x.id == c.getId()) trouve = true;
    verifier(!trouve, "la commande n'est plus proposée pour une autre livraison");

    qInfo() << "4) Règles métier";
    Livraison L2(0, 3, e.getID(), "Livreur Testeur", "999 TUN 0001", "Ariana", "Dépôt", "Ariana",
                 dep.addSecs(3600), dep.addSecs(5 * 3600), "Planifiée");
    verifier(!L2.ajouter(), "créneau qui chevauche refusé : " + L2.getErreurSql());
    verifier(!c.supprimer(c.getId()), "commande liée à une livraison non supprimable : " + c.erreur());

    qInfo() << "5) Livraisons : modification -> Livrée";
    Livraison L3(L.getId(), c.getId(), e.getID(), "Livreur Testeur", "999 TUN 0001", "Tunis Nord",
                 "Dépôt WaterFlow", "Tunis, Bardo", dep, dep.addSecs(3 * 3600), "Livrée");
    verifier(L3.modifier(), "Livraison::modifier " + L3.getErreurSql());
    verifier(val("SELECT STATUT FROM COMMANDE WHERE ID_COMMANDE=" + idc) == "Livrée", "commande passée « Livrée »");
    verifier(val("SELECT COUNT(*) FROM LIVRAISON WHERE ID_LIVRAISON=" + id + " AND DATE_LIVRAISON_REELLE IS NOT NULL") == "1",
             "date de livraison réelle remplie");
    Livraison L4(L.getId(), c.getId(), e.getID(), "Livreur Testeur", "999 TUN 0001", "Tunis Nord",
                 "Dépôt WaterFlow", "Tunis, Bardo", dep, dep.addSecs(3 * 3600), "Échouée");
    verifier(L4.modifier(), "statut « Échouée » accepté " + L4.getErreurSql());
    verifier(val("SELECT STATUT FROM COMMANDE WHERE ID_COMMANDE=" + idc) == "En attente", "commande repassée « En attente »");

    qInfo() << "6) Lecture (tableau Livraisons)";
    QSqlQueryModel *m = L.rechercher("testeur", "");
    verifier(m->rowCount() == 1 && m->index(0, 2).data().toString() == "guiza",
             "recherche par livreur -> 1 ligne, client guiza");
    delete m;

    qInfo() << "7) Nettoyage";
    Livraison tmp;
    verifier(tmp.supprimer(L.getId()), "Livraison::supprimer " + tmp.getErreurSql());
    verifier(val("SELECT COUNT(*) FROM REGROUPE WHERE ID_LIVRAISON=" + id) == "0", "lien REGROUPE supprimé");
    verifier(c.supprimer(c.getId()), "Commande::supprimer (plus liée) " + c.erreur());
    verifier(ca.supprimer(ca.getId()), "suppression commande accents");
    verifier(e.supprimer(e.getID()), "Employe::supprimer " + e.erreur());

    qInfo().noquote() << QString("\n%1 échec(s)").arg(echecs);
    return echecs ? 1 : 0;
}
