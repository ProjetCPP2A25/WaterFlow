#include "connection.h"
#include <QSqlError>

Connection &Connection::instance()
{
    static Connection c;              // créée une seule fois
    return c;
}

bool Connection::createconnection()
{
    m_erreur.clear();

    // Connexion par défaut : QSqlQuery / QSqlQueryModel l'utilisent automatiquement.
    QSqlDatabase db = QSqlDatabase::contains(QSqlDatabase::defaultConnection)
                          ? QSqlDatabase::database()
                          : QSqlDatabase::addDatabase("QODBC");

    db.setDatabaseName("Source_Projet2A25");   // nom de la source ODBC
    db.setUserName("wf");
    db.setPassword("azerty");

    if (db.isOpen())
        return true;

    if (!db.open()) {
        m_erreur = db.lastError().text();
        return false;
    }
    return true;
}

bool Connection::isOpen() const
{
    return QSqlDatabase::database().isOpen();
}
