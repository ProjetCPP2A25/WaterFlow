#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QString>

// Patron Singleton : une seule connexion à la base de données pour toute l'application.
class Connection
{
public:
    static Connection &instance();

    bool createconnection();          // ouvre la connexion (appelée une seule fois, dans main)
    bool isOpen() const;
    QString lastError() const { return m_erreur; }

    Connection(const Connection &) = delete;
    Connection &operator=(const Connection &) = delete;

private:
    Connection() = default;
    QString m_erreur;
};

#endif // CONNECTION_H
