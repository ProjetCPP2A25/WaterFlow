#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QString>

// Patron Singleton : une seule connexion Oracle pour toute l'application.
// C'est la connexion PAR DÉFAUT de Qt : QSqlQuery / QSqlQueryModel créés
// sans paramètre (Employe, Livraison, Commande) l'utilisent automatiquement.
class Connection {
public:
    static Connection& createInstance();

    bool createConnection();              // ne rouvre pas si déjà ouverte
    QSqlDatabase getDatabase() const { return QSqlDatabase::database(); }
    QString lastError() const { return m_erreur; }

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

private:
    Connection() = default;
    QString m_erreur;
};

#endif // CONNECTION_H
