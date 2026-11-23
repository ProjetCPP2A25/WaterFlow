#include "employe.h"
#include <QSqlError>
#include <QVariant>

Employe::Employe(int id, QString nom, QString prenom, QString poste, QString departement,
                 QDate dateEmbauche, QString telephone, QString mail, QString shift, QString statut)
{
    this->id = id;
    this->nom = nom;
    this->prenom = prenom;
    this->poste = poste;
    this->departement = departement;
    this->dateEmbauche = dateEmbauche;
    this->telephone = telephone;
    this->mail = mail;
    this->shift = shift;
    this->statut = statut;
}

QString Employe::getMatricule() const
{
    return QString("EMP-%1").arg(id, 3, 10, QChar('0'));
}

int Employe::prochainId()
{
    QSqlQuery query;
    if (query.exec("SELECT NVL(MAX(id_employe), 0) + 1 FROM EMPLOYE") && query.next())
        return query.value(0).toInt();
    return -1;
}

// ---------------------------------------------------------------- CREATE
bool Employe::ajouter()
{
    m_erreur.clear();

    const bool idGenere = (id <= 0);
    if (idGenere) {
        id = prochainId();
        if (id <= 0) {
            id = 0;
            m_erreur = "Impossible de générer l'identifiant de l'employé.";
            return false;
        }
    }

    QSqlQuery query;
    query.prepare("INSERT INTO EMPLOYE "
                  "(id_employe, nom, prenom, poste, departement, date_embauche, "
                  "telephone, mail, shift, statut) "
                  "VALUES (:id, :nom, :prenom, :poste, :departement, :date_embauche, "
                  ":telephone, :mail, :shift, :statut)");

    // Création des variables liées
    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":poste", poste);
    query.bindValue(":departement", departement);
    query.bindValue(":date_embauche", QVariant(dateEmbauche));
    query.bindValue(":telephone", telephone);
    query.bindValue(":mail", mail);
    query.bindValue(":shift", shift);
    query.bindValue(":statut", statut);

    if (!query.exec()) {
        m_erreur = query.lastError().text();
        if (idGenere)
            id = 0;
        return false;
    }
    return true;
}

// ---------------------------------------------------------------- READ
QSqlQueryModel *Employe::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    // Colonnes 0..7 : affichées ; 8..10 : utilisées par le formulaire (masquées dans la vue).
    model->setQuery(
        "SELECT 'EMP-' || CASE WHEN id_employe < 1000 "
        "THEN LPAD(TO_CHAR(id_employe), 3, '0') ELSE TO_CHAR(id_employe) END AS matricule, "
        "nom, prenom, poste, departement, shift, statut, mail, "
        "date_embauche, telephone, id_employe "
        "FROM EMPLOYE ORDER BY id_employe");

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Poste"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Département"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Shift"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Statut"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("E-mail"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("Date d'embauche"));
    model->setHeaderData(9, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(10, Qt::Horizontal, QObject::tr("N° interne"));

    // Charge toutes les lignes (certains pilotes ODBC les livrent par paquets).
    while (model->canFetchMore())
        model->fetchMore();

    return model;
}

// ---------------------------------------------------------------- UPDATE
bool Employe::modifier()
{
    m_erreur.clear();

    QSqlQuery query;
    query.prepare("UPDATE EMPLOYE SET nom = :nom, prenom = :prenom, poste = :poste, "
                  "departement = :departement, date_embauche = :date_embauche, "
                  "telephone = :telephone, mail = :mail, shift = :shift, statut = :statut "
                  "WHERE id_employe = :id");

    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":poste", poste);
    query.bindValue(":departement", departement);
    query.bindValue(":date_embauche", QVariant(dateEmbauche));
    query.bindValue(":telephone", telephone);
    query.bindValue(":mail", mail);
    query.bindValue(":shift", shift);
    query.bindValue(":statut", statut);
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_erreur = query.lastError().text();
        return false;
    }
    if (query.numRowsAffected() == 0) {
        m_erreur = "Aucun employé correspondant n'a été trouvé dans la base.";
        return false;
    }
    return true;
}

// ---------------------------------------------------------------- DELETE
bool Employe::supprimer(int id)
{
    m_erreur.clear();

    QSqlQuery query;
    query.prepare("DELETE FROM EMPLOYE WHERE id_employe = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_erreur = query.lastError().text();
        return false;
    }
    if (query.numRowsAffected() == 0) {
        m_erreur = "Aucun employé correspondant n'a été trouvé dans la base.";
        return false;
    }
    return true;
}

// ---------------------------------------------------------------- Utilitaire
bool Employe::existeDeja() const
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM EMPLOYE "
                  "WHERE LOWER(nom) = LOWER(:nom) AND LOWER(prenom) = LOWER(:prenom) "
                  "AND id_employe <> :id");
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":id", id);

    if (query.exec() && query.next())
        return query.value(0).toInt() > 0;
    return false;
}
